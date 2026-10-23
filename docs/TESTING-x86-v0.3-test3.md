# Win7 x86 v0.3-test3 三语及正常配置回归清单

- [ ] Win7 简体系统自动显示简体中文；菜单和错误提示无乱码
- [ ] `--lang=zh-TW` 显示繁體中文，菜单选择子操作后保持该语言
- [ ] `--lang=en` 显示 English，菜单选择子操作后保持该语言
- [ ] `--test` 启动独立配置，`--status` 能显示三路径计数、PID 是否正在运行
- [ ] Edge 已在运行时拒绝启动第二个带 Hook 的 Edge
- [ ] `--normal` 未输入大写 `YES` 时不启动浏览器
- [ ] Edge 全部退出后，`--backup` 复制配置并且不覆盖已有备份文件夹
- [ ] `--normal` 在备份后使用原有配置，保持原有书签和登录态（仅测试账户）
- [ ] 测试收藏夹跨设备新增、修改、删除；重启 Win7 后再测试
- [ ] `installed_sites=3`、`patched>0` 与 `edge://sync-internals/` 服务连接正常

**不要用主力微软账户或真实密码/付款信息同步来做回归。**

## 已确认的上一轮 x86 结果

Windows 7 SP1 x86 / Edge 109.0.1518.140 x86 / LegacyEdgeSync v0.3-test2：
`installed_sites=3`、`hits=18`、`patched=18`、`skipped=0`、`setterB_patched=4`、`format_patched=14`。Edge 同步内部页显示 `Server Connection: OK`、`Last Synced: Just now` 并已产生 `Commit Request`。

**这些数值来自 v0.3-test2 实机截图，不能当作 v0.3-test3 的测试结果。** 还需独立确认新界面、新配置、备份及跨设备同步稳定性。
