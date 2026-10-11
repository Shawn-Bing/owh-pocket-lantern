# P2.6 ZeroTier 联调与验收记录

日期：2026-10-10（America/Denver）。主要执行：GPT-6.1 Sol High。分支 `codex/reliable-monitor-p0-p2`，开始基线 `812a119`。本轮保持 P0–P2.5 观察、告警和助手逻辑，未合并 main、未进入 P3。结果仅覆盖下表明确范围；模拟事实、假 DOM、真实 HTTP、真实游戏分别记录。

## 当前验收结果

| 项目 | 结果 | 证据和边界 |
|---|---|---|
| 现有 ZeroTier 网卡与服务发现 | PASS | 只读检查：ZeroTierOneService、iphlpsvc Running/Automatic；ZeroTier 1.14.2，网卡 Up，IPv4 Preferred，IPv6 绑定启用；本机与用户提供 Mac 地址属于同一 `/16` Managed 子网。精确地址见设置指南 |
| 选定入口方案 A | PASS | 已选定方案：仅将 Windows ZeroTier Managed IPv4 的 8765 转发到 `127.0.0.1:8765`；Go 默认 localhost、本地助手行为不变；未实施双地址监听 |
| Windows ZeroTier CLI/网络状态 | PASS | 本次用户允许管理员配置后只读CLI返回ONLINE、network OK、PRIVATE、Managed IP 10.243.245.219/16；未打印或修改管理secret |
| Mac成员授权/控制器配置审计 | NOT TESTED | 未操作Mac或Central；Windows ONLINE/OK和已有其他TCP连接不证明Mac授权或本项目跨机API可达 |
| Windows ZeroTier 转发/防火墙实际配置 | PASS | 用户明确要求重试UAC后，管理员PID34392完成限定配置，05:56:32 UTC结果PASS；映射仅10.243.245.219:8765→127.0.0.1:8765；ActiveStore实测两条Inbound/TCP8765/localZT规则，Allow仅Mac10.243.12.243、Block其余IPv4，所有profile启用 |
| Windows 实际 loopback Dashboard、角色与 GET/SSE | PASS | `build/p2-5-local-20261010-134345/results.json`：17 PASS、1 BLOCKED；实际 dist、独立 18773 端口和数据目录，Agent 读/View 写/未认证读均拒绝，GET 与 SSE 派生状态一致；另`build/p2-6-local8765-results.json`验证独立验收服务：正确GET/SSE200、错误角色401、30秒3快照 本次配置后再测`build/p2-6-remote-20261010/local-after-setup-results.json`：healthz/View GET/SSE200，错误Token/错误角色/无认证401，30秒3快照无断流；仅localhost范围 |
| 实际 main.cpp telemetry 开始/停止 HTTP | PASS | 包含实际 main 的无输入集成 fixture；开始 ticket 模拟，真实 telemetry 线程与 setRunning(false)。无 worker、OCR、GUI 或真实局内证明 |
| 发布助手 GUI 本轮开始/停止 | BLOCKED | 默认保护跳过需管理员的发布助手；未重新启动或操控管理员 GUI，未复用先前关闭的测试 PID |
| 实际服务器证据过期、传感器缺失隔离、恢复和重启 | PASS | C++ 序列化模拟游戏事实，经真实 HTTP；18 秒自然失效为 UNVERIFIED；新模拟 proof 恢复；服务重启不继承绿或告警运行预期 |
| 实际服务器缩放阈值告警 | PASS | 本地真实计时器 `-alert-after 2s`，到期、解除、重启禁用均通过；这是协议集成测试，不是五分钟真实游戏验收 |
| Mac ZeroTier healthz、认证 GET、SSE | NOT TESTED | 无 Mac 执行权限或实测返回；完整隐藏 Token 命令见设置指南；本机回环 PASS 不替代跨机 PASS |
| Mac 浏览器登录、撤绿、重连、刷新告警计时 | NOT TESTED | Node 假 DOM 回归通过；本轮未观测真实 Mac 浏览器 |
| 两阶段各30分钟运行、GET/SSE连续性 | PASS | 每组361样本/61 GET、1条独立SSE，182/300快照，无意外断流；预热后每组300样本，详见下文 |
| Bridge 本30分钟private平台检查 | FAIL | 仅指该窗口未建立平台：预热后五个5分钟窗持续增长35.449→38.738MiB；这是已观察结果，是否泄漏尚未确认 |
| 更长时段内存稳定/泄漏归因 | NOT TESTED | 未追加更长测试或堆/分配跟踪；不能声称无泄漏或已证明泄漏 |
| CPU Package 成功读数 | BLOCKED | 延续既有设备访问权限限制；接受 null/cpu_access_denied，不改驱动、设备 ACL 或提升助手权限 |
| 整机功耗、完整 TCP/ZeroTier 带宽 | NOT MEASURED | 没有可信计量源。GPU W 不代替整机功耗；GET/SSE 正文字节不代表完整网络流量 |
| 真实游戏与五分钟连续异常 | NOT TESTED | 未控制真实游戏、GameViewer 或现用助手，逐项勾选表如下 |

