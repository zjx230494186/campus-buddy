# Round 40 — Qt 低压力联系会话 UI + 广场合同小修

**日期**: 2026-05-24
**状态**: PASS
**基线提交**: `f45e352` → 待提交

## 目标

补齐会话列表/详情/发送消息 UI，修正广场 sceneType 枚举和详情 GET 合同问题。

## 修改清单

### A. 广场 UI 合同小修

**PlazaWidget sceneType 枚举修正**：
- 旧：STUDY/SPORT/TRAVEL/FOOD/OTHER
- 新：MEAL/STUDY/SPORT/COURSE_TEAM/INNOVATION_PROJECT（与后端枚举一致）

**PlazaWidget 详情 GET 接入**：
- 旧：onItemSelected 仅展示列表项摘要
- 新：调用 PartnerPostApiService::getPostDetail(postId)，展示标题、描述、发布者、认证状态、信用摘要、场景、时间、地点、目标要求、标签、ownPost
- 详情失败时显示 errorCode/errorMessage

**PostEditorWidget sceneType 同步修正**：
- 同步为 MEAL/STUDY/SPORT/COURSE_TEAM/INNOVATION_PROJECT

### B. ConversationsWidget (新增)

- 会话列表：刷新按钮 + QListWidget，展示对方昵称、关联发布标题、最后消息摘要、状态
- 选择会话后：调用 queryMessages(conversationId, 0, 50) 加载消息
- 消息列表：QListWidget 展示发送者、时间、内容摘要
- 发送消息：输入框 + 发送按钮，空消息给 UI 提示，发送成功后刷新消息
- 不做 WebSocket、定时轮询、未读数、标记已读

### C. HomePageWidget 集成

- 新增"会话" Tab（第 5 个 Tab）
- 顺序：认证、发布草稿、我的发布、广场、会话

### D. 测试更新

- StudentPostPlazaWidgetTest：新增 ConversationsWidget 文件清单 + 凭据检查
- CampusApiClientTest：新增 ConversationsWidget 文件清单

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

### student_post_plaza_widget_test (5/5 PASS)
- widgetLayerDoesNotDirectlyUseNetworkAccessManager（含 ConversationsWidget）
- postEditorWidgetSourceDoesNotContainHardcodedCredentials
- myPostsWidgetSourceDoesNotContainHardcodedCredentials
- plazaWidgetSourceDoesNotContainHardcodedCredentials
- conversationsWidgetSourceDoesNotContainHardcodedCredentials

### campus_buddy_desktop --smoke-test: exit 0

### Server Integration Smoke: 16/16 PASS

## 安全测试

- UI 层不直接使用 QNetworkAccessManager ✓
- 不硬编码密码/邮箱 ✓
- 消息内容仅在 UI 展示，不写入 validation 或日志 ✓

## 构建环境

- Build dir: `desktop/build-qt6103-round40`

## 未覆盖风险

- ConversationsWidget 无自动刷新/轮询，需用户手动刷新
- 消息发送后仅刷新当前会话，不推送新会话到列表
- 无 WebSocket 实时推送
- Widget 交互未做自动化功能测试
