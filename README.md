# LegacyEdgeSync

**旧版 Microsoft Edge 云同步兼容性修复研究**  
*Experimental cloud sync compatibility patch for legacy Microsoft Edge.*

[![Release](https://img.shields.io/github/v/release/yuyu107/LegacyEdgeSync?include_prereleases&label=Pre-release)](https://github.com/yuyu107/LegacyEdgeSync/releases)
[![Platform](https://img.shields.io/badge/Verified-Windows%207%20SP1%20x64-blue)](#兼容性与测试结果)
[![Status](https://img.shields.io/badge/Status-Experimental-orange)](#注意事项与已知限制)

> [!WARNING]
> **本项目目前是实验性研究原型，不是稳定版。** 当前仅在 Windows 7 SP1 x64 + Microsoft Edge **109.0.1518.140 x64** 上完成验证。Edge 109 已停止常规安全更新；本工具不会提供浏览器安全修复。建议只在有快照的虚拟机中使用，且不要开启密码、支付信息等敏感数据同步。

## 为什么需要修复云同步？

Edge 109 可能仍能登录微软账户，却因为**旧同步发现端点返回 HTTP 404**，以及之后的 `UPGRADE_CLIENT` 客户端版本限制而无法同步收藏夹。

完整背景、故障复现和修复边界已独立整理在 **[为什么需要修复旧版 Edge 的云同步？](docs/WHY-SYNC-FIX.md)**，这里不展开技术过程。

## 项目简介

由于 Microsoft Edge 109 使用的旧同步发现接口失效，以及微软同步服务器的客户端版本要求，Windows 7 上的 Edge 109 可能无法正常同步收藏夹等数据。

**LegacyEdgeSync** 尝试通过独立启动器及进程内运行时修复模块，让经过验证的旧版 Edge 重新使用其原生云同步功能。

- **无需 Fiddler 或 x64dbg**：不必使用 HTTPS 解密代理、手动修改内存或保持调试器附加。
- **运行时修复**：启动时指定同步服务器地址，并对特定同步请求头的客户端版本字段做条件替换。
- **不修改原版 Edge 文件**：不会永久修改磁盘上的 `msedge.dll`。
- **独立测试配置文件**：避免直接修改默认 Edge 用户配置；提供 Hook 状态与命中计数供诊断。

> 这里的版本字段替换只是解决当前观测到的服务端版本门槛，**不等于将 Edge 109 升级为 Edge 133**，也不保证所有同步数据类型兼容。

## 下载

### [**下载 v0.1-test3（实验性预发布版本）**](https://github.com/yuyu107/LegacyEdgeSync/releases/tag/v0.1-test3)

**当前 Release 附件：** [`Edge109-SyncFix-v0.1-test3-Win7-x64.zip`](https://github.com/yuyu107/LegacyEdgeSync/releases/download/v0.1-test3/Edge109-SyncFix-v0.1-test3-Win7-x64.zip)

GitHub Releases：[查看全部版本](https://github.com/yuyu107/LegacyEdgeSync/releases)

> 压缩包内仍使用早期开发名称 `Edge109SyncFix.exe` 和 `Edge109SyncHook.dll`，属于正常情况；项目对外名称已统一为 **LegacyEdgeSync**。

## 兼容性与测试结果

| 项目 | 当前状态 |
| --- | --- |
| Windows 7 SP1 64 位 | ✅ 已验证 |
| Microsoft Edge 109.0.1518.140 x64 | ✅ 已验证 |
| 无 Fiddler / x64dbg 的独立修复 | ✅ 已验证 |
| 云同步连接恢复 | ✅ 已验证 |
| 收藏夹双向同步（Win7 与 Win11） | ✅ 已验证 |
| 重启 Windows 7 后，保留登录状态继续同步 | ✅ 已验证一次 |
| 连续多次重启、长期稳定性 | 🧪 待进一步测试 |
| Windows 8.1 / 32 位系统 | ❔ 尚未验证 |
| 其他 Edge 版本 | ❔ 尚未验证 |
| 密码及支付信息同步 | ⚠️ 不建议测试 |

在已验证的环境中，`edge://sync-internals/` 曾显示：

```text
Transport State: Active
Disable Reasons: None
Server Connection: OK
```

修复模块也曾记录：

```text
INSTALLED: hook active, experimental build
hits=3
patched=3
skipped=0
```

**注意：** `INSTALLED` 只表示 Hook 已报告安装成功。必须结合 `hits`、`patched`、实际同步状态和跨设备数据变化判断修复是否真正生效；`hits=0` 可能表示修复尚未命中或错过了首个同步请求。

## 使用方法

1. 准备 **Windows 7 SP1 x64**，并安装 **Microsoft Edge 109.0.1518.140 x64**。建议先创建虚拟机快照。
2. 从 [v0.1-test3 Release](https://github.com/yuyu107/LegacyEdgeSync/releases/tag/v0.1-test3) 下载压缩包，**完整解压**到一个文件夹。
3. 完全退出 Edge，并确认没有残留的 `msedge.exe` 进程。
4. 保持 `Edge109SyncFix.exe` 和 `Edge109SyncHook.dll` 位于同一目录。
5. 运行 `01-Launch-Test.cmd`，使用实验性启动器启动 Edge。
6. 运行 `02-Show-Status.cmd`，检查 `INSTALLED`、`pid`、`hits`、`patched` 与 `skipped`。
7. 在启动的 Edge 测试配置文件中登录**专用测试微软账户**，打开 `edge://sync-internals/`，查看同步状态。
8. 在 Win7 和另一台受支持的设备之间新增或修改**专用测试收藏夹**，确认更改能双向到达；随后重启 Win7 再测试一次。

启动器使用 `%TEMP%\Edge109SyncFix-Test` 独立用户数据目录。测试结束后可先备份所需内容，关闭所有测试 Edge 进程，再手动清理该目录。**直接双击普通 Edge 快捷方式不保证应用修复。**

## 技术原理

当前研究确认了两项影响 Edge 109 同步的因素：

1. 旧的 `/sync/v2/feeds/me/settings/homeCloud` 同步发现接口在测试时返回 HTTP 404；通过 `--sync-url` 指向实测可用的同步地址，可绕过发现环节。
2. 服务端可能进一步返回 `UPGRADE_CLIENT`，要求更高客户端版本。版本来自 `X-AFS-ClientInfo` 中的 `appVer`，修改普通 User-Agent 无法解决。

实验性 Hook 在特定 `msedge.dll` 构建中验证目标机器码及内存字符串后，将 `appVer=109.0.1518.140` **等长**替换为 `appVer=133.0.3065.120`。这属于临时的进程内修改，不涉及证书注入或磁盘上 Edge DLL 的替换。

更多技术背景见 [研究记录](docs/RESEARCH.md)。

## 多语言版本开发进度

**v0.2-test3 多语言测试构建正在整理到仓库，尚未正式作为 GitHub Release 发布。** 它提供简体中文、繁體中文及 English 自动检测，并保留独立测试和原有 Edge 配置两种模式。当前公开的正式预发布附件仍是 **v0.1-test3**，下载及使用说明请以 Releases 为准。

- [多语言功能与测试指南](docs/MULTILINGUAL.md)
- [v0.2-test3 更新记录](docs/CHANGELOG-v0.2-test3.md)

> 注意：多语言构建的启动器完整源码与可执行文件仍在同步处理中，请勿将仓库中的旧版 `src/launcher.c` 与 v0.2-test3 构建混用。

## v0.1-test3 更新摘要

- 移除 v0.1-test2 启动后固定等待 2.5 秒的机制，改为尽早安装 Hook，改善重启后自动同步的成功率。
- 在状态信息中增加 PID，帮助确认修复模块所在的 Edge 进程。
- 保留目标版本、机器码及待修改字符串检查。
- 在 Windows 7 SP1 x64 测试环境中验证了同步恢复，以及一次重启后的再次同步。

[查看完整 Release 说明](https://github.com/yuyu107/LegacyEdgeSync/releases/tag/v0.1-test3)

## 注意事项与已知限制

- **仅供研究及测试**，尚未进行充分的稳定性与安全性审计。运行时注入和机器码 Hook 存在多线程竞态、启动时序及崩溃风险。
- 当前只验证了一个特定的 Edge x64 构建，其他 Edge 版本或其他操作系统可能不适用。
- **不要启用密码、支付信息同步**，也不要在主力微软账户中测试。请先备份重要收藏夹。
- Edge 109 已停止安全更新；本项目不能代替更新浏览器及操作系统。
- 不应以关闭防病毒防护、降低系统安全设置等方式强行运行。
- 报告问题时，**不要公开邮箱、身份验证令牌、未经脱敏的 NetLog、浏览器用户数据或内存转储**。
- Microsoft 服务端行为可能变化，即使同版本 Edge 之后也可能再次无法同步。

## 项目资料与反馈

- [源码（src/）](src/)
- [研究记录（RESEARCH.md）](docs/RESEARCH.md)
- [测试指南（TESTING.md）](docs/TESTING.md)
- [版本说明（RELEASE_NOTES.md）](docs/RELEASE_NOTES.md)
- [反馈问题（Issues）](https://github.com/yuyu107/LegacyEdgeSync/issues)
- [实验版下载（Releases）](https://github.com/yuyu107/LegacyEdgeSync/releases)

目前的公开源码与 Release 已上传。若你遇到问题，请在 Issue 中说明 Windows 版本、Edge **完整版本号**、补丁状态及复现步骤，并对诊断信息进行脱敏。

## 许可证与声明

目前仓库尚未添加开源许可证。代码公开仅供查看和研究，**不代表自动获得再分发或商业使用授权**。

Microsoft Edge 是 Microsoft 的商标。本项目是独立的兼容性研究项目，**与 Microsoft 无关，也未经 Microsoft 认可或支持**。

---

### English summary

LegacyEdgeSync is an **experimental, build-specific proof of concept** for restoring Microsoft Edge cloud sync on Windows 7 SP1 x64. Testing has confirmed successful sync with Edge **109.0.1518.140 x64**, including bookmark synchronization and one successful test after a Windows reboot.

The launcher uses a runtime hook to rewrite a specific client-version field in sync requests. It **does not** update Edge's security components, guarantee compatibility with other Edge versions, or support production use.

**[Download experimental v0.1-test3](https://github.com/yuyu107/LegacyEdgeSync/releases/tag/v0.1-test3)** · **[Report an issue](https://github.com/yuyu107/LegacyEdgeSync/issues)**
