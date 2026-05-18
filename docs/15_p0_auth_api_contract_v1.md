# 15 P0-1 账号准入与身份认证接口契约 V1

本文收束《校园搭子平台》P0-1“账号准入与身份认证”的第一版后端 REST 接口契约。当前线程处于设计阶段，只定义接口边界、状态、错误响应、测试入口和需求追踪关系，不进入业务代码实现。

## 0. 设计阶段判断

- 当前阶段：P0-1 接口契约设计收束。
- 设计方法：按项目规则采用“无情审问式”设计收束；由于 `docs/13_detailed_design_v1.md` 已明确 P0-1 的对象、状态、流程与约束，本版先给出推荐契约草案，不再空泛追问。
- 上游依据：
  - `D:\big_homework\docs\13_detailed_design_v1.md` 第 4.4 节。
  - `D:\big_homework\docs\12_code_generation_constraints_v1.md` 第 10 节。
  - `D:\big_homework\docs\09_codearts_requirement_tables_v1.md` 中 `SF1`、`IR1`、`IR2`。
  - `D:\big_homework\docs\08_requirement_mapping_for_codearts_v1.md` 中 `SF1`、`IR1`、`IR2`。
  - 第一轮技术探路 A-G 验证记录。

## 1. 契约边界

### 1.1 本版最小接口范围

P0-1 第一版后端 REST 接口至少覆盖：

1. 注册或账号创建入口。
2. 登录入口。
3. 刷新 token 入口。
4. 退出登录/撤销 token 入口。
5. 校园邮箱验证码发送入口。
6. 校园邮箱验证码校验入口。
7. 认证资料提交入口。
8. 认证状态查询入口。

### 1.2 推荐路径前缀

- 账号与令牌：`/api/auth`
- 当前用户认证资料：`/api/me/authentication`
- 当前用户状态：`/api/me`

路径采用当前登录用户视角，不在学生侧接口中暴露任意 `userId` 操作入口。

## 2. 统一响应约束

### 2.1 成功响应

成功响应使用 JSON 对象。字段命名采用 lower camel case。

### 2.2 统一错误响应

所有错误响应使用既有技术探路中确认的结构：

```json
{
  "code": "ERROR_CODE",
  "message": "Human readable message",
  "details": "Optional detail or object",
  "traceId": "request-trace-id"
}
```

错误响应不得包含密码、验证码、token 原文、认证材料原文、对象存储凭据或其他敏感信息。

### 2.3 P0-1 错误码候选

| 错误码 | HTTP 状态 | 适用场景 |
|---|---:|---|
| `VALIDATION_FAILED` | 400 | 请求字段缺失、格式错误、长度超限 |
| `INVALID_CAMPUS_EMAIL_DOMAIN` | 400 | 校园邮箱域名不在白名单 |
| `EMAIL_VERIFICATION_CODE_INVALID` | 400 | 验证码错误 |
| `EMAIL_VERIFICATION_CODE_EXPIRED` | 400 | 验证码过期，当前有效期为 10 分钟 |
| `EMAIL_VERIFICATION_TOO_FREQUENT` | 429 | 验证码发送过于频繁 |
| `EMAIL_ALREADY_REGISTERED` | 409 | 注册邮箱已存在 |
| `ACCOUNT_NOT_FOUND` | 404 | 登录账号不存在；实现时也可按安全策略合并为通用登录失败 |
| `INVALID_LOGIN_CREDENTIALS` | 401 | 登录凭证错误 |
| `UNAUTHENTICATED` | 401 | 未携带 access token 或 token 无效 |
| `FORBIDDEN_ACCOUNT_STATUS` | 403 | 账号被限制或状态不允许当前操作 |
| `CAMPUS_EMAIL_NOT_VERIFIED` | 409 | 未完成校园邮箱验证，不能提交认证资料 |
| `AUTHENTICATION_ALREADY_PENDING` | 409 | 已有待审核认证资料，不允许重复提交 |
| `AUTHENTICATION_ALREADY_VERIFIED` | 409 | 已认证用户不允许重复提交普通认证资料 |
| `ATTACHMENT_REQUIRED` | 400 | 缺少必要认证材料附件引用 |
| `ATTACHMENT_INVALID` | 400 | 附件类型、大小、归属或引用无效 |
| `REFRESH_TOKEN_INVALID` | 401 | refresh token 不存在、被撤销、过期或与会话不匹配 |
| `REFRESH_TOKEN_REVOKED` | 401 | refresh token 已撤销 |
| `RESOURCE_NOT_FOUND` | 404 | 接口或资源不存在 |
| `INTERNAL_ERROR` | 500 | 未预期服务端错误 |

