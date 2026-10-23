# LegacyEdgeSync v0.3-test7 — Win7 / Win8.1 x86/x64 云同步整合测试版

> **实验性 Pre-release，非稳定版。** 只研究 Microsoft Edge **109.0.1518.140** 特定 x86/x64 构建的同步兼容性。整合包 v0.3-test7 尚未完成跨系统实机回归；成功案例来自前几个单架构测试版本。

## 更新内容

- **新增统一启动入口**：`LegacyEdgeSync.exe` 自动检查 Edge 的 PE 架构和指定版本，再选择 `LegacyEdgeSync32.exe` 或 `Edge109SyncFix.exe`；不会按系统位数或 `Program Files (x86)` 目录名猜测。
- **整合 Win8.1 x64 已验证的 Hook**：x64 DLL 采用 v0.3-test6 的双偏移方案，先精确检查 `base+0x49`（Win7 路径），再检查 `base+0x45`（Win8.1 路径）；只在 `appVer=109.0.1518.140` 完全匹配且内存可写时执行版本字段替换。
- **保留 x86 已成功的三路径 Hook**：`setterA`、`setterB`、`format`，没有改写核心算法。
- **新增组件自检**：`04-Check-Package.cmd` 检查子启动器、DLL 是否齐全及 PE 架构；不启动 Edge，也不执行注入。
- **增强架构诊断**：`03-Diagnose-Architecture.cmd` 核对 Edge EXE/DLL 版本、架构及匹配修复文件。
- **原有功能保留**：简体中文、繁體中文、English；隔离测试配置、原有配置模式（输入 `YES`）、备份与退出说明。
- **留存回退文件**：`Rollback-Hooks/Win7-x64-v0.2-test3/` 保存旧的 Win7 x64 Hook DLL，便于回归对照。

## 已知实验结果与验证范围

| 测试环境 | 既有测试证据 | v0.3-test7 整包 |
| --- | --- | --- |
| Windows 7 SP1 x86 + Edge 109 x86 | v0.3-test2：`hits=18`、`patched=18`、`skipped=0`，同步连接正常；v0.3-test3 原有配置模式也报告成功 | **待回归** |
| Windows 7 SP1 x64 + Edge 109 x64 | 原 x64 Hook `0x49` 路径已成功同步 | **新双偏移 DLL 待回归** |
| Windows 8.1 x64 + Edge 109 x64 | v0.3-test6：`hits=14`、`patched=14`、`patch_at_0x45=14`，`SYNCER_OK` | **统一入口待回归** |
| Windows 7 x64 + Edge 109 x86 | 未完成正式验证 | 待测试 |
| Windows 8.1 x86 + Edge 109 x86 | 尚未测试 | 暂缓，无须专门安装系统 |

**请勿将早期版本验证结果视为本次整合包已通过完整回归。**

## 下载与使用

**附件文件名：** `LegacyEdgeSync-v0.3-test7-Win7-Win81-Universal-TEST.zip`

请将 ZIP 完整解压。内部 `Win7-x86/`、`Win7-x64/` 为历史命名，实际用于区分 **Edge x86 / x64 架构**，并不代表不能在 Windows 8.1 上使用。

1. 建立 VMware 快照，使用测试 Microsoft 账户；确保目标 Edge 版本为 **109.0.1518.140**。
2. 运行 `04-Check-Package.cmd`；然后运行 `03-Diagnose-Architecture.cmd`。
3. 退出 Edge 的全部进程，运行 `00-LegacyEdgeSync-Menu.cmd`，先选择**独立测试配置**。
4. 查看 `02-Show-Status.cmd` 的 `hits`、`patched`、`skipped`，同时在 `edge://sync-internals/` 检查 `Server Connection` 和真实收藏夹数据是否同步。**仅显示 `INSTALLED` 不代表修复成功。**
5. 如果要使用原有配置，先关闭 Edge，执行 `06-Backup-Original-Profile.cmd`，再运行 `05-Launch-Normal.cmd` 并按提示输入 `YES`。

## 安全提示与已知问题

- Edge 109 已停止安全更新，本项目不能修复浏览器安全漏洞，不建议用主力账户或同步真实密码、支付信息。
- 启动器使用跨进程 DLL 加载，Hook 修改进程内存，存在兼容性、竞态与崩溃风险；未经过安全审计。
- 360 安全卫士曾将 x86/x64 启动器标记为 `HEUR/QVM20.1` 系列风险。**未经厂商确认，不可直接宣称误报。** 不建议关闭或绕过安全软件来强行运行。
- 原有配置模式可能通过云同步修改或删除真实数据；本地备份不能撤销云端变更。
- 用户头像问题尚未解决；其他 Edge 构建、Windows 8.1 x86、自定义安装路径及长期稳定性未充分验证。
- 发布调试截图前遮挡邮箱、账号标识、令牌及其他隐私数据。

## 完整性校验（SHA-256）

**Release ZIP：**

```text
44a672420b8e36b5c9f4bd8a1c3dad35e0175645e2902a4dc193376e740b7238  LegacyEdgeSync-v0.3-test7-Win7-Win81-Universal-TEST.zip
```

**ZIP 内部主要文件：**

```text
2f8354e33c1c0ee96129248bb8b27ec7f36ab723ed691426d45127bd21a6ab35  LegacyEdgeSync.exe
278240138f51659e0bcbd50c71ef1e3da976e3ea6be3e3b4e9110a7ba274cf4d  Win7-x86/LegacyEdgeSync32.exe
bbfcd3b60af3788ce100c7d020c927c4aedcad95a7fbb13ff14785e685edb71c  Win7-x86/LegacyEdgeSyncHook32.dll
547f016a70ddae153861a21f9d6a1732287546ae34ce4b948466f89c5ef8f832  Win7-x64/Edge109SyncFix.exe
163ac18069b8980e0e685b8cb3354d60c063b47a5b75e92e8970c314f7acbb65  Win7-x64/Edge109SyncHook.dll
793aba700707c5051b41466a2f6a340bfa945245248ceb2413db493e54068aa9  Rollback-Hooks/Win7-x64-v0.2-test3/Edge109SyncHook.dll
```

SHA-256 用于比对文件字节是否一致，并非安全认证。

## 项目与反馈

- GitHub：https://github.com/yuyu107/LegacyEdgeSync
- 更新记录：https://github.com/yuyu107/LegacyEdgeSync/blob/main/docs/CHANGELOG-v0.3-test7.md
- 回归测试清单：https://github.com/yuyu107/LegacyEdgeSync/blob/main/docs/TESTING-v0.3-test7.md
- 统一入口源码：https://github.com/yuyu107/LegacyEdgeSync/tree/main/src/unified
- x86 源码：https://github.com/yuyu107/LegacyEdgeSync/tree/main/src/x86
- x64 源码：https://github.com/yuyu107/LegacyEdgeSync/tree/main/src/x64

反馈时请附上系统版本、Edge 架构和完整版本、运行模式、脱敏后的诊断计数及复现步骤。不要公开原始配置文件或同步令牌。

---

**发布设置：** Tag = `v0.3-test7`；勾选 **Set as a pre-release**；附件上传上述 ZIP。该 Markdown 是**待发布草稿**，同步到仓库不等于已经创建 Release。
