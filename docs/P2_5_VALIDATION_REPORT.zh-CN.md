# P2.5 联调、告警与验证记录

本轮从已推送且干净的 `codex/reliable-monitor-p0-p2` / `79c2c9cdab1e5c6c83448f5871e402cd1b501e04` 继续。保留 v0.5.13 自动化语义，不合并 main，不自动推送，不操作真实游戏或用户原配置。

## 结论与验收范围

| 项目 | 结果 | 证据与限制 |
|---|---|---|
| 分支、基线与远端一致 | PASS | 初始 git status 无输出；public 同名分支 ls-remote 与 HEAD 相同 |
| 发布 dev 助手真实启动、HTTP v2 | PASS | 用户关闭原助手后授权重试，实际 PID 31216 上报 STOPPED/running=false/dry_run=true/UNVERIFIED |
| 发布助手 GUI 自动开始/停止 | BLOCKED | 窗口控制未取得 running=true；MainWindowHandle 变为 0，OS 拒绝脚本关闭。用户随后仅手动关闭该测试 PID；没有主动 RunAs 或修改 manifest |
| 实际 main.cpp/telemetry 网络路径 | PASS | 非提权集成 fixture 包含实际 main.cpp，运行实际 telemetry 线程与 setRunning(false)；开始 ticket 模拟，不运行 worker、输入、GUI 或游戏 |
| C++→实际 HTTP→Go GET/SSE | PASS | 现有 C++ 完整序列化 envelope，适配机器名与 UTC 后发送；HUD/游戏事实是模拟夹具 |
| 18 秒自然过期、全传感器缺失隔离、新 proof 恢复、服务重启降级 | PASS | 实际 dist dashboard；读取不触发采样，CPU null 不影响独立观察 |
| 五分钟连续异常、去重、解除 | PASS | 11 组新顶级 Go 告警测试用可控时间；另实际服务器用 2 秒缩放阈值验证 timer/解除/重启 |
| Dashboard 脚本与断线撤绿/告警待同步 | PASS | 真实 app.js 的假 DOM 回归及纯 JS；真实服务器提供嵌入资源 |
| 真实浏览器视觉、战令房/死斗、GameViewer、最小化、游戏恢复 | NOT TESTED | 未将模拟 HUD 或假 DOM 写成实机验证；需用户人工矩阵 |
| Tailscale 与 Mac 实链路 | BLOCKED | PATH、标准安装目录、服务和卸载信息未发现可调用 Tailscale；当前没有 CLI 可执行 version/help/status。未安装、重配 VPN/代理、改变 Serve 或开放公网 |
| CPU Package 成功读数 | BLOCKED | 现有 PawnIO 服务 Running；OPEN_EXISTING 设备打开实测 Win32 error 5；继续 null/cpu_access_denied |
| GPU 真实采样 | PASS | 既有 NVML，短测样本 41.55 W；不是整机功耗或局内证明 |
| 短时 HTTP/SSE 稳定性 | PASS | 开启组总 128.19 秒、244 次样本、23 个 SSE snapshot，无意外断流 |
| 长期内存稳定/完整助手与游戏 A/B/整机功耗 | NOT TESTED | 组合 WS 有增长，下文按进程公开；不能声称无泄漏；无可信整机计量源 |

首次未取得进程的启动尝试没有足够证据区分单实例/授权原因；不能仅凭 requireAdministrator manifest 断言实际启动失败。后续用户授权重试的真实启动与 HTTP 已单独记为 PASS，开始/停止仍保留 BLOCKED。默认复现脚本不启动需要管理员的发布助手，只有显式 `-TryActualHelper` 才尝试普通启动。

人工游戏矩阵、Tailscale 修改影响及 macOS 隐藏输入 Token 的 curl/SSE 步骤见 [人工验收指南](P2_5_MANUAL_VALIDATION.zh-CN.md)。本轮未控制 Mac，Mac 浏览器/API 为 NOT TESTED。原主机 8765 未发现已有监听，测试仅用独立 18765–18770 回环端口；测试服务已清理。

## 本轮修改