## 3. 状态定义

### 3.1 用户认证状态 `UserAuthenticationStatus`

| 状态 | 含义 | 说明 |
|---|---|---|
| `UNVERIFIED` | 未认证 | 已注册或可登录，但尚未提交校内身份认证资料 |
| `PENDING_REVIEW` | 待审核 | 已提交认证资料，等待管理员审核 |
| `VERIFIED` | 已认证 | 管理员审核通过，获得已认证学生能力 |
| `REJECTED` | 已驳回 | 管理员驳回认证申请，可重新提交 |
| `SUSPENDED` | 已限制 | 治理处置导致账号能力受限，不由普通认证流程产生 |

### 3.2 校园邮箱验证状态 `CampusEmailVerificationStatus`

| 状态 | 含义 | 说明 |
|---|---|---|
| `UNVERIFIED` | 未验证 | 尚未完成校园邮箱验证码校验 |
| `CODE_SENT` | 已发送验证码 | 验证码仍可能有效或已过期，查询时可返回剩余时间 |
| `VERIFIED` | 已验证 | 校园邮箱已通过验证码校验 |
| `EXPIRED` | 已过期 | 最近一次验证码已过期 |
| `LOCKED` | 暂时锁定 | 发送或校验尝试过多，暂时限制 |

### 3.3 refresh token / 会话状态 `RefreshTokenStatus`

| 状态 | 含义 | 说明 |
|---|---|---|
| `ACTIVE` | 有效 | 可用于刷新 access token |
| `ROTATED` | 已轮换 | 已通过刷新接口换出新 refresh token |
| `REVOKED` | 已撤销 | 用户退出登录或服务端撤销 |
| `EXPIRED` | 已过期 | 超过有效期，不可再使用 |

### 3.4 认证资料提交状态 `AuthenticationSubmissionStatus`

| 状态 | 含义 | 说明 |
|---|---|---|
| `NOT_SUBMITTED` | 未提交 | 当前没有待审核资料 |
| `SUBMITTED` | 已提交 | 资料已提交，进入审核队列 |
| `UNDER_REVIEW` | 审核中 | 管理端开始处理；可与 `SUBMITTED` 在实现中合并 |
| `APPROVED` | 审核通过 | 对应用户认证状态进入 `VERIFIED` |
| `REJECTED` | 审核驳回 | 对应用户认证状态进入 `REJECTED` |
| `PURGED` | 敏感材料已清理 | 审核完成或驳回后，原始敏感材料已删除或解除业务引用 |

## 4. 接口契约

### 4.1 发送校园邮箱验证码

| 项 | 内容 |
|---|---|
| 接口名称 | 发送校园邮箱验证码 |
| 路径 | `POST /api/auth/campus-email/verification-codes` |
| 是否需要认证 | 否 |
| 对应对象/状态 | `CampusEmailVerification`；`UNVERIFIED` -> `CODE_SENT` |
| 追踪来源 | `SF1`、`IR1`、`13_detailed_design_v1.md` 4.4.5 |
| 后续测试入口建议 | `CampusEmailVerificationEndpointTest.sendCodeRejectsInvalidDomain`；`sendCodeReturnsCooldownMetadata` |

请求体字段：

| 字段 | 类型 | 必填 | 说明 |
|---|---|---|---|
| `campusEmail` | string | 是 | 校园邮箱，需命中学校域名白名单 |
| `purpose` | string | 是 | `REGISTER_OR_LOGIN`、`BIND_OR_VERIFY`；V1 可先支持 `REGISTER_OR_LOGIN` |

成功响应字段：

| 字段 | 类型 | 说明 |
|---|---|---|
| `campusEmail` | string | 脱敏后的校园邮箱 |
| `verificationStatus` | string | `CODE_SENT` |
| `expiresInSeconds` | number | 验证码剩余有效秒数，默认 600 |
| `resendAfterSeconds` | number | 下次允许发送前的等待秒数 |

