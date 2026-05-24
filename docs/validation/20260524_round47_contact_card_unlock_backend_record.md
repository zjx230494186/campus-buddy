# Round 47 Validation — Contact Card and Unlock Flow

**日期**: 2026-05-24
**目标**: 实现联系方式卡片与双方解锁后端最小闭环。

---

## 1. 变更清单

| 变更项 | 路径 | 说明 |
|--------|------|------|
| Flyway V11 | `V11__create_contact_card_and_unlock_confirm.sql` | 新增 contact_card + contact_unlock_confirm 表 |
| ContactCard 实体 | `ContactCard.java` | 新增 |
| ContactCardRepository | `ContactCardRepository.java` | 新增 |
| ContactCardService | `ContactCardService.java` | 新增 GET/PUT 逻辑+校验 |
| ContactCardController | `ContactCardController.java` | 新增 /api/me/contact-card 端点 |
| ContactUnlockConfirm 实体 | `ContactUnlockConfirm.java` | 新增 |
| ContactUnlockConfirmRepository | `ContactUnlockConfirmRepository.java` | 新增 |
| ContactUnlockService | `ContactUnlockService.java` | 新增解锁状态/确认/查看对方卡片逻辑 |
| ContactConversationController | `ContactConversationController.java` | 新增 3 个 unlock 端点 |
| ContactUnlockRecordRepository | `ContactUnlockRecordRepository.java` | 新增 findByConversationId |
| SecurityConfiguration | `SecurityConfiguration.java` | 新增 /api/me/contact-card authenticated |
| ContactCardUnlockEndpointTest | `ContactCardUnlockEndpointTest.java` | 新增 13 个测试 |

## 2. 是否新增 Flyway 迁移

是，V11__create_contact_card_and_unlock_confirm.sql。表：`contact_card` (id, user_id UNIQUE, wechat_id, phone_number, qq_number, created_at, updated_at) + `contact_unlock_confirm` (id, conversation_id, user_id, confirmed_at, UNIQUE(conversation_id, user_id))。

## 3. 数据设计选择

**方案 B**：新增独立 `contact_unlock_confirm` 表记录单方确认，只在双方都确认且有有效卡片时才写入现有 `ContactUnlockRecord`。

理由：
- `ContactUnlockRecord` 的 `conversationId` 是 UNIQUE 的，每会话最多一条记录
- `isContactUnlocked()` 仍通过 `contactUnlockRecordRepository.existsByConversationId()` 判断
- 单方确认不会创建 `ContactUnlockRecord`，因此 `isContactUnlocked()` 仍返回 false
- 不会提前触发 6 星评价
- 最小化对既有代码的侵入：ReviewService 无需修改，ContactContextService 无需修改

## 4. 红灯测试

命令：`mvn test -Dtest=ContactCardUnlockEndpointTest`

实现前所有 13 个测试均失败（端点不存在、实体/Repository 不存在）。

## 5. 绿灯测试

命令：`mvn test -Dtest=ContactCardUnlockEndpointTest`

结果：13/13 PASS

通过测试列表：
1. unauthenticatedAccessToContactCardReturns401
2. getContactCardWithoutCardReturnsHasCardFalse
3. putContactCardWithAtLeastOneFieldSucceeds
4. putContactCardWithAllEmptyReturnsValidationFailed
5. nonParticipantCannotViewUnlockStatus
6. closedConversationCannotConfirmUnlock
7. singleConfirmResultsInWaitingForPeerAndNotUnlocked
8. bothConfirmWithCardsResultsInUnlocked
9. unlockedUserCanViewPeerContactCard
10. notUnlockedCannotViewPeerContactCard
11. closedConversationCannotViewPeerContactCardEvenIfUnlocked
12. unlockedAllowsSixStarReview
13. notUnlockedRejectsSixStarReview

## 6. contact 包回归

全部通过（ContactCardUnlockEndpointTest 13 + ContactConversationStateEndpointTest 11 + ContactConversationEndpointTest + ContactContextServiceTest + ContactPersistenceIntegrationTest）

## 7. review 关键回归

- notUnlockedRejectsSixStarReview: 6星评价在未解锁时被正确拒绝 (VALIDATION_FAILED)
- unlockedAllowsSixStarReview: 解锁后6星评价成功 (201 Created, rating=6)
- ReviewService 无修改，完全通过既有逻辑消费 ContactContextService.isContactUnlocked()

## 8. 后端全量回归

命令：`mvn test`

结果：249/249 PASS（236 + 13 新增）

## 9. 服务器部署/重启

- JAR 部署到 `/srv/campus-buddy/` 和 `/opt/campus-buddy/`（两个位置均更新）
- `systemctl restart campus-buddy-backend` → active(running)

## 10. Health Check

```
curl -s http://114.116.203.78/api/health → 200 {"status":"UP"}
```

## 11. Flyway Validated

V1-V11 applied/validated。应用启动成功证明 Flyway 迁移已执行。

## 12. 服务器 Smoke

| # | 测试项 | 结果 |
|---|--------|------|
| 1 | Health | PASS |
| 2 | Smoke user upsert contact card | PASS |
| 3 | Admin upsert contact card | PASS |
| 4 | Find existing conversation | PASS (conversationId=2) |
| 5 | Smoke user confirm → WAITING_FOR_PEER | PASS |
| 6 | Admin confirm → UNLOCKED | PASS |
| 7 | View peer contact card | PASS (fields present, values not printed) |
| 8 | Close then view peer card → CONTACT_UNLOCK_NOT_AVAILABLE | PASS |
| 9 | Re-contact reopens | SKIP (no published post in plaza for admin) |
| 10 | Unlock status check | UNLOCKED (6-star allowed) |

## 13. 敏感信息检查结论

- [x] smoke 凭据不硬编码在源码中
- [x] validation 不记录 token、密码
- [x] smoke 不打印联系方式明文，只记录字段是否存在
- [x] 测试数据使用明显虚构值（wx_smoke_placeholder, 19900000000, 100000001）
- [x] validation 中不记录这些具体值

## 14. Git Status 摘要

修改文件 11 个 + 新增文件 7 个：
- V11 迁移 (新增)
- ContactCard.java (新增)
- ContactCardRepository.java (新增)
- ContactCardService.java (新增)
- ContactCardController.java (新增)
- ContactUnlockConfirm.java (新增)
- ContactUnlockConfirmRepository.java (新增)
- ContactUnlockService.java (新增)
- ContactUnlockRecordRepository.java (修改)
- ContactConversationController.java (修改)
- SecurityConfiguration.java (修改)
- ContactCardUnlockEndpointTest.java (新增)

## 15. 未覆盖风险

| 风险 | 说明 | 建议 |
|------|------|------|
| Qt UI 未适配 | 本轮仅后端，Qt 桌面端尚未接入 | Round 48 Qt 适配 |
| 联系方式变更通知 | 对方修改卡片后，已解锁方看到的仍是查询时刻的快照 | 可后续加变更通知 |
| 批量解锁 | 只支持单个会话 | 可后续扩展 |
| HTTPS | 公网仍为 HTTP | 真实试用前必须加 |
| Step 9 SKIP | recontact smoke 因广场无帖而跳过 | 不影响功能验证 |
