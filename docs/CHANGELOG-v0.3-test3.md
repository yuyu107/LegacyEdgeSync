# v0.3-test3 (x86 launcher)

- 保留 v0.3-test2 已测试的三路径 Hook DLL，SHA-256 与原版一致。
- 新增 **简体中文、繁體中文、English** 自动语言选择和命令行手动覆盖。
- 新增 `--normal` 原有 Edge 配置模式，启动前必须键入 `YES`；不会强制结束现有 Edge 进程。
- 新增 `--backup`：关闭所有 Edge 进程后，调用 Win7 自带 Robocopy 复制原始 Edge 配置到桌面独立目录。
- 新增 `--menu`、`--guide`，改进 `--status` 对旧 PID/状态文件的检查。
- 新建独立测试配置目录 `LegacyEdgeSync-x86-TestProfile-test3`，避免与已成功的 test2 共用配置。
- 只修改启动器与文档，不更换 x86 Hook 算法；x64 直接沿用既有多语言构建。

**后续验证补充：** x86 v0.3-test2 Hook 的同步连接成功有实机截图证据；用户后来反馈 v0.3-test3 的**原有配置模式也成功同步**。但三语言各环境、备份边界条件和长期稳定性仍需完整回归。

**版本归档：** v0.3-test3 未单独发布 GitHub Release；其 x86 源码已同步至 [`src/x86/`](../src/x86/)。当前公开预发布是 [v0.3-test7](https://github.com/yuyu107/LegacyEdgeSync/releases/tag/v0.3-test7)，整包仍需回归。