主要错误码：`VALIDATION_FAILED`、`INVALID_CAMPUS_EMAIL_DOMAIN`、`EMAIL_VERIFICATION_TOO_FREQUENT`。

实现回链：

- 2026-05-18 已完成该接口第一版最小实现。
- 测试记录：`D:\big_homework\docs\validation\20260518_backend_campus_email_verification_code_test_record.md`
- 测试文件：`D:\big_homework\backend\src\test\java\com\campusbuddy\auth\CampusEmailVerificationEndpointTest.java`
- 实现文件：
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\auth\CampusEmailVerificationController.java`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\auth\CampusEmailVerificationService.java`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\auth\CampusEmailVerificationCodeSender.java`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\auth\NoopCampusEmailVerificationCodeSender.java`
- 当前实现说明：
  - 已支持合法校园邮箱返回 `CODE_SENT`、`expiresInSeconds = 600`、`resendAfterSeconds = 60`。
  - 已支持非白名单邮箱返回 `INVALID_CAMPUS_EMAIL_DOMAIN`。
  - 已支持缺失或格式错误字段返回 `VALIDATION_FAILED`。
  - 已支持 60 秒内重复发送返回 `EMAIL_VERIFICATION_TOO_FREQUENT`。
  - 当前白名单域名为最小闭环测试值 `campus.edu.cn`；真实学校域名后续需要配置化。
  - 当前邮件发送为 no-op 测试替身，不接入真实生产邮件发送。

### 4.2 校验校园邮箱验证码

| 项 | 内容 |
|---|---|
| 接口名称 | 校验校园邮箱验证码 |
| 路径 | `POST /api/auth/campus-email/verifications` |
| 是否需要认证 | 否；登录后绑定或换绑场景可在后续扩展为需要认证 |
| 对应对象/状态 | `CampusEmailVerification`；`CODE_SENT` -> `VERIFIED` |
| 追踪来源 | `SF1`、`IR1`、`13_detailed_design_v1.md` 4.4.5 |
| 后续测试入口建议 | `CampusEmailVerificationEndpointTest.verifyCodeMarksEmailVerified`；`verifyCodeRejectsExpiredCode` |

请求体字段：

| 字段 | 类型 | 必填 | 说明 |
|---|---|---|---|
| `campusEmail` | string | 是 | 校园邮箱 |
| `code` | string | 是 | 邮件验证码 |
| `purpose` | string | 是 | 与发送验证码时一致 |

成功响应字段：

| 字段 | 类型 | 说明 |
|---|---|---|
| `campusEmail` | string | 脱敏后的校园邮箱 |
| `verificationStatus` | string | `VERIFIED` |
| `verifiedAt` | string | ISO-8601 时间 |
| `verificationTicket` | string | 短期校验凭据，用于注册或后续提交；不等同于登录 token |

主要错误码：`EMAIL_VERIFICATION_CODE_INVALID`、`EMAIL_VERIFICATION_CODE_EXPIRED`、`INVALID_CAMPUS_EMAIL_DOMAIN`、`VALIDATION_FAILED`。

实现回链：
- 2026-05-18 已完成该接口第一版最小实现。
- 测试记录：`D:\big_homework\docs\validation\20260518_backend_campus_email_verification_check_test_record.md`
- 测试文件：`D:\big_homework\backend\src\test\java\com\campusbuddy\auth\CampusEmailVerificationEndpointTest.java`
- 实现文件：
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\auth\CampusEmailVerificationController.java`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\auth\CampusEmailVerificationService.java`
- 当前实现说明：
  - 已支持正确验证码返回 `VERIFIED`、`verifiedAt`、`verificationTicket`。
  - 已支持错误验证码返回 `EMAIL_VERIFICATION_CODE_INVALID`。
  - 已支持过期验证码返回 `EMAIL_VERIFICATION_CODE_EXPIRED`；测试通过可注入 `Clock` 做可控时间验证。
  - 已支持非白名单邮箱返回 `INVALID_CAMPUS_EMAIL_DOMAIN`。
  - 已支持缺失或格式错误字段返回 `VALIDATION_FAILED`。
  - 当前验证码记录仍为进程内内存实现，验证码只保存哈希，不在响应或日志中输出明文。
- 2026-05-18 09:19 复核：本批接口测试 9 个通过；完整回归受 Docker/Testcontainers 不可用阻塞，非容器回归 15 个通过。

