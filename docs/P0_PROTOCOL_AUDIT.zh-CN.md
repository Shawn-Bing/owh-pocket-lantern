# P0 仓库审计与协议修复（v0.5.13）

基线：`public/main`，提交 `7f14496`。本阶段仅统一 Automation State 的传输契约，不改变开发版、简易版的进房、死斗、恢复、前台/后台输入语义。

## 已确认的问题和修复

客户端实际发布但旧 Go 白名单未接收的状态共有 6 个：

| 状态 | 现有含义 |
|---|---|
| `STANDBY` | 用户操作电脑时暂停输入和识别 |
| `MATCHMAKING` | 已识别死斗排队画面 |
| `WAITING_PAGE` | 已识别等待页，不发送输入 |
| `WAITING_GAME_EXIT` | 已请求关闭游戏，但尚未确认旧进程退出 |
| `IDLE` | 已知的非对局页面 |
| `NEAC_RETRY` | 等待清理已核验的加载器/客户端残留 |

这些心跳此前返回 HTTP 400，服务端保留上一次接收成功的数据，直到新状态被接受或 90 秒离线阈值生效。远端因此可能短暂保留旧 `ACTIVE`。原服务端测试只覆盖少数手写状态，未核对全部客户端发布值。

`server/protocol/automation_states.inc` 现在作为唯一 wire-name 清单，包含旧服务端全部 25 个状态及上述 6 个状态。`core/automation_state.hpp` 从清单产生 C++ 枚举与序列化函数；`agent/main.cpp::publish` 只接受该枚举，拼写错误或未登记状态不能通过客户端编译。`server/automation_states.go` 在构建时嵌入同一清单，构造严格白名单。清单在运行时无文件依赖，异常/重复项会使服务端启动失败。Dashboard 为原来缺失的 4 个状态补充中文标签。

兼容范围：原 Heartbeat 的字段、名称、类型、端点、令牌角色、接收时间、保存格式保持不变。旧客户端全部有效状态继续接收；新增字段仍被旧协议严格拒绝，P2 需明确新增可选字段。`IN_MATCH` 等将属于独立观察字段，不能直接当作 Automation State 塞进 `state`。

## 模块关系与扩展位置

| 模块 | 已有职责 | P1/P2 接入建议 |
|---|---|---|
| `agent/main.cpp` | UI、工作线程、现有会话驱动；独立 `telemetry()` 线程 | 工作线程向独立观察器提交事实，网络线程读取不可变观察快照 |
| `core/core.hpp::RoomSession` | 保留战令房输入资格；未知画面保留已建会话 | 复用来源，不能以 `active()` 代替新鲜视觉确认 |
| `core/deathmatch.hpp::DeathmatchSession` | 排队、对局、结算、离开、随机输入节奏 | `trusted()` 包括排队；局内须再核对当前 Combat 证据 |
| `core/cn_recovery.hpp::CNJoinGate` | 目标选择/加入来源与连续 Room 确认 | `awaitingRoom(now)` 是请求来源，不是成功证明；`State::joined` 表示连续两次 Room |
| `core/cn_recovery.hpp::CNRoomStart` | 用户声明已在当前 PID/创建时间会话内的有限窗口 | 声明用于来源约束，仍需两次有效局内视觉 |
| `core/semantic.hpp` | OCR/图像页面语义、HUD和目标上下文 | 使用 `SemanticObservation` 的 page、evidence、liveRoomHud、verifiedDeathmatch，保留模式来源约束 |
| `agent/vision.hpp` | 可取消 OCR 子进程、图像缓存、识别后画面校验 | OCR 等待后必须保留现有进程/代次/新截图检查；不要额外启动并行 OCR |
| `agent/platform.hpp` | Win32 进程身份、桌面、窗口、截图、输入、HTTP | 复用 PID+创建时间核验；截图失败/最小化/遮挡须单独报告 |
| `agent/background_input.hpp` | 核验目标窗口后发送成对窗口消息 | API 成功只能说明消息提交，不得作为局内视觉证据 |
| `agent/startup.hpp` | 启动参数及诊断报告 | 复用窗口/权限诊断；PrintWindow 诊断不是可靠后台截图管线 |
| `core/recovery_lifecycle.hpp` | 游戏退出等待、反作弊启动失败与恢复生命周期 | 观察器只消费事实，不控制预算、退出或重启 |
| `server/main.go` | Agent 写入、View 读取、SSE、限量事件、缓存持久化 | P2 为 Heartbeat 增加可选观察/遥测字段，读取接口只返回缓存 |

建议文件结构：

- `core/match_observer.hpp`：与 Win32 无关的状态归约、两帧确认、15 秒证据过期、身份/会话/代次撤销；时间由调用方传入，便于确定性测试。
- `agent/match_observation.hpp`：把现有识别与窗口/进程事实转换为观察输入；不发输入、不操作现有会话。
- `core/telemetry.hpp`：可选遥测值、采样时间、来源、错误原因的数据契约。
- `agent/hardware_telemetry.hpp`：低频、可停、可降频的传感器适配；缺失值返回 null，不用 0 占位；不阻塞现有工作线程。
- `server/protocol/`：跨语言共享协议清单；`server/*_test.go` 验证旧版/扩展版兼容与缓存查询。
- `tests/match_observer_tests.cpp`：纯状态观察序列测试，覆盖用户指定的 10 种场景。

