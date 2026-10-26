# LegacyEdgeSync v1.0 — Windows 7 / 8.1 Edge 109 云同步恢复工具

> **视频演示公开版｜实验性发行。** 这是基于 `v0.3-test8` 自适应识别测试构建整理的 **v1.0** 版本。v1.0 是首次视频演示/公开版的发行编号，**不代表所有系统已通过完整回归，也不意味着 Edge 109 获得了安全更新**。

## 功能介绍

旧版 Edge 109 在 Windows 7 / 8.1 上虽然可能正常登录微软账户，但在项目测试中，同步服务发现接口曾返回 `404`；手动指定同步服务器地址后又会遇到 `UPGRADE_CLIENT`，要求新版客户端。

LegacyEdgeSync 通过启动参数和进程内运行时兼容 Hook，在特定 Edge 构建上尝试恢复微软账户云同步。浏览器磁盘上的 `msedge.dll` 不会被永久改写。

## v1.0 有哪些功能？

- **Windows 7 / 8.1、Edge x86 / x64 的统一启动入口**，根据 Edge 文件实际 PE 架构选择修复模块。
- 对目标 Edge **109.0.1518.140** 执行架构与版本检查，其他版本不保证适配。
- x64 同时支持 Win7 已验证的 `0x49`、Win8.1 已验证的 `0x45` 两条精确匹配路径。
- 继承 `v0.3-test8` 的 **有界自适应识别**：仅当找到唯一、完整且可安全修改的版本字段时，才尝试处理未知偏移；不确定时跳过。
- x86 保留此前成功的三路径 Hook，不修改其核心实现。
- 支持**简体中文、繁體中文、English**，以及独立测试配置、原有配置模式、备份、诊断与状态查看。
- 保留历史 x64 Hook，用于人工对照或回退。

## 下载及使用

**附件名称：** `LegacyEdgeSync-v1.0-Win7-Win81-Universal.zip`

1. 在有快照的 Windows 7 SP1 / Windows 8.1 虚拟机中安装 **Microsoft Edge 109.0.1518.140**（x86 或 x64）。
2. 将 ZIP **完整解压**，运行 `04-Check-Package.cmd` 检查组件，再运行 `03-Diagnose-Architecture.cmd` 检查 Edge 版本和实际位数。
3. 退出所有 `msedge.exe` 进程，运行 `00-LegacyEdgeSync-Menu.cmd`，优先选择**独立测试配置**。
4. 打开 `edge://sync-internals/`，核对 `Server Connection`、`Last Synced` 等字段，并以测试收藏夹验证跨设备实际同步。
5. 需要使用原有配置时，请先运行 `06-Backup-Original-Profile.cmd` 完成备份，再运行 `05-Launch-Normal.cmd`，按提示确认。云端操作可能影响其他设备，本地备份不能撤销云端改动。

## 兼容性记录

| 环境 | 此前实验成果 | v1.0 的验证状态 |
| --- | --- | --- |
| Win7 x86 + Edge 109 x86 | 三路径 Hook 恢复同步 | 待整包回归 |
| Win7 x64 + Edge 109 x64 | 定点 Hook 恢复同步 | 新自适应 Hook 待回归 |
| Win8.1 x64 + Edge 109 x64 | `0x45` 修复路径、`SYNCER_OK` | 新自适应 Hook 待回归 |
| Win7 x64 + Edge 109 x86 | 架构识别实验 | 完整同步待验证 |
| Win8.1 x86 + Edge 109 x86 | 尚未测试 | 暂缓 |

新的自适应字段匹配器通过了 **13 组离线逻辑测试**；这不等于其已在各种未知 Windows 环境下实测成功。**`INSTALLED` 或 `patched>0` 也不能单独替代实际的跨设备同步验证。**

## 为什么控制台里还有旧版本号？

这次为避免重新编译已知来源的组件，**v1.0 仅调整对外发行版本和文档，EXE/DLL 保持与 `v0.3-test8` 测试包字节一致**。因此部分状态/菜单可能显示 `v0.2-test3`、`v0.3-test3`、`v0.3-test7`、`v0.3-test8`。这些是**内部组件构建标识**，不代表错误安装。详见压缩包中的 `VERSION.txt`。

## 安全提示

- Edge 109 **已结束安全更新**。本项目不会给 Edge 添加新安全补丁，也不会把 Edge 109 真正升级为 Edge 133。
- 工具通过运行时 DLL 注入和内存 Hook 工作，可能受到系统和安全软件拦截，也可能出现崩溃或时序竞争。
- 此前 360 对部分启动器报告过 `HEUR/QVM20.1` 类检测，**未经厂商复核，不能认定为误报**。不建议关闭安全软件或绕过防护强行运行。
- 建议使用隔离虚拟机和测试账户，不同步密码、付款资料等敏感数据。
- 微软服务器策略可能变化，今天成功不保证长期持续有效。

## 文件校验

**ZIP SHA-256：**

```text
a4994f4315088057f32b79ae88b31eb7972713b43e0564db695eddda9dde303e  LegacyEdgeSync-v1.0-Win7-Win81-Universal.zip
```

压缩包内部主要 EXE/DLL 的 SHA-256 见 `SHA256SUMS.txt`。校验只验证字节一致，不构成安全认证。

项目：[yuyu107/LegacyEdgeSync](https://github.com/yuyu107/LegacyEdgeSync) · [反馈 Issues](https://github.com/yuyu107/LegacyEdgeSync/issues)

---

**推荐发布设置：** Tag `v1.0`，标题 `LegacyEdgeSync v1.0 — Windows 7/8.1 Edge 109 云同步恢复`。在缺少 v1.0 整包实机回归时，建议勾选 **Set as a pre-release**，并保留以上风险说明。
