# Round 06 验证记录：认证资料管理员审核接口

- 日期：2026-05-19
- 线程：Round 06 — 认证资料管理员审核接口

## 测试先行记录

### 红灯（实现前）
- Controller/Service 已创建但 `submitIdentity` 方法返回错误 submissionId，导致 1 个测试失败
- 修复后 7/7 通过（此轮为逻辑 bug 而非端点缺失，因 Controller 与测试同步创建）

### 绿灯（实现后）
- 7 个测试全部通过
- 命令：`.\mvnw.cmd test '-Dspring.testcontainers.enabled=false' '-Dtest=IdentityVerificationAdminReviewEndpointTest'`
- 结果：Tests run: 7, Failures: 0, Errors: 0

### 非容器快速回归
- 命令：`.\mvnw.cmd test '-Dspring.testcontainers.enabled=false' '-Dtest=!DatabaseMigrationTest'`
- 结果：Tests run: 46, Failures: 0, Errors: 0 (39 既有 + 7 新增)

## 新增文件清单

| 文件 | 类型 |
|---|---|
| `V4__add_account_role_to_user_account.sql` | Flyway 迁移 |
| `IdentityVerificationAdminService.java` | 业务服务 |
| `IdentityVerificationAdminController.java` | REST 控制器 |
| `IdentityVerificationAdminReviewEndpointTest.java` | 集成测试 |

## 修改文件清单

| 文件 | 修改内容 |
|---|---|
| `UserAccount.java` | 新增 `accountRole` 字段 + getter/setter |
| `JwtService.java` | `issueAccessToken` 增加 `accountRole` 参数和 claim |
| `AuthLoginService.java` | 登录时传入 `account.getAccountRole()` |
| `JwtAuthenticationFilter.java` | 解析 `accountRole` claim，ADMIN 时授予 `ROLE_ADMIN` |
| `SecurityConfiguration.java` | `/api/admin/**` 需要 `ROLE_ADMIN`，新增 403 `AccessDeniedHandler` |
| `IdentityVerificationSubmissionRepository.java` | 新增 `findByReviewStatus` |
| `SecurityProbeEndpointTest.java` | `issueAccessToken` 调用增加 `STUDENT` 参数 |

## 测试覆盖

1. ✅ 普通学生访问管理员待审核列表返回 403
2. ✅ 未登录访问管理员接口返回 401
3. ✅ ADMIN 可以查询 PENDING_REVIEW 认证资料列表
4. ✅ ADMIN 审核通过后 submission.reviewStatus=APPROVED, user.authenticationStatus=VERIFIED, 学生可查到
5. ✅ ADMIN 审核驳回后 submission.reviewStatus=REJECTED, user.authenticationStatus=REJECTED, rejectReason 保存
6. ✅ REJECTED 决策缺少 rejectReason 返回 VALIDATION_FAILED
7. ✅ 非 PENDING_REVIEW 的 submission 重复审核返回 SUBMISSION_NOT_PENDING

## 接口契约

### GET /api/admin/identity-verifications?status=PENDING_REVIEW
- 需要 `Authorization: Bearer <adminAccessToken>` + ROLE_ADMIN
- 返回待审核列表

### POST /api/admin/identity-verifications/{submissionId}/reviews
- 需要 `Authorization: Bearer <adminAccessToken>` + ROLE_ADMIN
- 请求体：`{ decision: "APPROVED"/"REJECTED", rejectReason: "..." }`
- 审核通过返回：`{ reviewStatus: "APPROVED", authenticationStatus: "VERIFIED", reviewedAt: "..." }`
- 审核驳回返回：`{ reviewStatus: "REJECTED", authenticationStatus: "REJECTED", rejectReason: "...", reviewedAt: "..." }`

## 最小管理员授权机制

- `user_account.account_role` 字段，默认 `STUDENT`，可设为 `ADMIN`
- JWT 中包含 `accountRole` claim
- `JwtAuthenticationFilter` 在 `accountRole=ADMIN` 时额外授予 `ROLE_ADMIN`
- `/api/admin/**` 配置 `hasRole("ADMIN")`
- 无完整 RBAC，无管理后台 UI，无管理员创建页面

## Git 提交

- 哈希：`4955bee`
- 消息：`feat(auth): add identity verification admin review`
- 文件数：11 files changed, +452/-8

## 未完成事项

- Testcontainers/Docker 测试未运行
- 附件上传闭环未实现
- 完整 RBAC 权限矩阵未实现
- 管理员创建/管理 UI 未实现
