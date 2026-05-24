# Round 44 Validation — Qt 管理员审核合同修复与 Smoke 收口

**日期**: 2026-05-24
**目标**: 修复 submissionId UUID 类型错误、认证审核 UI 字段、server smoke 步骤 25-26 VALIDATION_FAILED。

---

## 1. 变更清单

| 变更项 | 路径 | 说明 |
|--------|------|------|
| submissionId 类型修复 | `desktop/src/domain/AdminReviewModels.h` | `long long` → `QString` |
| API 签名修复 | `desktop/src/api/AdminReviewApiService.h/.cpp` | `reviewIdentityVerification(const QString &submissionId, ...)` |
| parsePendingItem 修复 | `desktop/src/api/AdminReviewApiService.cpp` | `submissionId` 用 `toString()` 而非 `toInteger()` |
| Widget 类型修复 | `desktop/src/ui/AdminReviewWidget.h/.cpp` | `selectedSubmissionId_` 改为 QString, `isEmpty()`/`clear()` |
| 认证列表字段扩展 | `desktop/src/ui/AdminReviewWidget.cpp` | 显示 realName, studentNumber, college, major, grade, materialContentType |
| 测试 UUID mock | `desktop/tests/AdminReviewApiServiceTest.cpp` | 使用 UUID 字符串 mock + UUID 路径断言 |
| **Smoke scenePayload 修复** | `desktop/tests/QtServerIntegrationSmoke.cpp` | 步骤 25 增加 `scenePayload["studyGoal"]="pass exam"` |
| Smoke errorMessage 输出 | `desktop/tests/QtServerIntegrationSmoke.cpp` | FAIL 行增加 `errorMessage` 输出便于定位 |

## 2. 根因分析

Server smoke 步骤 25 `submit-review` 返回 `VALIDATION_FAILED` 的根因：

后端 `PartnerPostService.validateForSubmission()` 第 243-253 行：当 `sceneType = "STUDY"` 时，`SCENE_REQUIRED_KEYS` 映射要求 `scenePayload` 必须包含 `studyGoal` 键且值非空。Smoke 步骤 25 创建 draft 时未设置 `scenePayload`，导致验证失败。

修复：在 draft 创建中增加 `draftReq.scenePayload["studyGoal"] = "pass exam"`。

## 3. 绿灯测试

### 3.1 Qt 本地单元测试

| 测试套件 | 结果 |
|----------|------|
| `api_client_config_test` | 7/7 通过 |
| `campus_api_client_test` | 11/11 通过 |
| `auth_token_store_test` | 6/6 通过 |
| `server_smoke_security_test` | 3/3 通过 |
| `partner_post_api_service_test` | 通过 |
| `contact_conversation_api_service_test` | 通过 |
| `my_partner_post_api_service_test` | 通过 |
| `student_post_plaza_widget_test` | 通过 |
| `review_credit_api_service_test` | 通过 |
| `admin_review_api_service_test` | 通过 |

**合计 10/10 通过**。

### 3.2 campus_buddy_desktop --smoke-test

EXIT=0，本地 smoke 通过。

### 3.3 Qt Server Integration Smoke (27/27)

| # | 端点 | 结果 |
|---|------|------|
| 1 | GET /health | PASS: status=UP |
| 2 | POST /auth/login | PASS: token length=414 |
| 3 | GET /me/credit-summary | PASS |
| 4 | POST /auth/identity-verifications/materials | PASS |
| 5 | DELETE /auth/identity-verifications/materials/{id} | PASS |
| 6 | GET /partner-posts | PASS |
| 7 | POST /partner-posts/{id}/contact-requests | PASS |
| 8 | GET /me/conversations | PASS |
| 9 | POST /me/conversations/{id}/messages | PASS |
| 10 | GET /me/conversations/{id}/messages | PASS |
| 11 | POST /me/partner-posts (create draft) | PASS |
| 12 | PUT /me/partner-posts/{id} (update draft) | PASS |
| 13 | GET /me/partner-posts | PASS |
| 14 | GET /me/partner-posts/{id} | PASS |
| 15 | POST /me/partner-posts/{id}/submit-review | PASS |
| 16 | POST /me/partner-posts/{id}/withdraw-review | PASS |
| 17 | Find otherParticipantId | PASS |
| 18 | Ensure 2+ USER_TEXT each side | PASS |
| 19 | GET /me/credit-summary | PASS |
| 20 | POST /me/reviews | NOTE (repeat run) |
| 21 | GET /me/reviews/given | PASS |
| 22 | GET /me/reviews/received | PASS |
| 23 | Admin login | PASS |
| 24 | GET /admin/partner-posts/review-queue | PASS |
| **25** | **Withdraw + create + submit for admin review** | **PASS: status=PENDING_REVIEW** |
| **26** | **Admin detail + REJECT smoke post** | **PASS: status=REJECTED reviewedAt=yes** |
| 27 | GET /admin/identity-verifications?status=PENDING_REVIEW | PASS |

**27/27 全部通过**。

## 4. 服务器状态

```
curl -s http://114.116.203.78/api/health → 200 {"status":"UP"}
systemctl status campus-buddy-backend → active(running)
后端 225/225 测试通过
```

## 5. 敏感信息检查

- [x] smoke 凭据通过环境变量传入，不硬编码
- [x] validation 文档不记录密码明文
- [x] Smoke 凭据已通过数据库重置为已知值（本轮一次性操作，密码不写入仓库）

## 6. 未覆盖风险

| 风险 | 说明 | 建议 |
|------|------|------|
| 身份认证提交+审核闭环 | smoke 步骤 27 无 PENDING_REVIEW 认证 | 需要上传材料+提交认证+admin审核闭环 |
| HTTPS/TLS | 公网仍为 HTTP | 真实试用前必须加 HTTPS |
| Scene payload 其他类型 | 只测试了 STUDY+studyGoal | 可补充 MEAL+canteen 等场景 |
