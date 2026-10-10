# 硬件遥测与 v2 Agent 上报（P2）

硬件采集与挂机输入、OCR、自动恢复解耦。助手默认启动同目录的 `owh-sensors.exe` 小型桥接；关闭助手时只关闭其私有 Job 内的桥接，不操作 FanControl 或游戏。桥接缺失、退出、权限不足、传感器卡死均不会阻塞挂机线程或网络线程。

## 构建与使用

普通 Windows 构建自动使用已安装 .NET Framework 编译器构建并打包桥接：

```powershell
.\scripts\build-windows.ps1 -Go C:\path\to\go.exe
```

也可仅构建桥接；不下载 SDK、NuGet 包、驱动或监控软件：

```powershell
.\sensors\build.ps1
.\build\windows\owh-sensors.exe --self-test
.\build\windows\owh-sensors.exe --output D:\local\hardware.json --samples 3
```

默认间隔 5000 ms；可用 `--interval-ms 3000..60000` 降频。有效期固定为间隔的三倍（默认 15 秒）。`--lhm-dir` 指定现有 FanControl 安装目录，默认只探测标准 Program Files 路径。`--gpu-uuid GPU-...` 显式选择显卡；恰好一张 NVIDIA GPU 时可省略，多卡必须选择，不能默认相加。`--no-cpu` 禁用 CPU 路径。常驻运行时省略 `--samples`。输出目录应是本地磁盘；不要放网络共享中。

助手自身数据目录的 `settings.ini` 中可选配置如下；这些设置在重启助手后应用，开发版与简易版分别使用自身目录：

```ini
[agent]
telemetry_v2=1
hardware_enabled=1
hardware_interval_ms=5000
hardware_cache=D:\local\hardware.json
hardware_lhm_directory=C:\Program Files (x86)\FanControl
hardware_gpu_uuid=
```

省略 `hardware_cache` 时使用自身数据目录的 `hardware.json`。不要让两个桥接写同一缓存；同一路径有互斥锁。手工命令用于独立诊断；正常使用助手自动管理的桥接。`hardware_enabled=0` 会同时关闭自动启动与缓存读取。

`telemetry_v2=0` 可向旧服务器只发原版 Heartbeat。新 Go 服务器同时接受旧版及可选 `v2`。简易版默认服务地址和令牌为空，不会继承开发版令牌；可在简易版自身 `settings.ini` 中显式设置原有 `server` 与 `token`，随后启动/切换模式保存仍会保留。界面不增加高级配置项。令牌仅通过 Authorization Header 发送，禁止放 URL、日志或分享截图。

## 真实数据来源与安全边界

