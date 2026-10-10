# 远程监控协议 v2（P2）

本阶段沿用 v0.5.13 的 Go Dashboard、HTTP Heartbeat、SSE 和两种独立 Bearer Token。原有 `state` 是 Automation State，含义保持原样；`ACTIVE` 不代表当前已经确认在对局内。对局观察和硬件采集都是输出信息，服务端没有遥控键鼠或恢复游戏的接口。

## 兼容与接口

| 接口 | 方法 | 认证 | 行为 |
|---|---|---|---|
| `/api/heartbeat` | POST | Agent Token | 写入一条完整快照，最大 16 KiB |
| `/api/status` | GET | View Token | 返回缓存与按当前时间派生的有效状态 |
| `/api/events` | GET | View Token | SSE `snapshot`；变化时推送，平时每 15 秒刷新 |
| `/healthz` | GET | 无 | 服务健康检查 |

Heartbeat 原字段 `machine/state/detail/pid/uptime/restarts/score/dry_run/version` 全部保留。旧客户端可以不发送 `v2`；服务端继续接受 P0 的全部 31 个 Automation State，且不会将 v1 的 `ACTIVE` 显示为绿色局内状态。v1 覆盖之前的 v2 时清除当前观察/硬件信息，保留内部防重放水位。

新增的 `v2` 是可选对象；其内部 `hardware` 可省略或为 null。未知 JSON 字段、错误类型、非法枚举、uint64 溢出或字段越界返回 HTTP 400。合法字段但缺乏可靠证据的局内声明会降为 `UNVERIFIED`，而非丢掉整个心跳。乱序、重复、旧实例或旧会话返回 HTTP 409，不替换缓存。

Token 只通过 `Authorization: Bearer ...` 传输，不放 URL、状态、日志或文档。Agent Token 无权读取状态，View Token 无权写入。网页只在内存保存 View Token。

## 输入字段合同

所有表中标为 `uint64 string` 的字段都是**十进制 JSON 字符串**，不能发送 JSON number、浮点、负号或带前导零的表示。这样 FILETIME、序号和 tick 不受 JavaScript 53 位整数限制。零写作 `"0"`。PID 和 TTL/间隔是 JSON number。

| `v2` 字段 | 类型与用途 |
|---|---|
| `agent_pid` | 非零 uint32，助手进程 |
| `agent_created` | 非零 uint64 string，助手进程创建 FILETIME |
| `sequence` | 非零 uint64 string，唯一网络线程递增；不随助手 start/stop 重置 |
| `sent_at` | UTC RFC3339，可含小数秒，实际发送时间 |
| `sent_at_ms` | uint64 string，本机 `GetTickCount64`；同一实例每条心跳严格递增 |
| `observation` | 下表的完整观察快照 |
| `hardware` | 可选采样快照，不存在时不继承上一条的值 |

| `observation` 字段 | 类型与含义 |
|---|---|
| `session_id` | 最长 256 字节的不透明会话 ID；助手运行时必填 |
| `revision` | uint64 string，开始/停止产生新修订；初始未运行可为零 |
| `mode` | `CUSTOM_ROOM` / `DEATHMATCH` |
| `automation_state` | P0 的 31 个状态之一；与顶层 state 独立采样，允许短暂不同 |
| `running` | bool，助手是否运行 |
| `game_pid` / `game_created` | uint32 / uint64 string；不存在游戏时都是零 |
| `state` | `IN_MATCH` / `OUT_OF_MATCH` / `UNVERIFIED` / `OFFLINE` |
| `confidence` | `NONE` / `PROCESS` / `VISUAL` / `REPEATED_VISUAL` |
| `source` | `NONE` / `TARGET_TITLE` / `TARGET_JOIN` / `EXPLICIT_START` / `DEATHMATCH_QUEUE` / `DEATHMATCH_SCOREBOARD` |
| `last_confirmed_at_ms` | uint64 string，最后一次有效连续局内确认的原截图 tick |
| `last_observed_at_ms` | uint64 string，最后观察 tick |
| `last_reliable_visual_at_ms` | uint64 string，最后可靠视觉 tick |
| `process_observed_at_ms` | uint64 string，最后进程身份扫描 tick |
| `page` | P1 `CNPage` 的 PascalCase 名称，如 `Room` / `Combat` / `Menu` / `None` |
| `window` | bool 字段 `minimized/occluded/foreground/background_input` |
| `last_error` | 最长 2000 字节的诊断原因 |
| `evidence_ttl_ms` / `process_ttl_ms` | 默认 15000，各不得超过 15000 |

