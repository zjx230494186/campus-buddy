# Round 30 — PartnerPost 管理员审核验证记录

**日期**: 2026-05-23
**轮次**: Round 30
**目标**: 实现管理员审核队列、审核详情、通过/驳回裁定

---

## 1. 实现内容

### 1.1 新增端点

- `GET /api/admin/partner-posts/review-queue?page=0&size=20` — 审核队列（仅 PENDING_REVIEW，按 updatedAt DESC）
- `GET /api/admin/partner-posts/{postId}` — 审核详情（含发布者摘要）
- `POST /api/admin/partner-posts/{postId}/review` — 审核裁定（APPROVE/REJECT）

### 1.2 审核队列摘要字段
postId, publisherId, publisherDisplayName, sceneType, status, title, summary (description 截断50字), timeText, locationText, updatedAt

### 1.3 审核详情
完整发布字段 + 发布者摘要（publisherId, publisherDisplayName, publisherAuthenticationStatus）
不返回 campusEmail, studentNumber, realName, objectKey

### 1.4 审核裁定

APPROVE:
- status = PUBLISHED
- reviewedBy = admin userId
- reviewedAt = now
- publishedAt = now
- rejectReason = null

REJECT:
- status = REJECTED
- reviewedBy = admin userId
- reviewedAt = now
- rejectReason = reason (必填，1-200字符)
- publishedAt = null

### 1.5 PUBLISHED 数量上限
APPROVE 时校验发布者已有 PUBLISHED 数量，≥10 返回 PUBLISHED_POST_LIMIT_EXCEEDED

### 1.6 安全
复用 SecurityConfiguration 已有的 `/api/admin/**` → hasRole("ADMIN")
无需修改 SecurityConfiguration

### 1.7 无新增 Flyway 迁移

---

## 2. 红灯测试

```bash
mvn test -Dtest=PartnerPostAdminReviewEndpointTest -pl .
```

```
Tests run: 13, Failures: 11, Errors: 0, Skipped: 0
BUILD FAILURE
```

2 通过（未认证401 + 学生403），11 失败（404 端点不存在）。

---

## 3. 绿灯测试

### PartnerPostAdminReviewEndpointTest (13 个测试)

| # | 测试 | 结果 |
|---|------|------|
| 1 | reviewQueueRequiresAuthentication → 401 | PASS |
| 2 | reviewQueueRequiresAdminRole → 403 | PASS |
| 3 | adminCanViewReviewQueueWithOnlyPendingReview | PASS |
| 4 | adminDetailReturnsPostFieldsAndPublisherSummary | PASS |
| 5 | adminDetailExcludesSensitiveFields | PASS |
| 6 | adminApproveChangesStatusToPublished | PASS |
| 7 | adminRejectChangesStatusToRejected | PASS |
| 8 | rejectWithoutReasonReturnsValidationFailed | PASS |
| 9 | invalidDecisionReturnsValidationFailed | PASS |
| 10 | reviewNonPendingReviewReturnsPostStatusConflict | PASS |
| 11 | reviewNonexistentPostReturnsPostNotFound | PASS |
| 12 | approveExceedsPublishedLimitReturnsError | PASS |
| 13 | withdrawnPostThenAdminReviewReturnsPostStatusConflict | PASS |

---

## 4. 全量回归

```
Tests run: 190, Failures: 0, Errors: 0, Skipped: 0
BUILD SUCCESS (01:55 min)
```

---

## 5. 服务器部署

- 构建并上传 jar
- `systemctl restart campus-buddy-backend` → active(running)
- Flyway: validated 8 migrations, schema at v8, no migration necessary
- Health: `{"status":"UP"}`

### 管理员账号
通过 SQL 临时创建/提升 smoke admin 账号（复用既有测试账号密码哈希，未记录哈希值），account_role=ADMIN。

---

## 6. 服务器 Smoke Test

| # | 操作 | 方法 | 路径 | 状态码 | 验证点 |
|---|------|------|------|--------|--------|
| 1 | 学生创建草稿 | POST | /api/me/partner-posts | 200 | status=DRAFT |
| 2 | 学生提交审核 | POST | /api/me/partner-posts/{id}/submit-review | 200 | status=PENDING_REVIEW |
| 3 | 管理员查看审核队列 | GET | /api/admin/partner-posts/review-queue | 200 | items=1, PENDING_REVIEW |
| 4 | 管理员查看详情 | GET | /api/admin/partner-posts/{id} | 200 | publisherDisplayName存在 |
| 5 | 管理员审核通过 | POST | /api/admin/partner-posts/{id}/review | 200 | PUBLISHED, reviewedBy/reviewedAt/publishedAt有值 |

### 驳回验证
仅本地测试覆盖（REJECT 测试 13 项全通过）。服务器 smoke 未执行驳回操作。

---

## 7. PUBLISHED 数量上限验证
本地测试 approveExceedsPublishedLimitReturnsError 通过：构造 10 条 PUBLISHED fixture，APPROVE 返回 PUBLISHED_POST_LIMIT_EXCEEDED (409)

---

## 8. 敏感信息检查
- 源码中无真实密码、token、AK/SK
- validation 中未写入 smoke 账号邮箱
- 结论：通过

---

## 9. 实际修改文件

- `backend/src/main/java/com/campusbuddy/post/PartnerPost.java` — 添加 setReviewedBy/setReviewedAt
- `backend/src/main/java/com/campusbuddy/post/PartnerPostRepository.java` — 添加 findByStatus/countByPublisherIdAndStatus
- `backend/src/main/java/com/campusbuddy/post/PartnerPostAdminService.java` — 新增
- `backend/src/main/java/com/campusbuddy/post/PartnerPostAdminController.java` — 新增
- `backend/src/test/java/com/campusbuddy/post/PartnerPostAdminReviewEndpointTest.java` — 新增 13 个测试

---

## 10. 未覆盖风险

- 驳回后学生重新编辑再提交的完整闭环（已有理论支持，但无端到端测试）
- 并发审核冲突（两个管理员同时审核同一帖子）未处理
- 审核队列排序仅按默认 JPA 排序（updatedAt DESC 需 @SortDefault 或显式 Sort）
- 管理员对 PUBLISHED/REJECTED 状态帖子的详情可查看，但审核裁定只允许 PENDING_REVIEW
- 审核通知/审计日志未实现（本轮明确排除）
- 服务器 smoke 中使用 SQL 临时创建/提升管理员测试账号；未泄露密码或哈希值，但后续应提供受控管理员初始化/清理方案，避免长期依赖手工 SQL 操作。

---

## 11. 下一轮建议

P0-3 广场发现与推荐后端：已发布需求列表、筛选、搜索、详情和发布者公开摘要。