GPU 通过 NVIDIA 随驱动安装的 System32 `nvml.dll` 动态调用 `nvmlDeviceGetPowerUsage`，将毫瓦换算为瓦。每次采样不启动 PowerShell 或 nvidia-smi；DLL 只从系统目录加载。显卡身份使用 NVML UUID。GPU 功耗不是整机功耗，也不能证明游戏处于局内。[NVIDIA 官方接口](https://docs.nvidia.com/deploy/nvml-api/latest/api/group__nvmlDeviceQueries.html)

CPU 复用现有 FanControl 的 LibreHardwareMonitor 库和已运行 PawnIO 驱动，仅启用 CPU，关闭主板、GPU、磁盘、内存、控制器、网络、PSU、PowerMonitor、Battery。当前已验证适配路径是 AMD Family17（包括本机 Ryzen 5900X）的 Core (Tctl/Tdie) 与 Package 传感器；其他 CPU 返回 `cpu_model_not_validated`，不能把主板温度、单核功耗冒充 Package 数值。[LHM Computer 接口](https://github.com/LibreHardwareMonitor/LibreHardwareMonitor/blob/master/LibreHardwareMonitorLib/Hardware/Computer.cs)、[AMD Package 传感器](https://github.com/LibreHardwareMonitor/LibreHardwareMonitor/blob/master/LibreHardwareMonitorLib/Hardware/Cpu/Amd17Cpu.cs)

FanControl 的插件 API 是传感器注入入口，不保证能导出内置传感器；运行 FanControl 不等于运行 LHM Web Server。[FanControl 官方插件文档](https://github.com/Rem0o/FanControl.Releases/wiki/Plugins)

当前 CPU 路径只接受已审核的 LHM 0.9.6.0 .NET Framework 构建：

```text
SHA256 85C97A4069BA38422B022DBCD4D4797D17FEB0BCBC670B555AC5E65B1AD81B63
```

版本号不足以证明安全。已核对该 DLL 的 Computer.Open/Close 和 PawnIo 初始化 IL：Computer 打开传感器分组；PawnIo 只读注册表、打开已存在设备及调用 DeviceIoControl，没有服务安装、启动或 Ring0 回退。桥接只查询现有 PawnIO 服务状态，并以 OPEN_EXISTING 检查设备权限；无安装、启动服务、重启 FanControl 或自动提权代码。[PawnIo 官方源码](https://github.com/LibreHardwareMonitor/LibreHardwareMonitor/blob/master/LibreHardwareMonitorLib/PawnIo/PawnIo.cs)

未知或升级后的 DLL 返回 `lhm_build_not_audited`。扩展允许列表必须由开发者核对对应源码和实际二进制 CPU 调用路径、依赖及驱动行为，重新做无驱动变更测试后修改源码；不提供跳过校验参数。该限制不会影响独立 NVML GPU 采集。

## 缓存、故障与协议

桥接每次采样有两条固定后台工作线程，每条最多一个在途操作，整体最多等待 1500 ms；超时不会产生无上限新线程。卡住的提供者返回 `cpu_sample_timeout` / `gpu_sample_timeout`，另一来源继续工作。崩溃时缓存自然过期；关闭助手可终止自己拥有的桥接。初始化错误目前需要修正依赖/权限后重启桥接，不自动装软件或请求管理员权限。

缓存先写同卷临时文件，再原子替换。Agent 独立线程每秒读有限缓存，限制 16 KiB，校验 JSON、日历 UTC 时间、数值范围和单调时间；网络线程只复制内存快照。未来时钟、损坏日期、过大或截断缓存转为本地合法失败尝试，不能让观察状态 Heartbeat 被拒收。正常过期值清为 null；原采样时刻不会因重复读取而改变。

`hardware` 包含 UTC `sampled_at`、十进制字符串 `sampled_tick_ms` / `sequence`、`interval_ms` / `stale_after_ms`，以及：

- `cpu_package_temperature`：`unit=celsius`。
- `cpu_package_power`、`gpu_power`：`unit=watts`。
- 每项独立 `value`、`unit`、`source`、`sensor`、`reason`。可用值有真实来源和标识；缺值为 null 且 reason 非空。

失败尝试也有时间戳，但所有失败项保持 null，时间戳不代表成功读数。典型原因包括 `cpu_access_denied`、`pawnio_not_running`、`pawnio_module_unavailable`、`sensor_not_found`、`gpu_selection_required`、`nvml_power_<错误码>`、`sample_stale`、`sample_future`、`bridge_cache_invalid`。LHM 无效 kernel 读取可能产生零填充；当前 CPU 路径额外检查 PawnIO 模块已加载，对未经证实的零 Package 读数返回 null/`cpu_zero_unverified`。

状态改变最低 2 秒合并上报；常规 Heartbeat 30 秒。IN_MATCH 期间只有真实新局内确认、OUT_OF_MATCH 期间只有真实新非局内视觉证据、OFFLINE 期间只有成功扫描确认不存在的新进程事实，才约每对应 TTL 的 2/3（默认 10 秒）续报；所有硬件读数均为 null 时也维持这三种状态的证据时效。有可用新硬件样本时约每硬件有效期的 2/3 续报，避免 30 秒心跳造成半周期空值。不会每 5 秒采样就立即发送。旧证据、重复硬件序号不能刷新远程有效期。服务端 GET/SSE 只返回缓存与派生快照，不触发采集。

v2 保留 Automation State，独立传输 P1 MatchSnapshot；FILETIME、revision、sequence、单调毫秒全部使用十进制字符串，避免 JavaScript 精度损失。线程启动前建立合法 STOPPED 快照。局内判定不依赖硬件读数。

## 已验证与待验证

自动化覆盖实际 C++ v2 fixture → Go POST/GET/SSE、旧协议兼容、三种已核验状态的新证据 2/10/30 秒调度、64 位精度、null 原因、损坏/未来/过大缓存、工作线程超时与迟到结果、简易版显式远程配置 load/UI-save/reload。

本机实际 NVML 已读取约 40 W 的 GPU 功耗；CPU 设备访问被当前 token 拒绝，真实返回 `cpu_access_denied`，未为了取得数字主动提权。CPU 三方库与既有 FanControl 并行的真实读数、管理员用户会话下长期采集、游戏挂机时的稳定性均待实机验证。

短时间桥接进程 CPU/内存测量在 [hardware-performance.json](validation/hardware-performance.json)。该数据是实际进程增量，不是受控整机功耗对照；CPU Package 实际采集开销因权限不足尚未测得。没有可信整机功耗计，整机瓦数保留 null。进程 CPU 时间未观察到增长时仅表示短窗口内低于 Windows 计数粒度，不能写成零成本。长时间、CPU 传感器可用条件下的内存趋势和功耗仍待测。
