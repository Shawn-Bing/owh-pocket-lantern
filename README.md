# OverwatchHelper

面向国服战网版的 Windows 状态监控与恢复助手，附远程运行看板。当前版本：**0.1.0 试用版**。

本地端使用 C++20 / Win32；看板使用 Go 标准库、HTTP 和 SSE，无第三方运行时或神经网络模型。首次试用需要用实际游戏页面完成校准。**这不是已通过国服实机验收的免配置成品。**

## 快速开始

下载并解压 Windows 试用包，先阅读 [Windows 试用指南](docs/QUICKSTART.zh-CN.md)。

1. 双击 `overwatch-helper.exe`，默认开启演练模式，不发送键鼠、不启动或关闭游戏。
2. 手动打开战网或游戏页面，通过“3 秒后录制”圈选独特页面特征，并标记按钮/输入框。
3. 先验证演练识别结果；停止后取消演练，再开始实际动作。**F8 全局停止；切换窗口会释放 Shift。**
4. 如需远程看板，运行 `owh-dashboard.exe`，浏览器打开 `http://127.0.0.1:8765`。将服务端的 Agent 令牌配置到客户端，用查看令牌登录网页。

附带 `owh-demo.exe` 和 `start-demo.cmd`，可先用模拟大厅、房间和断线页面练习，不需要打开游戏。演示与正式客户端使用独立配置目录。

## 已实现

- 通过窗口与进程身份查找国服游戏，读取 PID / 创建时间，避免根据过期 PID 结束其他进程。
- 游戏缺失时启动配置的战网程序或快捷方式；通过校准的战网按钮启动游戏，等待最多 120 秒。
- 窗口连续 30 秒未响应或已校准的登录/错误页面连续出现 90 秒时，尝试恢复。
- 每小时最多 3 次恢复，带冷却；超时、识别歧义、操作无进展时显示明确状态。
- 自定义房间导航：页面特征 → 两次确认 → 操作 → 再识别。支持点击、Enter、Esc、输入文字、房间就绪与恢复页面。
- 进入校准的目标房间后保持 Shift；停止、失焦和识别丢失时释放按键。
- 原生本地面板：配置、状态、校准列表、匹配分数、恢复次数、日志入口。
- 远程看板：30 秒常规心跳、状态变化上报、90 秒离线判定、200 条事件历史、重连与本地持久化。

## 当前边界

第一版使用**前台输入和可见窗口截图**。小尺寸窗口应保持校准时的宽高比；最小化到任务栏、锁屏、后台不可见、HDR 与滤镜的兼容性未验证。每次开始或恢复时会尝试一次前台切换；手动切走窗口后会显示 `WAITING_FOCUS`，不会反复抢回前台。

不自动填写账号密码或处理验证码。启动器登录过期时需要人工处理。正确的房间名、页面特征与点击位置需要你在本机校准；没有预置未经验证的国服坐标。

截图与恢复工作在后台线程，看板网络上报在独立线程。Windows 截图 API 的极端阻塞情形尚未做到独立进程隔离。不能据此保证无人值守恢复所有图形驱动故障。

没有修改防火墙、注入游戏/宿主进程、改写样本文件或要求管理员权限。本项目原生实现与 `reverse-analysis` 中的旧样本分开。

## 构建与测试

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
core/              跨平台图像匹配、动作去抖与恢复预算
tests/             核心行为测试
server/            Go 服务端与内嵌网页
scripts/           构建脚本
docs/              试用、架构与测试说明
reverse-analysis/  原始样本的静态分析与证据
```

配置及日志保存在 `%LOCALAPPDATA%\OverwatchHelper`；演示模式为 `%LOCALAPPDATA%\OverwatchHelperDemo`。服务端数据位于启动目录下的 `owh-server-data`。凭据、日志、构建产物和逆向样本二进制不进入 Git。

更多信息：[架构与协议](docs/ARCHITECTURE.md)、[验证记录](docs/VALIDATION.md)。
