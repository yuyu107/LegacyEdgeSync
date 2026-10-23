# v0.3-test3 (x86 launcher)

- 保留 v0.3-test2 已测试的三路径 Hook DLL，SHA-256 与原版一致。
- 新增 **简体中文、繁體中文、English** 自动语言选择和命令行手动覆盖。
- 新增 `--normal` 原有 Edge 配置模式，启动前必须键入 `YES`；不会强制结束现有 Edge 进程。
- 新增 `--backup`：关闭所有 Edge 进程后，调用 Win7 自带 Robocopy 复制原始 Edge 配置到桌面独立目录。
- 新增 `--menu`、`--guide`，改进 `--status` 对旧 PID/状态文件的检查。
- 新建独立测试配置目录 `LegacyEdgeSync-x86-TestProfile-test3`，避免与已成功的 test2 共用配置。
- 只修改启动器与文档，不更换 x86 Hook 算法；x64 直接沿用既有多语言构建。

**验证状态：** x86 v0.3-test2 Hook 同步连接成功已获截图证据；v0.3-test3 新 UI、正常配置和备份模式尚需实机验证。不要提前称为稳定版。

**注意：** 该说明页记录本地实验构建，当前 `main` 分支的 `src/` 仍是 x64 源码；新 x86 源码须另行同步至 `src/x86/`。GitHub Releases 尚未发布 v0.3-test3。
