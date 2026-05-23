# Round 37 — Qt 学生侧发布 API Client 对接

**日期**: 2026-05-23
**状态**: PASS
**基线提交**: `cc40539` → 待提交

## 目标

新增 Qt 学生侧"我的发布/草稿/提交审核"API service、domain model、测试和服务器 smoke；不做 Qt UI 页面，不改后端。

## 新增文件

### Domain Model
- `desktop/src/domain/MyPartnerPostModels.h` — MyPostDraftRequest, MyPostItem, MyPostResult, MyPostListResult, PostActionResult

### API Service
- `desktop/src/api/MyPartnerPostApiService.h` — 7 个 API 方法
- `desktop/src/api/MyPartnerPostApiService.cpp` — 实现

### Test
- `desktop/tests/MyPartnerPostApiServiceTest.cpp` — 12 个测试用例

## 修改文件

### CampusApiClient 扩展
- `desktop/src/api/CampusApiClient.h` — 新增 `putJson` 方法声明
- `desktop/src/api/CampusApiClient.cpp` — 实现 `putJson`（用于 updateDraft PUT 请求）

### CMakeLists.txt
- `desktop/CMakeLists.txt` — 新增 my_partner_post_api_service_test target，更新 DESKTOP_SOURCES 和 qt_server_integration_smoke sources

### QtServerIntegrationSmoke
- `desktop/tests/QtServerIntegrationSmoke.cpp` — 新增 include，扩展步骤 11-16

## API 方法清单

| 方法 | HTTP | 路径 | 说明 |
|------|------|------|------|
| createDraft | POST | /me/partner-posts | 创建草稿 |
| updateDraft | PUT | /me/partner-posts/{id} | 更新草稿 |
| listMyPosts | GET | /me/partner-posts?page=&size=&status= | 我的发布列表 |
| getMyPostDetail | GET | /me/partner-posts/{id} | 详情（含 allowedActions） |
| submitReview | POST | /me/partner-posts/{id}/submit-review | 提交审核 |
| withdrawReview | POST | /me/partner-posts/{id}/withdraw-review | 撤回审核 |
| unpublish | POST | /me/partner-posts/{id}/unpublish | 主动下架 |

## 本地测试结果（12/12 PASS）

| 测试 | 结果 |
|------|------|
| createDraftUsesPostAndCorrectPath | PASS |
| createDraftSendsBearerToken | PASS |
| createDraftSendsDraftFieldsInBody | PASS |
| updateDraftUsesPutAndCorrectPath | PASS |
| listMyPostsUsesCorrectPathWithQuery | PASS |
| listMyPostsWithStatusFilter | PASS |
| listMyPostsUsesBearerToken | PASS |
| getMyPostDetailParsesAllowedActionsAndScenePayload | PASS |
| submitReviewUsesCorrectPath | PASS |
| withdrawReviewUsesCorrectPath | PASS |
| unpublishUsesCorrectPath | PASS |
| errorResponseConvertsToServiceResult | PASS |

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

| 步骤 | 描述 | 结果 |
|------|------|------|
| 1 | GET /health | PASS |
| 2 | POST /auth/login | PASS |
| 3 | GET /me/credit-summary | PASS |
| 4 | POST /auth/identity-verifications/materials | PASS |
| 5 | DELETE /auth/identity-verifications/materials/{id} | PASS |
| 6 | GET /partner-posts (plaza list) | PASS |
| 7 | POST /partner-posts/{id}/contact-requests (admin) | PASS |
| 8 | GET /me/conversations | PASS |
| 9 | POST /me/conversations/{id}/messages | PASS |
| 10 | GET /me/conversations/{id}/messages?afterMessageId | PASS |
| 11 | POST /me/partner-posts (create draft) | PASS |
| 12 | PUT /me/partner-posts/{id} (update draft) | PASS |
| 13 | GET /me/partner-posts (list my posts) | PASS |
| 14 | GET /me/partner-posts/{id} (detail + allowedActions) | PASS |
| 15 | POST /me/partner-posts/{id}/submit-review | PASS |
| 16 | POST /me/partner-posts/{id}/withdraw-review | PASS |

## 调试过程

1. 初次 smoke create draft 失败：`timeMode=FLEXIBLE` 不合法，后端要求 `EXACT_TIME|TIME_RANGE|TEXT_PREFERENCE`，改为 `TEXT_PREFERENCE`。
2. submit-review 失败：缺少 `participantCount`、`targetRequirement`、`contactPreference`、`scenePayload.studyGoal`，补齐后通过。

## 安全测试

- 无硬编码密码 ✓
- 无硬编码邮箱 ✓
- 环境变量引用检查 ✓
- 不打印 token/Authorization/密码/邮箱/contactPreference ✓

## 构建环境

- CMake: `G:\Qt\Tools\CMake_64\bin\cmake.exe`
- Ninja: `G:\Qt\Tools\Ninja\ninja.exe`
- g++: `G:\Qt\Tools\mingw1310_64\bin\g++.exe` (GNU 13.1.0)
- Qt6: `G:\Qt\6.10.3\mingw_64`
- Build dir: `desktop/build-qt6103-round37`
- campus_buddy_desktop 构建成功 ✓

## unpublish 覆盖方式

- 仅本地 service test 覆盖（mock HTTP server 返回正确响应）
- 未在服务器 smoke 中验证，因无可控 PUBLISHED post 属于 smoketest

## 后端状态

- 后端 225/225 测试通过（未修改）
- 服务器 active(running)

## 未覆盖风险

- unpublish 未做服务器 smoke（需 PUBLISHED post）
- 管理员审核通过后的完整生命周期未在 smoke 中覆盖
- sceneType 非 STUDY 的 scenePayload 验证未在 smoke 中覆盖
