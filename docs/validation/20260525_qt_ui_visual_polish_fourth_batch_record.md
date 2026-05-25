# Qt UI Visual Polish Fourth Batch Record

日期：2026-05-25

## 1. 本轮目标

第四批继续做 Qt 桌面端 UI 细节体验收口，重点处理答辩现场容易暴露的两个问题：

- 发布表单校验失败时，错误只堆在底部状态栏，用户很难知道该改哪个字段。
- 评价与信用页面请求按钮缺少加载态，评价列表为空时反馈不够明确。

范围仍限定在 Qt UI、必要测试和 validation/handoff/current plan，不修改后端、Flyway、deploy、服务器配置或 API 合同。

## 2. 实际修改文件

- `desktop/src/ui/AppStyles.cpp`
- `desktop/src/ui/UiHelpers.cpp`
- `desktop/src/ui/PostEditorWidget.cpp`
- `desktop/src/ui/PostEditorWidget.h`
- `desktop/src/ui/ReviewCreditWidget.cpp`
- `desktop/tests/StudentPostPlazaWidgetTest.cpp`
- `docs/validation/20260525_qt_ui_visual_polish_fourth_batch_record.md`
- `docs/03_current_plan.md`
- `handoff/latest.md`

## 3. 是否修改后端/Flyway/deploy

- 修改 `backend/**`：否
- 修改 Flyway migration：否
- 修改 `deploy/**`：否
- 修改服务器配置：否
- 修改后端 API 合同：否

## 4. 主要改造内容

### 发布表单字段级错误

- `PostEditorWidget` 新增字段级错误标签。
- `title`、`description`、`timeText`、`locationText`、`targetRequirement`、`contactPreference` 和 `scenePayload.*` 校验错误会显示在对应输入框附近。
- 保存草稿、更新草稿、提交审核前会先清理旧错误。
- 后端返回的字段名会映射为更接近用户语言的中文标签。
- 保留底部状态栏汇总错误，方便快速扫一眼全局状态。

### 发布表单加载态

- 保存草稿、更新草稿、提交审核接入 `UiHelpers::setButtonBusy`。
- 请求中按钮禁用并显示“保存中... / 更新中... / 提交中...”，降低重复点击风险。

### 评价与信用页面体验

- `ReviewCreditWidget` 内容区改为可滚动区域，避免较矮窗口下内容挤出底部。
- 刷新信用摘要、提交评价、修改评价、刷新已发出、刷新已收到均接入按钮忙碌态。
- 已发出评价、已收到评价为空时使用统一 `UiHelpers::emptyStateText` 文案。

### 样式

- `AppStyles` 新增 `QLabel[error="true"]` 字段级错误样式，使用克制红色，不改变整体浅色校园工具风。
- `UiHelpers::emptyStateText` 补充 `givenReviews` 和 `receivedReviews` 场景。

## 5. 测试与 Red/Green

新增测试：

- `StudentPostPlazaWidgetTest::fourthBatchUsesFieldErrorsAndReviewBusyStates`

红灯：

- 新增测试后，`student_post_plaza_widget_test` 失败。
- 失败原因：发布表单尚未暴露字段级错误处理，评价页尚未统一接入 busy/empty helper。

绿灯：

- 新增字段级错误标签、忙碌态和空状态后，目标测试通过。

## 6. 构建结果

命令：

```powershell
cd D:\big_homework\desktop
$env:Path = "G:\Qt\Tools\CMake_64\bin;G:\Qt\Tools\Ninja;G:\Qt\Tools\mingw1310_64\bin;G:\Qt\6.10.3\mingw_64\bin;$env:Path"
cmake --build D:\big_homework\desktop\build-qt6103-ui-polish
```

结果：通过。

## 7. Qt ctest 结果

命令：

```powershell
cd D:\big_homework\desktop\build-qt6103-ui-polish
ctest --output-on-failure -j4
```

结果：10/10 PASS。

说明：第三批记录中的同一 `ctest` 批量进程 `0xc0000135` 异常本轮未复现。

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

本轮未在聊天、文档或代码中写入真实账号、密码或 token。

## 10. 人工视觉检查结果

- 本轮没有登录真实后端页面截图。
- 原因：当前 shell 缺少私有 smoke 账号环境变量，避免为截图引入凭据暴露风险。
- 通过源码布局、构建、ctest 和 desktop smoke 验证基础渲染链路未破坏。

## 11. 敏感信息检查结论

检查范围：

- `desktop/src/ui/*.cpp`
- `desktop/src/ui/*.h`
- `desktop/tests/*.cpp`
- `docs/03_current_plan.md`
- `handoff/latest.md`

结论：

- 未新增真实 token、Authorization 值、OBS AK/SK、Secret、真实邮箱或真实联系方式。
- 命中项为既有测试假 token/假密码、测试正则以及文档中的敏感配置名称说明，不是具体敏感值。

## 12. Git status 摘要

本轮相关改动集中在 Qt UI、Qt 测试、validation、current plan 和 handoff。

仓库仍存在前置未跟踪 `deploy/*.sh`、`tmp_*`、历史 prompts/docs 等文件。本轮不修改、不暂存、不提交这些前置脏文件。

## 13. 提交哈希

- 提交前基线：`7b64398`
- 本轮提交哈希：见最终回复中的 Git commit 结果。

## 14. 未覆盖风险

- `qt_server_integration_smoke` 未运行真实链路，原因是缺少私有账号环境变量。
- 字段级错误目前覆盖发布表单的主要字段；更多复杂后端错误仍会落到底部状态栏汇总。
- 本轮没有窄窗口截图；评价页已改滚动容器，但还缺真实 GUI 截图证据。

## 15. 下一步建议

1. 有私有 smoke 账号环境变量后，补登录后主链路截图，尤其是发布表单校验错误和评价页空列表。
2. 若继续 UI 收口，优先做窄窗口尺寸人工检查和按钮文本压缩，而不是扩展新业务功能。
3. 若准备答辩，建议转入最终验证线程，按演示清单跑一遍真实链路。