| 文件 | 职责 |
|---|---|
| `tests/agent_telemetry_integration.cpp`、`scripts/validate-local-monitor.ps1`、`CMakeLists.txt` | 无输入的实际 main/telemetry 集成、真实 loopback API/SSE、过期/重启/硬件失败验收 |
| `CMakeLists.txt` sensor target | 修复最终构建暴露的 CSC 路径问题：CSC 对生成的 D:/ 路径丢失目录；复用现有 sensors/build.ps1 传入原生路径。没有修改采集逻辑 |
| `server/alerts.go`、`alerts_test.go`、`main.go` | 独立告警代次、单调计时、去重、API/事件、每秒运行期推进及可配置阈值 |
| `server/monitor.go` | 有运行期 receipt 时，主机年龄采用其单调时间；落盘数据仍保守处理 |
| `server/web/app.js`、`monitor.js`、`style.css`、`browser_test.cjs` | 原看板新增一行异常计时，混合状态文案、断线待同步及短 TTL 回归 |
| `scripts/measure-monitor.ps1`、`docs/validation/p2-5-performance.json` | 独立缓存/进程测量，500 ms 采样、分进程内存/线程/handle、真实 API/SSE payload/延迟 |
| `scripts/build-windows.ps1` | 打包新增 P2.5 性能 JSON；现有通配规则继续复制 P2.5 文档 |
| 协议文档、本报告、人工指南 | API 字段、边界、复现与未验证事项 |

## 告警合同