### 4.3 注册或账号创建

| 项 | 内容 |
|---|---|
| 接口名称 | 注册或账号创建 |
| 路径 | `POST /api/auth/register` |
| 是否需要认证 | 否 |
| 对应对象/状态 | `User`、`UserProfile`；`UserAuthenticationStatus = UNVERIFIED`；`CampusEmailVerificationStatus = VERIFIED` |
| 追踪来源 | `SF1`、`IR1`、`IR2`、`04_product_design_v1.md` 7.1 |
| 后续测试入口建议 | `AuthRegistrationEndpointTest.registerCreatesUnverifiedAccount`；`registerRejectsUnverifiedCampusEmail` |

请求体字段：

| 字段 | 类型 | 必填 | 说明 |
|---|---|---|---|
| `campusEmail` | string | 是 | 已验证校园邮箱 |
| `verificationTicket` | string | 是 | 校园邮箱校验成功后返回的短期凭据 |
| `password` | string | 是 | 密码；后续实现必须 BCrypt 哈希存储 |
| `displayName` | string | 是 | 昵称或展示名 |
| `college` | string | 否 | 学院，可在认证资料提交时补全 |
| `grade` | string | 否 | 年级，可在认证资料提交时补全 |

成功响应字段：

| 字段 | 类型 | 说明 |
|---|---|---|
| `userId` | string | 用户标识 |
| `campusEmail` | string | 脱敏校园邮箱 |
| `displayName` | string | 展示名 |
| `authenticationStatus` | string | `UNVERIFIED` |
| `campusEmailVerificationStatus` | string | `VERIFIED` |
| `createdAt` | string | ISO-8601 时间 |

本接口 V1 契约推荐不自动返回 access token，注册后可复用登录接口获取令牌，以降低第一项实现复杂度。若后续实现线程希望注册即登录，必须在测试入口中明确补充 token 响应断言。

主要错误码：`EMAIL_ALREADY_REGISTERED`、`CAMPUS_EMAIL_NOT_VERIFIED`、`VALIDATION_FAILED`、`INVALID_CAMPUS_EMAIL_DOMAIN`。

