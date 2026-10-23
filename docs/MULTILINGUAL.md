# 多语言功能（v0.2-test3 测试版）

**本页介绍已编译的多语言测试构建；截至编写时，本构建的 Windows 7 实机回归结果尚未反馈。** 此构建继续使用此前已验证的运行时 Hook DLL，不包含头像显示修复。

## 自动语言判断

启动器通过 Windows API `GetUserDefaultUILanguage()` 判断**当前用户界面显示语言**；获取失败时尝试 `GetSystemDefaultUILanguage()`。这不是键盘布局、地理位置或“非 Unicode 程序的语言”。

| 系统 UI 语言 | 程序提示 |
| --- | --- |
| 简体中文：`zh-CN`、`zh-SG` | 简体中文 |
| 繁體中文：`zh-TW`、`zh-HK`、`zh-MO` | 繁體中文 |
| 英文及其他语言 | English |

启动器使用 Windows Unicode 控制台输出；启动 CMD 保持 ASCII，不再固定执行 `chcp 936`。语言选择只影响 LegacyEdgeSync 本身的提示，不会改变 Edge 的语言或 Windows 设置。

## 用法

将 v0.2-test3 测试包完整解压后，运行 `00-LegacyEdgeSync-Menu.cmd`。菜单提供原有配置、独立测试、状态检查、原有配置备份及退出说明。**正常模式使用真实配置文件，建议预先备份。**

## 手动检查三种翻译

在 CMD 的程序目录下运行：

```cmd
Edge109SyncFix.exe --menu --lang=zh-CN
Edge109SyncFix.exe --menu --lang=zh-TW
Edge109SyncFix.exe --menu --lang=en
```

也可以测试：

```cmd
Edge109SyncFix.exe --help --lang=zh-TW
Edge109SyncFix.exe --status --lang=en
```

不指定 `--lang` 时，自动检测当前用户 UI 语言。繁体/英文系统应优先检查中文字符和 Windows 控制台字体兼容性。

## 同步回归与风险

1. 退出所有 `msedge.exe`，先测试独立配置模式。
2. 在 `edge://sync-internals/` 查看 Active、服务器状态，并通过测试收藏夹确认双向同步。
3. 关闭全部 Edge 进程，再测试使用原有配置模式。
4. 重启后再次验证；确保错误显示与备份/恢复提示均为所选语言。

当前仅验证的组合仍是 Windows 7 SP1 x64 + Edge 109.0.1518.140 x64；不建议密码与支付信息同步。用户头像无法显示的问题尚未解决。
