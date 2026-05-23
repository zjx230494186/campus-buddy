# Round 38 — Qt 我的发布 API Client 合同测试补强

**日期**: 2026-05-23
**状态**: PASS
**基线提交**: `be9077c` → 待提交

## 目标

补强 MyPartnerPostApiServiceTest 的请求合同断言，将空心测试替换为真实捕获 HTTP 请求并验证 method/path/query/headers/body 的测试。

## 问题

Round 37 的测试用例命名声称校验 path、method、Bearer token、request body，但 `serveSingleResponse` 只返回固定响应，不捕获原始请求。测试实际只断言 `success`，未保护 API 合同。

## 修改

### MyPartnerPostApiServiceTest.cpp — 完全重写

新增基础设施：
- `RawRequest { headers; body }` — 捕获完整 HTTP 请求头和请求体
- `serveAndCaptureRequest()` — mock HTTP server，捕获请求后返回预设响应
- `extractMethod()` — 从请求行提取 HTTP method
- `extractPath()` — 从请求行提取完整路径（含 query）
- `extractHeader()` — 按名称提取 header 值

12 个测试用例全部改用 `serveAndCaptureRequest`，每个都真实断言请求合同。

## 补实的请求合同断言清单

| 测试用例 | 断言内容 |
|----------|----------|
| createDraftUsesPostAndCorrectPath | method=POST, path=/api/me/partner-posts |
| createDraftSendsBearerToken | Authorization: Bearer my-bearer-token 存在 |
| createDraftSendsDraftFieldsInBody | body JSON 含 sceneType/title/description/timeMode/timeText/locationText/participantCount/targetRequirement/contactPreference/tags/attachmentIds/scenePayload; scenePayload.studyGoal 正确序列化 |
| updateDraftUsesPutAndCorrectPath | method=PUT, path=/api/me/partner-posts/{postId}; body JSON 含 title |
| listMyPostsUsesCorrectPathWithQuery | method=GET, path 含 /api/me/partner-posts?, query 含 page=0 和 size=20 |
| listMyPostsWithStatusFilter | method=GET, path 含 status=DRAFT, page=0, size=10 |
| listMyPostsUsesBearerToken | Authorization: Bearer header 存在 |
| getMyPostDetailParsesAllowedActionsAndScenePayload | method=GET, path=/api/me/partner-posts/abc-123; allowedActions 3 项; scenePayload.studyGoal 正确 |
| submitReviewUsesCorrectPath | method=POST, path=/api/me/partner-posts/abc-123/submit-review; body 不含 token/password |
| withdrawReviewUsesCorrectPath | method=POST, path=/api/me/partner-posts/abc-123/withdraw-review |
| unpublishUsesCorrectPath | method=POST, path=/api/me/partner-posts/abc-123/unpublish |
| errorResponseConvertsToServiceResult | VALIDATION_FAILED errorCode/errorMessage 正确 |

## ctest 全量结果（7/7 PASS）

| Target | 结果 |
|--------|------|
| api_client_config_test | PASS |
| campus_api_client_test | PASS |
| auth_token_store_test | PASS |
| partner_post_api_service_test | PASS |
| contact_conversation_api_service_test | PASS |
| my_partner_post_api_service_test | PASS |
| server_smoke_security_test | PASS |

## Server Integration Smoke（16/16 PASS）

与 Round 37 一致，未修改 smoke 代码。

## 安全测试

- 无硬编码密码/邮箱 ✓
- 不打印 token/Authorization/密码/contactPreference ✓
- submitReview 测试显式断言 body 不含 token/password ✓

## 构建环境

- Build dir: `desktop/build-qt6103-round38`
- campus_buddy_desktop 构建成功 ✓

## 未覆盖风险

- MyPostResult 暂不保存 error.details，UI 表单字段级错误展示需后续补齐
- CampusApiClient::putJson 底层合同通过 MyPartnerPostApiServiceTest 间接覆盖，未独立单测
