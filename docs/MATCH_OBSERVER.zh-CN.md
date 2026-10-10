# 独立对局观察器（P1）

观察器只消费现有进程扫描、截图及视觉识别结果，不发送输入、不调用恢复，不改变 RoomSession、DeathmatchSession 或 Automation State 的决策。ACTIVE 表示旧自动化会话仍可运行，不能当作局内确认。

## 状态含义

- IN_MATCH：同一助手运行代次、同一游戏 PID 与进程创建时间，连续两张可信局内 HUD，且有对应模式来源。
- OUT_OF_MATCH：可靠视觉明确识别主菜单、设置菜单、队列、目标房间大厅、结算或登录错误等非局内页面。设置菜单可能覆盖一场仍在进行的比赛，此状态只说明当前画面不能确认为局内 HUD。
- UNVERIFIED：未知、加载、英雄选择、重生、失焦、遮挡、最小化、截图失败、助手停止或证据过期。
- OFFLINE：本机进程扫描明确确认目标游戏进程不存在。findProcess 返回空时，再用可区分错误的完整进程枚举核实；枚举失败或发现 PID 但身份不可读取均为 UNVERIFIED，不会刷新进程事实时间。助手停止不等于游戏退出；服务器/网络离线由远程协议另行表示，不能与这个状态混同。

confidence 是证据种类（NONE、PROCESS、VISUAL、REPEATED_VISUAL），不是统计概率。绿色只对应 IN_MATCH；进程存活、功耗或 PostMessage 成功不能提升它。

## 证据与来源

agent/match_observer.hpp 复用现有窗口/进程检查，并以 tick 结束保护覆盖所有提前 continue 的无观察分支；不新增截图或 OCR。core/match_evidence.hpp 在旧自动化修改会话之前转换真实识别结果：

- 战令房来源可由现有 CNJoinGate 的待加入目标、实时识别的完整目标大厅标题，或绑定 PID+创建时间的 CNRoomStart 显式开始建立。只有 liveRoomHud 与当前 combatHud 像素都成立才计入局内连续帧。目标标题大厅本身不算战斗画面。
- 死斗来源可由真实死斗队列、已提交死斗匹配请求之后的队列/英雄选择、实时个人排名计分板，或显式当前对局开始建立。Combat 与当前 HUD 必须同时成立；计分板来源还需当前 deathmatchScoreboard 像素成立。
- RoomSession.active() 不构成观察器来源。DeathmatchSession.trusted() 可由任意 Combat 自己建立，也不构成观察器来源。任意手动 Ready/Combat 模板不能代替这些证据。
- 登录、结算以及明确离开目标模式的导航会立即撤销来源。未知、加载或短暂不可见会中断两帧连续性。

阈值集中在 MatchObserverConfig：进程不存在事实的 TTL 默认 15 秒（阻塞扫描不能永久声称 OFFLINE），视觉 TTL 默认 15 秒，两帧最大间隔 10 秒，模式来源最长保留 180 秒。IN_MATCH 和 OUT_OF_MATCH 的快照读取也会检查 TTL，即使 OCR/worker 阻塞也不会无限保留旧状态。未知或不可捕获无需等待 TTL，立即 UNVERIFIED。OCR 结果使用原截图时间，不能用返回时间刷新旧证据。

两帧必须同时满足递增的捕获时间及帧序号。重复提交同一帧、回退时钟、旧代次、旧 PID 或同 PID 的旧创建时间会被拒绝。助手开始、停止以及同运行代次的手动重启都会独立更新观察器 revision，旧线程不能恢复新会话的绿色状态。

模式来源保留与视觉 TTL 不同：15 秒后撤销当前确认，但三分钟内可在恢复可见后以两张新 HUD 重新确认。超过三分钟没有新的可信同模式证据会撤销来源；此时无标题 HUD 可能持续黄色，需要实时标题/死斗队列/计分板，或用户使用“已在局内，开始挂机”重新绑定后再确认两帧。现有自动化仍可继续，它不会读取观察器状态来改变操作。实际截图是否支持无交互恢复待实机验证。

## P2 接口

MatchObserver::snapshot(monotonicNow) 返回独立 MatchSnapshot，包含：

- mode、automation、running；
- game.pid、game.created、processObservedAt；
- state、confidence、source；
- lastConfirmedAt、lastObservedAt、lastReliableVisualAt；
- page、window.minimized、window.occluded、window.foreground、window.backgroundInput；
- sessionId、revision、lastError、evidenceTtlMs、processTtlMs。

时间均为助手本机单调毫秒值，0 表示尚无该类证据。lastConfirmedAt 是最近一次完成连续确认的真实截图时间，只能由新的可靠局内帧刷新；lastObservedAt 可包含不可捕获尝试，不能用作绿灯续租。sessionId 包含助手实例标识、观察器 revision、进程序列及 PID/创建时间。

P2 必须每次发送前调用 snapshot，而非缓存先前绿色快照。状态变化应及时发送；维持绿色的续报必须包含新的 lastConfirmedAt，普通心跳不得延长旧证据。远端需独立处理传输延迟、agent/服务器失联及会话更换。

## 测试与限制

tests/match_observer_tests.cpp 用真实 recognizeSemantic、CNJoinGate、CNRoomStart、DeathmatchSession 和像素 HUD/计分板夹具覆盖任务要求的十类确定性场景，并额外覆盖任意手动模板自证、PID 复用、缺失创建时间、重复帧、OCR 迟到、快照 TTL、三分钟来源失效及显式重绑。并发停止测试以 promise 控制旧线程返回顺序，不依赖 sleep 或游戏环境。

tests/simple_ui_tests.cpp 还实际调用 setRunning(false)，检查 UI 停止立即撤销观察器并拒绝旧 worker 的状态发布；真实进程观察适配注入枚举失败、成功空列表和有 PID 但身份不可得的结果，验证 process_scan_failed 不会变成 OFFLINE 或刷新旧的进程事实。

P1 验证：开发版/简易版及全部 C++ 目标构建通过；CTest 19/19 通过，Go 普通测试通过，当前工作区 dist 打包完成。观察器 TTL 元数据还以自定义配置单独回归，避免传输层重复常数。

这些是模拟与适配测试，不等于真实游戏识别准确率。真实战令房/死斗画面、不同分辨率/语言、GameViewer 连接、遮挡、最小化后恢复、长时间挂机及 CPU/功耗变化均需实机验证；没有实测数据时不报告准确率或功耗估算。