初始 STOPPED 可发送 `revision:"0"`、空 session、`running:false` 和 `UNVERIFIED`。正常 stop 会递增 revision 并更换 session，服务端不要求保持停止前的 session ID。

`last_confirmed <= last_reliable_visual <= last_observed <= sent_tick`，所有事实 tick 都不得在发送 tick 之后。局内状态还要求 confirmed 与 reliable visual 相等且均有效，进程证据也没有过期；窗口必须前台、未最小化、无遮挡且没有后台输入。战令房要求 `Room` 页面及目标房间/明确开始来源；死斗要求 `Combat` 页面及死斗/明确开始来源。服务端信任已认证 Agent 的两帧观察声明，不接收或重新识别截图。

| `hardware` 字段 | 类型与含义 |
|---|---|
| `sampled_at` | UTC RFC3339，同次采样尝试开始时间 |
| `sampled_tick_ms` | 非零 uint64 string，同次采样本机 tick |
| `sequence` | 非零 uint64 string，桥接进程可独立重启并从 1 开始；不会仅因序号重置拒收 |
| `interval_ms` | 1000–100000，默认 5000，可降频 |
| `stale_after_ms` | 默认 15000，不超过 interval 的 3 倍，绝对上限 300000 |
| `cpu_package_temperature` | 下述 Reading，单位 `celsius` |
| `cpu_package_power` / `gpu_power` | Reading，单位 `watts` |

Reading 为 `{value:number|null,unit,source,sensor,reason}`。缺失 value 必须是 null 且有非空 reason；不能用假的 0W、0°C 或预测值代替。有效数值须有限，温度范围 -100–200°C、功率范围 0–10000W，有非空 source/sensor 且 reason 为空。零只有确实来自传感器时才合法。桥接不可用、CPU 权限不足等独立失败可以保留其他有效传感器。

## 有效性、首见期限与防重放

服务端接收时先扣掉 `sent_tick - proof_tick` 的已有年龄，再扣 UTC 可见的运输延迟；明显未来发送时间（超过 1 秒）视为时钟不可信。很久以前的首次请求也不能得到绿色。未来不超过 1 秒的时间额外扣 1 秒，保守处理小幅时钟偏差。

每个实例在内部 `protocolGuard` / `receipt` 保存以下首见锚点：

- `VisualProof` 与 `visualDeadline`：可靠视觉。
- `ConfirmedProof` 与 `confirmedDeadline`：连续局内确认。
- `ProcessObserved` 与 `processDeadline`：进程事实。
- `HardwareProof` 与 `hardwareDeadline`：采样事实，以 sample tick 而不是硬件 sequence 识别。

相同 proof 的后续心跳只能缩短首见 deadline，不能延长。即使 sequence 增加、UTC 重新填写、send tick 只增加 1 ms，也无法把同一旧截图或样本重新变为 15 秒新鲜。v1 暂时覆盖不会丢弃锚点。API 输出还再次按当前服务器经过时间扣减，绝不依赖远端主动刷新来更新硬件。

同助手实例要求网络 sequence 和发送 tick 严格递增；创建 FILETIME 更旧的实例拒收。revision 退回、停止后的旧运行结果、最近已退出的 session、同 revision 退回的 process-observed 水位均拒收。有限的 retired-session 集合会淘汰旧条目，防重放还依靠序号与进程水位；不会因为恢复超过 256 次永久拒收新会话。新实例/会话完整替换，不拼接以前的观察或硬件。

`IN_MATCH` 的视觉或进程事实过期即 `UNVERIFIED`；`OUT_OF_MATCH` 的可靠视觉同样会过期；`OFFLINE` 是最近确认游戏进程不存在，进程事实过期也回到 `UNVERIFIED`。主机连接 90 秒未收到心跳另显示离线，不能与游戏进程 OFFLINE 混用。

持久化保存原始输入与防重放 proof/序号水位，**不保存可继续信任的运行时 deadline**。服务端重启后已有 v2 统一降为 `UNVERIFIED`，硬件变 null；同一持久 proof 即使换更高网络序号也不能恢复绿，必须收到真正更晚的视觉/进程事实。快照读取使用上限 16 MiB 的有界读取，以容纳 v2 和既有 128 设备/200 事件限制。

