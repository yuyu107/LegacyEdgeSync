# LegacyEdgeSync 多语言说明

> 当前发布的 [**v0.3-test7（Pre-release）**](https://github.com/yuyu107/LegacyEdgeSync/releases/tag/v0.3-test7) 内含统一启动入口。本文的首要说明适用于统一入口；文末保留旧版直接运行子程序的方式。

## 自动选择语言

`LegacyEdgeSync.exe` 通过 Windows 用户界面语言自动选择提示文本：

| 系统语言 | 提示语言 |
| --- | --- |
| `zh-CN` / `zh-SG` | 简体中文 |
| `zh-TW` / `zh-HK` / `zh-MO` | 繁體中文 |
| 英文及其他语言 | English |

程序使用 Windows 用户 UI 语言，不按键盘布局或地理位置识别。它只更改工具提示，不改变 Edge 或系统的显示语言。

## v0.3-test7 统一入口（推荐）

从 [Release](https://github.com/yuyu107/LegacyEdgeSync/releases/tag/v0.3-test7) 下载并完整解压。在根目录运行：

```cmd
00-LegacyEdgeSync-Menu.cmd
```

手动验证语言：

```cmd
LegacyEdgeSync.exe --diagnose --lang=zh-CN
LegacyEdgeSync.exe --diagnose --lang=zh-TW
LegacyEdgeSync.exe --diagnose --lang=en
```

也可直接运行 `08-Test-Language-zhCN.cmd`、`09-Test-Language-zhTW.cmd`、`10-Test-Language-en.cmd`。

统一入口负责检测 Edge PE 架构、版本并转发语言参数至对应子启动器。子程序保留 `--menu`、`--test`、`--normal`、`--status`、`--backup` 和 `--guide` 等模式。**翻译和自动转发功能尚未在所有 Windows 语言环境中完整回归。**

## 历史 v0.2-test3 直接运行方式

旧 x64 多语言启动器曾使用：

```cmd
Edge109SyncFix.exe --menu --lang=zh-CN
Edge109SyncFix.exe --menu --lang=zh-TW
Edge109SyncFix.exe --menu --lang=en
```

这是历史用法。**在当前 Release 根目录中请优先运行 `LegacyEdgeSync.exe` 或对应 CMD 脚本**；不要把 x64 程序误用于 x86 Edge。

## 需要验证的事项

- Windows 7 和 Windows 8.1 既有环境的三语言显示、菜单操作、错误提示。
- 统一入口选择的 Edge 架构是否与实际 PE 一致；不要根据文件夹名 `Program Files (x86)` 判断位数。
- 当现有 Edge 进程正在运行时，启动器应拒绝启动第二次带 Hook 的 Edge。
- `--normal` 必须明确确认；先备份原有配置。
- `--status` 应显示实际 Hook 计数，不能只凭 `INSTALLED` 声明同步成功。

请参考 [v0.3-test7 回归清单](TESTING-v0.3-test7.md)。Win8.1 x86 仍未验证；**不必仅为此次预发布另装 32 位 Win8.1**。

Edge 109 已结束安全维护，本工具不提供安全补丁，也不能保证安全软件风险提示属于误报。建议使用测试账户，不同步敏感数据。
