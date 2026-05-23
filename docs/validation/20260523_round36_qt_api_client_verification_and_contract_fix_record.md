# Round 36 — Qt API Client 构建验证与合同修正

**日期**: 2026-05-23
**状态**: PASS
**基线提交**: `4fab1c4` → 待提交

## 目标

验证 Round 35 新增的 PartnerPostApiService / ContactConversationApiService 在本地构建和远程服务器集成的正确性，修正 URL 编码和 buildUrl 合同问题。

## 修改清单

### 1. CampusApiClient::buildUrl — 正确处理 path 中含 query string

**问题**: 当 path 为 `/partner-posts?page=0&size=20` 时，旧 `buildUrl` 把整个 string 当作 URL path 设置，导致 `?page=0&size=20` 被编码为路径的一部分，服务器收到 `/api/partner-posts%3Fpage%3D0%26size%3D20`，返回 RESOURCE_NOT_FOUND。

**修复**: `buildUrl` 改为先将 path 解析为 `QUrl relativeUrl(path)`，分别取 path 和 query，再拼到 baseUrl 上。

**文件**: `desktop/src/api/CampusApiClient.cpp:130-152`

### 2. PartnerPostApiService::listPosts — QUrlQuery URL encoding

**问题**: 旧代码手动拼接 `QString path = "/partner-posts?page=" + ...`，参数未做 URL encoding。

**修复**: 使用 `QUrlQuery` 构建查询参数，`path = "/partner-posts?" + query.toString()`。

**文件**: `desktop/src/api/PartnerPostApiService.cpp:20-30`

### 3. ContactConversationApiService — QUrlQuery + page 修复

- `listConversations(page,size)`: 使用 QUrlQuery
- `queryMessages(page,size)`: 不再忽略 page 参数，使用 QUrlQuery
- `queryMessages(afterMessageId,size)`: 使用 QUrlQuery

**文件**: `desktop/src/api/ContactConversationApiService.cpp`

### 4. QtServerIntegrationSmoke — InMemorySessionTokenStore include

**问题**: 编译错误，缺少 `#include "auth/InMemorySessionTokenStore.h"`。

**修复**: 添加 include。

**文件**: `desktop/tests/QtServerIntegrationSmoke.cpp:16`

### 5. QtServerIntegrationSmoke — 步骤 7 使用 admin 凭据发起联系

**问题**: smoketest 是 PUBLISHED post 的发布者，不能联系自己的 post（CANNOT_CONTACT_OWN_POST）。

**修复**:
- 新增环境变量 `CAMPUS_BUDDY_SMOKE_ADMIN_EMAIL` / `CAMPUS_BUDDY_SMOKE_ADMIN_PASSWORD`
- 步骤 7 先用 admin 凭据登录，再用 admin 的 ContactConversationApiService 发起联系

**文件**: `desktop/tests/QtServerIntegrationSmoke.cpp:142-143, 299-338`

### 6. ServerSmokeSecurityTest — 增加 admin 环境变量检查

**文件**: `desktop/tests/ServerSmokeSecurityTest.cpp:47-50`

## 验证结果

### 本地 ctest（6/6 PASS）

| 测试 | 结果 |
|------|------|
| api_client_config_test | PASS |
| campus_api_client_test | PASS |
| auth_token_store_test | PASS |
| server_smoke_security_test | PASS |
| partner_post_api_service_test | PASS |
| contact_conversation_api_service_test | PASS |

### Server Integration Smoke（10/10 PASS）

| 步骤 | 描述 | 结果 |
|------|------|------|
| 1 | GET /health | PASS |
| 2 | POST /auth/login (smoketest) | PASS |
| 3 | GET /me/credit-summary | PASS |
| 4 | POST /auth/identity-verifications/materials (upload) | PASS |
| 5 | DELETE /auth/identity-verifications/materials/{id} | PASS |
| 6 | GET /partner-posts (plaza list) | PASS |
| 7 | POST /partner-posts/{id}/contact-requests (admin→smoketest post) | PASS |
| 8 | GET /me/conversations | PASS |
| 9 | POST /me/conversations/{id}/messages | PASS |
| 10 | GET /me/conversations/{id}/messages?afterMessageId=X&size=1 | PASS |

### 安全测试

- 无硬编码密码 ✓
- 无硬编码邮箱 ✓
- CAMPUS_BUDDY_SMOKE_EMAIL 环境变量引用 ✓
- CAMPUS_BUDDY_SMOKE_PASSWORD 环境变量引用 ✓
- CAMPUS_BUDDY_SMOKE_ADMIN_EMAIL 环境变量引用 ✓
- CAMPUS_BUDDY_SMOKE_ADMIN_PASSWORD 环境变量引用 ✓

## 构建环境

- CMake: `G:\Qt\Tools\CMake_64\bin\cmake.exe`
- Ninja: `G:\Qt\Tools\Ninja\ninja.exe`
- g++: `G:\Qt\Tools\mingw1310_64\bin\g++.exe` (GNU 13.1.0)
- Qt6: `G:\Qt\6.10.3\mingw_64`
- Build dir: `desktop/build-qt6103-round36`

## 后端状态

- 后端 225/225 测试通过（未修改）
- 服务器 active(running) on `114.116.203.78`