实现回链：
- 2026-05-18 已完成该接口第一版最小实现。
- 测试记录：`D:\big_homework\docs\validation\20260518_backend_auth_register_test_record.md`
- 测试文件：`D:\big_homework\backend\src\test\java\com\campusbuddy\auth\AuthRegistrationEndpointTest.java`
- 实现文件：
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\auth\AuthRegistrationController.java`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\auth\AuthRegistrationService.java`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\auth\CampusEmailVerificationService.java`
- 当前实现说明：
  - 已支持已验证校园邮箱和有效 `verificationTicket` 创建账号，返回 `userId`、脱敏 `campusEmail`、`displayName`、`authenticationStatus=UNVERIFIED`、`campusEmailVerificationStatus=VERIFIED`、`createdAt`。
  - 已支持无效或未验证 `verificationTicket` 返回 `CAMPUS_EMAIL_NOT_VERIFIED`。
  - 已支持重复邮箱返回 `EMAIL_ALREADY_REGISTERED`。
  - 已支持非白名单邮箱返回 `INVALID_CAMPUS_EMAIL_DOMAIN`。
  - 已支持缺失或格式错误字段返回 `VALIDATION_FAILED`。
  - 当前账号与 ticket 为进程内内存实现，密码仅保存 BCrypt 哈希；本批不签发 access token 或 refresh token。
  - 2026-05-18 10:32 复核补充：已新增 `AuthRegistrationEndpointTest.registerStoresBCryptPasswordHashOnly`，本批注册接口测试更新为 6 个用例，明确覆盖“不保存明文密码、仅保存 BCrypt 哈希”约束。

### 4.4 登录

| 项 | 内容 |
|---|---|
| 接口名称 | 登录 |
| 路径 | `POST /api/auth/login` |
| 是否需要认证 | 否 |
| 对应对象/状态 | `User`、`RefreshTokenRecord`；创建 `ACTIVE` 会话 |
| 追踪来源 | `SF1`、`IR1`、`13_detailed_design_v1.md` 4.4.5 |
| 后续测试入口建议 | `AuthLoginEndpointTest.loginReturnsAccessAndRefreshTokens`；`loginRejectsInvalidCredentials` |

请求体字段：

| 字段 | 类型 | 必填 | 说明 |
|---|---|---|---|
| `campusEmail` | string | 是 | 校园邮箱 |
| `password` | string | 是 | 登录密码 |
| `clientName` | string | 否 | 客户端名称，例如 `windows-qt` |
| `deviceId` | string | 否 | 设备标识；V1 可作为非必填占位 |

成功响应字段：

| 字段 | 类型 | 说明 |
|---|---|---|
| `accessToken` | string | 短期访问令牌 |
| `accessTokenExpiresInSeconds` | number | access token 有效期 |
| `refreshToken` | string | 刷新令牌 |
| `refreshTokenExpiresInSeconds` | number | refresh token 有效期 |
| `tokenType` | string | `Bearer` |
| `user` | object | 当前用户摘要 |
| `user.userId` | string | 用户标识 |
| `user.displayName` | string | 展示名 |
| `user.authenticationStatus` | string | 用户认证状态 |
| `user.campusEmailVerificationStatus` | string | 校园邮箱验证状态 |

主要错误码：`INVALID_LOGIN_CREDENTIALS`、`FORBIDDEN_ACCOUNT_STATUS`、`VALIDATION_FAILED`。

实现回链：
- 2026-05-18 已完成该接口第一版最小实现。
- 测试记录：`D:\big_homework\docs\validation\20260518_backend_auth_login_test_record.md`
- 测试文件：`D:\big_homework\backend\src\test\java\com\campusbuddy\auth\AuthLoginEndpointTest.java`
- 实现文件：
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\auth\AuthLoginController.java`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\auth\AuthLoginService.java`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\auth\AuthRegistrationService.java`
- 当前实现说明：
  - 已支持已注册账号通过校园邮箱和密码登录，成功返回占位 `accessToken`、`refreshToken`、过期秒数、`tokenType=Bearer` 和当前用户摘要。
  - 登录密码校验复用注册时保存的 BCrypt 哈希，通过 `BCryptPasswordEncoder.matches` 完成，不保存或比较明文密码。
  - 已支持错误邮箱或错误密码统一返回 `INVALID_LOGIN_CREDENTIALS`，避免暴露账号是否存在。
  - 已支持缺失或格式错误字段返回 `VALIDATION_FAILED`。
  - 当前 token 仍为最小闭环占位格式，不是真实 JWT；refresh token 仅保存 SHA-256 哈希会话占位，不实现刷新轮换或退出登录。
  - 本批没有实现 RBAC、认证资料提交、Qt 页面、数据库业务表、JPA Entity/Repository 或真实 token 验签链路。

### 4.5 刷新 token

| 项 | 内容 |
|---|---|
| 接口名称 | 刷新 token |
| 路径 | `POST /api/auth/token/refresh` |
| 是否需要认证 | 否；使用请求体中的 refresh token 完成认证 |
| 对应对象/状态 | `RefreshTokenRecord`；`ACTIVE` -> `ROTATED`，并创建新 `ACTIVE` |
| 追踪来源 | `13_detailed_design_v1.md` 3.3、4.4.4、4.4.5 |
| 后续测试入口建议 | `AuthTokenEndpointTest.refreshRotatesRefreshToken`；`refreshRejectsRevokedToken` |

请求体字段：

| 字段 | 类型 | 必填 | 说明 |
|---|---|---|---|
| `refreshToken` | string | 是 | 当前 refresh token |
| `deviceId` | string | 否 | 设备标识；用于后续会话策略 |

成功响应字段：

| 字段 | 类型 | 说明 |
|---|---|---|
| `accessToken` | string | 新 access token |
| `accessTokenExpiresInSeconds` | number | access token 有效期 |
| `refreshToken` | string | 新 refresh token |
| `refreshTokenExpiresInSeconds` | number | refresh token 有效期 |
| `tokenType` | string | `Bearer` |

主要错误码：`REFRESH_TOKEN_INVALID`、`REFRESH_TOKEN_REVOKED`、`FORBIDDEN_ACCOUNT_STATUS`、`VALIDATION_FAILED`。

### 4.6 退出登录/撤销 token