结果约定：PASS 代表该范围有实际证据；FAIL 代表已执行且不符；BLOCKED 代表被可描述条件阻止；NOT TESTED 代表尚未执行。NOT MEASURED 专用于缺少计量的数值，不填估算。本轮协议回归未出现FAIL；bridge平台检查的FAIL仅限“该30分钟窗口未建立平台”，不表示已证实泄漏。

网络管理员配置、地址与命令、影响、回滚及 Mac 操作集中于 [ZeroTier 设置指南](P2_6_ZEROTIER_SETUP.zh-CN.md)。P2.5 的 Tailscale 部分保留为历史，现行流程由 ZeroTier 指南取代，不安装 Tailscale，不修改已有 VPN、代理或 ZeroTier 网络。

## 真实游戏短勾选表

由用户操作游戏及现用助手；记录 UTC 时间和可核验画面，保留未含凭据的截图。每行独立判定，不以游戏进程在、GPU 有功耗、Automation ACTIVE 或输入 API 成功推断 IN_MATCH。以下观测栏为空表示没有实测；不能用期望值补填。自动恢复不等于助手手动 stop/restart。

| 模式/操作 | 检查点 | 结果 | 实际画面/UTC | Observer 状态/source | Dashboard/API 状态 | 延迟 ms | 错误 | 误报/漏报 |
|---|---|---|---|---|---|---|---|---|
| 战令房：主菜单 | 明确认出的菜单为 OUT_OF_MATCH | NOT TESTED | | | | | | |
| 战令房：自动导航/进房 | 来源和同一游戏身份有效，连续两帧才确认 | NOT TESTED | | | | | | |
| 战令房：局内稳定运行 | 新视觉续期，API/网页与画面一致 | NOT TESTED | | | | | | |
| 战令房：结束 | 原局内确认撤回，按当前事实降级 | NOT TESTED | | | | | | |
| 战令房：重新开始 | 新连续可信 HUD 后恢复 | NOT TESTED | | | | | | |
| 战令房：返回菜单/断线 | 明确菜单 OUT，画面不可用 UNVERIFIED | NOT TESTED | | | | | | |
| 战令房：恢复进房 | 不继承旧 proof，自动恢复不清连续异常 | NOT TESTED | | | | | | |
| 死斗：匹配/等待 | 队列与加载不能直接 IN_MATCH | NOT TESTED | | | | | | |
| 死斗：局内 | 来源、Combat/HUD、两帧均满足 | NOT TESTED | | | | | | |
| 死斗：结算 | 及时撤回旧确认，不对结算页继续绿色 | NOT TESTED | | | | | | |
| 死斗：重新排队/重进局内 | 重排不是可信局内；新证据后才解除异常 | NOT TESTED | | | | | | |
| 任一模式：GameViewer 连接/遮挡 | 实测软件对画面、前台与原输入的影响 | NOT TESTED | | | | | | |
| 任一模式：最小化 | 无新画面不能保留过期绿色 | NOT TESTED | | | | | | |
| 任一模式：最小化恢复 | 新来源及连续证据后恢复 | NOT TESTED | | | | | | |
| 任一模式：失焦/恢复 | 无可靠前台证据时降级；恢复使用新证据 | NOT TESTED | | | | | | |
| 任一模式：OCR 失败/黑屏/未知 | 保守 UNVERIFIED，不伪造画面结论 | NOT TESTED | | | | | | |
| 任一模式：游戏 PID 退出/重启 | 核对创建时间；旧实例不得续绿 | NOT TESTED | | | | | | |
| 任一模式：助手手动 stop | 运行期结束、撤绿、禁用并解除告警 | NOT TESTED | | | | | | |
| 任一模式：助手 restart/start | 新运行代次及新证据；旧结果不能恢复 | NOT TESTED | | | | | | |
| 连续异常满五分钟 | 真实 running 非dry-run；默认300000ms，只提醒一次 | NOT TESTED | | | | | | |
| 五分钟异常期间刷新/重登录 | 服务端计时不重置，记录同一episode | NOT TESTED | | | | | | |
| 恢复可信局内/第二次异常 | 解除；再次异常可新episode触发 | NOT TESTED | | | | | | |
| Mac 断网/重连 | 及时撤绿/告警待同步；新快照恢复 | NOT TESTED | | | | | | |

