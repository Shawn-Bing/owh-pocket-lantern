# P2.5 人工游戏与 Mac 联调步骤

本文是待执行步骤，不是实机通过记录。结果统一填 PASS / FAIL / BLOCKED / NOT TESTED，并记录日期、助手模式、进程身份、服务端时间和实际截图。请勿在截图、命令历史或提交中包含 Token。

## Windows 本地准备

1. 使用本功能分支的构建包，保留原 v0.5.13 可用版本；两版助手的配置目录各自独立。不要同时启动两个助手控制同一游戏。
2. Dashboard 继续仅监听 `127.0.0.1:8765`。先检查该端口是否已有服务；如果有，确认用途后复用，不结束未知进程。
3. 助手配置本机 Dashboard 地址与 Agent Token；浏览器使用不同的 View Token。先在助手停止时确认 API 有新心跳和 `v2`，`running=false`，没有挂机异常计时。
4. 分别观察真实助手窗口、`/api/status` 和网页；不要单凭助手的 `ACTIVE` 判断局内。
5. 真实游戏操作由用户完成。测试期间保留 F8 紧急停止；不要为传感器读数额外提升整个助手权限或安装驱动。

## 游戏状态矩阵

下表每行都需独立记录；没有亲自执行则保持 NOT TESTED。期望状态以当前可验证画面为准，不能用进程存活、高功耗或输入 API 成功替代视觉。

| 模式/操作 | 预期结果与检查点 | 初始记录 |
|---|---|---|
| 战令房：未启动游戏，助手开始 | 成功扫描确认游戏不存在才是 observation `OFFLINE`；主机仍可在线 | NOT TESTED |
| 战令房：主菜单 | 明确识别后为 `OUT_OF_MATCH` | NOT TESTED |
| 战令房：自动导航 | 原导航动作正常；已识别非局内页为 OUT，未知/加载为 UNVERIFIED | NOT TESTED |
| 战令房：进入目标房间 | 目标来源和同一 PID+创建时间有效；连续两张可信 HUD 后才 IN_MATCH | NOT TESTED |
| 战令房：稳定挂机 | 新视觉证据续期；记录第一次绿色出现所需时间，不填写估算准确率 | NOT TESTED |
| 战令房：结束并重新开始 | 非局内/未知时撤绿，新的连续可信 HUD 才恢复绿色 | NOT TESTED |
| 战令房：返回菜单或断线 | 菜单明确为 OUT；截图不可用/未知为 UNVERIFIED；原恢复逻辑正常 | NOT TESTED |
| 战令房：自动恢复进房 | 旧进程和旧截图不能确认新会话；新来源及两帧后恢复 | NOT TESTED |
| 死斗：开始匹配、等待对局 | 匹配/队列不能直接确认为 IN_MATCH；加载/英雄选择按实际证据降级 | NOT TESTED |
| 死斗：进入对局 | 死斗来源、真实 Combat/HUD 和连续两帧均满足后 IN_MATCH | NOT TESTED |
| 死斗：结束、菜单、重新匹配 | 退出旧确认；连续未确认时间不因自动重试/换游戏进程清零 | NOT TESTED |
| 死斗：重新进入对局 | 新可信连续证据恢复 IN_MATCH 并解除上一异常 | NOT TESTED |
| 任一模式：失焦/遮挡/GameViewer | 无可靠画面立即 UNVERIFIED；记录远程软件是否影响原挂机输入 | NOT TESTED |
| 任一模式：最小化与恢复 | 最小化不能永久保留绿色；恢复后需要新证据 | NOT TESTED |
| 任一模式：游戏退出、重启、PID 复用 | 不继承旧进程确认；成功空扫描才确认游戏不存在 | NOT TESTED |
| 任一模式：助手停止、重启 | 手动停止立即禁用告警；旧线程不能重新变绿，新启动建立新运行期 | NOT TESTED |
| 任一模式：OCR 失败、黑屏、未知画面 | UNVERIFIED；旧确认最多到有效期，不冒充新画面 | NOT TESTED |
| 任一模式：不可见超过三分钟后恢复 | 来源可能已过期，需要重新识别标题/队列/计分板或显式绑定当前局；记录实际恢复情况 | NOT TESTED |

若发现状态与画面不符，记录助手模式、Automation State、Match State、source/confidence、四个证据时间戳、PID/创建时间、会话 ID 和错误原因，并保留该时刻截图。不要提交账户凭据或包含 Token 的配置。

## Tailscale 准备与影响

本轮诊断若报告 Tailscale 未安装或未登录，这一段需要用户执行。先按 Tailscale 官方安装说明完成 Windows 和 Mac 登录同一 Tailnet；不要替换其他 VPN、代理或卸载网络组件。

