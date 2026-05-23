# Round 34 Validation — 低压力联系查询合同补强

## 本轮目标

修复 Round 33 遗留的两个查询合同问题：
1. 会话列表排序未生效
2. afterMessageId 增量拉取未受 size 上限保护

## 实际修改文件

- `backend/src/main/java/com/campusbuddy/contact/ContactConversationService.java` — listConversations 改用 sorted repository 查询; getMessages afterMessageId 分支改用 Pageable
- `backend/src/main/java/com/campusbuddy/contact/ConversationRepository.java` — 新增 findByParticipantOrderByUpdatedAtDesc
- `backend/src/main/java/com/campusbuddy/contact/ConversationMessageRepository.java` — 新增 Pageable 版本 findByConversationIdAndIdGreaterThanOrderByIdAsc
- `backend/src/test/java/com/campusbuddy/contact/ContactConversationEndpointTest.java` — 新增 3 个测试

## 是否新增 Flyway 迁移

否

## 红灯测试

- 命令: `mvn test -Dtest="ContactConversationEndpointTest#conversationListSortedByUpdatedAtDesc+messagesAfterMessageIdRespectsSizeLimit+messagesAfterMessageIdCapsSizeAtMaxPageSize"`
- 结果: 3 tests, 2 failures, 1 passed
- 原因: 排序未生效(items 按 A/B/C 返回而非 C/B/A); afterMessageId 无 size 上限(返回 55 条超过 50)

## 绿灯测试

- 命令: `mvn test`
- 结果: 225/225 passed

## 新增测试覆盖

1. conversationListSortedByUpdatedAtDesc — 3 条不同 updatedAt 会话，断言严格 C/B/A 顺序
2. messagesAfterMessageIdRespectsSizeLimit — afterMessageId + size=2，返回不超过 2 条
3. messagesAfterMessageIdCapsSizeAtMaxPageSize — size=999，返回不超过 50 条

## contact 包回归

全部通过

## post/review 关键回归

225/225 全量通过，无破坏

## 后端全量回归

225/225 passed

## 服务器部署/重启

active(running)

## health check

200 UP

## Flyway validated

V1-V9，无新迁移

## 会话列表排序 smoke

ConvList 返回包含 conversationId, updatedAt, lastMessagePreview，且最新操作的会话排在首位

## afterMessageId 分页保护 smoke

- afterMessageId + size=1 → count=1 (<=1)
- afterMessageId + size=2 → count=2 (<=2)

## 敏感字段检查结论

会话列表响应不含 campusEmail、studentNumber、realName、passwordHash 等敏感字段

## 修复详情

### 1. 会话列表排序

- 旧: `findByParticipant1IdOrParticipant2Id` 无排序 + Java 内存排序未赋回
- 新: `findByParticipantOrderByUpdatedAtDesc` JPQL 查询直接 ORDER BY updatedAt DESC

### 2. afterMessageId 分页保护

- 旧: `findByConversationIdAndIdGreaterThanOrderByIdAsc(convId, afterMessageId)` 返回 List 无限制
- 新: `findByConversationIdAndIdGreaterThanOrderByIdAsc(convId, afterMessageId, pageable)` 返回 Page，受 size 和 MAX_PAGE_SIZE=50 限制

## 未覆盖风险

- CLOSED 会话重开仍未实现
- 消息长度上限 30 字符较保守
- Qt 客户端尚未对接
- WebSocket 实时推送未实现

## 下一轮建议

1. Qt 客户端联系/消息 UI 对接
2. CLOSED 会话重开状态机
3. 消息长度上限放宽