| 项 | 内容 |
|---|---|
| 接口名称 | 退出登录/撤销 token |
| 路径 | `POST /api/auth/logout` |
| 是否需要认证 | 是，需 access token；请求体可携带 refresh token 用于精确撤销 |
| 对应对象/状态 | `RefreshTokenRecord`；`ACTIVE` -> `REVOKED` |
| 追踪来源 | `13_detailed_design_v1.md` 4.4.1、4.4.5 |
| 后续测试入口建议 | `AuthLogoutEndpointTest.logoutRevokesRefreshToken`；`logoutWithoutAccessTokenReturns401` |

请求体字段：

| 字段 | 类型 | 必填 | 说明 |
|---|---|---|---|
| `refreshToken` | string | 否 | 当前会话 refresh token；未传时后续实现可按当前主体撤销当前会话 |
| `logoutAllDevices` | boolean | 否 | 是否退出所有设备；V1 默认 `false`，可先不实现全设备撤销 |

成功响应字段：

| 字段 | 类型 | 说明 |
|---|---|---|
| `revoked` | boolean | 是否完成撤销 |
| `revokedAt` | string | ISO-8601 时间 |

主要错误码：`UNAUTHENTICATED`、`REFRESH_TOKEN_INVALID`、`FORBIDDEN_ACCOUNT_STATUS`。

### 4.7 提交认证资料

| 项 | 内容 |
|---|---|
| 接口名称 | 认证资料提交 |
| 路径 | `POST /api/me/authentication/submissions` |
| 是否需要认证 | 是 |
| 对应对象/状态 | `AuthenticationRecord`、`UserProfile`、`Attachment`；`UNVERIFIED` 或 `REJECTED` -> `PENDING_REVIEW` |
| 追踪来源 | `SF1`、`IR2`、`13_detailed_design_v1.md` 4.4.3、4.4.4、4.4.5 |
| 后续测试入口建议 | `AuthenticationSubmissionEndpointTest.submitCreatesPendingReviewRecord`；`submitRejectsWhenEmailNotVerified` |

请求体字段：

| 字段 | 类型 | 必填 | 说明 |
|---|---|---|---|
| `realName` | string | 是 | 真实姓名；仅用于认证审核，不在普通接口展示 |
| `studentNumber` | string | 是 | 学号 |
| `college` | string | 是 | 学院 |
| `grade` | string | 是 | 年级 |
| `major` | string | 否 | 专业 |
| `campusEmail` | string | 是 | 已验证校园邮箱，应与当前账号绑定邮箱一致或进入后续换绑流程 |
| `materialAttachmentIds` | array<string> | 是 | 认证材料附件引用，具体上传接口不在本线程实现 |
| `statementAccepted` | boolean | 是 | 用户确认材料真实、授权用于认证审核 |

成功响应字段：

| 字段 | 类型 | 说明 |
|---|---|---|
| `authenticationRecordId` | string | 认证记录标识 |
| `authenticationStatus` | string | `PENDING_REVIEW` |
| `submissionStatus` | string | `SUBMITTED` |
| `submittedAt` | string | ISO-8601 时间 |
| `expectedReviewHint` | string | 审核时效提示，例如通常不超过 1 天 |

主要错误码：`UNAUTHENTICATED`、`CAMPUS_EMAIL_NOT_VERIFIED`、`AUTHENTICATION_ALREADY_PENDING`、`AUTHENTICATION_ALREADY_VERIFIED`、`ATTACHMENT_REQUIRED`、`ATTACHMENT_INVALID`、`VALIDATION_FAILED`、`FORBIDDEN_ACCOUNT_STATUS`。

### 4.8 查询认证状态

| 项 | 内容 |
|---|---|
| 接口名称 | 认证状态查询 |
| 路径 | `GET /api/me/authentication/status` |
| 是否需要认证 | 是 |
| 对应对象/状态 | `User`、`UserProfile`、`AuthenticationRecord`、`CampusEmailVerification` |
| 追踪来源 | `SF1`、`IR2`、`04_product_design_v1.md` 身份认证页、`13_detailed_design_v1.md` 4.4.5 |
| 后续测试入口建议 | `AuthenticationStatusEndpointTest.getStatusReturnsCurrentUserAuthenticationState`；`getStatusDoesNotReturnSensitiveMaterials` |

请求参数：无。

成功响应字段：

