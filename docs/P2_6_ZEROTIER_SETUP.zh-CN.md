# P2.6 ZeroTier 访问与回滚操作单

本轮采用 **方案 A：Windows portproxy**。Go Dashboard 保持 127.0.0.1:8765；系统只在 Windows 的 ZeroTier IPv4 上增加 TCP 入口，不修改 Go 监听代码。用户明确授权并要求重试UAC后，已于2026-10-11 05:56 UTC（America/Denver：10月10日23:56）完成本机配置。以下管理命令用于审查、复现和局部回滚；Mac实链路尚未验收，已配置主机不要重复添加。

## 本次地址与检查

| 项目 | 本次值 / 结果 |
|---|---|
| Windows ZeroTier IPv4 | 10.243.245.219 |
| Mac ZeroTier IPv4 | 10.243.12.243（用户提供） |
| ZeroTier | 1.14.2；服务 Running / Automatic；网卡 Up；IPv6 绑定启用 |
| IP Helper | Running / Automatic |
| 8765 / portproxy 初始状态 | 无监听 / 无条目 |
| 防火墙 | 三个 Profile 仍ON、BlockInbound/AllowOutbound；管理员结果实测两条精确规则存在于ActiveStore |
| Windows网络状态 | 管理员只读CLI：ONLINE、network OK、PRIVATE，Managed IP 10.243.245.219/16 |
| Mac/控制器授权 | Mac未操作，Central未审计；Windows ONLINE/OK不证明Mac已授权或API可达 |
| 当前映射/监听 | 10.243.245.219:8765→127.0.0.1:8765；只见这两个IPv4监听，无0.0.0.0或IPv6入口 |
| 当前规则 | OWH-P26-ZT-8765-Allow-Mac仅允许10.243.12.243；OWH-P26-ZT-8765-Block-Other阻止其余IPv4；均只针对本机ZT地址的TCP8765 |

此前测试服务关闭后，本次为用户远程联调重新隐藏启动独立 Dashboard，PID **4608**，仅监听 **127.0.0.1:8765**，/healthz=200；当前保持运行。ZeroTier入口由系统portproxy拥有（核验时PID5708），不是Go的第二个监听。继续复用数据目录：

~~~text
D:\CodexWorktrees\22a0\OverwatchHelper\build\p2-6-remote-20261010
~~~

agent-token.txt 仅供 Windows 助手，view-token.txt 仅供浏览器/Mac。不要把内容贴进聊天、URL、日志或 Git。未修改现有助手配置；接入真实助手时，用户在 GUI 配置该 localhost 地址与对应 Agent Token。空 agents 列表仅说明尚无上报。


当前服务已经运行；仅在它停止后，先确认8765的localhost监听没有其他服务占用，再在普通PowerShell运行下列命令并保持窗口开启。既有系统portproxy入口可保留，不能把它当成未知Go进程停止。此命令复用同一数据及Token，不创建系统服务：

~~~powershell
Set-Location 'D:\CodexWorktrees\22a0\OverwatchHelper'
.\dist\owh-dashboard.exe -listen '127.0.0.1:8765' -data 'D:\CodexWorktrees\22a0\OverwatchHelper\build\p2-6-remote-20261010'
~~~

上述前台方式用Ctrl+C停止；当前隐藏服务可在普通PowerShell核对身份后停止：

~~~powershell
$p = Get-CimInstance Win32_Process -Filter 'ProcessId = 4608'
if ($p -and $p.ExecutablePath -eq 'D:\CodexWorktrees\22a0\OverwatchHelper\dist\owh-dashboard.exe' -and $p.CommandLine.Contains('D:\CodexWorktrees\22a0\OverwatchHelper\build\p2-6-remote-20261010')) {
    Stop-Process -Id 4608
} else {
    throw 'PID已变化或身份不符；先重新核对，不结束未知进程。'
}
~~~

