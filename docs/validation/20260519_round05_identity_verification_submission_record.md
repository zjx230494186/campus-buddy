# Round 05 验证记录：P0 认证资料提交接口

- 日期：2026-05-19
- 线程：Round 05 — P0 认证资料提交接口

## 测试先行记录

### 红灯（实现前）
- 7 个测试因端点 404 全部失败
- 命令：`.\mvnw.cmd test '-Dspring.testcontainers.enabled=false' '-Dtest=IdentityVerificationEndpointTest'`
- 结果：Tests run: 7, Failures: 7, Errors: 0

### 绿灯（实现后）
- 7 个测试全部通过
- 结果：Tests run: 7, Failures: 0, Errors: 0

### 非容器快速回归
- 命令：`.\mvnw.cmd test '-Dspring.testcontainers.enabled=false' '-Dtest=!DatabaseMigrationTest'`
- 结果：Tests run: 39, Failures: 0, Errors: 0 (32 既有 + 7 新增)

## 新增文件清单

| 文件 | 类型 |
|---|---|
| `V3__create_identity_verification_submission_table.sql` | Flyway 迁移 |
| `IdentityVerificationSubmission.java` | JPA 实体 |
| `IdentityVerificationSubmissionRepository.java` | JPA Repository |
| `IdentityVerificationService.java` | 业务服务 |
| `IdentityVerificationController.java` | REST 控制器 |
| `IdentityVerificationEndpointTest.java` | 集成测试 |

## 修改文件清单

| 文件 | 修改内容 |
|---|---|
| `UserAccount.java` | 新增 `setAuthenticationStatus` 方法 |
| `SecurityConfiguration.java` | 新增 `/api/auth/identity-verifications` 和 `/me` 需 JWT |

## 测试覆盖

1. ✅ 未登录提交认证资料返回 401
2. ✅ UNVERIFIED 用户提交完整资料后返回 PENDING_REVIEW
3. ✅ 提交后查询状态返回 PENDING_REVIEW、submittedAt 和资料摘要
4. ✅ 缺少必填字段返回 VALIDATION_FAILED
5. ✅ PENDING_REVIEW 用户重复提交返回 IDENTITY_VERIFICATION_PENDING
6. ✅ REJECTED 用户重新提交后回到 PENDING_REVIEW
7. ✅ VERIFIED 用户重新提交返回 IDENTITY_ALREADY_VERIFIED

## 接口契约

### POST /api/auth/identity-verifications
- 需要 `Authorization: Bearer <accessToken>`
- 请求体：`{ realName, studentNumber, college, major, grade }`
- 成功返回：`{ authenticationStatus, submittedAt, realName, studentNumber, college, major, grade }`

### GET /api/auth/identity-verifications/me
- 需要 `Authorization: Bearer <accessToken>`
- 返回：`{ authenticationStatus, reviewStatus, submittedAt, reviewedAt, rejectReason, realName, studentNumber, college, major, grade, allowedActions }`

## 本轮边界

- 不处理附件二进制上传（学生证/校园卡 OBS 上传导留后续附件闭环）
- 不实现管理员审核接口
- identity_verification_submission 表仅保存文本身份资料，不保存 base64 或附件内容

## Git 提交

- 哈希：`2ee5b68`
- 消息：`feat(auth): add identity verification submission`
- 文件数：8 files changed, +655 insertions

## 未完成事项

- Testcontainers/Docker 测试未运行（需 Docker 可用）
- 附件上传闭环未实现
- 管理员审核接口未实现
