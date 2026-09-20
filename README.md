# OverwatchHelper

面向国服战网版的 Windows 状态监控与恢复助手，附远程运行看板。当前版本：**0.3.0 试用版**。

[本次改动说明](docs/CHANGELOG.zh-CN.md) 包含功能变化、验证结果与尚待实机确认的项目。

本地端使用 C++20 / Win32；看板使用 Go 标准库、HTTP 和 SSE，无第三方运行时或神经网络模型。新增基于本人流程截图的国服预设与“重启游戏”按钮，支持最稳轮换版导航；其他布局仍可手动校准。识别已通过历史截图回放，真实启动、授权后进房的整条流程仍需实机试用。

## 快速开始

如果今晚先定位“挂机途中为什么掉线”，可独立运行 `owh-recorder.exe`：记录游戏截图、相关进程、网络参考探测、网卡/连接表、原生日志和 Windows 事件，不发送按键或恢复游戏。[整夜记录步骤与数据边界](docs/RECORDER.zh-CN.md)。

下载并解压 Windows 试用包，先阅读 [Windows 试用指南](docs/QUICKSTART.zh-CN.md)。

1. 双击 `overwatch-helper.exe`，默认开启演练模式，不发送键鼠、不启动或关闭游戏。
2. 保留“国服预设”，填写战网和 OWNeacClient.exe 路径；点“重启游戏”可演练预览重启。操作步骤见试用指南开头。
3. 停止后取消演练，点“重启游戏”实际执行，或点“开始”只监控当前页面。继续使用原 PowerShell 点按脚本时，取消“房间内发送 Shift”。**F8 停止助手；它不会停止外部脚本。**
4. 如需远程看板，运行 `owh-dashboard.exe`，浏览器打开 `http://127.0.0.1:8765`。将服务端的 Agent 令牌配置到客户端，用查看令牌登录网页。

附带 `owh-demo.exe` 和 `start-demo.cmd`，可先用模拟大厅、房间和断线页面练习，不需要打开游戏。演示与正式客户端使用独立配置目录。

此前新增的“运行诊断”按钮和 `diagnose.cmd`，报告检查进程、窗口、启动器与校准，不包含令牌、机器名、服务器地址或搜索文字。F8 现在也能取消录制倒计时和校准覆盖层；停止后重新开始时，旧检测轮次不能继续发送输入。输入发送失败也计入重试上限。

## 已实现

- 通过窗口与进程身份查找国服游戏，读取 PID / 创建时间，避免根据过期 PID 结束其他进程。
- “重启游戏”共用自动恢复流程；在关闭游戏前检查路径和启动识别配置。游戏缺失时启动配置的战网程序或快捷方式，通过预设或校准按钮启动游戏，等待最多 120 秒。
- 内置国服“连接战网服务超时”多区域识别，连续出现 90 秒才恢复；普通蓝色加载画面不会触发。也支持自定义错误模板及窗口连续 30 秒未响应恢复。
- 内置“开始 → 更多模式 → 自定游戏 → 最稳轮换版 → 自动加入”；同时核对浏览页标题、目标模式文字和按钮状态，先选择目标卡片再加入。
- 战网重新联网弹窗遮挡游戏超过 5 秒时，尝试一次切回游戏，恢复画面检测。
- 游戏完全退出后核验配置路径、PID 与创建时间，再清理残留 `OWNeacClient.exe`。路径或权限无法核验则暂停，不结束其他安装目录的同名进程。
- 锁屏或安全桌面期间暂停输入和启动超时；每次 NeacProtect 授权仍需用户手动处理。
- 每小时最多 3 次恢复，带冷却；超时、识别歧义、操作无进展时显示明确状态。
- 自定义房间导航：页面特征 → 两次确认 → 操作 → 再识别。支持点击、Enter、Esc、输入文字、房间就绪与恢复页面。
- 连续确认房间参考后每秒点按左 Shift（按下 20 ms），也可选择前台持续按住。观战、结算、加载期间保留房间会话；已识别菜单、错误页、歧义、进程更换或停止清除会话。
- 原生本地面板：配置、状态、校准列表、匹配分数、恢复次数、日志入口。
- 远程看板：30 秒常规心跳、状态变化上报、90 秒离线判定、200 条事件历史、重连与本地持久化。