| 字段 | 类型 | 说明 |
|---|---|---|
| `userId` | string | 当前用户标识 |
| `authenticationStatus` | string | 用户认证状态 |
| `campusEmail` | string | 脱敏校园邮箱 |
| `campusEmailVerificationStatus` | string | 校园邮箱验证状态 |
| `submissionStatus` | string | 当前认证资料提交状态 |
| `submittedAt` | string/null | 最近一次提交时间 |
| `reviewedAt` | string/null | 最近一次审核时间 |
| `rejectionReason` | string/null | 驳回原因；不得包含敏感材料原文 |
| `verifiedAt` | string/null | 认证通过时间 |
| `sensitiveMaterialRetained` | boolean | 审核完成后应为 `false`；待审核时可为 `true` |

主要错误码：`UNAUTHENTICATED`、`FORBIDDEN_ACCOUNT_STATUS`。

## 5. 接口与需求追踪矩阵

| 接口 | 需求编号/来源 | 设计对象 | 状态关注点 | 后续测试入口 |
|---|---|---|---|---|
| `POST /api/auth/campus-email/verification-codes` | `SF1`、`IR1` | `CampusEmailVerification` | `UNVERIFIED` -> `CODE_SENT` | 邮箱域名、发送冷却、过期时间 |
| `POST /api/auth/campus-email/verifications` | `SF1`、`IR1` | `CampusEmailVerification` | `CODE_SENT` -> `VERIFIED` | 正确验证码、错误验证码、过期验证码 |
| `POST /api/auth/register` | `SF1`、`IR1`、`IR2` | `User`、`UserProfile` | 初始 `UNVERIFIED` | 创建账号、重复邮箱、未验证邮箱拒绝 |
| `POST /api/auth/login` | `SF1`、`IR1` | `User`、`RefreshTokenRecord` | 创建 `ACTIVE` 会话 | 成功登录、错误凭证、受限账号 |
| `POST /api/auth/token/refresh` | 技术约束 JWT + refresh token | `RefreshTokenRecord` | `ACTIVE` -> `ROTATED` | 刷新轮换、撤销/过期 token 拒绝 |
| `POST /api/auth/logout` | `13_detailed_design_v1.md` 4.4 | `RefreshTokenRecord` | `ACTIVE` -> `REVOKED` | 撤销当前 token、未认证 401 |
| `POST /api/me/authentication/submissions` | `SF1`、`IR2` | `AuthenticationRecord`、`Attachment` | `UNVERIFIED/REJECTED` -> `PENDING_REVIEW` | 邮箱未验证拒绝、资料完整性、附件引用 |
| `GET /api/me/authentication/status` | `SF1`、`IR2` | `User`、`AuthenticationRecord` | 状态回显 | 状态字段完整、不返回敏感材料 |

## 6. OpenAPI 与测试入口建议

后续实现时应按接口先契约化原则补充 OpenAPI 3 文档：

- 每个接口声明认证要求。
- DTO 字段必须有说明、必填约束和示例。
- 错误响应统一引用 `ApiErrorResponse`。
- access token 安全方案命名建议为 `bearerAuth`。
- refresh token 接口不使用 bearerAuth，而在请求体中提交 refresh token。

后续测试建议按以下顺序建立：

1. 邮箱验证码发送接口测试。
2. 邮箱验证码校验接口测试。
3. 注册接口测试。
4. 登录接口测试。
5. 刷新 token 接口测试。
6. 退出登录接口测试。
7. 认证资料提交接口测试。
8. 认证状态查询接口测试。

每个实现线程必须遵守 `docs/12_code_generation_constraints_v1.md`：先写测试，确认红灯，再做最小实现，绿灯后回归并留档。

## 7. 当前不实现

本线程明确不实现以下内容：

- 不新增业务代码。
- 不创建 Controller、Service、Entity、Repository、DTO 或 Qt 页面代码。
- 不实现完整注册、登录、真实 JWT 签发验签、refresh token 轮换、撤销策略或 RBAC。
- 不实现数据库业务表、Flyway 业务迁移、JPA 实体或 Repository。
- 不实现客户端 token 存储、`SecureTokenStore`、认证头注入或 Qt 登录页面。
- 不实现认证材料附件上传接口；本版只引用 `materialAttachmentIds`，附件上传契约需另行收束。
- 不实现管理员审核认证资料接口；审核动作归入 P1 管理端治理，当前只定义审核结果会影响用户认证状态。
- 不扩展到需求发布、审核、广场、低压力联系、评价、投诉或治理模块。

