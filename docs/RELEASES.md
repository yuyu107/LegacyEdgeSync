# Releases / 版本索引

> **最新发布：v1.0（2026-10-26，视频演示版）** — 基于 v0.3-test8 自适应 x64 Hook，[完整 ZIP 下载](https://github.com/yuyu107/LegacyEdgeSync/releases/download/v1.0/LegacyEdgeSync-v1.0-Win7-Win81-Universal.zip) · [Release 正文](RELEASE-v1.0.md) · [更新说明](CHANGELOG-v1.0.md) · [验证清单](TESTING-v1.0.md)。发布不等于整包已完成实机回归。

本页区分**已发布的二进制包**与历史实验源码。不要将 `main` 分支源码版本误认为其对应的可下载 Release 已经通过实机验证。

| Tag | 发布情况 | 附件 | 主要用途 |
| --- | --- | --- | --- |
| **[`v1.0`](https://github.com/yuyu107/LegacyEdgeSync/releases/tag/v1.0)** | **已发布正式 Release** | [v1.0 完整运行包](https://github.com/yuyu107/LegacyEdgeSync/releases/download/v1.0/LegacyEdgeSync-v1.0-Win7-Win81-Universal.zip) | 基于 v0.3-test8，x64 有界自适应 Hook，统一 x86/x64 启动；新构建仍待完整 Windows 回归 |
| [`v0.3-test7`]](https://github.com/yuyu107/LegacyEdgeSync/releases/tag/v0.3-test7)** | **已发布 Pre-release** | [完整双架构测试包](https://github.com/yuyu107/LegacyEdgeSync/releases/download/v0.3-test7/LegacyEdgeSync-v0.3-test7-Win7-Win81-Universal-TEST.zip) | 统一架构选择、Win7/8.1 x64 双偏移 Hook、x86 三路径 Hook；本版整包仍待回归 |
| [`v0.1-test3`](https://github.com/yuyu107/LegacyEdgeSync/releases/tag/v0.1-test3) | 已发布 Pre-release | [Win7 x64 实验包](https://github.com/yuyu107/LegacyEdgeSync/releases/download/v0.1-test3/Edge109-SyncFix-v0.1-test3-Win7-x64.zip) | 原 Win7 x64 同步修复验证版本 |

**尚未以独立 Release 形式发布的测试节点：**

- `v0.2-test3`：历史 x64 多语言双模式源码，保留于 [`src/`](../src/)。
- `v0.3-test2`：Win7 x86 三路径 Hook 成功实验。
- `v0.3-test3`：x86 新增原有配置模式与多语言；相关源码见 [`src/x86/`](../src/x86/)。
- `v0.3-test8`：x64 自适应匹配器 13 组离线逻辑测试通过，已成为已发布 v1.0 的基线；新 DLL 尚待 Windows 回归。
- `v0.3-test6`：Win8.1 x64 的 `base+0x45` 修复成功；已被整合进 v0.3-test7 的 x64 Hook。

## 相关文档

- [v1.0 Release 正文](RELEASE-v1.0.md)
- [v1.0 更新记录](CHANGELOG-v1.0.md)
- [v1.0 回归清单](TESTING-v1.0.md)
- [v1.0 SHA-256](SHA256SUMS-v1.0.txt)

- [v0.3-test7 Release 正文](RELEASE-v0.3-test7.md)
- [v0.3-test7 更新记录](CHANGELOG-v0.3-test7.md)
- [v0.3-test7 回归清单](TESTING-v0.3-test7.md)
- [v0.3-test7 SHA-256](SHA256SUMS-v0.3-test7.txt)
- [v0.1-test3 历史 Release 说明](RELEASE_NOTES.md)
- [研究日志](RESEARCH.md)

**所有发布均属实验用途**。用户应严格核对 Edge `109.0.1518.140` 版本及 PE 架构，不建议通过关闭安全软件或同步敏感信息来进行测试。