在 Windows 上先只读检查：

```powershell
tailscale version
tailscale status
tailscale serve --help
tailscale serve status
```

确认已连接、CLI 支持命令且现有 Serve 配置没有冲突后，执行：

```powershell
tailscale serve --bg 8765
tailscale serve status
```

**影响：**这会保存后台 Serve 配置，在 Tailnet 的 HTTPS 地址代理本机 8765 服务，重启后可能继续生效；它会修改 Serve 状态。若已有 Serve 配置或 HTTPS 入口，先保存并核对原配置，勿盲目覆盖。此步骤不需要把 Go 监听地址改成公网地址，不需要路由器端口转发，也不使用 Funnel。以 `serve status` 输出的实际 HTTPS 地址为准。

不要使用全局 `serve reset` 清理本测试，因为可能移除其他服务。需要撤销时先按当前 CLI 的帮助确认仅关闭本次入口的操作。

官方说明：[Tailscale Serve](https://tailscale.com/docs/reference/tailscale-cli/serve)。

## Mac Terminal：连接、认证与 SSE

以下适用于 macOS 默认 zsh。`BASE` 必须替换为上述实际 HTTPS 地址；地址中不要放 Token。View Token 从本机私密来源取得，不要贴进聊天或仓库。

```zsh
BASE='https://你的主机.你的tailnet.ts.net'
curl --silent --show-error --include --connect-timeout 5 --max-time 10 "$BASE/healthz"
```

预期 HTTP 200。连接失败时依次确认 Windows Dashboard 本地健康、两端 Tailscale 登录/连接、Serve 状态、Tailnet ACL 和实际主机名。不要通过关闭防火墙或开启 Funnel 绕过问题。

在终端隐藏输入 View Token，命令历史不会含真实值；通过标准输入传给 curl，避免把 Token 放在 URL：

```zsh
read -rs "VIEW_TOKEN?View Token: "
printf '\n'
printf 'header = "Authorization: Bearer %s"\n' "$VIEW_TOKEN" |
  curl --silent --show-error --fail-with-body --config - \
       --connect-timeout 5 --max-time 10 "$BASE/api/status"
printf '\n'
```

预期 HTTP 200 JSON，包含 `agents`；连接助手后对应设备应含 `v2` 与 `monitor`。没有设备时空列表属于合法结果，不代表局内。错误 Token 应返回 401。不要使用 Agent Token 登录看板；两个角色不可互换。

检查 SSE：

```zsh
printf 'header = "Authorization: Bearer %s"\n' "$VIEW_TOKEN" |
  curl --silent --show-error --no-buffer --config - \
       --connect-timeout 5 --max-time 35 "$BASE/api/events"
unset VIEW_TOKEN
```

预期出现 `event: snapshot` 与 JSON `data:`，首次立即返回，此后状态变化或约 15 秒收到快照。35 秒到期 curl 返回 28 是这条限时诊断的预期结束，不等于服务故障。若本机 curl 不支持 `--fail-with-body`，状态查询改用 `--fail`；先用 `curl --version` 核对。

## Mac 浏览器：断线、恢复、刷新与告警

1. 在 Safari/Chrome 打开同一 HTTPS 地址，使用 View Token。比较网页与 API 的设备、运行模式、对局状态、传感器缺失原因。
2. 用户在 Windows 手动停止助手挂机：观察 running=false、撤绿和异常清除；停止挂机不等于停止助手进程或 Go 服务。
3. 在持续运行期间使画面不可验证，确认默认超过五分钟后产生一次异常。恢复两帧可信 IN_MATCH 后应解除；第二次独立异常可再次触发。
4. 在异常期间刷新页面、关闭重开并重新登录：计时应由服务端保持，不因浏览器刷新重新开始。区分当前明确 OUT 与此前包含未知画面的连续异常，不把累计未知时间描述为已明确不在局内的时间。
5. 用户短暂断开 Mac 网络：页面撤绿并显示连接异常。恢复网络后 SSE 应重连并使用新快照，不直接恢复缓存绿色。
6. 用户短暂停止并恢复测试用 Go 服务：重启后旧局内证据不能恢复绿色；告警不能从未验证落盘运行状态恢复旧计时，必须收到新心跳。
7. 用户短暂断开 Windows 客户端链路：超过主机心跳阈值后显示主机失联，与游戏进程不存在分开；恢复后核对快照、告警去重及解除。
8. Mac 睡眠再唤醒：旧绿色立即失效，等待新快照。P2.5 没有原生通知，本轮不验证休眠期间系统通知送达。

以上涉及网络/服务中断的操作应只作用于自己的测试实例，不结束未知进程或重配置全局网络。真实结果请填写本轮报告；未执行时保持 NOT TESTED。
