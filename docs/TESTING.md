# 测试指南 / 复现模板

**前置条件**：已做 VMware 快照；Windows 7 SP1 x64；Edge 109.0.1518.140 x64；测试账户；Fiddler 和 x64dbg 均关闭。切勿进行密码/支付信息同步。

| ID | 测试 | 通过标准 |
| --- | --- | --- |
| T01 | 全新快照重新安装 Edge，使用测试启动器登录 | `INSTALLED`，同步 `Active`，实际有收藏夹下载 |
| T02 | 关闭所有 Edge 进程后重复启动 | 无崩溃、无 `UPGRADE_CLIENT`，Hook 计数正常 |
| T03 | 重启 Windows 7，保留此前登录态，用启动器启动 | 自动同步恢复，不需要重新登录 |
| T04 | 连续重启 3 次，分别启动修复器 | 每轮无手工修复，状态均正确 |
| T05 | Win7 新建专用测试收藏夹，Win11 接收 | 另一端收到新增条目 |
| T06 | Win11 重命名/移动测试收藏夹，Win7 接收 | 变更正确传达 |
| T07 | Win7/Win11 删除专用测试收藏夹 | 双端一致，无误删其他收藏夹 |
| T08 | 持续使用 30–60 分钟 | 无异常退出、同步仍可用 |
| T09 | 停用启动器，在独立环境用同样 sync-url 作对照 | 记录服务器错误，避免把登录缓存误当补丁成功 |
| T10 | 启动器重复运行、有 Edge 残留进程 | 明确记录行为；目前尚未保证正确处理 |

**关键字段**：`INSTALLED`/`FAILED`、`pid`、`hits`、`patched`、`skipped`、`Transport State`、`Disable Reasons`、`Server Connection`、`Last Synced`、`Successful Commits`。

**发布诊断资料前脱敏**：不要附带原始 `edge://sync-internals/` 全页截图、账号邮箱、令牌、浏览器用户数据目录、`NetLog`、内存转储、保存的凭据；最好仅报告字段值及测试结论。

## 已记录的 test3 结果

- Win7 x64 + Edge 109.0.1518.140，截图中状态：`INSTALLED: hook active`，`pid=2464`，`hits=3`，`patched=3`，`skipped=0`。
- 同屏 `Transport State: Active`，`Disable Reasons: None`，包含 `SYNCER_OK` 提交响应。
- 用户随后确认重启 Windows 7 后仍可同步。单次重启确认，并非三次或长期压力测试。
