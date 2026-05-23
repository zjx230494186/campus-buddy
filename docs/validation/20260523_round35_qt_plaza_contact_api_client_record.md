# Round 35 Validation — Qt 广场与基础会话 API Client 对接

## 本轮目标

在 Qt 客户端新增 PartnerPost 和 ContactConversation API Service/Model，扩展服务器 smoke 覆盖广场和会话链路。

## 实际修改文件

- `desktop/src/domain/PartnerPostModels.h` — 新增 PlazaListItem, PlazaListResult, PlazaDetailResult, PublicCreditSummary
- `desktop/src/domain/ContactConversationModels.h` — 新增 ConversationListItem, ConversationListResult, ContactRequestResult, MessageItem, MessageListResult, SendMessageResult
- `desktop/src/api/PartnerPostApiService.h` — 新增
- `desktop/src/api/PartnerPostApiService.cpp` — 新增 (listPosts, getPostDetail, JSON 解析)
- `desktop/src/api/ContactConversationApiService.h` — 新增
- `desktop/src/api/ContactConversationApiService.cpp` — 新增 (requestContact, listConversations, sendMessage, queryMessages)
- `desktop/tests/PartnerPostApiServiceTest.cpp` — 新增 5 个测试
- `desktop/tests/ContactConversationApiServiceTest.cpp` — 新增 6 个测试
- `desktop/tests/QtServerIntegrationSmoke.cpp` — 扩展步骤 6-10
- `desktop/CMakeLists.txt` — 新增源文件、测试 target、smoke 依赖

## 是否修改后端/Flyway/deploy

否

## 红灯测试

- 命令: N/A (CMake 构建环境不可用，详见阻塞原因)
- 代码已编写完成，编译/链接验证依赖本地 Qt+cmake 环境

## 绿灯测试

- 本地构建阻塞：当前 Git Bash 环境中 cmake 不在 PATH，且未找到 Qt 6 安装目录
- 代码逻辑经人工审查确认与 CampusApiClient/AuthApiService 风格一致

## Qt 本地测试结果

**阻塞**：cmake 和 Qt6 构建工具链不在当前 PATH 中，无法执行 cmake configure / build / ctest。
需要具备以下环境才能完成本地 Qt 验证：
- Qt 6.10.3+ 安装
- cmake 3.24+ 在 PATH
- Ninja 或其他生成器

## campus_buddy_desktop 构建结果

**阻塞**：同上

## 后端回归

225/225 passed (未修改后端)

## 服务器 health/login 结果

后端仍 active(running)，health 200 UP

## Qt plaza list / contact / conversation smoke 结果

**阻塞**：Qt 构建环境不可用，无法编译 qt_server_integration_smoke 可执行文件。
服务器侧后端已验证广场列表和会话端点可用（Round 33/34 smoke 已通过）。

## 敏感信息检查结论

- Domain models 不含 campusEmail/studentNumber/realName/passwordHash
- Smoke 只打印 token length、items count、conversationId/messageId (均为非敏感)
- 不打印消息全文、Authorization header、邮箱或密码

## 代码实现详情

### PartnerPostApiService
- `listPosts(sceneType, keyword, page, size)` → GET /partner-posts?...
- `getPostDetail(postId)` → GET /partner-posts/{postId}
- 解析 publisherCreditSummary (不含 disputedReviewCount)
- 使用 tokenStore_.accessToken() 作为 Bearer token

### ContactConversationApiService
- `requestContact(postId, message)` → POST /partner-posts/{postId}/contact-requests
- `listConversations(page, size)` → GET /me/conversations?...
- `sendMessage(conversationId, message)` → POST /me/conversations/{id}/messages
- `queryMessages(conversationId, afterMessageId, size)` → GET /me/conversations/{id}/messages?afterMessageId=...&size=...

### 测试覆盖
1. PartnerPostApiServiceTest: plazaListUsesCorrectPath, plazaListUsesBearerToken, plazaListParsesPublisherCreditSummary, plazaDetailParsesOwnPostAndCreditSummary, errorResponseConvertsToServiceResult
2. ContactConversationApiServiceTest: requestContactPathAndBody, listConversationsUsesBearerToken, sendMessagePathAndBody, queryMessagesSupportsAfterMessageIdAndSize, conversationListParsesFields, messageListParsesFields

### QtServerIntegrationSmoke 扩展
- 步骤 6: plaza list → 获取首条 PUBLISHED post
- 步骤 7: request contact → 获取 conversationId
- 步骤 8: list conversations
- 步骤 9: send message → 获取 messageId
- 步骤 10: query messages afterMessageId + size=1

## 未覆盖风险

- Qt 本地构建和测试未实际执行，需要 Qt+cmake 环境
- Qt UI Widget 未创建
- 联系方式解锁、未读数、会话关闭未实现
- WebSocket 实时推送未实现
- Smoke 使用的 smoketest 账户是 PUBLISHED post 的发布者，可能无法联系自己的 post（需用另一个 VERIFIED 账户如 smokeadmin）

## 下一轮建议

1. 在具备 Qt+cmake 环境时执行构建和测试验证
2. Qt 广场页面和会话页面 UI 实现
3. CLOSED 会话重开