停止Dashboard不移除持久portproxy或防火墙规则；完整撤销入口见下方回滚。未创建开机服务或任务，Windows重启后Dashboard需要重新启动。精确管理员配置证据在忽略的`build/p2-6-remote-20261010/network-setup-result.json`，去身份状态摘要在同目录`network-verification-summary.json`；原始CLI输出不提交。

## 信任边界

访问地址是 **HTTP**，不是公网 HTTPS；应用本身没有 TLS。ZeroTier 通常提供节点间加密和认证，但本机应用到本机代理仍是 HTTP，端点、授权设备和网络管理员必须可信。保持网络私有、逐台核对成员身份；保留应用 Token 和入口限制。不要使用关闭链路加密的 Trusted Paths。

参见 [ZeroTier 协议与 Trusted Paths](https://docs.zerotier.com/protocol/) 和 [私有网站访问](https://docs.zerotier.com/website/)。本轮未读取或修改现有 Trusted Paths 配置，不把默认安全属性当成当前配置审计结果。

## 用户核对授权（只读）

Windows 管理员 PowerShell：

~~~powershell
& 'C:\Program Files (x86)\ZeroTier\One\zerotier-cli.bat' info
& 'C:\Program Files (x86)\ZeroTier\One\zerotier-cli.bat' listnetworks
Get-Service ZeroTierOneService,iphlpsvc
Get-NetIPAddress -IPAddress '10.243.245.219'
netsh interface portproxy show all
Get-NetTCPConnection -State Listen -LocalPort 8765
~~~

Mac：

~~~zsh
sudo zerotier-cli info
sudo zerotier-cli listnetworks
~~~

Mac CLI 不在 PATH 时使用已安装 ZeroTier 菜单的网络详情，不重复安装或新建网络。在原有 Central 管理页逐一核对两端节点 ID、相同网络、授权状态、Managed IP。网卡 Up / 存在 IP 不等于当前授权成功。参见 [官方设备授权说明](https://docs.zerotier.com/quickstart/)。

## 添加入口：需用户执行或明确授权

只改变 **10.243.245.219:8765/TCP**：允许 Mac 10.243.12.243，显式阻止其他 IPv4 来源进入该地址的该端口，避免普通宽泛 Allow 规则扩大入口。不改全局策略、其他端口、ZeroTier 网络、代理、VPN、服务启动方式或 localhost 监听。系统规则不会随本次 PowerShell 退出消失。

下列脚本已执行成功，当前无需重复运行。以后复现时在管理员PowerShell **整段执行一次**。如果将包含中文的代码保存为`.ps1`并用Windows PowerShell5执行，必须保存为UTF-8带BOM；本次先用实际PS5的Parser确认无语法错误后执行。同名规则、相同入口或地址变化均停止，不覆盖。IP Helper 未运行时停止，不自行启停服务；已有 IPsec Allow-Bypass 策略时停止，先审查影响。

~~~powershell
$ErrorActionPreference = 'Stop'
$ztIP = '10.243.245.219'
$macIP = '10.243.12.243'
$allowName = 'OWH-P26-ZT-8765-Allow-Mac'
$blockName = 'OWH-P26-ZT-8765-Block-Other'
$principal = [Security.Principal.WindowsPrincipal]::new([Security.Principal.WindowsIdentity]::GetCurrent())
if (-not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    throw '请在管理员 PowerShell 执行；未作网络修改。'
}
$address = Get-NetIPAddress -IPAddress $ztIP -AddressFamily IPv4
$adapter = Get-NetAdapter -InterfaceIndex $address.InterfaceIndex
if ($adapter.InterfaceDescription -notlike '*ZeroTier*' -or $adapter.Status -ne 'Up') {
    throw '该地址不是当前已启用的 ZeroTier 网卡地址。'
}
if ((Get-Service iphlpsvc).Status -ne 'Running') { throw 'IP Helper 未运行，请先核查。' }
if ((Invoke-WebRequest 'http://127.0.0.1:8765/healthz' -UseBasicParsing -TimeoutSec 5).StatusCode -ne 200) {
    throw '先确认 localhost Dashboard 健康。'
}
$conflicts = @(Get-NetTCPConnection -State Listen -LocalPort 8765 | Where-Object LocalAddress -ne '127.0.0.1')
if ($conflicts.Count) { throw '8765 已有非 localhost 监听，先核对归属。' }
$proxyKey = 'HKLM:\SYSTEM\CurrentControlSet\Services\PortProxy\v4tov4\tcp'
if (Test-Path -LiteralPath $proxyKey) {
    $proxyNames = (Get-Item -LiteralPath $proxyKey).GetValueNames()
    if ($proxyNames -contains "$ztIP/8765" -or $proxyNames -contains '0.0.0.0/8765') {
        throw '已有相关 portproxy，不覆盖。'
    }
}
if (Get-NetFirewallRule -Name $allowName,$blockName -ErrorAction SilentlyContinue) {
    throw '已有同名规则，不覆盖。'
}
$profiles = Get-NetFirewallProfile -PolicyStore ActiveStore
if ($profiles | Where-Object { $_.Enabled -eq 'False' -or $_.DefaultInboundAction -eq 'Allow' -or $_.AllowLocalFirewallRules -eq 'False' }) {
    throw '防火墙未启用、默认放行或组策略禁用本地规则，先核查。'
}
$bypass = @(Get-NetFirewallRule -PolicyStore ActiveStore -Enabled True -Direction Inbound | Get-NetFirewallSecurityFilter | Where-Object OverrideBlockRules -eq 'True')
if ($bypass.Count) { throw '存在 Allow-Bypass 策略，需人工审查，暂不配置。' }

# 先阻止其他来源，再放行 Mac，最后添加转发。
New-NetFirewallRule -Name $blockName -DisplayName $blockName -Direction Inbound -Action Block -Enabled True -Profile Any -Protocol TCP -LocalPort 8765 -LocalAddress $ztIP -RemoteAddress @('0.0.0.0-10.243.12.242','10.243.12.244-255.255.255.255') | Out-Null
New-NetFirewallRule -Name $allowName -DisplayName $allowName -Direction Inbound -Action Allow -Enabled True -Profile Any -Protocol TCP -LocalPort 8765 -LocalAddress $ztIP -RemoteAddress $macIP | Out-Null
netsh interface portproxy add v4tov4 listenaddress=10.243.245.219 listenport=8765 connectaddress=127.0.0.1 connectport=8765 protocol=tcp
if ($LASTEXITCODE -ne 0) { throw '添加失败；按下方回滚仅移除本次规则。' }
netsh interface portproxy show v4tov4
Get-NetFirewallRule -Name $allowName,$blockName | Select-Object Name,Enabled,Direction,Action,Profile
Get-NetTCPConnection -State Listen -LocalPort 8765
~~~

排除范围按本次 Mac IP 精确计算。**Mac IP 改变不能只改 $macIP**：先回滚，再计算两个排除范围。Windows IP 变化也先撤销旧 IP 入口，再配置新 IP。不要改成 0.0.0.0 或开放整个子网。

参考：[Microsoft portproxy](https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/netsh-interface)、[防火墙地址范围](https://learn.microsoft.com/en-us/powershell/module/netsecurity/new-netfirewallrule)、[IP Helper 要求](https://learn.microsoft.com/en-us/windows/deployment/do/mcc-ent-deploy-to-windows)。

## 回滚（只作用于本次入口）

用户在管理员 PowerShell 先确认地址与规则名确属本次创建，再执行：

~~~powershell
netsh interface portproxy delete v4tov4 listenaddress=10.243.245.219 listenport=8765 protocol=tcp
Remove-NetFirewallRule -Name 'OWH-P26-ZT-8765-Allow-Mac'
Remove-NetFirewallRule -Name 'OWH-P26-ZT-8765-Block-Other'
netsh interface portproxy show all
Get-NetTCPConnection -State Listen -LocalPort 8765
~~~

最后应只剩原 localhost 监听。不执行 portproxy reset、重置整个防火墙或删除其他规则；中途失败时只回滚本次确实创建成功的条目。

重启后需用户复核 IP、两项服务、规则、监听和 Mac 访问。本轮不重启用户主机；即使系统转发配置保留，Dashboard 也必须运行。未创建开机任务或系统服务。

## Mac 可复制测试

适用默认 zsh。curl 的 --noproxy 只让本次请求直连，不修改系统代理；浏览器访问也应走 ZeroTier 地址，不向未知代理传递 Token。

~~~zsh
BASE='http://10.243.245.219:8765'
curl --noproxy '*' --silent --show-error --include \
     --connect-timeout 5 --max-time 10 "$BASE/healthz"
~~~

预期 HTTP 200。失败时依次检查授权/IP、localhost 健康、转发、IP Helper、精确防火墙规则和路由，不关闭防火墙排障。

错误 Token 检查（没有使用真实凭据）：

~~~zsh
printf 'header = "Authorization: Bearer intentionally-invalid"\n' |
  curl --noproxy '*' --silent --show-error --config - --output /dev/null \
       --write-out 'HTTP %{http_code}\n' --connect-timeout 5 --max-time 10 \
       "$BASE/api/status"
~~~

预期 HTTP 401。从 Windows 的 view-token.txt 私下取得 View Token，隐藏输入；不要在命令行字面量、环境变量中写 Token，也不要开启 shell tracing。

~~~zsh
read -rs "VIEW_TOKEN?View Token: "
printf '\n'
printf 'header = "Authorization: Bearer %s"\n' "$VIEW_TOKEN" |
  curl --noproxy '*' --silent --show-error --config - \
       --write-out '\nHTTP %{http_code}\n' --connect-timeout 5 --max-time 10 \
       "$BASE/api/status"
~~~

预期 HTTP 200 JSON，含 agents。空列表不代表局内。接入真实助手后对应设备应含 v2、monitor、alert。

确认 View Token 没有写权限：

~~~zsh
printf 'header = "Authorization: Bearer %s"\n' "$VIEW_TOKEN" |
  curl --noproxy '*' --silent --show-error --config - --request POST --data '{}' \
       --output /dev/null --write-out 'HTTP %{http_code}\n' \
       --connect-timeout 5 --max-time 10 "$BASE/api/heartbeat"
~~~

预期 401，不写入设备。Agent Token 不能读状态的反向检查在 Windows 本地完成，不必把它分发给 Mac。

SSE：

~~~zsh
printf 'header = "Authorization: Bearer %s"\n' "$VIEW_TOKEN" |
  curl --noproxy '*' --silent --show-error --no-buffer --config - \
       --connect-timeout 5 --max-time 90 "$BASE/api/events"
unset VIEW_TOKEN
~~~

预期立即出现 event: snapshot / data:，以后变化时或约每 15 秒出现快照。90 秒到期的 curl 退出码 28 属于主动限时结束；提前 EOF、401 或持续无快照需记录。重做隐藏输入与 SSE 命令，应立即读取最新快照。

浏览器打开 **http://10.243.245.219:8765/**，输入 View Token，记录：

- [ ] API 与网页同设备、同状态、同告警 episode。
- [ ] 用户手动停止挂机：running=false、撤绿、告警禁用。
- [ ] 用户在 Mac 短暂断网：网页撤绿并提示异常，20 秒无快照保护生效。
- [ ] 恢复网络：自动重连，使用新快照与剩余有效期，不复活缓存绿色。
- [ ] 刷新、关闭再打开：服务端计时不清零，同 episode 不重复触发。
- [ ] 用户自行重启测试 Dashboard：旧局内和计时不从磁盘恢复，新心跳后重新建立预期。
- [ ] Windows 本地助手仍向 localhost 成功上报，不因 Mac 断线停止。
- [ ] Windows 重启后重新核对配置、IP 和访问（本轮未自动执行）。

未实际在 Mac 执行的项目保持 NOT TESTED / BLOCKED。本机访问本机地址或系统已有其他端口连接都不能代替 Mac 对 8765 的实测。
