# v0.3-test8 — 自适应 x64 请求头匹配实验版

> **视频演示候选版本，尚未通过 Windows 实机回归。** 基于 v0.3-test7 双架构整合包开发，专门针对此前 Win7 x64 的 `0x49` 和 Win8.1 x64 的 `0x45` 字段偏移差异。版本号限于 **Microsoft Edge 109.0.1518.140**；它不是 Edge 升级工具，也不提供浏览器安全更新。

## 本次调整

- x64 Hook **优先保留 v0.3-test6 的两个精确匹配位置**：`base+0x49`、`base+0x45`。
- 两个固定位置都未匹配时，才在限定的相对偏移 `0x20`～`0xBF` 搜索完整旧版本字段。
- 匹配时检查字段边界、可访问内存和候选唯一性；不确定时拒绝修改，仅记录诊断状态。
- 新增 `patch_adaptive`、`adaptive_last_offset_decimal`、`adaptive_ambiguous`、`adaptive_readonly`、`adaptive_other_version`、`adaptive_no_match` 等状态字段。
- x86 启动器与三路径 Hook、统一入口和 x64 子启动器保持原有构建；**仅 x64 Hook DLL 重新编译**。
- 包中保留 v0.3-test6 Win7/8.1 x64 Hook 和更早 Win7 x64 Hook，便于手动回退。
- 自适应匹配器已通过 **13 组离线逻辑测试**，Windows 真实同步仍待验证。

## 重要区分

- 已有实机成果：Win7 x86、Win7 x64、Win8.1 x64 对应**此前测试版**曾恢复同步。
- 未完成：**v0.3-test8 新 x64 Hook** 在真实 Windows 上的回归，尤其是原有 `0x45` 和 `0x49` 路径是否保持正常。
- 未测环境：Win8.1 x86；并不要求为了视频额外安装系统。
- 画面中出现 `INSTALLED`、`hits` 增加，不代表云端成功接受同步；需观察 `patched` 计数、`edge://sync-internals/` 和跨设备测试收藏夹。

## 视频演示与下载说明

作者计划使用 **v0.3-test8** 录制 LegacyEdgeSync 演示视频。请注意：此文档同步到源码仓库**不代表 GitHub Release ZIP 已上传**。在上传并核实附件前，不应将它列为可下载的公开版本。

**预定测试包名：** `LegacyEdgeSync-v0.3-test8-Adaptive-Win7-Win81-TEST.zip`

候选 ZIP SHA-256：

```text
d09a56ebbd537005ec136bbbe379b7234e2d3b4be3dcc89e1d514414c527161e  LegacyEdgeSync-v0.3-test8-Adaptive-Win7-Win81-TEST.zip
```

测试建议：先在 Win7 x64 和 Win8.1 x64 既有虚拟机中复核相应旧路径，随后再测试跨设备收藏夹、重启后同步。**不建议使用主力账户或同步密码和支付信息。** 工具存在进程注入、Hook 稳定性风险；此前 360 HEUR/QVM 检测尚未获得厂商误报确认，勿为运行而禁用安全防护。
