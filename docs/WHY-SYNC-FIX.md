# 为什么要修复 Microsoft Edge 的云同步？

> **这不是“Edge 109 无法登录微软账户”的问题，而是“旧版 Edge 可以登录，却无法再与当前云同步服务正常通信”的问题。** 本页专门介绍 LegacyEdgeSync 项目的背景、实际发现与修复边界。
>
 > 本文以 **2026 年 10 月 Windows 7 SP1 x64 + Microsoft Edge 109.0.1518.140 x64** 的最初研究为基础，之后已补充 Win7 x86 与 Win8.1 x64 的成功实验。微软没有在下列官方资料中宣布“某天统一关闭了 Edge 109 所有账户的同步”；不要将我们的测试现象理解成一份官方停服公告。

## 1. 为什么还有人在使用 Edge 109？

微软已经结束对 Windows 7 和 Windows 8.1 的 Edge 新版本支持。**Edge 109 是最后一个正式支持这些系统的主版本**；Edge 110 起需要 Windows 10 或更高系统。微软在 2022 年 12 月公布了相关计划，Edge 109 在 2023 年 2 月之后不再作为 Windows 7 / 8.1 的受支持浏览器版本。

- [微软官方公告：结束 Windows 7 / 8.1 的 Edge 支持](https://blogs.windows.com/msedgedev/2022/12/09/microsoft-edge-and-webview2-ending-support-for-windows-7-and-windows-8-8-1/)
- [微软 Edge 生命周期页面](https://learn.microsoft.com/en-us/lifecycle/products/microsoft-edge)

在需要保留旧操作系统的测试机、虚拟机或软件兼容性研究环境中，仍可能使用 Edge 109。**“浏览器能够启动、能够打开网页”不等于“所有在线服务仍然兼容”**。云同步依赖的是微软运营的服务器和接口，即使本地浏览器没有变化，服务端也可能更新。

## 2. 用户能看到什么问题？

最容易误解的地方是：**微软账户登录与浏览器数据云同步不是同一件事。**

Edge 109 可能仍显示已经登录微软账户，收藏夹也能在本地正常使用；但是从另一台设备添加或修改收藏夹时，这台旧版 Edge 却没有收到变化，或者本机新增的收藏夹无法上传。

可以在地址栏输入：

```text
edge://sync-internals/
```

查看浏览器自身记录的同步状态。例如在研究过程中，曾观察到：

| 项目 | 出现过的情况 |
| --- | --- |
| Transport State | `Disabled` |
| 同步发现阶段 | 请求返回 HTTP `404` |
| 尝试绕过发现问题后 | `UPGRADE_CLIENT` |
| 服务端提示 | `Client version not supported. Please upgrade to version 133 or later.` |

这些内容比“头像没有显示”或“账号显示已登录”更能说明同步引擎究竟有没有工作。

## 3. 研究中发现了哪两个障碍？

以下是**项目在指定测试环境中的实测结果**，并非微软发布的全局服务政策。

### 障碍 A：旧的同步地址发现流程出错

研究发现，Edge 109 在请求以下旧发现地址时返回了 `404`：

```text
https://edge.microsoft.com/sync/v2/feeds/me/settings/homeCloud
```

这使得 Edge 无法正常获取后续同步服务地址，出现等待同步 URL 或传输状态关闭的情况。

使用 Edge 内置的 `--sync-url` 命令行参数指定在测试中可用的地址：

```text
https://edge.microsoft.com/sync/v1/feeds/me/syncEntities
```

可以跳过上述发现错误。然而，**只解决地址问题还不足以完成同步**。

### 障碍 B：服务器拒绝旧版客户端版本

指定同步 URL 之后，服务器仍曾返回：

```text
UPGRADE_CLIENT
Client version not supported. Please upgrade to version 133 or later.
```

从网络调试信息中发现，Edge 109 的同步请求会使用专门的 `X-AFS-ClientInfo` 请求头上报客户端信息，其中包括：

```text
appVer=109.0.1518.140
```

仅修改浏览器常见的 `User-Agent` 字符串并不能改变这里的 `appVer`，所以常见的 User-Agent 伪装办法在这次测试中无效。

## 4. LegacyEdgeSync 做了什么？

项目的实验方案分为两部分：

1. **指定经测试可用的同步服务地址**，避免走已经返回 `404` 的旧发现流程。
2. **在正在运行的 Edge 进程内，按条件替换同步请求头中的特定版本字段**，避免当前测试环境中触发的服务端旧版限制。

开发过程经历了代理修改请求头、x64dbg 内存断点验证，以及不依赖代理或调试器的原生运行时 Hook。后来还增加了使用原有 Edge 配置文件的启动模式、简体中文／繁體中文／English 界面，以及按 Edge 文件架构自动选择 x86/x64 修复器的统一入口。

修复前后的一个简单对比：

| 项目 | 未修复的实测情况 | 应用实验补丁后的实测情况 |
| --- | --- | --- |
| 同步服务发现 | `404` / 等待同步地址 | 指定同步 URL |
| 客户端版本检查 | `UPGRADE_CLIENT` | 已观察到正常连接 |
| 同步引擎 | `Transport State: Disabled` | `Transport State: Active` |
| 收藏夹变化 | 无法可靠跨设备传达 | 已在 Win7 和 Win11 间成功双向同步 |

这并不是修改微软账户本身、破解用户密码，或把 Edge 109 真正升级成 Edge 133。它只是**在具体构建的具体请求路径上进行兼容性实验**。

## 5. 为什么不直接升级 Edge？

在受支持的操作系统上，**直接使用仍然接受安全更新的新版 Edge 才是推荐方案**。本项目是为仍需运行 Windows 7 的测试和兼容性研究场景准备的，不应被当成新版浏览器的替代品。

尤其需要区分：**恢复同步能力 ≠ 获得新版浏览器的漏洞修复**。Edge 109 本身已经停止常规安全维护。

## 6. 如何判断是否真的修好了？

请不要只看“微软账户已登录”或工具输出的 `INSTALLED`。需要结合以下两种证据：

- 在 `edge://sync-internals/` 中检查 `Transport State: Active`、`Disable Reasons: None`，以及真实的服务器连接和近期同步记录。
- 用**专门的测试收藏夹**，在两台设备上互相新建、修改和同步，确认实际数据发生变化。

在已经记录的 Windows 7 实测中，实验 Hook 曾输出 `hits=3, patched=3, skipped=0`，并观察到收藏夹双向同步，以及一次系统重启后的再次同步。

## 7. 项目尚不能解决什么？

- **不是所有旧版 Edge 通用补丁。** 已分别在 Windows 7 x86/x64、Windows 8.1 x64 的指定 Edge `109.0.1518.140` 构建实验成功，**但 v0.3-test7 整合版仍未完成跨系统回归**；Windows 8.1 x86 和其他 Edge 构建尚未验证。
- **不保证未来一直有效。** 云服务的接口或版本要求可能再次变化。
- **不等于所有数据类型都能安全同步。** 密码和支付信息等敏感内容不建议使用该实验工具同步。
- **用户头像显示异常是另一个问题。** 该问题已记录，但与同步引擎恢复是两个独立的研究方向。
- **注入式 Hook 仍有风险。** 包括启动时序、多线程竞态与潜在崩溃；暂不适合生产环境。

如果你的系统可以升级，优先使用受支持的操作系统与浏览器。若必须保留 Windows 7，请在有快照的虚拟机中使用测试账户进行研究。

---

返回：[项目首页](../README.md) · [研究过程](RESEARCH.md) · [测试方法](TESTING.md) · [实验版下载](https://github.com/yuyu107/LegacyEdgeSync/releases)

当前下载：[v0.3-test7 实验预发布版](https://github.com/yuyu107/LegacyEdgeSync/releases/tag/v0.3-test7)。其整合版回归测试仍在进行中。

*LegacyEdgeSync 为非官方、独立的兼容性研究项目，与 Microsoft 无关。*
