# Qt UI Visual Polish Second Batch Record

日期：2026-05-25

## 1. 本轮目标

延续第一批 Qt UI 视觉打磨，继续处理仍像调试面板的三个页面：

- `IdentityVerificationWidget`
- `MyPostsWidget`
- `AdminReviewWidget`

范围仍限定在 Qt 桌面端 UI、必要测试和 validation/handoff，不修改后端、Flyway、deploy 或服务器配置。

## 2. 实际修改文件

- `desktop/src/ui/IdentityVerificationWidget.cpp`
- `desktop/src/ui/MyPostsWidget.cpp`
- `desktop/src/ui/AdminReviewWidget.cpp`
- `desktop/src/ui/AdminReviewWidget.h`
- `desktop/tests/StudentPostPlazaWidgetTest.cpp`
- `docs/validation/20260525_qt_ui_visual_polish_second_batch_record.md`
- `docs/validation/20260525_qt_ui_visual_polish_second_batch_login_screenshot.png`
- `docs/03_current_plan.md`
- `handoff/latest.md`

## 3. 是否修改后端/Flyway/deploy

- 修改 `backend/**`：否
- 修改 Flyway migration：否
- 修改 `deploy/**`：否
- 修改服务器配置：否
- 修改后端 API 合同：否

## 4. 改造页面列表

### IdentityVerificationWidget

- 接入 `UiHelpers`。
- 改为页头 + 基本信息卡片 + 认证材料卡片 + 状态提示。
- 文件选择、上传材料、提交认证按钮做主次区分。
- 驳回原因和操作状态使用统一状态标签样式。

### MyPostsWidget

- 接入 `UiHelpers`。
- 增加页头说明。
- 改为发布列表 / 详情与操作左右分栏。
- 列表项展示标题、中文状态、中文场景。
- 详情区用富文本层级展示 ID、时间、地点、可用操作。
- 空列表提示改成可理解的空状态文案。

### AdminReviewWidget

- 接入 `UiHelpers`。
- 增加页头说明。
- 发布审核和认证审核都改为队列 / 详情左右分栏。
- 通过按钮使用 primary，驳回按钮使用 danger。
- 发布审核队列保存当前列表项，不再选中时重复拉队列。
- 修复认证审核队列选择处理：新增 `onIdentityQueueItemClicked` 和 `identityQueueItems_`，选中认证申请后才能通过/驳回。

## 5. 测试与 Red/Green

新增测试：

- `StudentPostPlazaWidgetTest::secondBatchUiUsesSharedVisualHelpersAndIdentitySelection`

红灯：

- 新增测试后，`student_post_plaza_widget_test` 失败。
- 失败原因：第二批页面未全部接入 `UiHelpers`，管理员认证审核缺少 identity queue 选择 handler 和列表项缓存。

绿灯：

- 接入 helper 并补 `onIdentityQueueItemClicked` / `identityQueueItems_` 后，目标测试通过。

## 6. 构建结果

命令：

```powershell
cd D:\big_homework\desktop
$env:Path = "G:\Qt\Tools\CMake_64\bin;G:\Qt\Tools\Ninja;G:\Qt\Tools\mingw1310_64\bin;G:\Qt\6.10.3\mingw_64\bin;$env:Path"
& "G:\Qt\Tools\CMake_64\bin\cmake.exe" --build build-qt6103-ui-polish
```

结果：通过。

## 7. Qt ctest 结果

命令：

```powershell
cd D:\big_homework\desktop\build-qt6103-ui-polish
ctest --output-on-failure -j4
```

结果：10/10 PASS。

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

- 启动桌面端到登录页并截图：`docs/validation/20260525_qt_ui_visual_polish_second_batch_login_screenshot.png`
- 因没有 smoke 账号环境变量，本轮没有登录进入真实后端数据页面截图。
- 第二批页面通过源码布局、构建、ctest 和 desktop smoke 验证基础渲染链路未破坏。

## 11. 敏感信息检查结论

检查范围：

- `desktop/src/ui/*.cpp`
- `desktop/src/ui/*.h`
- `desktop/tests/StudentPostPlazaWidgetTest.cpp`

结论：

- 未新增真实 token、Authorization 值、OBS AK/SK、Secret、真实邮箱或真实联系方式。
- 命中项仅为局部变量名 `password`、测试中的安全扫描正则和源码路径字符串，不是具体敏感值。

## 12. Git status 摘要

本轮相关改动集中在 Qt UI、Qt 测试、validation、current plan 和 handoff。

仓库仍存在前置未跟踪 `deploy/*.sh`、`tmp_*`、历史 prompts/docs 等文件。本轮不修改、不暂存、不提交这些前置脏文件。

## 13. 提交哈希

- 提交前基线：`230ea93`
- 本轮提交哈希：见最终回复中的 Git commit 结果。

## 14. 未覆盖风险

- `qt_server_integration_smoke` 未运行真实链路，原因是缺少私有账号环境变量。
- 第二批没有做真实登录后的页面截图。
- `AdminReviewWidget` 视觉已分区，但身份认证材料内容预览仍未做，这是既有范围外边界。
- 更细的字段级错误贴近输入框、弱网 loading、窗口尺寸响应式验证仍可继续做第三批。

## 15. 下一批 UI 建议

1. 有 smoke 账号环境变量后，补登录后主链路截图：认证、我的发布、管理员审核、广场、会话。
2. 第三批做空状态/加载态/禁用态一致化，尤其是网络请求中的按钮防重复。
3. 做小窗口尺寸人工检查，确认左右分栏不会在较窄窗口中挤压严重。
