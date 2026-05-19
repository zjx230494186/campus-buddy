# Round 08 验证记录：Qt 认证集成契约审计与修正

## 日期

2026-05-19

## 目标

修正 Qt 客户端与后端认证 API 的契约，使登录、发送验证码、校验验证码、注册、查询认证状态能够按真实后端接口语义工作。

## 提交

- `e53f11a` fix(desktop): align auth flow with backend contract

## 修复项

### 1. Token 存储：禁用 QSettings 持久化
- **修复前**：`AuthTokenStore` 使用 `QSettings` 保存 access token，违反 `docs/13_detailed_design_v1.md` 约束
- **修复后**：引入 `SecureTokenStore` 抽象 + `InMemorySessionTokenStore` 内存实现；`AuthTokenStore` 继承 `SecureTokenStore`，仅使用进程内存存储
- **临时说明**：当前为内存会话存储，进程退出即丢失；后续需实现 Windows Credential Manager 适配

### 2. AuthResult 字段语义修正
- **修复前**：`accessToken` 被复用来承载 `authenticationStatus`，语义混乱
- **修复后**：`AuthResult` 新增 `authenticationStatus`、`verificationTicket` 独立字段

### 3. 发送验证码请求体修正
- **修复前**：`{ campusEmail }` 缺少 `purpose`
- **修复后**：`{ campusEmail, purpose: "REGISTER_OR_LOGIN" }`

### 4. 校验验证码请求体修正
- **修复前**：`{ campusEmail, code }` 缺少 `purpose`，未读取 `verificationTicket`
- **修复后**：`{ campusEmail, code, purpose: "REGISTER_OR_LOGIN" }`，成功后读取 `verificationTicket`

### 5. 注册请求体修正
- **修复前**：`{ realName, studentNumber, campusEmail, password, verificationCode }` 与后端契约不符
- **修复后**：`{ campusEmail, verificationTicket, password, displayName }`，符合后端真实 DTO

### 6. 查询认证状态字段修正
- **修复前**：`result.accessToken` 承载 `authenticationStatus`
- **修复后**：`result.authenticationStatus` 独立字段

### 7. 注册页面 UI 适配
- **修复前**：realName + studentNumber + email + password + code → 直接注册
- **修复后**：email → 发送验证码 → 校验验证码（获取 ticket）→ displayName + password → 注册

## 测试结果

### 红灯确认（修复前）
- `auth_token_store_test`：`tokenIsNotPersistedToQSettings` 失败（QSettings 确实写入了 token）

### 绿灯确认（修复后）
- `api_client_config_test`：Passed
- `campus_api_client_test`（9 个测试）：Passed
  - getJsonParsesSuccessPayload
  - errorResponseConvertsToClientError
  - widgetLayerDoesNotDirectlyUseNetworkAccessManager
  - postJsonSendsBodyAndParsesResponse
  - getJsonWithAuthSendsAuthorizationHeader
  - postJsonWithAuthSendsAuthorizationHeader
  - sendVerificationCodeRequestBodyContainsPurpose
  - verifyCampusEmailRequestBodyContainsPurpose
  - registerRequestBodyContainsVerificationTicketAndDisplayName
- `auth_token_store_test`（6 个测试）：Passed
  - defaultStoreIsEmpty
  - setAndRetrieveAccessToken
  - clearRemovesAccessToken
  - hasAccessTokenReturnsFalseAfterClear
  - tokenIsNotPersistedToQSettings

### 后端回归
- 未修改后端代码，56/56 不变

## 已知遗留

- Windows Credential Manager 适配器未实现（InMemorySessionTokenStore 是临时实现）
- 认证资料提交 UI 未实现
- 附件上传 UI 未实现
- 管理员审核 UI 未实现
- 完整 RBAC 未实现
