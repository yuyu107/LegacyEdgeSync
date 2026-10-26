# LegacyEdgeSync v1.0 — 视频演示版变更记录

发行版本采用 **v1.0**，技术基线是 **v0.3-test8**。v1.0 对外发行包中的 EXE/DLL 与 v0.3-test8 测试包**字节完全一致**，只重新整理名称、版本说明、校验值和公开使用文档。**不是重新编译后的新核心。**

## 继承功能

- v0.3-test7：PE 架构与版本识别、x86/x64 统一启动、完整性诊断、独立/正常配置模式、配置备份及三语言。
- v0.3-test6：Win7/Win8.1 x64 已验证的 `base+0x49` 和 `base+0x45` 精确匹配修复路径。
- v0.3-test8：x64 限定偏移 `0x20`–`0xBF` 的自适应候选搜索，要求完整字段、有效边界和唯一匹配；异常时跳过。已通过 13 组离线测试，Windows 运行时回归仍待完成。
- x86：保持已成功的三路径 Hook，不改动二进制。
- 保留原版 x64 Hook 作为回退文件。

## 版本标识与发布状态

- v1.0 是整个 ZIP 的公开版本号；内部程序/诊断可能仍显示 `v0.2-test3`、`v0.3-test3`、`v0.3-test7`、`v0.3-test8`，它们是历史组件构建 ID。
- **v1.0 已于 2026-10-26 在 GitHub 正式发布**，并附带 `LegacyEdgeSync-v1.0-Win7-Win81-Universal.zip`（125174 字节）：[Release 页面](https://github.com/yuyu107/LegacyEdgeSync/releases/tag/v1.0)。
- `src/x64/hook.c` 的 v1.0 自适应新源码仍需从源码套件同步到 GitHub；此文档的提交不代表源码已全部同步。

详细参见 [v1.0 Release 正文](RELEASE-v1.0.md) 和 [验证清单](TESTING-v1.0.md)。
