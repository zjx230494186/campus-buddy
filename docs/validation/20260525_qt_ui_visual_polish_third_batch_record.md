# Qt UI Visual Polish Third Batch Record

日期：2026-05-25

## 1. 本轮目标

延续 Qt UI 视觉打磨，第三批只收口主演示链路中的细节体验：

- 空状态文案一致化。
- 网络请求中的按钮加载态与防重复点击。
- 继续保持 Qt UI 层内聚，不改后端、Flyway、deploy 或服务器配置。

## 2. 实际修改文件

- `desktop/src/ui/UiHelpers.h`
- `desktop/src/ui/UiHelpers.cpp`
- `desktop/src/ui/IdentityVerificationWidget.cpp`
- `desktop/src/ui/MyPostsWidget.cpp`
- `desktop/src/ui/PlazaWidget.cpp`
- `desktop/src/ui/ConversationsWidget.cpp`
- `desktop/src/ui/AdminReviewWidget.cpp`
- `desktop/tests/StudentPostPlazaWidgetTest.cpp`
- `docs/validation/20260525_qt_ui_visual_polish_third_batch_record.md`
- `docs/03_current_plan.md`
- `handoff/latest.md`

## 3. 是否修改后端/Flyway/deploy

- 修改 `backend/**`：否
- 修改 Flyway migration：否
- 修改 `deploy/**`：否
- 修改服务器配置：否
- 修改后端 API 合同：否

## 4. 主要改造内容

### 共享 UI helper

- 新增 `UiHelpers::setButtonBusy`，统一网络请求中的按钮禁用与“处理中”文案。
- 新增 `UiHelpers::emptyStateText`，统一我的发布、广场、会话、消息、管理员审核队列的空状态文案。

### 主链路页面

- `IdentityVerificationWidget`：上传材料、提交认证期间按钮进入忙碌态，避免重复点击。
- `MyPostsWidget`：刷新、撤回审核、下架期间按钮进入忙碌态；空列表文案改走统一 helper。
- `PlazaWidget`：刷新广场、发起联系期间按钮进入忙碌态；筛选无结果时显示更明确的空状态。
- `ConversationsWidget`：刷新会话、发送消息、标记已读、关闭会话、保存联系方式、确认交换、查看对方联系方式均接入忙碌态；无会话和无消息时使用统一空状态。
- `AdminReviewWidget`：刷新发布审核队列、刷新认证审核队列、通过/驳回发布、通过/驳回认证均接入忙碌态；空审核队列使用统一空状态。

## 5. 测试与 Red/Green

新增测试：

- `StudentPostPlazaWidgetTest::thirdBatchUsesBusyAndEmptyStateHelpers`

红灯：

- 新增测试后，`student_post_plaza_widget_test` 失败。
- 失败原因：`UiHelpers` 尚未暴露 busy/empty helper，第三批目标页面尚未统一接入。

绿灯：

- 新增 helper 并接入目标页面后，`student_post_plaza_widget_test` 通过。

## 6. 构建结果

命令：

```powershell
cd D:\big_homework\desktop\build-qt6103-ui-polish
$env:Path = "G:\Qt\Tools\CMake_64\bin;G:\Qt\Tools\Ninja;G:\Qt\Tools\mingw1310_64\bin;G:\Qt\6.10.3\mingw_64\bin;$env:Path"
cmake --build D:\big_homework\desktop\build-qt6103-ui-polish
```

结果：通过。

## 7. Qt ctest 结果

目标测试命令：

```powershell
ctest -R student_post_plaza_widget_test --output-on-failure
```

结果：通过。

完整批量命令：

```powershell
ctest --output-on-failure -j4
ctest --output-on-failure -j1
```

结果：两次均出现 `0xc0000135`，表现为同一次 `ctest` 进程中只有第一个测试能启动，后续测试立即报 DLL 搜索/启动失败。

复核命令：

```powershell
$tests = @(
  'api_client_config_test',
  'campus_api_client_test',
  'auth_token_store_test',
  'server_smoke_security_test',
  'partner_post_api_service_test',
  'contact_conversation_api_service_test',
  'my_partner_post_api_service_test',
  'student_post_plaza_widget_test',
  'review_credit_api_service_test',
  'admin_review_api_service_test'
)
foreach ($t in $tests) {
  ctest -R "^$t$" --output-on-failure
}
```

结果：10/10 单独运行均通过。

结论：本批代码相关测试通过；当前环境存在同一 `ctest` 批量进程下的 DLL 搜索异常，需要后续环境线程单独排查，不把它记录为业务代码失败。

## 8. campus_buddy_desktop --smoke-test 结果

命令：

```powershell
.\campus_buddy_desktop.exe --smoke-test
```

结果：通过。

## 9. qt_server_integration_smoke 结果

命令：

```powershell
.\qt_server_integration_smoke.exe
```

结果：阻塞，当前 shell 未注入私有 smoke 账号环境变量：

- `CAMPUS_BUDDY_SMOKE_EMAIL`
- `CAMPUS_BUDDY_SMOKE_PASSWORD`
- `CAMPUS_BUDDY_SMOKE_ADMIN_EMAIL`
- `CAMPUS_BUDDY_SMOKE_ADMIN_PASSWORD`

该阻塞符合项目安全边界；本轮未写入真实账号、密码或 token。

## 10. 人工视觉检查结果

- 本批没有新增登录后真实数据截图。
- 原因：当前 shell 缺少私有 smoke 账号环境变量，避免为截图而在仓库或聊天中暴露账号、密码、token 或联系方式。
- 本批通过源码布局检查、构建、单测、逐项 ctest 和 desktop smoke 验证基础渲染链路未破坏。

## 11. 敏感信息检查结论

检查范围：

- `desktop/src/ui/*.cpp`
- `desktop/src/ui/*.h`
- `desktop/tests/StudentPostPlazaWidgetTest.cpp`

结论：

- 未新增真实 token、Authorization 值、OBS AK/SK、Secret、真实邮箱或真实联系方式。
- 本批新增内容为状态文案、helper 调用和测试源码路径字符串，不包含敏感值。

## 12. Git status 摘要

本轮相关改动集中在 Qt UI、Qt 测试、validation、current plan 和 handoff。

仓库仍存在前置未跟踪 `deploy/*.sh`、`tmp_*`、历史 prompts/docs 等文件。本轮不修改、不暂存、不提交这些前置脏文件。

## 13. 提交哈希

- 提交前基线：`49467d3`
- 本轮提交哈希：见最终回复中的 Git commit 结果。

## 14. 未覆盖风险

- `qt_server_integration_smoke` 未运行真实链路，原因是缺少私有账号环境变量。
- 本批没有真实登录后的页面截图。
- 同一 `ctest` 批量进程下的 `0xc0000135` 需要后续环境复核；逐项运行目前全部通过。
- 更细的窄窗口布局检查和字段级 inline error 仍可作为生产化收口事项。

## 15. 下一步建议

1. 若准备答辩演示，先在私有 shell 注入 smoke 账号环境变量，跑 `qt_server_integration_smoke` 并做登录后主链路截图。
2. 若继续生产化收口，优先处理同一 `ctest` 批量进程下的 DLL 搜索异常，避免后续验证记录反复出现歧义。
3. 继续 UI 细节时，只建议小范围做窄窗口响应式和字段级错误贴近输入框，不再扩大业务功能。
