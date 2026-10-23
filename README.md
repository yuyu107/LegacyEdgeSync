# LegacyEdgeSync

**旧版 Microsoft Edge 云同步兼容性修复研究**  
*Experimental Microsoft Edge cloud sync compatibility patch for legacy Windows.*

[![Release](https://img.shields.io/github/v/release/yuyu107/LegacyEdgeSync?include_prereleases&label=Pre-release)](https://github.com/yuyu107/LegacyEdgeSync/releases)
[![Win7 / Win8.1](https://img.shields.io/badge/Experimental-Win7%20%2F%20Win8.1%20x86%20%2F%20x64-blue)](#兼容性与验证情况)
[![Status](https://img.shields.io/badge/Status-Experimental-orange)](#已知问题与注意事项)

> [!WARNING]
> **实验性研究项目，并非正式稳定版。** 已有成功实验：**Windows 7 SP1 x86、Windows 7 SP1 x64、Windows 8.1 x64**，均针对 **Microsoft Edge 109.0.1518.140** 的对应架构。Windows 8.1 x86 尚未测试，整合后的 v0.3-test7 也尚未完成跨系统回归。Edge 109 已停止安全更新，本项目不能提供新版浏览器的安全修复。建议只在有快照的虚拟机中使用测试账户，不要同步密码或支付信息。

## 项目简介

Windows 7 上的 Microsoft Edge 109 仍可能正常启动、登录 Microsoft 账户，却无法与当前云同步服务完成通信。在项目实测中，旧版同步发现接口返回 HTTP 404；即使绕过该接口，服务器还会拒绝旧版同步客户端（`UPGRADE_CLIENT`）。

LegacyEdgeSync 通过**指定同步地址 + 进程内运行时兼容补丁**，尝试恢复 Edge 109 自带的云同步功能。它不会永久修改磁盘上的 `msedge.dll`，不需要 Fiddler HTTPS 解密代理，也不需要持续附加 x64dbg。

想先了解背景？请阅读：

- **[为什么旧版 Edge 需要修复云同步？](docs/WHY-SYNC-FIX.md)**
- **[为什么不直接通过扩展内核运行新版 Edge？](docs/WHY-NOT-EXTENDED-KERNEL.md)**

这两篇说明放在独立页面，README 主要介绍项目现状、下载和使用方法。

## 版本与下载

| 渠道 | 版本 | 当前状态 |
| --- | --- | --- |
| **[GitHub Releases](https://github.com/yuyu107/LegacyEdgeSync/releases/tag/v0.1-test3)** | `v0.1-test3` | 已发布的实验性预发布版；主要使用独立测试配置 |
| **[main 分支源码](src/)** | `v0.3-test7` | 已包含统一入口、x86/x64 双架构源码和 Win8.1 x64 双偏移 Hook；**尚未发布 v0.3-test7 Release** |
| **历史 x64 源码** | `v0.2-test3` | 保留在根目录 `src/` 中，便于对照已验证的旧版实现 |

**当前已发布的二进制文件：**  
[`Edge109-SyncFix-v0.1-test3-Win7-x64.zip`](https://github.com/yuyu107/LegacyEdgeSync/releases/download/v0.1-test3/Edge109-SyncFix-v0.1-test3-Win7-x64.zip)

> 项目已更名为 **LegacyEdgeSync**，但内部程序仍使用 `Edge109SyncFix.exe` 和 `Edge109SyncHook.dll`。这是正常的开发阶段命名，并不是不同项目。

**源码不等于已发布的二进制：** 当前 `main` 分支包含 **v0.3-test7 源码**，但 GitHub Releases 中的 `v0.1-test3` ZIP 仍是较早的实验构建，**不包含**最新版统一架构检测、Win8.1 兼容 Hook 或完整双架构菜单。请勿将源码版本误认为最新公开下载版。

## 当前开发功能（v0.3-test7 整合版源码）

| 功能 | 说明 |
| --- | --- |
| 云同步兼容补丁 | 针对 Edge 109.0.1518.140 的特定 x64 / x86 构建分别实现运行时请求头修复；**EXE/DLL 不可跨架构混用** |
| **统一自动识别入口** | 读取 Edge PE 架构和目标版本，自动选择 x86/x64 子启动器，不按照系统位数或安装文件夹名称猜测 |
| **整包诊断** | `--bundle-check` 检查两套子 EXE / Hook DLL 的 PE 架构及文件是否齐全，不启动 Edge，也不注入 |
| **Win8.1 x64 兼容** | x64 Hook 保留 Win7 已验证的 `base+0x49`，并加入 Win8.1 已验证的 `base+0x45` 精确匹配路径 |
| **回退留档** | v0.3-test7 运行包包含原 Win7 x64 Hook 备份，便于实机回归后比较和回退 |
| **正常配置模式** | 使用 Edge 原有用户配置、收藏夹和登录状态 |
| **独立测试模式** | 使用单独的临时配置目录，避免直接操作现有用户配置 |
| 中文与英文提示 | 根据 Windows 当前用户显示语言自动选择 **简体中文、繁體中文或 English** |
| 状态与诊断 | 显示 Hook 安装情况、进程 PID、命中与修改计数 |
| 原有配置备份 | 提供备份入口；备份前应确保所有 Edge 进程已退出 |
| 退出说明 | 说明如何结束临时 Hook，以及如何保护现有配置数据 |
| 已运行进程检查 | 检测到 Edge 正在运行时拒绝启动，不强制结束进程或注入已有进程 |

**语言判断：** 使用 `GetUserDefaultUILanguage()` 获取 Windows 用户界面显示语言，简体中文（大陆／新加坡）采用简体提示，繁体中文（台湾／香港／澳门）采用繁體提示，其他系统语言默认英文。可以使用 `--lang` 参数手动检查三套文本；语言切换不会改变 Edge 或 Windows 的语言设置。

详细内容：[v0.3-test7 更新记录](docs/CHANGELOG-v0.3-test7.md) · [v0.3-test7 回归清单](docs/TESTING-v0.3-test7.md) · [多语言使用与测试](docs/MULTILINGUAL.md) · [旧版 x86 变更记录](docs/CHANGELOG-v0.3-test3.md)。

## 如何使用

### 已发布的 v0.1-test3 实验版

1. 在 Windows 7 SP1 64 位环境安装 **Edge 109.0.1518.140 x64**，建议先建立虚拟机快照。
2. 从 [v0.1-test3 Release](https://github.com/yuyu107/LegacyEdgeSync/releases/tag/v0.1-test3) 下载 ZIP，完整解压。
3. 关闭所有 Edge 窗口，确认没有残留的 `msedge.exe` 进程。
4. 确保 `Edge109SyncFix.exe` 和 `Edge109SyncHook.dll` 位于同一目录，运行 `01-Launch-Test.cmd`。
5. 在启动的独立 Edge 配置中使用测试 Microsoft 账户登录，再用 `02-Show-Status.cmd` 检查修复情况。
6. 打开 `edge://sync-internals/` 查看同步状态，在不同设备间新建或修改专用测试收藏夹验证实际双向同步。

此版本使用独立测试目录 `%TEMP%\Edge109SyncFix-Test`，不直接使用默认 Edge 用户配置。

### 历史版 v0.2-test3 多语言双模式（x64）

如果你使用的是**与 v0.2-test3 源码对应的测试构建**，可以运行：

| 文件 / 参数 | 操作 |
| --- | --- |
| `00-LegacyEdgeSync-Menu.cmd` | 打开三语言自动识别菜单 |
| `01-Launch-Test.cmd` / `--test` | 独立测试配置模式（默认） |
| `05-Launch-Normal.cmd` / `--normal` | 使用原有 Edge 配置；需要输入 `YES` 确认 |
| `02-Show-Status.cmd` / `--status` | 查看 Hook 诊断状态 |
| `06-Backup-Original-Profile.cmd` / `--backup` | 备份原有配置 |
| `07-How-To-Stop.cmd` / `--guide` | 查看退出与恢复说明 |

测试翻译时，可在命令行输入：

```cmd
Edge109SyncFix.exe --menu --lang=zh-CN
Edge109SyncFix.exe --menu --lang=zh-TW
Edge109SyncFix.exe --menu --lang=en
```

不传 `--lang` 则自动检测系统显示语言。这些命令保留为历史 x64 测试方法；若使用新整合包，应优先运行**整合包根目录**中的统一入口。原有配置模式会接触真实的本地及云端收藏夹，务必提前备份。

### Win7 32 位：x86 v0.3-test2 / v0.3-test3 实验记录

前一个实验版本 **v0.3-test2** 在 Windows 7 SP1 x86 + Edge 109.0.1518.140 x86 上，已经通过截图确认三路径 Hook 的 `installed_sites=3`、`hits=18`、`patched=18`、`skipped=0`；其中 `setterB_patched=4`、`format_patched=14`。Edge 同步页面显示 `Server Connection: OK`、`Last Synced: Just now` 和 `Commit Request`。仍须继续测试跨设备完整数据一致性及重启后的稳定性。

**v0.3-test3** 沿用成功的 v0.3-test2 x86 Hook，只升级多语言菜单、独立测试、原有配置模式（需 `YES`）、Robocopy 备份和状态提示。测试者已报告正常配置模式也可以同步。对应 x86 源码现存于 [`src/x86/`](src/x86/)，但完整稳定性回归仍未完成。

x86 实验所需详细测试内容见：[v0.3-test3 更新记录](docs/CHANGELOG-v0.3-test3.md) 和 [Win7 x86 回归清单](docs/TESTING-x86-v0.3-test3.md)。

### 当前 v0.3-test7 统一测试包的使用方式

若你已获得与这份源码对应的 **v0.3-test7 实验构建**，请从整合包根目录启动：

| 文件 | 用途 |
| --- | --- |
| `04-Check-Package.cmd` | 检查双架构组件完整性，无需安装 Edge |
| `03-Diagnose-Architecture.cmd` | 验证 Edge 版本及 PE 架构，确认选择哪套修复器 |
| `00-LegacyEdgeSync-Menu.cmd` | 进入多语言统一菜单 |
| `01-Launch-Test.cmd` | 默认独立测试配置模式 |
| `05-Launch-Normal.cmd` | 运行原有配置模式，按提示确认后继续 |
| `06-Backup-Original-Profile.cmd` | 备份原有 Edge 配置 |

以上脚本属于**独立运行包**，GitHub 源码目录仅提供源码和构建脚本。该整合版尚未通过 Win7/8.1 全部实机回归，也尚未发布 GitHub Release。

## 兼容性与验证情况

以下为**项目测试中实际确认的结果**，不是对所有系统或账户的兼容性承诺：

| 项目 | 验证结果 |
| --- | --- |
| Windows 7 SP1 x64、Edge 109.0.1518.140 x64 | ✅ 已验证 |
| Windows 7 SP1 x86、Edge 109.0.1518.140 x86，v0.3-test2 Hook | ✅ 已验证 Hook 实际修改请求及同步连接 |
| Windows 7 x86、v0.3-test3 原有配置模式 | ✅ 测试者已报告可以同步；其他菜单/备份功能仍需完整回归 |
| Windows 8.1 x64、Edge 109.0.1518.140 x64，v0.3-test6 Hook | ✅ 已验证 `hits=14`、`patched=14`、`patch_at_0x45=14`、`SYNCER_OK` |
| Windows 8.1 x86、Edge 109 x86 | ❔ 尚未测试 |
| v0.3-test7 整合包在各系统上的回归 | 🧪 待验证 |
| 无需 Fiddler / x64dbg 的独立 Hook | ✅ 已验证 |
| Microsoft 云同步连接及收藏夹双向同步 | ✅ 已验证 |
| 保留登录状态，重启 Win7 后再次同步 | ✅ 已成功测试 |
| v0.2-test1、v0.2-test2 双模式同步 | ✅ 测试者反馈两种模式均可同步 |
| v0.2-test3 三语言自动切换、各语言系统完整回归 | 🧪 仍待进一步验证 |
| 长期运行、多次连续重启及更多用户环境 | 🧪 尚未充分验证 |
| 其他 Edge 构建（非 109.0.1518.140） | ❔ 尚未验证，不应直接套用特定 RVA Hook |
| 密码、支付信息等敏感数据类型 | ⚠️ 不建议测试 |

测试成功时，`edge://sync-internals/` 曾显示：

```text
Transport State: Active
Disable Reasons: None
Server Connection: OK
```

Hook 状态曾显示：

```text
INSTALLED: hook active, experimental build
hits=3
patched=3
skipped=0
```

`INSTALLED` 并不等于同步一定成功；还应检查 `patched` 是否增加、服务器连接状态和实际的跨设备数据变化。

## 实现概要

研究发现，Edge 109 同步失败至少涉及两项独立问题：

1. 旧的 `/sync/v2/feeds/me/settings/homeCloud` 服务发现请求在测试中返回 404；使用 `--sync-url` 可以绕过该发现步骤。
2. 同步请求中的 `X-AFS-ClientInfo` 仍上报 `appVer=109.0.1518.140`，服务器可能返回 `UPGRADE_CLIENT`。运行时 Hook 经过版本、代码及字符串检查后，尝试将该字段等长改写为 `133.0.3065.120`。

**这不是升级浏览器版本**：不会补齐新版 Edge API、协议能力或安全补丁。详细过程见 [RESEARCH.md](docs/RESEARCH.md)。

## 已知问题与注意事项

- **用户头像可能无法显示**：已经观察到，计划在后续版本研究；当前不归因于已确认的同步故障。
- **安全软件检测**：360 安全卫士曾将 x86/x64 启动器的跨进程注入行为标记为 `HEUR/QVM20.1` 系列风险。未经安全厂商复核，**不能保证属于误报**。不建议关闭安全防护或忽略告警来强行运行。请核对官方来源和 SHA-256；仅建议在隔离的研究虚拟机内自行评估。
- 注入式 Hook 具有启动时序、多线程竞态及进程崩溃风险；暂不适合作为日常浏览器补丁。
- 原有配置模式可能导致本地与云端数据变化，使用前请备份；关闭 Edge 后进程内补丁才会消失。
- 不建议在主力账户中同步密码、支付信息，也不要关闭系统安全防护来强行运行。
- 微软服务端策略可能变化；目前的实验成功不能保证未来持续有效。
- Issues 反馈请提供 **Windows 完整版本、Edge 完整版本、运行模式、状态计数和复现步骤**，勿公开邮箱、令牌、用户配置文件、未经脱敏的 NetLog 或内存转储。

## 相关资料

- [为什么需要修复 Edge 云同步？](docs/WHY-SYNC-FIX.md)
- [为什么不直接使用扩展内核运行新版 Edge？](docs/WHY-NOT-EXTENDED-KERNEL.md)
- [研究过程](docs/RESEARCH.md)
- [测试方法](docs/TESTING.md)
- [多语言说明](docs/MULTILINGUAL.md)
- [v0.2-test3 更新记录](docs/CHANGELOG-v0.2-test3.md)
- [v0.3-test3 x86 变更](docs/CHANGELOG-v0.3-test3.md)
- [v0.3-test7 跨系统整合版变更](docs/CHANGELOG-v0.3-test7.md)
- [v0.3-test7 兼容性回归清单](docs/TESTING-v0.3-test7.md)
- [统一自动架构选择器源码](src/unified/)
- [x86 Hook 和启动器源码](src/x86/)
- [x64 双偏移 Hook 及回退源码](src/x64/)
- [Win7 x86 回归测试清单](docs/TESTING-x86-v0.3-test3.md)
- [源代码](src/)
- [GitHub Releases](https://github.com/yuyu107/LegacyEdgeSync/releases)
- [问题反馈](https://github.com/yuyu107/LegacyEdgeSync/issues)

## 许可证与声明

当前仓库尚未提供开源许可证。公开代码并不自动授予复制、修改、分发或商业使用许可。

Microsoft Edge 是 Microsoft 的商标。LegacyEdgeSync 是非官方、独立的兼容性研究项目，与 Microsoft 无关。

---

### English summary

**LegacyEdgeSync** is an experimental, build-specific runtime compatibility patch for Microsoft Edge **109.0.1518.140** on Windows 7 and Windows 8.1. Successful sync has been observed with **Win7 x86**, **Win7 x64**, and **Win8.1 x64**, using build-specific hooks. **Win8.1 x86 remains untested**.

The latest **published pre-release is v0.1-test3**. The repository's `main` branch now contains **v0.3-test7 unified x86/x64 source code** including the Win8.1 x64 exact-offset fallback. The new unified binary bundle has **not** been published as a GitHub Release and still requires regression testing.

The project does not provide browser security updates and is not intended for production use. Avoid syncing passwords or other sensitive data.

[Download released test build](https://github.com/yuyu107/LegacyEdgeSync/releases/tag/v0.1-test3) · [Source](src/) · [Report issues](https://github.com/yuyu107/LegacyEdgeSync/issues)