API 在每台 Agent 上新增 `alert`，字段为 enabled、episode_id、kind、reason、duration_ms、type_duration_ms、threshold_ms、reached；详见 [协议](MONITOR_PROTOCOL.zh-CN.md#p25-连续异常告警)。默认五分钟，可通过服务器 `-alert-after 5m` 配置。

只有新接受的可信 v2 running 且非 dry_run 心跳建立预期。stop、v1 覆盖、演练、新助手实例或 start/stop revision 变化清除旧预期。自动恢复仅换游戏 PID/session、同代客户端时钟失败不会当成手动停止。可靠 IN_MATCH 结束本轮异常，之后再次异常可再次提醒。

OUT_OF_MATCH、UNVERIFIED、主机无心跳 OFFLINE 连续累计总时长，另有当前类型时长，避免把未知时间说成明确不在局内。在线游戏进程不存在是 OUT_OF_MATCH/game_process_absent；告警 OFFLINE 只表示超过 90 秒无主机心跳，与 P1 游戏 OFFLINE 分开。

服务端每秒推进，已知 proof deadline 决定失效起点，无查看者也不会推迟计时；覆盖旧快照前先推进，长空档的新绿不能抹去已达阈值历史。每个 episode 只产生一次 threshold_reached；started/kind_changed/resolved 通过现有 SSE snapshot 事件发送。运行计时不落盘，重启只保留历史事件，等待新合法运行心跳。网页刷新不重置，断线标待同步，类型文案说明是上次同步事实。没有 OS 通知或外部推送。

## CPU 诊断

CPU 路径为现有 FanControl 安装的已审核 LHM PawnIO 后端；GPU 使用独立 NVML。实际 FanControl 进程存在、PawnIO 服务运行，但本进程以与桥接相同的 OPEN_EXISTING 参数打开 `\\?\GLOBALROOT\Device\PawnIO` 返回无效句柄和 Win32=5。失败明确发生在现有设备访问权限层，未进入有效 Package 传感器读取，不以 0 替代。

没有重启 FanControl、安装或启动驱动、改设备 ACL、为读 CPU 提权整个助手。没有验证 FanControl 具有可导出缓存的接口，更没有假设它提供 HTTP。继续保持 CPU null/原因与 GPU 正常采集，成功读取及与既有监控长期并行仍需用户授权的后续实机工作，不阻塞本轮协议验收。

## 性能与稳定性数据

原始去凭据、去 GPU 身份的记录：[p2-5-performance.json](validation/p2-5-performance.json)。沿用本轮开始时的 P2 dashboard/桥接产物，使用实际 main/telemetry fixture；开启组增加真实桥接与缓存读取。没有运行完整发布 UI、worker、截图识别或游戏；新增告警层另做功能验收，未单独做其性能 A/B。

| 12.6 秒启动 smoke | 硬件关闭 | 硬件开启 |
|---|---:|---:|
| 单核口径 CPU 均值 | 0.621% | 1.362% |
| 单核口径 CPU 峰值 | 6.05% | 28.45% |
| 组合 WS 峰值 | 20.64 MiB | 60.19 MiB |
| GET 平均 RTT | 0.915 ms | 0.555 ms |
| GET/SSE 应用 payload | 4.08 KB/s | 5.95 KB/s |
| SSE snapshot / 500 ms 样本 | 4 / 24 | 4 / 24 |

这是启动 smoke，不能据此判断开启后更快或长期内存稳定。CPU 为 server+fixture(+bridge) 进程时间之和，均值/峰值以一个核心为 100%，整机口径除以 24 逻辑核；Windows CPU 计数粒度和 500 ms 采样限制峰值精度。

另跑开启组 68.15 秒，再针对已观察到的 WS 增长追加一次 128.19 秒分进程测量；后者 244 次样本、217 个 10–123.5 秒稳态样本、23 SSE snapshot，单核 CPU 均值 0.390%、峰值 16.54%（全窗含初始化），GET RTT 均值 0.477 ms，最大 10.52 ms。

| 128 秒复测，10.1→123.5 秒 | WS，十进制 MB | Private，十进制 MB | Threads | Handles |
|---|---:|---:|---:|---:|
| Go dashboard | 10.41→50.61 | 49.03→50.61 | 10→8 | 162→164 |
| 实际 telemetry fixture | 11.18→20.26 | 2.79→2.96 | 8→8 | 232→241 |
| Sensor bridge | 40.80→48.50 | 36.56→36.84 | 7→7 | 289→291 |

WS 增长真实存在，私有分配增量小得多；本轮未确认根因或长期平台期，不能写无泄漏，也不猜测 GC/OS 成因。线程未观察到持续增加、短测 SSE 未断，长期稳定仍为 NOT TESTED。

应用 payload 仅累计 GET/SSE 响应正文，不含 heartbeat 上传、HTTP/TLS/Tailscale 或 NIC 流量。每 500 ms 主动 GET 是诊断负载，不是正常用户带宽。整机功耗为 null/no_trusted_meter，GPU W 不能代替整机。

## 可复现检查与最后测试

```powershell
# 使用既有工具链完成构建（本机需前置 CMake/MinGW PATH）
.\scripts\build-windows.ps1 -Generator 'MinGW Makefiles' -Go E:\GitHubRepos\OverwatchHelper\build\tools\go\bin\go.exe
.\scripts\validate-local-monitor.ps1 -Port 18770 -AlertAfter 2s
.\scripts\measure-monitor.ps1 -Port 18769 -ActiveSeconds 120 -OnlyHardwareOn
# 后续人工长测可把 ActiveSeconds 提到 1800；不意味着本轮已做 30 分钟验收
```

所有数据写当前工作树忽略的 build/，不修改用户现有配置；不得把测试 Token 放 URL、日志或提交。每个测试端口先确认空闲，只清理自己创建的进程；不删除目录。真实发布助手仅在用户明确同意后用显式 TryActualHelper，可能需手动处理系统授权/关闭。

- PASS：最后全量 Windows build/package，24/24 CTest，0 失败、0 跳过，15.12 秒。
- PASS：完整 Go test（包含真实 C++ v1/v2 跨语言 fixture、11 组新告警顶级回归及浏览器测试）。
- PASS：完整 go vet、Node 浏览器测试、git diff --check。最终窄审仅补脚本清理/解析与告警文案，随后 Node 回归、全部三个 PowerShell 脚本解析、Dashboard 单独重构建再次通过；没有重复整套测试。
- PASS：新 dist 实际 loopback 缩放告警验收，18 项记录中 17 PASS，发布助手默认保护为 1 BLOCKED，不是功能测试失败。
- NOT TESTED：Windows race；沿用此前 TSan 地址空间 error 87 限制，本轮未重装工具链或重复耗时尝试。

构建日志：`build/p2-5-final-build.log`。实际 API/告警证据：`build/p2-5-local-20261010-123547/results.json`。用户授权发布助手启动证据：`build/p2-5-local-20261010-121634/actual-helper-result.json`。进程级测量的 build 原始 JSON 与已发布去身份 JSON 分开保留。

## P3 决策

API、去重告警与自动化回归已具备 SwiftUI 客户端消费条件；建议先由用户完成 Tailnet/Mac 浏览器基础联调及两种模式真实截图校准，再开始 P3 原生界面。当前不宣称真实挂机或 Mac 远程链路已验收，也不直接开始 P3。
