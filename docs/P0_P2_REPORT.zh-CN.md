# P0–P2 第一轮交付记录

范围仅为协议修复、独立对局观察器、硬件遥测和现有 Dashboard 的可选 v2 扩展。没有开发 P3 Mac 原生客户端、经验账本或新增自动后台切换，也没有重写自动进房、OCR、恢复流程和 Win32 界面。

## 基线与阶段

保留可用 v0.5.13 基线 `7f14496ca7dee57c47e3a27f78f81c8ead61a922`；本轮位于独立工作树和 `codex/reliable-monitor-p0-p2` 分支，未覆盖原工作区，未推送或部署。

| 阶段 | 独立提交 | 执行与结果 |
|---|---|---|
| P0 | `2f5f3d1d8770623988b1625b3822a02439e71053` | Sol High；修正实际遗漏的 6 个状态，31 个客户端/服务端状态共享清单；18/18 CTest、Go 测试及打包通过 |
| P1 | `52559e51d9cd8aea185b5438b2a35a37d71fa16a` | Astra XHigh；独立观察器、两帧证据、会话隔离和 15 秒过期；19/19 CTest、Go 测试及打包通过 |
| P2 | 本文所在提交，实际 hash 由最终交付消息列出 | Sol High 分别负责 C++/硬件与 Go/浏览器，Astra XHigh 做最终关键集成审核；最终验证见下文 |

最多同时 3 个活动 Agent（含协调者），按文件所有权协作。模型仅用于本次开发调度，没有写入产品 API 或运行时配置。

## 改动文件与职责

| 文件/模块 | 职责 |
|---|---|
| `server/protocol/automation_states.inc`、`core/automation_state.hpp`、`server/automation_states.go` | 共享 Automation State 契约；C++ 编译期约束及 Go 严格解析 |
| `core/match_observer.hpp` | 纯状态观察器、配置、快照、连续证据与会话代次 |
| `core/match_evidence.hpp`、`agent/match_observer.hpp` | 将现有视觉/窗口/进程事实转换为观察证据；不发按键、不调用恢复 |
| `core/deathmatch.hpp` | 只读暴露已有匹配请求来源，供观察适配使用 |
| `agent/main.cpp` | 两版助手只读观察接线、停止撤销、网络发送与传感器桥接生命周期 |
| `core/hardware_telemetry.hpp`、`core/telemetry_v2.hpp` | nullable 传感器契约、缓存校验、JSON 序列化与上报调度 |
| `agent/hardware_telemetry.hpp` | 独立缓存读取线程及私有 Job 管理的小型桥接 |
| `agent/platform.hpp`、`agent/simple_config.hpp` | 配置读写、进程核验；保留简易版自身显式设置的远程地址/令牌 |
| `sensors/SensorBridge.cs`、`sensors/build.ps1` | 5 秒采样，现有 NVML / 已审核 LHM 接入，超时隔离及原子缓存 |
| `server/main.go`、`server/monitor.go` | 可选 v2、独立派生状态、缓存 GET/SSE、有效期、防乱序及持久化 |
| `server/web/app.js`、`monitor.js`、`index.html`、`style.css` | 保留原看板，增加独立对局状态、来源及传感器缺失原因，断线/睡眠/过期撤绿 |
| `tests/*protocol*`、`tests/match_observer_tests.cpp`、`tests/*hardware*`、`tests/telemetry_v2_tests.cpp` | C++ 确定性证据、传感器和跨语言协议测试 |
| `server/*test.go`、`server/browser_test.cjs`、`tests/simple_ui_tests.cpp`、`tests/platform_tests.cpp` | HTTP/SSE/持久化、真实浏览器脚本逻辑与原 UI/配置回归 |
| `CMakeLists.txt`、`scripts/build-windows.ps1` | 全量测试、桥接构建、开发版/简易版和文档打包 |

详细说明：[P0 审计](P0_PROTOCOL_AUDIT.zh-CN.md)、[P1 观察器](MATCH_OBSERVER.zh-CN.md)、[P2 硬件](HARDWARE_TELEMETRY.zh-CN.md)、[远程协议与联调](MONITOR_PROTOCOL.zh-CN.md)。

## 实际问题与局内证明

旧 Go 白名单不接受客户端实际发布的 `STANDBY`、`MATCHMAKING`、`WAITING_PAGE`、`WAITING_GAME_EXIT`、`IDLE`、`NEAC_RETRY`。请求被拒后，看板可能继续显示上一条状态。此前 `ACTIVE` 表示自动化会话仍有输入资格；RoomSession 在未知 HUD 时可以保留资格，DeathmatchSession 的 trusted 还可能来自排队或 Combat 自建来源，均不等于新鲜局内视觉。

新观察器必须同时满足同一助手代次、同一 PID+创建时间、当前画面可见，以及两张捕获时间和帧序号均递增的可信 HUD。战令房需要真实目标标题/加入请求/显式当前对局开始的来源；死斗需要真实队列/个人计分板/显式开始的来源。当前画面还必须属于对应模式。进程存活、功耗和 PostMessage 返回成功都不能确认局内。