## 性能、趋势与计量边界

两阶段测量使用已有 `dist/owh-dashboard.exe`、实际 main/telemetry 集成 fixture，开启阶段增加真实 `owh-sensors.exe` 桥接和缓存读取。每阶段全新 dashboard 进程和缓存，先预热 300 秒，1800 秒 active，5000 ms 采样，30000 ms 主动 GET，独立 1 条 SSE 连接。关闭/开启顺序执行，未运行 fixture 的自动化 worker、OCR 或 GUI；测量启动环境中已有真实游戏、FanControl、GameViewer；未逐秒审计其后续状态，所以不是受控整机 A/B。

本轮输出目录：`build/monitor-performance-20261010-134307-897`。原始每样本 JSONL 在各 phase 子目录，可能含本机测试 PID，不提交。完成后的 [去身份性能统计](validation/p2-6-performance.json) 由 `scripts/summarize-monitor.ps1` 生成，仅保留按角色统计、五分钟窗口、二进制哈希与去传感器身份的读数，不包含 Token、设备 UUID 或 PID。

CPU 是所拥有的 server+agent(+bridge) 进程时间之和，一个核心为100%；整机口径再除24逻辑核。不计测量 PowerShell、其他回归工具、真实游戏或既有软件 CPU。稳态均值由窗口首末累计进程 CPU 差除以实际间隔；窗口首末时刻和样本数均公开。每阶段工作集、private、threads、handles 分角色列出；工作集变大本身不证明泄漏。SSE 连续性按每次采样排空流记录，最大间隔存在5秒采样量化。

网络仅测主动 GET 和该独立 SSE 的 UTF-8 响应正文字节；不含 heartbeat 上传、HTTP头、TCP、ZeroTier、网卡流量。完整发送/接收总量为 null/NOT MEASURED。整机功耗也是 null/NOT MEASURED；GPU 读数只代表该GPU。没有把开启组的CPU/内存差异换算成电费。

两组已完成（America/Denver，2026-10-10）：关闭组 13:43:08 开始、14:13:17 左右结束；开启组 14:13:18 开始、14:43:26 左右结束。原始 JSON 保留 UTC 时间以便复核。均正常退出，每组361样本，其中预热后、1800秒前300稳态样本，五个窗口各60样本。

| 实测窗口指标 | 硬件关闭 | 硬件开启 |
|---|---:|---:|
| 墙钟总秒数 | 1809.15 | 1808.40 |
| 单核 CPU 全窗均值 / 峰值，% | 0.02505 / 0.937 | 0.14775 / 3.433 |
| 24核整机口径 CPU 均值，% | 0.001044 | 0.006156 |
| 预热后单核 CPU 均值，% | 0.02294 | 0.13562 |
| GET 次数 / 平均 RTT / 最大 RTT，ms | 61 / 0.658 / 9.070 | 61 / 0.406 / 1.492 |
| GET/SSE 响应正文总字节 / B每秒 | 536392 / 296.49 | 810431 / 448.15 |
| Owned SSE连接数 / snapshot数 | 1 / 182 | 1 / 300 |
| 最大观测snapshot间隔，s / 意外断流 | 15.056 / 无 | 15.055 / 无 |

