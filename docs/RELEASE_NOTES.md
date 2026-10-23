# v0.1-test3 — 历史实验版发布记录

> 本页**仅记录历史版本 v0.1-test3**，不是最新 Release。当前已发布实验版为 [v0.3-test7](https://github.com/yuyu107/LegacyEdgeSync/releases/tag/v0.3-test7)，版本对比见 [RELEASES.md](RELEASES.md)。

## 已实现

- 针对已验证 Edge 109.0.1518.140 x64 使用 `--sync-url` 指定同步服务。
- Win7 x64 原生启动器和进程内 DLL Hook；无需 Fiddler 代理或 x64dbg 附加。
- 启动后立即尝试注入，替代 test2 固定等待 2500ms 的方案。
- 安装时检查 `msedge.dll` 版本目录、PE、目标字节及请求头引用；请求头内容完全匹配才执行等长替换。
- 使用独立临时配置文件，输出 PID、命中次数、修改次数、跳过次数和状态。

## 用户已验证

- Windows 7 SP1 x64 上恢复 Edge 109 原生同步，收藏夹工作正常。
- `hits=3`、`patched=3`、`skipped=0`，`Transport State: Active`。
- 重启 Windows 7 后仍能恢复同步。

## 已知问题

- 尚无法严格保证在浏览器第一条同步请求发送前完成注入；可能仍有竞态。
- 只针对 109.0.1518.140 x64 的特定机器码，其他版本与 Win8.1 尚未验证。
- Hook 修改运行中进程机器码，存在潜在并发与崩溃问题；未经过安全审计。
- 不支持当作新版 Edge 使用；不推荐日常账户和密码同步。

**历史发布状态**：此版本已于 2026-10-22 作为 Pre-release 发布，实际附件为 [`Edge109-SyncFix-v0.1-test3-Win7-x64.zip`](https://github.com/yuyu107/LegacyEdgeSync/releases/download/v0.1-test3/Edge109-SyncFix-v0.1-test3-Win7-x64.zip)。
