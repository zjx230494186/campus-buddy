# Round 42 — Qt 评价信用合同与 Smoke 补强 Validation Record

日期: 2026-05-24
轮次: Round 42 (Round 41 质量收口)
范围: topTags tagName 解析修复、永真断言删除、预设标签对齐、评价 smoke 链路真实 PASS

## 1. 红灯阶段

修改 `ReviewCreditApiServiceTest` mock response 使用 `tagName` 字段（后端真实合同），替换永真断言。

红灯命令:
```
cd desktop/build-qt6103-round42 && ctest -R review_credit --output-on-failure
```

红灯结果: 1/1 test failed (2 个用例失败)
- `getMyCreditSummaryParsesDisputedReviewCount`: `topTags[0].tag` 实际="" 期望="守时"
- `getPublicCreditSummaryUsesCorrectPath`: `topTags[0].tag` 实际="" 期望="沟通顺畅"

原因: 旧代码按 `"tag"` 字段解析，后端实际发送 `"tagName"`。

## 2. 修复内容

### 2.1 topTags tagName 解析修复

文件: `desktop/src/api/ReviewCreditApiService.cpp`

两处解析改为优先读 `tagName`，fallback `tag`:
```cpp
tagItem.tag = tagObj.value(QStringLiteral("tagName")).toString();
if (tagItem.tag.isEmpty()) {
    tagItem.tag = tagObj.value(QStringLiteral("tag")).toString();
}
```

后端合同确认: `CreditSummaryTagResponse(String tagName, long count)` — 字段名为 `tagName`。

### 2.2 永真断言删除

文件: `desktop/tests/ReviewCreditApiServiceTest.cpp`

删除 `QVERIFY2(!result.success || true, ...)` 永真断言，替换为真实合同断言:
- 路径正确
- userId 正确
- averageRating 正确
- topTags.size() 正确
- topTags[0].tag 正确解析
- topTags[0].count 正确

### 2.3 预设标签对齐

后端预设标签 (ReviewService.PRESET_TAGS):
`守时, 沟通顺畅, 配合度高, 认真负责, 体验很好, 迟到, 失联, 临时变卦, 配合度低, 体验不佳`

测试 mock 和 smoke 统一使用 `"守时"` / `"沟通顺畅"` 替代英文标签 `"friendly"` / `"punctual"`。

### 2.4 Server Smoke 评价链路修复

文件: `desktop/tests/QtServerIntegrationSmoke.cpp`

步骤 17-22 重写:
- 步骤 17: 从 `GET /me/conversations` 获取 `otherParticipantId`（替代硬编码 UUID）
- 步骤 18: 双方各发 2 条短消息 "hi"（满足 isValidConversation 双方各 >=2 条 USER_TEXT）
- 步骤 19: `GET /me/credit-summary`（验证接口可用）
- 步骤 20: `POST /me/reviews` 使用真实 `otherParticipantId` + 预设标签 `"守时"`，create review 失败记为 FAIL
- 步骤 21: `GET /me/reviews/given`（验证 items >= 1）
- 步骤 22: `GET /me/reviews/received`

## 3. 绿灯阶段

### 3.1 ctest 结果

9/9 全部通过 (0.69s):

| # | 测试 | 结果 |
|---|------|------|
| 1 | api_client_config_test | Passed |
| 2 | campus_api_client_test | Passed |
| 3 | auth_token_store_test | Passed |
| 4 | server_smoke_security_test | Passed |
| 5 | partner_post_api_service_test | Passed |
| 6 | contact_conversation_api_service_test | Passed |
| 7 | my_partner_post_api_service_test | Passed |
| 8 | student_post_plaza_widget_test | Passed |
| 9 | review_credit_api_service_test | Passed |

### 3.2 Server Smoke 结果

22/22 全部通过，0 FAILURE:

| 步骤 | 描述 | 结果 |
|------|------|------|
| 1 | GET /health | PASS: status=UP |
| 2 | POST /auth/login | PASS: token length=414 |
| 3 | GET /me/credit-summary (auth) | PASS: avgRating=3.6 sampleCount=8 |
| 4 | POST /auth/identity-verifications/materials | PASS |
| 5 | DELETE /auth/identity-verifications/materials/{id} | PASS: 204 |
| 6 | GET /partner-posts (plaza list) | PASS: items=1 |
| 7 | POST /partner-posts/{id}/contact-requests (admin) | PASS: conversationId=2 |
| 8 | GET /me/conversations | PASS: items=2 |
| 9 | POST /me/conversations/{id}/messages | PASS |
| 10 | GET /me/conversations/{id}/messages (after) | PASS |
| 11 | POST /me/partner-posts (create draft) | PASS: DRAFT |
| 12 | PUT /me/partner-posts/{id} (update draft) | PASS |
| 13 | GET /me/partner-posts | PASS: items=14 |
| 14 | GET /me/partner-posts/{id} (detail) | PASS |
| 15 | POST /me/partner-posts/{id}/submit-review | PASS: PENDING_REVIEW |
| 16 | POST /me/partner-posts/{id}/withdraw-review | PASS: DRAFT |
| 17 | Find otherParticipantId | PASS: length=36 |
| 18 | Ensure 2+ USER_TEXT each side | PASS: admin sent 2, smoke sent 2 |
| 19 | GET /me/credit-summary | PASS: avgRating=3.6 sampleCount=8 disputed=0 |
| 20 | POST /me/reviews (create review) | **PASS: reviewId=2 status=ACTIVE** |
| 21 | GET /me/reviews/given | PASS: items=2 |
| 22 | GET /me/reviews/received | PASS: items=0 |

**关键里程碑: 步骤 20 评价提交真实 PASS（非 NOTE），reviewId=2, status=ACTIVE**

## 4. 本轮修改文件清单

修改:
- `desktop/src/api/ReviewCreditApiService.cpp` — topTags 解析改为 tagName 优先 + tag fallback
- `desktop/tests/ReviewCreditApiServiceTest.cpp` — mock 用 tagName + 删除永真断言 + 预设标签对齐
- `desktop/tests/QtServerIntegrationSmoke.cpp` — 步骤 17-22 重写，otherParticipantId + 双方补消息 + 预设标签

未修改: 后端/Flyway/deploy/其他 UI/其他测试

## 5. 敏感信息检查

validation 未记录 token、密码、Authorization header、消息全文。仅记录 conversationId、reviewId、items count、errorCode。

## 6. 结论

Round 42 质量收口完成:
- topTags tagName 解析与后端合同对齐（含 fallback 兼容）
- 永真断言已删除，替换为真实合同断言
- 预设标签对齐后端 PRESET_TAGS
- 评价提交 smoke 真实 PASS（Round 41 为 NOTE/CONVERSATION_NOT_REVIEWABLE）
- 评价主链路完整闭合