预热后首末样本资源（MiB=1048576字节）；CPU为各角色稳态单核均值/采样峰值。完整最小/最大、实际起止时刻、所有五分钟窗口的每角色指标见JSON。

| 组 / 角色 | Private MiB 首→末 | WS MiB 首→末 | Threads 首→末 | Handles 首→末 | CPU 均值/峰值 % |
|---|---:|---:|---:|---:|---:|
| hardware_off / agent | 2.719→2.793 | 10.719→10.789 | 7→7 | 242→242 | 0.01564/0.312 |
| hardware_off / server | 47.117→49.270 | 10.500→12.406 | 9→12 | 172→204 | 0.00730/0.625 |
| hardware_on / agent | 2.922→2.945 | 10.938→11.117 | 8→8 | 249→249 | 0.06259/0.623 |
| hardware_on / bridge | 35.449→38.738 | 39.555→38.184 | 6→5 | 295→295 | 0.06573/0.312 |
| hardware_on / server | 47.816→50.172 | 10.949→12.758 | 9→13 | 170→212 | 0.00730/0.624 |

开启组 bridge 的五分钟窗口专列如下，以免将末段WS下降误读成private已平台：

| 自启动分钟窗口 | Private MiB 首→末 | WS MiB 首→末 | Threads min–max | Handles min–max | CPU 均值/峰值 % |
|---|---:|---:|---:|---:|---:|
| 5–10 | 35.449→36.102 | 39.555→40.188 | 5–6 | 295–295 | 0.04757/0.312 |
| 10–15 | 36.141→36.727 | 40.219→40.844 | 5–6 | 295–295 | 0.03701/0.312 |
| 15–20 | 36.727→37.414 | 40.852→41.500 | 5–5 | 295–295 | 0.08458/0.312 |
| 20–25 | 37.414→38.043 | 41.508→44.137 | 5–5 | 295–295 | 0.08462/0.312 |
| 25–30 | 38.043→38.738 | 44.152→38.184 | 5–5 | 295–295 | 0.07932/0.312 |

范围内运行和SSE连续性为PASS；长期内存稳定尚未确认。Agent的private和句柄/线程较稳定。Go server有增长及回落，开启组最后五分钟private约50.1MiB、波动约0.203MiB、13线程/212句柄保持，属于局部平台证据，不能证明无限期稳定。Bridge预热后private 35.449→38.738MiB，五个窗口持续增加、尚无平台；句柄295和最终5线程稳定，WS最后一个窗口下降。当前数据不能区分有界分配、回收时机或泄漏，不假设CLR/GC原因、不据此改采集或告警逻辑。若进一步验收内存，需更长的同等负载测量及分配/回收证据。

开启组结束的真实传感器样本：GPU NVML 10.987W；CPU温度/Package功率均null/cpu_access_denied。单点GPU值不代表30分钟平均、真实局内负载或整机功耗。


### Bridge 持续增长的窄查与后续专项方法

仅检查 `sensors/SensorBridge.cs` 的采样循环和资源持有：每个后端只有一个常驻Worker及一个最新result，主循环没有保存所有样本的集合；CPU初始化失败原因缓存，不会每轮重新加载LHM；NVML初始化和delegate缓存；FileStream、SHA、设备SafeFileHandle及服务句柄分别有using/finally释放。每轮创建的Reading、JSON字符串、UTF-8字节及WaitHandle引用数组没有代码可见的历史持有。进程生命周期级的线程/事件/NVML库实例固定存在，未见能直接解释每窗持续增长的无界持有代码。此窄查不能排除库内部或运行时的增长，未强制GC、改CLR参数或为了压WS修改生产代码。

