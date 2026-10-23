# LegacyEdgeSync

**Windows 7 / Windows 8.1 上 Microsoft Edge 109 云同步兼容性研究工具**  
*Experimental Microsoft Edge 109 sync compatibility fix for legacy Windows.*

[![Pre-release](https://img.shields.io/github/v/release/yuyu107/LegacyEdgeSync?include_prereleases&label=pre-release)](https://github.com/yuyu107/LegacyEdgeSync/releases/tag/v0.3-test7)
[![Windows](https://img.shields.io/badge/Windows-7%20%2F%208.1-blue)](#兼容性与验证情况)
[![Architectures](https://img.shields.io/badge/Edge-x86%20%2F%20x64-blue)](#兼容性与验证情况)
[![Experimental](https://img.shields.io/badge/status-experimental-orange)](#注意事项和已知问题)

> [!WARNING]
> **本项目是实验性兼容性研究，未完成安全审计，也不能替代受安全支持的浏览器。** 目标仅为 **Microsoft Edge 109.0.1518.140** 的特定 x86/x64 构建。Edge 109 已停止安全更新；请在隔离虚拟机中使用测试账户，勿同步真实密码或支付信息。360 曾对启动器报告启发式风险，尚无厂商复核结论。

## 下载当前版本

<https://github.com/yuyu107/LegacyEdgeSync/releases/tag/v0.3-test7>

**最新已发布版本：`v0.3-test7`（Pre-release）**，支持在统一界面下选择 Edge x86 / x64 修复器，并将已在 Win8.1 x64 实验成功的双偏移 Hook 整合到 x64 DLL。

| 下载 | 说明 |
| --- | --- |
| **[v0.3-test7 完整测试包](https://github.com/yuyu107/LegacyEdgeSync/releases/download/v0.3-test7/LegacyEdgeSync-v0.3-test7-Win7-Win81-Universal-TEST.zip)** | 含统一入口、x86/x64 启动器、Hook DLL、诊断脚本和历史回退文件 |
| [全部 Releases](https://github.com/yuyu107/LegacyEdgeSync/releases) | 查看包括 `v0.1-test3` 在内的历史预发布版本 |
| [v0.3-test7 更新记录](docs/CHANGELOG-v0.3-test7.md) | 本版功能和复用的 Hook 来源 |
| [v0.3-test7 回归清单](docs/TESTING-v0.3-test7.md) | 明确哪些测试已完成、哪些仍待验证 |

**SHA-256（v0.3-test7 ZIP 候选文件）：**

```text
44a672420b8e36b5c9f4bd8a1c3dad35e0175645e2902a4dc193376e740b7238  LegacyEdgeSync-v0.3-test7-Win7-Win81-Universal-TEST.zip
```

[其他程序文件的哈希](docs/SHA256SUMS-v0.3-test7.txt)。哈希只能用于核对文件是否一致，不代表安全认证。

## 项目解决什么问题？

Edge 109 虽可在旧版 Windows 上启动和登录 Microsoft 账户，但在项目的指定测试环境中，同步服务发现请求返回过 `404`；直接指定同步地址后，服务器又返回 `UPGRADE_CLIENT`，要求客户端版本 133 或更高。

LegacyEdgeSync 在启动浏览器时使用 `--sync-url` 指定测试可用的服务地址，并通过**针对指定构建的进程内 Hook**，在符合严格条件时修改 `X-AFS-ClientInfo` 请求头里的 `appVer` 字段。该方式**不会升级浏览器或为它提供安全补丁**，也不会永久替换磁盘上的 `msedge.dll`。

详细背景：[同步故障与修复原理](docs/WHY-SYNC-FIX.md) · [研究记录](docs/RESEARCH.md) · [为什么暂不采用扩展内核路线](docs/WHY-NOT-EXTENDED-KERNEL.md)。

## 使用方法（v0.3-test7）

1. 使用 **Windows 7 SP1 / Windows 8.1** 的测试环境，安装 **Edge 109.0.1518.140** 对应 x86 或 x64 版本；建立虚拟机快照。
2. 从 Release 下载并**完整解压 ZIP**，不要只复制根目录的 `LegacyEdgeSync.exe`。旧文件夹名 `Win7-x86/`、`Win7-x64/` 是按 **Edge 架构**区分，而非限制在 Win7 运行。
3. 运行 `04-Check-Package.cmd` 检查双架构组件（**不需要已安装 Edge，也不会执行注入**）。
4. 运行 `03-Diagnose-Architecture.cmd`，确认程序识别到的 Edge 文件版本、PE 架构及匹配的子程序。
5. 完全关闭 Edge 和后台 `msedge.exe` 进程，然后运行 **`00-LegacyEdgeSync-Menu.cmd`**。先选择**独立测试配置**模式，使用测试 Microsoft 账户。
6. 打开 `edge://sync-internals/`，同时观察 `02-Show-Status.cmd` 的 `hits`、`patched`、`skipped`；再用专用测试收藏夹验证跨设备变化。
7. 如需使用原有配置，请先退出全部 Edge，通过 `06-Backup-Original-Profile.cmd` 备份，再运行 `05-Launch-Normal.cmd` 并输入大写 `YES`。云端同步操作不能靠本地备份自动撤销。

| 根目录脚本 | 作用 |
| --- | --- |
| `00-LegacyEdgeSync-Menu.cmd` | 统一菜单、自动选 x86/x64 |
| `01-Launch-Test.cmd` | 独立配置模式 |
| `02-Show-Status.cmd` | 查看修复状态及计数 |
| `03-Diagnose-Architecture.cmd` | 诊断实际 Edge 架构与版本 |
| `04-Check-Package.cmd` | 校验整包子程序和 DLL 是否齐全 |
| `05-Launch-Normal.cmd` | 使用原有 Edge 配置，需确认 |
| `06-Backup-Original-Profile.cmd` | 备份现有 Edge 配置 |
| `07-How-To-Stop.cmd` | 查看停止与恢复说明 |
| `08`–`10` 语言测试脚本 | 分别检查简体中文、繁體中文和 English |

**命令行语言切换示例：**

```cmd
LegacyEdgeSync.exe --diagnose --lang=zh-CN
LegacyEdgeSync.exe --diagnose --lang=zh-TW
LegacyEdgeSync.exe --diagnose --lang=en
```

不指定 `--lang` 时按 Windows 当前用户界面语言选择。完整说明见 [多语言功能](docs/MULTILINGUAL.md)。

## 兼容性与验证情况

以下明确区分**早期版本的实测成果**和**本版整合包的回归状态**。

| 操作系统 / Edge 架构 | 已记录的实验结果 | v0.3-test7 整包状态 |
| --- | --- | --- |
| Win7 SP1 x86 + Edge x86 | 三路径 Hook 修改成功，云同步连接恢复；正常配置模式曾反馈成功 | **待回归** |
| Win7 SP1 x64 + Edge x64 | 原 `0x49` 路径云同步成功，含部分跨设备收藏夹测试 | **双偏移 Hook 待回归** |
| Win8.1 x64 + Edge x64 | v0.3-test6 的 `0x45` 路径 `hits=14`、`patched=14`、`SYNCER_OK` | **统一入口待回归** |
| Win7 x64 + Edge x86 | 不应按系统位数误选 Hook；尚无完整同步验证 | 待测试 |
| Win8.1 x86 + Edge x86 | 尚未测试 | 暂缓 |

`v0.3-test7` **已发布不等于已完成稳定性回归**。目前尚不能保证首次请求竞争条件、重启后的长期稳定性、所有语言、所有数据类型和其他 Edge 构建。建议先在已有 Win7/Win8.1 虚拟机中回归，不必仅为此专门安装 Win8.1 x86。

判断成功不能只看 `INSTALLED`：还要有 `patched > 0`、正常服务端响应和实际跨设备数据一致性。

## 注意事项和已知问题

- **可能触发安全软件告警。** 360 曾将 x86/x64 启动器检测为 `HEUR/QVM20.1` 系列风险。检测尚未经过厂商复核，**不能保证为误报**。不建议关闭安全软件或绕过隔离强制运行。
- **存在注入与内存 Hook 风险。** 运行时修改机器码可能引起兼容性问题、时序竞争或崩溃；只对已校验的指定版本执行目标修复。
- **Edge 109 不再接受安全更新。** 恢复同步不等于使浏览器变得安全。强烈建议测试账户、虚拟机与无敏感数据配置。
- **原有配置模式涉及数据风险。** 云同步可能传播修改和删除；本地备份不能撤销云端状态。
- **账户头像可能无法显示。** 这是已记录但尚未解决的独立问题。
- **服务端行为可能变化。** 今天能够同步，不保证未来持续有效。

反馈请到 [Issues](https://github.com/yuyu107/LegacyEdgeSync/issues)，提供 Windows 版本、Edge 架构与完整版本、运行模式、脱敏后的 Hook 计数和同步状态；不要公开邮箱、账号标识、同步令牌、配置文件、NetLog 或内存转储。

## 源码和文档

| 目录或文档 | 内容 |
| --- | --- |
| [`src/unified/`](src/unified/) | 自动架构识别与诊断入口源码 |
| [`src/x86/`](src/x86/) | x86 启动器与三路径 Hook 源码 |
| [`src/x64/`](src/x64/) | Win7/Win8.1 双偏移 Hook、原 Win7 Hook 备份源码 |
| [`src/`](src/) | 历史版 x64 v0.2-test3 源码 |
| [`docs/RELEASE-v0.3-test7.md`](docs/RELEASE-v0.3-test7.md) | 当前 Release 正文归档 |
| [`docs/TESTING-v0.3-test7.md`](docs/TESTING-v0.3-test7.md) | 未完成的回归测试清单 |
| [`docs/RESEARCH.md`](docs/RESEARCH.md) | 同步请求研究记录 |
| [`docs/RELEASES.md`](docs/RELEASES.md) | 版本与历史文档索引 |

**GitHub `main` 是源码仓库，不包含运行包里的编译后 EXE/DLL。** 根目录 CMD 脚本需要与 Release 完整 ZIP 中的二进制文件配合使用。

## 许可证与声明

目前尚无 `LICENSE` 文件；公开源码不代表自动授予复制、修改、分发或商用许可。

Microsoft Edge 是 Microsoft 的商标。本项目是非官方、独立的兼容性研究，与 Microsoft 无隶属或官方支持关系。

---

### English summary

**LegacyEdgeSync v0.3-test7** is a published **experimental pre-release** for Microsoft Edge **109.0.1518.140** on Windows 7 / 8.1, selecting x86/x64 runtime hooks by the browser's actual PE architecture.

Earlier builds demonstrated sync recovery on Win7 x86, Win7 x64, and Win8.1 x64. **The v0.3-test7 consolidated bundle has not completed runtime regression**; Win8.1 x86 remains untested. The tool does not provide browser security fixes, has not passed a security audit, and some launchers have been flagged by antivirus heuristics.

[Download v0.3-test7](https://github.com/yuyu107/LegacyEdgeSync/releases/tag/v0.3-test7) · [Sources](src/) · [Issues](https://github.com/yuyu107/LegacyEdgeSync/issues)
