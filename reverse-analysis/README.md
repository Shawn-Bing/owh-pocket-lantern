# OverwatchHelper / ow.exe 逆向分析包

**核心结论：通过已打开且已登录的战网窗口取色点击来拉起游戏；没有发现账号密码登录或返回指定自定义房间的完整流程。**

本包是静态分析交付，不是已经实现的新挂机助手。没有执行原始或解包后的 Windows 程序。

- [完整逆向报告](docs/ow-reverse-analysis.zh-CN.md)：重启调用链、阈值、窗口类、模式边界、额外系统行为及证据地址。
- [低占用与稳定看板设计](docs/helper-design.zh-CN.md)：新助手状态机、资源控制、目标房间恢复与 Windows 验收场景。
- [重启伪代码](docs/restart-pseudocode.txt)：人工恢复的核心控制流，不可直接执行。
- `analysis/ow/evidence/`：完整/关键反汇编、字符串、导入与交叉引用。
- `analysis/ow/*.bin`：仅供分析的节名修复副本和解包副本，仍为 PE 文件。
- `analysis/ow/validation.json`：证据核验结果；`manifest.json`：交付文件校验值。

原样本 SHA-256：`fdba49702efb71deacfdb47459ec713be69fe004bf8e04d5bd4150a8f9ed8296`。

## 静态复现

需要 Python、`requirements-analysis.txt` 中的两个库，以及自行取得的官方 [UPX](https://github.com/upx/upx)。本次使用 UPX 5.2.1；没有在交付中捆绑第三方工具二进制。

在此目录下执行以下分析命令；`--upx` 改成实际 UPX 工具路径。命令只解析/解压样本，不启动样本。

```sh
python3 -m venv .venv
.venv/bin/python -m pip install -r requirements-analysis.txt
.venv/bin/python tools/unpack_ow.py /Users/shawn/Downloads/ow.exe \
  --upx /absolute/path/to/upx --out work/reproduced
.venv/bin/python tools/analyze_pe.py work/reproduced/ow.unpacked.bin work/reproduced/evidence
```

Windows 上可使用 `.venv\Scripts\python.exe`。解包脚本要求原样本校验值完全匹配；输出二进制已经存在时会拒绝覆盖。只修复副本的 `XXX0/XXX1` 节名；不修改原文件。

反汇编地址按首选基址 `0x140000000` 显示。`metadata.json` 的函数范围来自 PE 异常展开表，未必覆盖叶函数；字符串是候选结果，不能用孤立字符串代替控制流分析。

## 交付核验范围

已重新解包并比较两次产物，逐字节相同；核对了关键调用、坐标常量、120 秒/15 秒/9 秒阈值和模式分派；校验原文件 SHA-256 未变化。所有工具脚本通过 Python 语法编译检查。

未做 Windows 动态验证，未测量实际 CPU/内存，未验证当前战网 UI、登录状态或自定义房间回归成功率。设计文档中的性能参数是建议初值。
