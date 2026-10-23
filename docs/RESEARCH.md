# 研究经过：从 Win7 x64 到 Win7 / Win8.1 双架构（2026-10）

## 观察与假设

1. Windows 7 SP1 x64 上 Edge 109.0.1518.140 登录微软账户后，旧版发现端点 `/sync/v2/feeds/me/settings/homeCloud` 返回 HTTP 404，`edge://sync-internals/` 显示同步不可用。
2. 使用 `--sync-url=https://edge.microsoft.com/sync/v1/feeds/me/syncEntities` 可绕过发现端点错误，但服务器返回 `UPGRADE_CLIENT`，提示需要 Edge 133 或更高版本。
3. 仅修改一般的 User-Agent 不足以绕过限制；网络诊断发现独立请求头 `X-AFS-ClientInfo` 中有 `appVer=109.0.1518.140`。

## 逐步验证

- **Fiddler 研究阶段**：对指定同步 POST 请求的 `X-AFS-ClientInfo` 头进行运行时替换（`109.0.1518.140` -> `133.0.3065.92`），观察到同步引擎 `Active` 与收藏夹双向同步。该方案依赖 HTTPS 解密代理，仅用于概念验证，不作为发行方案。
- **静态定位阶段**：在 Edge 的 `msedge.dll` 发现请求头名称和格式字符串，进一步查找其代码交叉引用。对已验证构建号记录到 RVA `0x8AF3D95` 的 RIP-relative LEA（原始字节 `4C 8D 05 94 45 63 04`），引用 `X-AFS-ClientInfo`。相关字符串 RVA `0xD128330`。
- **x64dbg 动态阶段**：在 A 位置命中时，观察到已构造的 `platform=Windows; ...; appVer=109.0.1518.140; ...` 字段。对 `R14+0x50` 对应版本文本进行等长覆盖（`133.0.3065.120`），恢复收藏夹同步。随后使用自动断点命令重复覆盖，无需手动操作。
- **独立运行时阶段**：用 Windows x64 原生启动器与注入式 Hook DLL 代替调试器/代理，仅在版本、机器码及请求头内容匹配时覆盖字段。

## 版本演进

| 构建 | 观察 |
| --- | --- |
| `v0.1-test1` | 进程在 Win7 启动时报 `WaitForInputIdle` 导入错误；无法运行 |
| `v0.1-test2` | 初次登录同步成功；重启后出现 `INSTALLED` 但 `hits=0,patched=0`，同步失败，提示 `UPGRADE_CLIENT` |
| `v0.1-test3` | 移除固定 2.5 秒等待，尽早注入并显示 PID；用户观察到 `hits=3,patched=3,skipped=0`、`Active`、同步提交，随后反馈重启后仍能同步 |

## 后续实验：x86 与 Windows 8.1 x64

在完成最初的 Win7 x64 研究后，项目继续对同一 Edge **109.0.1518.140** 的其他环境研究：

| 环境 / 测试版本 | 观察 |
| --- | --- |
| Win7 x86 · v0.3-test1 | Hook 显示安装，但 `hits=0`、`patched=0`，仍收到 `UPGRADE_CLIENT` |
| Win7 x86 · v0.3-test2 | 三路径 Hook 实测 `installed_sites=3`、`hits=18`、`patched=18`、`skipped=0`；同步内部页显示服务器连接正常 |
| Win7 x86 · v0.3-test3 | 正常配置模式新增后，用户反馈同步成功 |
| Win8.1 x64 · v0.3-test5 | 已命中 Hook，但因版本字段位于 `base+0x45` 而非原有 `base+0x49`，未完成修改 |
| Win8.1 x64 · v0.3-test6 | 保留 `0x49` 并精确支持 `0x45`：`hits=14`、`patched=14`、`patch_at_0x45=14`，同步返回 `SYNCER_OK` |
| v0.3-test7 | 将上述既有 Hook 合入统一选择器，2026-10-24 已作为 [Pre-release](https://github.com/yuyu107/LegacyEdgeSync/releases/tag/v0.3-test7) 发布，**整包尚待跨系统回归** |

x86 和 x64 的请求头构造路径、指令编码和补丁入口不同，不能把一种架构的 DLL 直接用于另一种。Win8.1 x86 仍未验证。

## 结论的边界

现已分别观察到 Win7 x86、Win7 x64、Win8.1 x64 上对应实验 Hook 的成功结果。Win7 x64 曾实际验证收藏夹双向同步；Win7 x86 和 Win8.1 x64 的截图也记录了同步连接、修改计数与提交响应。**这些结果不能推广到所有账户、所有同步数据类型、Win8.1 x86、其他 Edge 构建或未来的服务端策略。v0.3-test7 统一整包仍待回归验证。**

目前对 test2 重启失败的“注入晚于自动同步请求”的归因与源代码时序高度吻合，但尚缺少精确时间线日志，因此是合理推测而非严格证明。test3 成功也不等于竞态已被完全消除。
