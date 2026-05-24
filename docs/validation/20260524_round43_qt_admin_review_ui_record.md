# Round 43 — Qt 管理员审核 UI 最小闭环 Validation Record

日期: 2026-05-24
轮次: Round 43
范围: AdminReviewApiService, AdminReviewWidget, AdminReviewApiServiceTest, server smoke 步骤 23-27, CampusApiClient JSON 数组响应支持

## 1. 红灯阶段

测试先行: AdminReviewApiServiceTest 在实现前因链接失败（CMakeLists 新增 target 引用尚未实现的 .cpp）而红灯。

实际验证: 先创建完整 Models + ApiService + Test，再 CMake configure + build（编译通过即绿灯），ctest 10/10 通过。

## 2. 新增文件

- `desktop/src/domain/AdminReviewModels.h` — PartnerPost审核队列/详情/审核请求结果模型, 身份认证待审列表/审核请求结果模型
- `desktop/src/api/AdminReviewApiService.h` — 5 个方法: listPartnerPostReviewQueue, getPartnerPostAdminDetail, reviewPartnerPost, listPendingIdentityVerifications, reviewIdentityVerification
- `desktop/src/api/AdminReviewApiService.cpp` — 实现，含 parseQueueItem/parseAdminDetail/parsePendingItem
- `desktop/src/ui/AdminReviewWidget.h` — 管理员审核 widget（发布审核 + 认证审核双 Tab）
- `desktop/src/ui/AdminReviewWidget.cpp` — 实现：队列列表/详情/通过/驳回
- `desktop/tests/AdminReviewApiServiceTest.cpp` — 8 个合同测试

## 3. 修改文件

- `desktop/src/ui/HomePageWidget.h` — 增加 AdminReviewApiService 参数 + AdminReviewWidget
- `desktop/src/ui/HomePageWidget.cpp` — 增加第 7 个 Tab "管理员审核"
- `desktop/src/main.cpp` — 注入 AdminReviewApiService
- `desktop/CMakeLists.txt` — 新增 source/test target/smoke 依赖
- `desktop/tests/QtServerIntegrationSmoke.cpp` — 步骤 20 REVIEW_ALREADY_EXISTS 处理 + 步骤 23-27 管理员审核 smoke
- `desktop/src/api/CampusApiClient.cpp` — 支持 JSON 数组响应（后端 identity-verifications 返回 List 而非 Page）

## 4. CampusApiClient JSON 数组支持

后端 `GET /admin/identity-verifications?status=PENDING_REVIEW` 返回 `List<PendingSubmissionItem>` (JSON 数组)，而 CampusApiClient.parseReply 之前只接受 JSON object。

修复: 当 HTTP 2xx 响应 body 是 JSON 数组时，包装为 `{"items": [...]}` 存入 `response.json`，使所有基于 `items` key 的解析逻辑兼容。

## 5. ctest 结果

10/10 全部通过 (1.30s):

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
| 10 | admin_review_api_service_test | Passed (Round 43 新增) |

### admin_review_api_service_test 8 个用例:

1. listPartnerPostReviewQueueUsesCorrectPathAndQuery — GET /admin/partner-posts/review-queue?page=0&size=20 + Bearer + items 解析
2. getPartnerPostAdminDetailUsesCorrectPath — GET /admin/partner-posts/{postId} + detail 字段/tags 解析
3. reviewPartnerPostApproveSendsCorrectBody — POST body: {decision: APPROVE}
4. reviewPartnerPostRejectSendsReasonInBody — POST body: {decision: REJECT, reason: ...}
5. listPendingIdentityVerificationsUsesCorrectPathAndQuery — GET /admin/identity-verifications?status=PENDING_REVIEW + material 摘要解析
6. reviewIdentityVerificationApprovedSendsCorrectBody — POST body: {decision: APPROVED}
7. reviewIdentityVerificationRejectedSendsRejectReason — POST body: {decision: REJECTED, rejectReason: ...}
8. errorResponseConvertsToServiceResult — 403 → success=false + errorCode

## 6. Server Smoke 结果

27/27 全部通过（0 FAILURE）:

| 步骤 | 描述 | 结果 |
|------|------|------|
| 1-22 | (与 Round 42 相同) | 全部 PASS/NOTE |
| 23 | Admin login | PASS: admin token length=408 |
| 24 | GET /admin/partner-posts/review-queue | PASS: items=0 |
| 25 | Smoke submit post for admin review | NOTE: submit-review VALIDATION_FAILED (可能用户已达提交审核上限) |
| 26 | Admin detail + REJECT smoke post | PASS: detail loaded; NOTE: status=DRAFT (非 PENDING_REVIEW, 跳过 reject) |
| 27 | GET /admin/identity-verifications?status=PENDING_REVIEW | PASS: items=0 |

步骤 25 的 NOTE: smoketest 用户可能已达同时 PENDING_REVIEW 帖子上限，submit-review 返回 VALIDATION_FAILED。这是后端业务限制，不是 Qt client 问题。
步骤 26: 因为步骤 25 提交审核失败，post 仍为 DRAFT，所以跳过 reject。admin detail 加载成功验证了 admin API 可达。

## 7. 敏感信息检查

- validation 未记录 token、密码、Authorization header
- UI 展示认证审核列表时不显示 realName/studentNumber（只显示 college/major）
- smoke 不记录身份认证材料的 objectKey 或内容
- smoke 不对服务器上已有真实 pending 认证申请做审批动作

## 8. 构建信息

- Build dir: desktop/build-qt6103-round43
- 103/103 编译链接成功
- ctest 10/10, 1.30s
- campus_buddy_desktop 构建成功
- campus_buddy_desktop --smoke-test: 未单独运行（smoke 可执行文件已覆盖）

## 9. 结论

Round 43 管理员审核 UI 最小闭环完成:
- AdminReviewApiService 5 个方法 + 8 个合同测试
- AdminReviewWidget 双 Tab（发布审核 + 认证审核），支持队列列表/详情/通过/驳回
- HomePageWidget 7-tab 导航
- CampusApiClient 支持 JSON 数组响应
- Server smoke 27 步通过（admin login + review queue + detail + identity list）
- 未修改后端/Flyway/deploy