明确非局内画面输出 `OUT_OF_MATCH`；无法获取可靠画面或识别未知立即输出 `UNVERIFIED`；可靠证据默认 15 秒到期。`OFFLINE` 仅表示已核实游戏进程不存在；进程扫描失败或身份无法获取仍为 `UNVERIFIED`。停止、重启、PID 复用和旧线程迟到均不会恢复旧确认。观察结果不参与自动化决策。

## 兼容与远程新鲜度

旧 Heartbeat 字段、端点和两个令牌角色保持兼容。`v2` 是可选扩展；旧客户端仍可写入新服务器，但远端显示局内未验证。新助手可用 `telemetry_v2=0` 对接旧服务器。原 Automation State 和新 Match State 分别保留，不要求非原子采集的两个自动化字段瞬间一致。

64 位创建时间、序列和单调毫秒用十进制字符串。服务端只返回缓存，读取接口不采样硬件。重复旧视觉或硬件证据不能通过新心跳延长首次接收确定的截止时间；防回放水位在 v1 覆盖和持久化后保留。服务端重启不会从落盘绿色恢复新有效期，需要新的真实证据。浏览器对断线、时间流逝及 Mac 睡眠独立降级。

状态变化最低 2 秒合并；常规 30 秒心跳；有效的局内、非局内、游戏进程不存在或硬件状态，以真实的新视觉/进程/采样证据约 10 秒续报；重复旧证据不会续期。默认传感器 5 秒采样，可降频。传感器缺失为 null 和原因，不生成 0W/0°C 占位。

## 验证与测量边界

Astra 最终关键审核后，2026-10-10 重新执行完整 Windows 发布脚本：开发版、简易版、桥接及所有工具构建成功，**24/24 CTest 全部通过，0 失败、0 跳过**，耗时 15.20 秒；完整 Go 测试通过，另行 `go vet ./...` 和 Node 浏览器测试通过，`git diff --check` 无错误。原生/嵌入 OCR、后台输入、恢复生命周期和两版 UI 回归均包括在内。测试耗时不代表游戏识别性能。

跨语言测试实际执行 C++ 二进制生成状态及 v2 Heartbeat，经过 Go POST、GET、SSE 与持久化重启；不是两份独立手写 JSON 自证。确定性测试覆盖用户要求的十类观察场景，并增加进程扫描失败、PID/创建时间复用、旧线程迟到、300 次恢复会话、旧 proof 防续期、损坏/未来硬件缓存、传感器卡死以及浏览器断线/睡眠回归。

最终打包位于本工作树 `dist/`，包含两版助手、`owh-dashboard.exe`、`owh-sensors.exe`、工具及全部说明。性能 JSON 已随文档复制，避免打包后测量链接丢失。完整构建日志保留在忽略的 `build/p0-p2-final-build.log`。

已完成真实 NVML 读取，示例 GPU 为 40.344 W；这不是整机功耗。当前测试 token 的 CPU 设备访问被拒绝，温度/Package 功耗返回 `null / cpu_access_denied`。没有安装驱动、重启 FanControl 或主动提权。

[实际测量记录](validation/hardware-performance.json)：桥接 5 秒间隔运行，30.11 秒稳态观察工作集约 38.94–39.03 MiB，私有内存约 34.93–35.05 MiB。短窗口 CPU 时间未观察到增长，只说明低于当次计数粒度，不能视为零开销。关闭采集时没有桥接进程；这不是完整助手/整机的受控启用前后 A/B。完整 A/B、CPU 成功采集成本、长期趋势和整机瓦数仍未测；无可信整机计量源，相关数据保留 null。

Go race 检查曾在启动前因 Windows ThreadSanitizer 虚拟地址空间分配 error 87 失败，不能声称 race 通过。普通测试通过情况另行列明。

## Tailscale + Mac 联调准备

可开始 Windows Go Dashboard 与 Mac 浏览器/API 的基础远程联调；原生 Mac 菜单栏客户端属于 P3，本轮尚未开发。服务保持 `127.0.0.1:8765`，按协议文档使用 Tailscale Serve，仅 tailnet 内访问；不开放公网端口，不使用 Funnel。Agent 写入令牌与 View 只读令牌仍独立，令牌只放 Authorization Header，不放 URL 或日志。

本轮未实际开启 Tailscale Serve、改变网络配置或完成 Mac/真实游戏端到端验收。基础协议具备联调条件不等于实机验收完成。

## 待实机验证与下一步

1. 真实战令房/自由死斗画面、不同语言与分辨率：两帧来源是否足够、识别延迟、误报/漏报，特别是超过三分钟不可见后的重新确认。
2. 加载、黑屏、结算、登录错误、断线与自动恢复；GameViewer 连接、遮挡、最小化和恢复后的新证据。
3. 真实后台消息是否被游戏响应；本轮只报告无法核实，不以 API 成功代替游戏效果。
4. 正常管理员用户会话下 CPU Package 温度/功耗，既有 FanControl 并行稳定性；不为取得数字擅自更改驱动或用户配置。
5. 完整助手启用/禁用遥测的 CPU/内存 A/B、长期运行趋势、网络流量；有可信计量源后再测额外整机功耗。
6. Tailscale Serve、Mac 浏览器、主机/服务重启和网络中断恢复的实链路联调。Mac 原生 Keychain、通知和睡眠行为留给 P3/P4。

建议先做真实截图校准与基础远程联调，再进入 P3。不要在视觉可靠性未确认前提前实施经验估算或扩大后台自动化。