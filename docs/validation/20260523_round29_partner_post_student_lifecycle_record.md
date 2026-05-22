# Round 29 — PartnerPost 学生提交/撤回/下架生命周期验证记录

**日期**: 2026-05-23
**轮次**: Round 29
**目标**: 实现学生侧三个状态动作：提交审核、撤回审核、主动下架，以及提交审核强校验

---

## 1. 实现内容

### 1.1 新增端点

- `POST /api/me/partner-posts/{postId}/submit-review` — DRAFT → PENDING_REVIEW（含强校验）
- `POST /api/me/partner-posts/{postId}/withdraw-review` — PENDING_REVIEW → DRAFT
- `POST /api/me/partner-posts/{postId}/unpublish` — PUBLISHED → DRAFT

### 1.2 提交审核强校验

公共字段：
- sceneType: 必填，MEAL/STUDY/SPORT/COURSE_TEAM/INNOVATION_PROJECT
- title: 必填，1-40 字符
- description: 必填，1-500 字符
- timeMode: 必填，EXACT_TIME/TIME_RANGE/TEXT_PREFERENCE
- EXACT_TIME → startAt 必填
- TIME_RANGE → startAt + endAt 必填，endAt > startAt
- TEXT_PREFERENCE → timeText 必填，1-60 字符
- locationText: 必填，1-80 字符
- participantCount: 必填，1-20
- targetRequirement: 必填，1-120 字符
- contactPreference: 必填，1-80 字符，禁止连续 11 位数字
- tags: 最多 8 个，每个 1-12 字符

场景字段：
- MEAL → scenePayload.canteen 必填
- STUDY → scenePayload.studyGoal 必填
- SPORT → scenePayload.sportType 必填
- COURSE_TEAM → scenePayload.courseName 必填
- INNOVATION_PROJECT → scenePayload.projectDirection 必填

### 1.3 allowedActions 更新

| 状态 | allowedActions |
|------|---------------|
| DRAFT | EDIT, SUBMIT_REVIEW |
| PENDING_REVIEW | WITHDRAW_REVIEW |
| PUBLISHED | UNPUBLISH |
| REJECTED | EDIT, VIEW_REJECT_REASON |

### 1.4 其他变更

- PartnerPost 添加 setRejectReason setter
- PostResponse record 添加 rejectReason 字段
- 主动下架清空 publishedAt

### 1.5 无新增 Flyway 迁移

现有 V8 表已支持所有字段，无需迁移。

---

## 2. 红灯测试

### 红灯命令
```bash
mvn test -Dtest=PartnerPostStudentLifecycleEndpointTest -pl .
```

### 红灯结果
```
Tests run: 14, Failures: 13, Errors: 0, Skipped: 0
BUILD FAILURE
```
13/14 失败，1 通过（submitReviewRequiresAuthentication 因为 `/**` 通配符已覆盖）。
所有失败均为 404（端点不存在），确认红灯。

---

## 3. 绿灯测试

### 绿灯命令
```bash
mvn test -Dtest=PartnerPostStudentLifecycleEndpointTest,PartnerPostStudentDraftEndpointTest -pl .
```

### PartnerPostStudentLifecycleEndpointTest (14 个测试)

| # | 测试 | 结果 |
|---|------|------|
| 1 | submitReviewRequiresAuthentication → 401 | PASS |
| 2 | submitReviewRequiresVerifiedStatus → 403 | PASS |
| 3 | submitReviewOtherUsersDraftReturnsPostNotFound → 404 | PASS |
| 4 | submitReviewWithCompleteFieldsSucceeds → 200, PENDING_REVIEW | PASS |
| 5 | submitReviewMissingPublicRequiredField → 400 VALIDATION_FAILED | PASS |
| 6 | submitReviewMissingSceneRequiredField → 400 VALIDATION_FAILED | PASS |
| 7 | submitReviewWithPhoneNumberInContactPreference → 400 VALIDATION_FAILED | PASS |
| 8 | submitReviewNonDraftReturnsPostStatusConflict → 409 | PASS |
| 9 | withdrawReviewFromPendingReviewSucceeds → 200, DRAFT | PASS |
| 10 | withdrawReviewNonPendingReviewReturnsPostStatusConflict → 409 | PASS |
| 11 | unpublishFromPublishedSucceeds → 200, DRAFT | PASS |
| 12 | unpublishNonPublishedReturnsPostStatusConflict → 409 | PASS |
| 13 | allowedActionsChangeWithStatus | PASS |
| 14 | rejectedStatusHasEditAndViewRejectReason | PASS |

---

## 4. 全量回归

```
Tests run: 177, Failures: 0, Errors: 0, Skipped: 0
BUILD SUCCESS (01:40 min)
```

---

## 5. 服务器部署

### 部署步骤
1. `mvn package -DskipTests` 构建 jar
2. scp 上传至 `/srv/campus-buddy/campus-buddy-backend-0.0.1-SNAPSHOT.jar`
3. `systemctl restart campus-buddy-backend`

### 服务状态
```
Active: active(running)
Flyway: validated 8 migrations, schema at v8, no migration necessary
```

### Health Check
```json
{"status":"UP"}
```

---

## 6. 服务器 Smoke Test

| # | 操作 | 方法 | 路径 | 状态码 | 验证点 |
|---|------|------|------|--------|--------|
| 1 | 创建完整草稿 | POST | /api/me/partner-posts | 200 | status=DRAFT, allowedActions=[EDIT, SUBMIT_REVIEW] |
| 2 | 提交审核 | POST | /api/me/partner-posts/{id}/submit-review | 200 | status=PENDING_REVIEW, allowedActions=[WITHDRAW_REVIEW] |
| 3 | 查看详情 | GET | /api/me/partner-posts/{id} | 200 | 确认 PENDING_REVIEW + WITHDRAW_REVIEW |
| 4 | 撤回审核 | POST | /api/me/partner-posts/{id}/withdraw-review | 200 | status=DRAFT, allowedActions=[EDIT, SUBMIT_REVIEW] |

### 主动下架
仅本地测试覆盖（通过 repository fixture 构造 PUBLISHED 状态）。服务器 smoke 不修改线上数据。

---

## 7. 敏感信息检查

- 源码中无真实密码、token、AK/SK
- validation 中未记录 smoke 账号邮箱
- 服务器测试中 token 未写入文档
- 结论：通过

---

## 8. 实际修改文件

- `backend/src/main/java/com/campusbuddy/post/PartnerPost.java` — 添加 setRejectReason
- `backend/src/main/java/com/campusbuddy/post/PartnerPostService.java` — 添加 submitReview/withdrawReview/unpublish + validateForSubmission + containsPhoneNumber + 更新 allowedActions + PostResponse 添加 rejectReason
- `backend/src/main/java/com/campusbuddy/post/PartnerPostController.java` — 添加三个端点
- `backend/src/test/java/com/campusbuddy/post/PartnerPostStudentLifecycleEndpointTest.java` — 新增 14 个测试

---

## 9. 未覆盖风险

- 管理员审核（通过/驳回）未实现，PENDING_REVIEW → PUBLISHED/REJECTED 流转留待 Batch 2
- 手机号检测只拦截连续 11 位数字，未覆盖带分隔符、国际号等变体
- 并发审核冲突（管理员与学生同时操作）未处理，留待管理侧批次
- EXACT_TIME/TIME_RANGE 的 startAt/endAt 未校验是否在未来（仅结构校验）

---

## 10. 下一轮建议

P0-2 Batch 2：管理员审核队列、审核详情、通过/驳回。这是 PENDING_REVIEW → PUBLISHED/REJECTED 流转的必要前提。