## 8. 发现的最小缺口与推荐取舍

当前设计可支撑 P0-1 第一版接口契约。唯一需要后续最小补齐的缺口是：认证材料附件上传接口尚未契约化，而认证资料提交接口需要引用附件 ID。

推荐取舍：

- 本线程不展开附件上传，避免扩展到对象存储和文件处理实现。
- 后续在实现认证资料提交前，若发现无法以占位附件 ID 完成测试，可新开一个最小设计闭环：`认证材料附件上传接口契约`。
- 该闭环只定义上传路径、文件类型、大小限制、附件归属和错误码，不实现对象存储生产能力。

## 9. 后续第一项真实实现建议

建议新开实现线程，第一项真实业务实现选择：`发送校园邮箱验证码接口`。

推荐理由：

- 它是 P0-1 账号准入链路的最小入口。
- 可严格测试先行，覆盖邮箱格式、校园域名白名单、发送冷却和统一错误结构。
- 可以先使用可控的邮件发送端口或测试替身，不必立即实现完整注册登录、真实 JWT、数据库账号体系或 Qt 页面。

建议实现边界：

- 只实现 `POST /api/auth/campus-email/verification-codes`。
- 先写接口测试并确认红灯。
- 实现最小 Controller/Service/DTO/测试替身时必须只服务该接口，不顺手实现注册登录。
- 绿灯后运行必要后端回归，并在 `docs/validation/` 新增测试记录。
- 更新 `docs/03_current_plan.md` 与 `handoff/latest.md`。

## 10. 可直接复制的下一线程启动 prompt

```text
请先阅读：
- D:\big_homework\AGENTS.md
- D:\big_homework\docs\03_current_plan.md
- D:\big_homework\handoff\latest.md
- D:\big_homework\docs\12_code_generation_constraints_v1.md
- D:\big_homework\docs\13_detailed_design_v1.md
- D:\big_homework\docs\15_p0_auth_api_contract_v1.md
- D:\big_homework\docs\validation\20260518_backend_rest_contract_test_record.md
- D:\big_homework\docs\validation\20260518_backend_jwt_probe_test_record.md

当前任务：测试先行实现《校园搭子平台》P0-1 第一项真实业务接口：发送校园邮箱验证码。

本线程目标：
1. 只实现 `POST /api/auth/campus-email/verification-codes` 一个接口的最小闭环。
2. 严格按测试先行推进：先写接口测试并确认未实现时红灯，再做最小实现，最后绿灯、回归、留档。
3. 实现结果必须回链到 `docs/15_p0_auth_api_contract_v1.md` 中对应接口契约。

严格限制：
1. 不实现完整业务系统。
2. 不一次性实现注册、登录、真实 JWT、refresh token、RBAC、认证资料提交或 Qt 页面。
3. 不创建与本接口无关的 Controller、Service、Entity、Repository 或 DTO。
4. 不接入真实邮件生产发送；可使用测试替身或最小邮件发送抽象，但不得扩大到完整通知系统。
5. 不保存或输出验证码明文到日志。

必须完成：
1. 新增后端接口测试，至少覆盖：
   - 合法校园邮箱发送验证码返回 `CODE_SENT`、`expiresInSeconds`、`resendAfterSeconds`。
   - 非白名单邮箱返回 `INVALID_CAMPUS_EMAIL_DOMAIN`。
   - 缺失或格式错误字段返回 `VALIDATION_FAILED`。
   - 频繁发送返回 `EMAIL_VERIFICATION_TOO_FREQUENT`，如当前最小实现暂不支持冷却，需要在测试记录中说明是否延后。
2. 确认接口未实现时测试红灯。
3. 做最小实现，使本批测试通过。
4. 运行必要后端回归测试。
5. 在 `D:\big_homework\docs\validation\` 写入本批测试记录。
6. 更新 `D:\big_homework\docs\03_current_plan.md` 和 `D:\big_homework\handoff\latest.md`。
```
- 2026-05-18 09:29 复核：`POST /api/auth/campus-email/verifications` 本批接口测试 `.\mvnw.cmd test -Dtest=CampusEmailVerificationEndpointTest` 通过，`Tests run: 9, Failures: 0, Errors: 0, Skipped: 0`；非容器回归 15 个测试通过，完整回归仍受 Docker/Testcontainers 不可用影响。
