# Round 39 — Qt 学生侧发布/广场/联系入口 UI

**日期**: 2026-05-24
**状态**: PASS
**基线提交**: `098902c` → 待提交

## 目标

实现可演示的 Qt 学生侧主链路 UI 一期：发布草稿、我的发布列表、广场列表/详情/发起联系。

## 新增 Widget

### PostEditorWidget (发布草稿)
- 表单字段：sceneType, title, description, timeMode, timeText, locationText, participantCount, targetRequirement, contactPreference, tags, scenePayload.studyGoal
- 按钮：保存草稿、更新草稿、提交审核（基于 allowedActions 动态启用）
- 支持从 MyPostsWidget 加载已有草稿（loadPost）

### MyPostsWidget (我的发布列表)
- 刷新按钮、列表控件、详情显示、编辑/撤回/下架按钮
- 选择条目后显示标题、状态、场景、allowedActions
- 编辑按钮 emit editPostRequested → 切到 PostEditorWidget Tab

### PlazaWidget (广场)
- 场景类型筛选、关键词搜索、刷新按钮
- 列表显示标题、发布者昵称、认证状态、信用星数、场景
- 选择条目后显示详情
- ownPost=false 时显示联系按钮和邀约消息输入框
- 发起联系成功后显示 conversationId

## 修改文件

### HomePageWidget — 集成 Tab 导航
- 4 个 Tab：认证、发布草稿、我的发布、广场
- 构造函数增加 MyPartnerPostApiService/PartnerPostApiService/ContactConversationApiService 参数
- onEditPostRequested → 切到发布草稿 Tab 并 loadPost

### main.cpp — 注入 API services
- 创建 MyPartnerPostApiService, PartnerPostApiService, ContactConversationApiService
- 传递给 HomePageWidget
- 窗口尺寸改为 960x720

### CMakeLists.txt — 新增 UI 源文件和测试 target

### CampusApiClientTest — 更新 widget layer 文件清单

## 测试结果

### ctest 全量（8/8 PASS）

| Target | 结果 |
|--------|------|
| api_client_config_test | PASS |
| campus_api_client_test | PASS |
| auth_token_store_test | PASS |
| server_smoke_security_test | PASS |
| partner_post_api_service_test | PASS |
| contact_conversation_api_service_test | PASS |
| my_partner_post_api_service_test | PASS |
| student_post_plaza_widget_test | PASS |

### student_post_plaza_widget_test (4/4 PASS)
- widgetLayerDoesNotDirectlyUseNetworkAccessManager
- postEditorWidgetSourceDoesNotContainHardcodedCredentials
- myPostsWidgetSourceDoesNotContainHardcodedCredentials
- plazaWidgetSourceDoesNotContainHardcodedCredentials

### campus_buddy_desktop --smoke-test
- exit code: 0

### Server Integration Smoke（16/16 PASS）

## 安全测试

- UI 层不直接使用 QNetworkAccessManager ✓
- 不硬编码密码/邮箱 ✓
- 错误提示只显示 errorCode/errorMessage ✓
- contactPreference 仅在编辑区显示 ✓

## 构建环境

- Build dir: `desktop/build-qt6103-round39`
- campus_buddy_desktop 构建成功 ✓
- 窗口默认 960x720

## 未覆盖风险

- Widget 交互未做自动化功能测试（需手动验证）
- PostEditorWidget 未对 tags 做严格格式校验
- PlazaWidget 未做详情页单独请求（仅展示列表项摘要）
- 无完整聊天 UI 入口
- 无评价 UI
