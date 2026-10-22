> ⚠️ **仓库上传状态 / Upload status (2026-10-22):** 当前仓库先公开研究文档及校验摘要。实验版的全部源码、启动脚本与 Windows x64 编译文件尚未完整提交到 GitHub；README 中提到的 `src/` 与 Releases 资源目前可能尚不存在。请勿将当前仓库视为可直接构建或可下载的完整发行版。

# LegacyEdgeSync

> **实验性兼容性研究 / Experimental compatibility research**  
> 在 Windows 7 SP1 x64 上恢复 Microsoft Edge 109.0.1518.140 的 Microsoft 账户同步。本项目与微软无关，不提供浏览器安全更新。

**状态：v0.1-test3 — 已在一台 Windows 7 SP1 x64 虚拟机中验证核心功能；非稳定版。**

## 项目背景

Microsoft Edge 109 目前连接微软同步服务时，可能遇到以下问题：

- 默认同步发现端点 `/sync/v2/feeds/me/settings/homeCloud` 返回 HTTP 404。
- 在指定可用同步地址后，服务端可能返回 `UPGRADE_CLIENT`：`Client version not supported. Please upgrade to version 133 or later.`

本研究使用 `--sync-url` 参数绕过旧发现端点，并尝试在进程内对 `X-AFS-ClientInfo` 请求头的 `appVer` 进行**临时、等长替换**。仅针对经过验证的具体构建版本；不会修改磁盘上的微软 Edge 安装文件。

**版本号覆盖不等于 Edge 133，也不包含 Edge 133 的安全补丁或所有同步协议特性。**

## 已验证的环境与结果（2026-10-22）

| 测试 | 结果 |
| --- | --- |
| Windows 7 SP1 64 位 + Edge 109.0.1518.140 x64 | 已验证 |
| 不运行 Fiddler / x64dbg，独立启动器 + DLL Hook | 已验证 |
| Hook 状态 `hits=3, patched=3, skipped=0` | 已观察到 |
| `Transport State: Active`、`Disable Reasons: None` | 已观察到 |
| 收藏夹同步（此前包括双向验证） | 已验证 |
| 保留登录态、重启 Windows 7 后再次启动修复器 | 已验证一次 |
| 连续多次重启、长时间稳定性、Windows 8.1 和其他构建号 | **未验证** |
| 密码和支付信息同步 | **不推荐测试** |

早期 `test2` 在重启后曾出现 `INSTALLED` 但 `hits=0, patched=0` 与 `UPGRADE_CLIENT`。 `test3` 删除了启动后的固定 2.5 秒延迟，并在一次重启复测中恢复同步。**尚不能确认完全消除了竞态。**

## 使用方法（只建议测试虚拟机）

1. 准备 Windows 7 SP1 x64 和 **Edge 109.0.1518.140 x64**，建立快照。使用专用测试账户，关闭密码/支付信息同步。
2. 退出所有 Edge 进程；关闭 Fiddler 和 x64dbg。
3. 从 Releases 获取 `LegacyEdgeSync-v0.1-test3-Win7-x64.zip`，完整解压，保留 `Edge109SyncFix.exe` 和 `Edge109SyncHook.dll` 在同一目录。
4. 运行 `01-Launch-Test.cmd`。工具会使用 `%TEMP%\Edge109SyncFix-Test` 独立用户配置目录。
5. 运行 `02-Show-Status.cmd`，查看 `INSTALLED`、`pid`、`hits`、`patched`、`skipped`。
6. 登录测试账户，打开 `edge://sync-internals/`，检查 `Active`、`Server Connection: OK`、`Last Synced` 与实际收藏夹同步。
7. 在 Win7 与另一台受支持设备上分别新建、修改专用测试收藏夹，验证双向同步；重启系统再测试。

`INSTALLED` 仅代表 Hook 报告已安装，**不代表服务器已接受或同步一定成功**。若 `hits=0`，可能错过了首个同步请求，或尚未执行目标代码。

## 原理概要

- `--sync-url=https://edge.microsoft.com/sync/v1/feeds/me/syncEntities` 指向实测可用同步端点。
- 单独的 `X-AFS-ClientInfo` 请求头携带 `appVer=109.0.1518.140`；修改普通 User-Agent 无效。
- 只针对已验证版本，检查 `msedge.dll` 的 PE 架构、版本路径及 RVA `0x8AF3D95` 处机器码，再对内存中的版本字段做等长覆盖为 `133.0.3065.120`。
- 启动器和 Hook 为原生 x64 Windows 程序；不安装 HTTPS 解密根证书，不更改磁盘上的 Edge DLL。
- 状态日志仅主动记录修复计数、PID 和安装结果，不主动写入同步内容或令牌。

研究经过、复现与限制分别见 [RESEARCH](docs/RESEARCH.md)、[TESTING](docs/TESTING.md)、[RELEASE_NOTES](docs/RELEASE_NOTES.md)。

## 安全与限制

- **仅供研究，不适合日常浏览或生产环境。** Edge 109 已停止安全更新。
- 程序使用 `CreateRemoteThread` 加载 DLL，在运行中进程打机器码跳转；存在多线程竞态、兼容性与崩溃风险。
- 当前仅验证指定的 Edge x64 构建。其他浏览器版本、Windows 8.1、32 位系统均未验证。
- **不要同步密码、支付信息**，不要使用主力账户；绝不要通过禁用安全防护来强行运行。
- 不要上传账号邮箱、Token、浏览器用户目录、原始 NetLog 或内存转储到 Issues。
- 关闭测试 Edge 进程即可清除进程内修改；测试配置留在 `%TEMP%\Edge109SyncFix-Test`，自行备份和清理。

## 许可证与商标

**暂未设置许可证。** 代码可供阅读，但公开代码不代表自动授予再分发或商业使用许可。Microsoft Edge 为微软商标，本项目非微软官方项目。

---

**English:** LegacyEdgeSync is an experimental, build-specific Windows 7 x64 proof of concept for restoring Microsoft Edge 109 cloud sync. The tested launcher injects a runtime hook that rewrites a sync-specific client version field. One clean Windows 7 VM and reboot test passed. It is **not** a security update, a general Edge 133 compatibility layer, or a production-ready tool. Do not use it for passwords or sensitive data.