后续专项复验应单独运行同一哈希bridge，独占一个全新本地缓存，保持相同5000ms采样和现有权限；例如`owh-sensors.exe --output <独立目录>\hardware.json --interval-ms 5000 --samples 1440`约两小时。只统计该PID的private/WS/handles/threads/CPU，每5秒一条、每5分钟汇总，明确预热及背景条件；不要把真实游戏CPU混入。如果已有可信的CLR分配/自然GC计数接口可只读采集，用它判断private变化与自然回收时点的关系；若没有则记录缺失，不强制GC也不安装工具/驱动。需要时再以`--no-cpu`独立对照失败CPU路径；所有专项结果当前NOT TESTED，本轮没有追加该长测。

## 复现和回归

```powershell
.\scripts\validate-local-monitor.ps1 -Port 18773 -AlertAfter 2s
.\scripts\measure-monitor.ps1 -Port 18771 -ActiveSeconds 1800 -WarmupSeconds 300 -SampleIntervalMs 5000 -StatusIntervalSeconds 30
.\scripts\summarize-monitor.ps1 -EvidenceDirectory .\build\monitor-performance-20261010-134307-897
D:\cmake\bin\ctest.exe --test-dir build/windows --output-on-failure
# server 目录
E:\GitHubRepos\OverwatchHelper\build\tools\go\bin\go.exe test -count=1 ./...
E:\GitHubRepos\OverwatchHelper\build\tools\go\bin\go.exe vet ./...
# 仓库根目录
& 'C:\Program Files\nodejs\node.exe' server/browser_test.cjs
```

- PASS：24/24 CTest，0失败，19.22秒。
- PASS：完整 Go test -count=1（含实际 C++ 序列化跨语言链路）、go vet。
- PASS：Node 浏览器 freshness/alert 和实际 app 假 DOM 回归。
- PASS：本轮真实 dist loopback 验收17项；1项发布助手 GUI 为 BLOCKED，未当失败或实机通过。
- PASS：性能汇总脚本用先前两阶段真实短测 JSONL smoke，输出在 `build/p2-6-summary-smoke.json`；本轮两阶段长测完成后生成最终去身份JSON。
- NOT TESTED：Windows Go race，沿用此前 TSan 地址空间 error87，不修改环境。

测试与文档变化不需要 C++ 全量重建。构建脚本已有所有 `.md` 文档复制规则，并新增复制 P2.6 性能 JSON；已将本轮文档和性能统计复制到dist，但未重新编译可执行文件或重复全量构建。原始数据及测试 Token 留在忽略的 build，未打印凭据，清理仅针对自有进程，没有批量或递归删除。

原始UAC取消记录保留在`build/p2-6-remote-20261010/network-launch-result.json`。用户随后明确要求重试：首次管理员PID5512因UTF-8无BOM被Windows PowerShell5按ANSI解析，原生Parser复现语法错误，脚本未执行；仅修正BOM并实际PS5解析通过后，再次UAC确认，管理员PID34392成功完成已授权范围，随后退出。没有更改服务启动方式、VPN、代理、现有ZeroTier网络或全局防火墙策略。

本次配置证据：`build/p2-6-remote-20261010/network-setup-result.json`（包含两条ActiveStore规则的地址/端口过滤器、映射注册表值和监听）；去身份状态摘要：`network-verification-summary.json`。仅两个监听：Go PID4608的127.0.0.1:8765与系统portproxy PID5708的10.243.245.219:8765，无wildcard或IPv6入口。其他本轮性能/协议测试进程已退出；为用户Mac联调保留新的localhost Dashboard PID4608运行，数据/Token复用忽略的build目录，未改现用助手配置。停止与局部网络回滚步骤见设置指南。

Windows本机源地址不属于Mac唯一allow范围，因此未以Windows请求ZT入口代替Mac测试或判本地回归失败。Windows ONLINE/OK仅证明本机当前CLI状态；Mac授权、真实跨机GET/SSE/浏览器与游戏仍未验证。

## P3 决策

**暂不正式进入 P3。** 本地协议与回归通过只证明对应范围；Mac ZeroTier API/浏览器、真实游戏两模式及默认五分钟告警仍未验收。用户完成入口配置、Mac检查和上表关键真实场景后，再根据证据决定 P3。未将软件网络方案准备完成写成远程或真实游戏验收完成。
