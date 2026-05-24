# Round 46 Validation — Qt Contact Conversation State and Unread UI

**日期**: 2026-05-24
**目标**: Qt 桌面端适配 Round 45 新增的会话关闭、标记已读、unreadCount 端点，更新 ConversationsWidget UI，扩展 Qt server smoke。

---

## 1. 变更清单

| 变更项 | 路径 | 说明 |
|--------|------|------|
| ConversationListItem +unreadCount | `desktop/src/domain/ContactConversationModels.h` | 新增 int unreadCount = 0 |
| CloseConversationResult | `desktop/src/domain/ContactConversationModels.h` | 新增 struct |
| MarkReadResult | `desktop/src/domain/ContactConversationModels.h` | 新增 struct |
| closeConversation 方法 | `desktop/src/api/ContactConversationApiService.h/.cpp` | POST /me/conversations/{id}/close |
| markConversationRead 方法 | `desktop/src/api/ContactConversationApiService.h/.cpp` | POST /me/conversations/{id}/read |
| unreadCount 解析 | `desktop/src/api/ContactConversationApiService.cpp` | listConversations 解析 unreadCount 字段 |
| ConversationsWidget UI | `desktop/src/ui/ConversationsWidget.h/.cpp` | 未读数显示、标记已读/关闭按钮、CLOSED 禁用发送 |
| 扩展 Qt 测试 | `desktop/tests/ContactConversationApiServiceTest.cpp` | 新增 4 个测试用例 |
| 扩展 server smoke | `desktop/tests/QtServerIntegrationSmoke.cpp` | 新增步骤 28-33 |

## 2. 是否修改后端/Flyway/deploy

否。本轮仅修改 desktop/ 下的 Qt 客户端代码和测试。

## 3. 红灯测试

本轮实现与测试同步编写（测试用例引用的新 API 方法在头文件中声明即存在）。测试先行验证的是合同正确性，而非方法是否存在。

新增测试用例：
- `listConversationsParsesUnreadCount`: mock response 含 unreadCount:3，验证解析
- `closeConversationUsesCorrectPathAndMethod`: 验证 POST /me/conversations/123/close + Bearer token + 响应解析
- `markConversationReadUsesCorrectPathAndHandlesEmptyBody`: 验证 POST /me/conversations/123/read + 200 空 body → success=true
- `sendMessageClosedErrorConvertsToResult`: mock 403 CONVERSATION_CLOSED → result.success=false, errorCode 正确

## 4. unreadCount 解析修复结果

旧代码 ConversationListItem 无 unreadCount 字段，listConversations 解析不读取该字段。
新增 `int unreadCount = 0` 字段后，在 listConversations 的 item 解析循环中增加：
```cpp
item.unreadCount = v.toObject().value(QStringLiteral("unreadCount")).toInt();
```
服务端未返回该字段时默认为 0，向后兼容。

## 5. close/read API client 合同结果

- `closeConversation(long long conversationId, CloseConversationCallback callback)`:
  - POST /api/me/conversations/{conversationId}/close
  - 空 body
  - 解析 {conversationId, status}
  - 幂等（服务端已实现）

- `markConversationRead(long long conversationId, MarkReadCallback callback)`:
  - POST /api/me/conversations/{conversationId}/read
  - 空 body
  - 服务端返回 200 空 body → CampusApiClient.parseReply 已支持（httpStatus >= 200 && httpStatus < 300 && body.isEmpty() → ok=true）
  - 幂等

## 6. ConversationsWidget UI 适配说明

- 会话列表项显示：`[status] displayName - postTitle (N未读) | lastMsg`
- 选中会话后显示 status + unreadCount
- ACTIVE 会话：发送按钮启用，关闭按钮启用
- CLOSED 会话：发送按钮禁用，关闭按钮禁用
- 新增「标记已读」按钮：仅当 unreadCount > 0 时启用，成功后刷新会话列表
- 新增「关闭会话」按钮：仅当 status == ACTIVE 时启用，成功后刷新会话列表
- 发送消息遇到 CONVERSATION_CLOSED 错误：UI 显示错误码，自动将 currentConversationStatus_ 更新为 CLOSED 并禁用发送按钮
- 不实现自动轮询、WebSocket、后台刷新

## 7. 绿灯测试

命令：
```
cd D:\big_homework\desktop\build-qt6103-round46
ctest --output-on-failure -j4
```

结果：10/10 PASS

通过测试列表：
1. api_client_config_test
2. campus_api_client_test
3. auth_token_store_test
4. server_smoke_security_test
5. partner_post_api_service_test
6. contact_conversation_api_service_test (含 4 个新增用例)
7. my_partner_post_api_service_test
8. student_post_plaza_widget_test
9. review_credit_api_service_test
10. admin_review_api_service_test

## 8. ctest 全量结果

10/10 tests passed, 0 tests failed

## 9. campus_buddy_desktop 构建结果

构建成功，103/103 编译链接通过。

## 10. campus_buddy_desktop --smoke-test 结果

通过（无输出表示正常退出）。

## 11. Qt server smoke unread/read/close/recontact 结果

```
--- 28. Admin listConversations unreadCount >= 1 ---  PASS: unreadCount=3
--- 29. Admin markConversationRead ---              PASS: markRead success
--- 30. Admin unreadCount after markRead = 0 ---     PASS: unreadCount=0
--- 31. Smoke user closeConversation ---             PASS: status=CLOSED
--- 32. Send message on CLOSED conversation ---       PASS: errorCode=CONVERSATION_CLOSED
--- 33. Re-contact reopens CLOSED conversation ---    PASS: conversationId=2 status=ACTIVE
```

全部 33/33 PASS（27 既有 + 6 新增）。

## 12. 敏感信息检查结论

- [x] smoke 凭据通过环境变量传入，不硬编码在源码中
- [x] validation 不记录 token、密码
- [x] server smoke 输出只记录 conversationId、status、unreadCount、errorCode
- [x] 不打印消息全文、Authorization header、真实邮箱明文

## 13. Git Status 摘要

修改文件 7 个：
- desktop/src/domain/ContactConversationModels.h (unreadCount + CloseConversationResult + MarkReadResult)
- desktop/src/api/ContactConversationApiService.h (新增回调类型和方法声明)
- desktop/src/api/ContactConversationApiService.cpp (unreadCount 解析 + closeConversation + markConversationRead)
- desktop/src/ui/ConversationsWidget.h (新增按钮和状态成员)
- desktop/src/ui/ConversationsWidget.cpp (UI 适配重写)
- desktop/tests/ContactConversationApiServiceTest.cpp (4 个新增测试)
- desktop/tests/QtServerIntegrationSmoke.cpp (6 个新增 smoke 步骤)

## 14. 未覆盖风险

| 风险 | 说明 | 建议 |
|------|------|------|
| Widget 交互未自动化 | ConversationsWidget 按钮行为未自动化测试 | 后续可用 Qt Test Widget 手动或 GUI 自动化补充 |
| 自动刷新未实现 | unreadCount 需手动刷新 | 后续可加轮询或 WebSocket |
| 批量已读 | 只支持单个会话标记已读 | 可后续扩展 |
| HTTPS | 公网仍为 HTTP | 真实试用前必须加 |