## 当前边界

导航点击只在前台执行。房间点按可在后台继续，但这是全局键盘事件，不是绑定游戏的后台输入；不受 CapsLock 控制，使用开始/F8 停止。画面检测支持可见、无遮挡的后台游戏；遮挡或最小化时暂停视觉掉线计时。若要检测蓝色掉线页，请让游戏持续可见。小窗口保持原宽高比；HDR 和滤镜仍需实测。

不自动填写账号密码或处理验证码。启动器登录过期时需要人工处理。国服预设针对已提供的约 16:9 游戏布局及最稳轮换版卡片位置；位置、排序或界面变化时会等待，不保证覆盖其他房间。可关闭预设并手动校准。E 图标只能确认托比昂对局画面，不能独立证明已进入指定房间或获得经验。

昨晚的网络中断、不活跃移出和战网遮挡证据见 [整夜记录分析](docs/OVERNIGHT-20260920.zh-CN.md)。

截图与恢复工作在后台线程，看板网络上报在独立线程。Windows 截图 API 的极端阻塞情形尚未做到独立进程隔离。不能据此保证无人值守恢复所有图形驱动故障。

没有修改防火墙、注入游戏/宿主进程、改写样本文件或要求管理员权限。本项目原生实现与 `reverse-analysis` 中的旧样本分开。

## 构建与测试

### Windows 一键构建

已安装 CMake、Go，以及 MSVC 或 MinGW-w64 时，在仓库根目录运行：

```powershell
.\scripts\build-windows.ps1
# 可选：Go 并发检测，需要 GCC
.\scripts\build-windows.ps1 -Race
# Go 不在 PATH 时
.\scripts\build-windows.ps1 -Go 'C:\path\to\go.exe'
```

脚本运行核心、Windows 平台及 Go 测试，将程序、启动脚本和指南放入 `dist`。首次构建会探测编译器；已存在构建缓存时复用其生成器。可显式传入 `-Generator 'Visual Studio 17 2022'`。打包 Windows 版本可运行 `python scripts/package.py --windows-only`。

### Windows / Visual Studio 2022

需要 CMake、Go，以及带 C++ 桌面工作负载的 MSVC。Go 的 race 测试另需兼容的 C 编译器：

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
cd server
go test ./...
New-Item -ItemType Directory -Force ../dist
go build -trimpath -ldflags="-s -w" -o ../dist/owh-dashboard.exe .
```

### macOS / Linux 交叉构建

使用 Zig 0.15.2 与 Go 1.22+：

```sh
python3 scripts/build.py --zig /absolute/path/to/zig
cmake -S . -B build/native
cmake --build build/native
ctest --test-dir build/native --output-on-failure
cd server && go test -race ./...
```

交叉构建会输出 Windows x64 客户端、演示窗口，以及 Windows / Linux x64 / macOS arm64 看板服务端。Windows 原生端无需 Python、Zig 或 Go 运行时。

## 项目结构

```text
agent/             Windows 面板、校准、采集、输入、恢复与演示窗口
core/              跨平台图像匹配、动作去抖、恢复预算与识别预设解析
profiles/          识别步骤数据文件（国服 16:9 预设）
tests/             核心行为测试
server/            Go 服务端与内嵌网页
scripts/           构建脚本
docs/              试用、架构与测试说明
reverse-analysis/  原始样本的静态分析与证据
```

配置及日志保存在 `%LOCALAPPDATA%\OverwatchHelper`；演示模式为 `%LOCALAPPDATA%\OverwatchHelperDemo`。服务端数据位于启动目录下的 `owh-server-data`。凭据、日志、构建产物和逆向样本二进制不进入 Git。

开发测试可使用 `overwatch-helper.exe --demo --data-dir "E:\test folder\demo"` 指定隔离配置。`--diagnose` 只写入该目录的 `diagnostics.txt`，不显示面板、不发送输入、不启动游戏、不上报网络。

更多信息：[架构与协议](docs/ARCHITECTURE.md)、[验证记录](docs/VALIDATION.md)。
