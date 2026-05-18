# 03 Current Plan

本文只维护当前阶段最重要事项，不承载全部历史。

## 文档编码规则

- 项目 Markdown 文档统一使用 UTF-8 编码保存。
- 不使用 GBK/ANSI 保存长期维护文档。
- 使用 PowerShell 写入中文文档时必须显式指定 `-Encoding UTF8`。

## 当前阶段主题

- 阶段：正式代码开发。
- 当前线程：P0 认证全链路已完成（Round 03-07）。
- 当前目标：P0 认证闭环已全部完成（注册→登录→JWT→认证资料提交→管理员审核→附件上传）；下一步进入 P1 模块或补齐未完成项。

## Git 提交历史

- `447b0af` chore:-establish-project-baseline-and-backend-configuration
- `18e2a01` chore(repo): clean generated artifacts and record git hygiene
- `157abc0` feat(auth): persist authentication flow and introduce jwt
- `3be73bf` docs(codearts): record round04 handoff and round05 prompt
- `2ee5b68` feat(auth): add identity verification submission
- `d5c78fa` docs(codearts): record round05 handoff and round06 prompt
- `4955bee` feat(auth): add identity verification admin review
- `8edd057` feat(auth): add identity material attachment upload

## 已完成事项

### 认证材料附件上传闭环（提交 `8edd057`）
- Flyway V5 迁移：`identity_material_attachment` 表 + ALTER `identity_verification_submission` ADD `material_attachment_id`
- JPA 实体：`IdentityMaterialAttachment` + Repository
- `ObjectStorageService` 接口 + `InMemoryObjectStorageService` 测试替身
- `IdentityMaterialAttachmentService` + `IdentityVerificationMaterialController`（multipart 上传）
- 修改 `IdentityVerificationService` 支持 `materialAttachmentId`（含归属校验）
- 修改 AdminService/AdminController 返回附件摘要 + 管理员受控读取 GET /{submissionId}/material
- Security 配置新增 /materials 需认证
- 9 个集成测试通过，55+1=56/56 回归

### 管理员审核接口（提交 `4955bee`）
- Flyway V4 迁移：`user_account.account_role`，默认 `STUDENT`。
- 最小 ADMIN 授权：`accountRole`、JWT claim、`ROLE_ADMIN`。
- 管理员审核 Service + Controller。
- Security 配置：`/api/admin/**` 需要 `ROLE_ADMIN`，并新增 403 handler。
- 管理员审核闭环：`PENDING_REVIEW -> APPROVED/VERIFIED` 或 `REJECTED/REJECTED`。
- 7 个管理员审核集成测试通过。

### 认证资料提交接口（提交 `2ee5b68`）
- Flyway V3 迁移：`identity_verification_submission` 表
- JPA 实体：`IdentityVerificationSubmission`
- Repository：`IdentityVerificationSubmissionRepository`
- Service：`IdentityVerificationService`（submit + getStatus）
- Controller：`IdentityVerificationController`（POST + GET /me）
- Security 配置：`/api/auth/identity-verifications` 和 `/me` 需要 JWT
- UserAccount 新增 `setAuthenticationStatus` 方法
- 7 个集成测试全部通过

### 业务规则实现
- UNVERIFIED 用户提交资料 → PENDING_REVIEW
- PENDING_REVIEW 用户重复提交 → 409 IDENTITY_VERIFICATION_PENDING
- REJECTED 用户重新提交 → PENDING_REVIEW
- VERIFIED 用户重新提交 → 409 IDENTITY_ALREADY_VERIFIED
- 缺少必填字段 → 400 VALIDATION_FAILED
- 未登录提交 → 401 UNAUTHORIZED

### 当前测试状态
- 非容器快速测试：56/56 通过
- Testcontainers 测试：需要 Docker 可用才能运行

## 本轮边界说明

- 不实现 refresh token 轮换、logout、RBAC
- 真实 OBS SDK 未适配（InMemoryObjectStorageService 是测试替身）
- 真实邮件发送未替换（仍为 no-op）
- 不实现预签名 URL，不让 Qt 客户端直传

## 配置边界摘要

可进入项目文档或模板的非敏感配置：
- `campus-buddy.campus-email.*`、`campus-buddy.object-storage.*`、`campus-buddy.security.jwt.*`（不含 secret）

不得进入项目文档、仓库、聊天或 Qt 客户端的敏感配置：
- `JWT_SECRET`（deploy profile）、`OBJECT_STORAGE_ACCESS_KEY_ID`、`OBJECT_STORAGE_SECRET_ACCESS_KEY`、`DB_PASSWORD`

## 推荐下一步

1. `P1 需求发布与审核模块`
   - 优先级：高。
   - 目标：实现需求（搭子帖）的 CRUD、分类、搜索、状态管理。
   - 需要先完成详细设计。

2. `真实 OBS SDK 适配器`
   - 优先级：中。
   - 目标：实现 HuaweiOBSObjectStorageService 替换 InMemoryObjectStorageService。
   - 需要华为云 OBS 凭据。

3. `替换 no-op 邮件发送`
   - 优先级：中。
   - 目标：接入 SMTP 或华为云邮件服务发送校园邮箱验证码。

4. `Testcontainers 集成测试`
   - 优先级：低。
   - 目标：在 Docker 可用环境运行完整集成测试。

5. `完整 RBAC 权限矩阵`
   - 优先级：低。
   - 目标：STUDENT / ADMIN / SUPER_ADMIN 角色权限细化。