接口方向应是 `existing facts -> observer -> snapshot -> telemetry -> Go cache`。观察状态不能写回 `RoomSession`、`DeathmatchSession`、`CNJoinGate`、输入资格或恢复预算。

## 不能从 Automation State 推导的事实

1. `RoomSession::active()` 是输入策略状态，识别不到 HUD 时仍保留，包括加载/观战/结算；`ACTIVE` 因而不保证刚看到了有效局内画面。
2. `DeathmatchSession::trusted()` 为 `queued_ || inMatch_`；`confirmedCombat(now)` 要连续两次 Combat、末次不超过 1500 ms，但这是现有输入门槛，不应直接成为远端完整观察状态。
3. `foregroundUse` 分支主动跳过截图、OCR、导航及恢复。最小化后台只能报告局内未核实；后台消息成功、进程存活及未来硬件功耗均不能独立确认对局。
4. `Capture::grab` 会先清空帧；OCR 请求之后现有代码重新核验同一进程、重新截图，并检查 `semanticStillVisible`。观察器必须使用通过这些检查的当前结果，不能提前发布旧帧。
5. `running`/`generation` 与工作线程 `epoch` 防止旧动作继续执行；新观察器需要同样拒收停止/重启后的过期结果。`publish` 本身只检查是否运行，不能作为跨代次结果的唯一保护。
6. `telemetry()` 每 2 秒最短间隔、平时 30 秒上报；`networkDirty` 请求加速，等待谓词当前只检查 quitting，因此通知仍可能等到约 1 秒周期结束。P2 可整理快照与唤醒，避免改变挂机线程。
7. Dashboard 当前将在线 `ACTIVE` 纳入绿色运行统计；P1/P2 必须显示独立对局证据和过期时间，避免把助手运行与确认局内混淆。

## 自动验证

新增 `automation-protocol` CTest 验证所有编译后的 C++ wire name 非空、唯一、格式正确，固定覆盖此前遗漏的 6 个状态。

新增 `automation-protocol-server` CTest 调用真实 C++ 二进制导出全部状态，再通过 Go HTTP 服务逐一验证 POST、GET、SSE 与保存/重载完全一致。CMake 找到 Go 时注册；`scripts/build-windows.ps1 -Go ...` 显式向 CMake 提供 Go，因此正常发布构建会执行跨语言检查。仅单独 `go test` 时该 compiled-client 检查会明确 skip；其余全部状态 HTTP/SSE/持久化测试仍执行。

Go 另固定所有旧状态兼容、拒绝未知/大小写错误状态、清单异常失败、Dashboard 标签完整性。现有输入、恢复、OCR、UI CTest 保持原样。

实测结果与本机工具链说明见本节后续记录。真实游戏截图、后台消息实际生效、远程连接影响及功耗数据均仍为「待实机验证」。P0 不声称已经完成对局识别或传感器采集。

### 本阶段实际验证（2026-10-10，Windows）

- `scripts/build-windows.ps1 -Generator 'MinGW Makefiles' -Go 'E:/GitHubRepos/OverwatchHelper/build/tools/go/bin/go.exe'` 完成开发版、简易版、演示/录制/视觉工具构建及打包。
- CTest：**18/18 通过，0 跳过**，包含原有 16 项及新增 2 项；真实 OCR 集成、嵌入 OCR、后台消息隐藏窗口测试与两版 UI 回归均通过。测试耗时 25.55 秒，不代表游戏识别延迟或功耗性能。
- Go：完整 `go test -count=1 ./...` 通过；全部 31 状态均经 POST、GET、SSE、落盘与重载检查；跨语言 CTest 也通过。
- 额外 `go test -race -count=1 ./...` 未能运行：Windows ThreadSanitizer 在测试启动前报告虚拟地址空间分配失败（error code 87）。不能据此声称竞态检查通过；普通测试与跨语言测试通过。
- `git diff --check` 通过。没有向真实游戏发送测试按键，没有推送或部署。

本机原 MinGW 内部 `cc1plus.exe` / 汇编器存在 DLL 搜索故障，最小 `int main()` 也失败。未修改系统安装或全局 PATH：只将现有编译器内部工具和所需 DLL 复制到忽略的 `build/p0-tools/`，通过 CMake 本地缓存的 `CMAKE_CXX_FLAGS` 中两个 `-B` 路径选择隔离工具及原 `D:/msys64/ucrt64/bin/` 汇编器/链接器；`CMAKE_RC_FLAGS` 的 `--preprocessor-arg` 同样传入两个 `-B`。该环境修复未写入源码或发布脚本；当前 `build/windows/` 可直接复用。其他正常 MinGW/MSVC 环境不需要此绕过。