## GET/SSE 派生输出

每台 Agent 新增只读 `monitor`：

```json
{"state":"IN_MATCH","reason":"","evidence_remaining_ms":12000,"process_remaining_ms":13000,"hardware_remaining_ms":11000,"clock_trusted":true,"host_online":true}
```

remaining 字段是安全范围内的 JSON number，表示**生成此响应时**还剩多少毫秒。`v2.observation.state/confidence/last_error` 同步保守降级；过期硬件在返回副本中 value=null、reason=`sample_expired`，保留来源和传感器名称。原始缓存不被这些查询修改。GET 与 SSE 使用同一派生函数和深拷贝，包括每个传感器的值指针；没有硬件读取、OCR 或进程查询回调。

对局有效状态变化可以产生最近事件；单纯硬件采样更新不会不断刷事件。SSE 仍定期刷新完整快照，因此只读用户能看到证据到期。

Agent 常规心跳约每 30 秒发送；有新的局内确认、局外视觉或游戏进程不存在事实时，约每 10 秒续报一次，使 15 秒事实 TTL 连续有效。续报必须携带新的 proof tick；稳定状态的旧截图或旧进程扫描不能续期。该观察续报独立于硬件是否全部缺失。

## 浏览器行为

原 Dashboard 布局、自动化状态文字和事件列表保留，新增独立局内状态、模式和三项硬件诊断。绿色/healthy 仅由新鲜 v2 `IN_MATCH` 产生；旧版 ACTIVE、后台消息成功、纯进程存活都不能变绿。

浏览器从 server remaining 扣除运输年龄和本地经过时间。经过时间取 `max(performance.now 增量, Date.now 增量)`：Mac 睡眠时单调计时器可能暂停，睡醒仍立即失效；壁钟回退仍使用单调经过时间，前跳则保守到期。重复同一 server timestamp 的快照不重置倒计时。SSE 断线/401 立即撤绿；超过 20 秒没有快照判定连接停滞并重连。客户端过期判断独立于网络回调，不能无限保持旧绿色。

传感器显示有效值与来源，缺失显示 `—` 和具体原因，过期显示 `sample_expired`；完整 sensor 和采样UTC可通过提示读取。没有在浏览器根据刷新请求重新采样。

## Tailscale 与联调范围

保持服务端默认监听 `127.0.0.1:8765`，在 Windows 主机通过 Tailscale Serve 复用原服务：

```powershell
tailscale serve --bg 8765
```

在 tailnet 内访问对应 HTTPS 地址，使用 View Token 调用 `/api/status` 或登录 Dashboard。不要开放公网端口，也不要启用 Funnel。Serve 用法见 [Tailscale 官方文档](https://tailscale.com/docs/reference/tailscale-cli/serve)。令牌不放链接。

本轮可以开始 Tailscale + Mac API 联调；尚未开启 Tailscale、部署远端服务或进行 macOS 实机验收，也未开发完整 Mac 菜单栏客户端。真实对局截图覆盖、后台效果、长时间远程连接与实际 CPU/内存/整机功耗对比仍需实机验证，不用估计值填报。

## 验证入口

- `go test -count=1 ./...`：v1/v2 HTTP、GET/SSE一致、时间/时钟/证据到期、首见期限、重启、乱序/旧实例/停止迟到、300 次恢复会话、硬件 null/过期/桥接重启、并发深拷贝、浏览器测试。
- `go vet ./...`：完整静态检查。
- `node browser_test.cjs`：纯 JS freshness 及实际 app 假 DOM 渲染，覆盖断线、睡眠、旧快照、注销及硬件失败。
- `TestCPPV2WireContract`：执行真实 `telemetry-v2-tests.exe --fixture`，从 C++ 生成整条 Heartbeat 经 Go POST、GET、SSE 和持久化重启验证。可用 `OWH_V2_PROTOCOL_FIXTURE` 显式指定；普通测试默认探测 `../build/windows/telemetry-v2-tests.exe`，缺失时明确 skip。
- P0 的全部状态和编译客户端协议检查保留。Windows race runtime 先前因 TSan 地址空间分配 error 87 无法启动，本轮不声称 race 验证通过。
