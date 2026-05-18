# 03 Current Plan

本文只维护当前阶段最重要事项，不承载全部历史。

## 文档编码规则

- 项目 Markdown 文档统一使用 UTF-8 编码保存。
- 不使用 GBK/ANSI 保存长期维护文档。
- 使用 PowerShell 写入中文文档时必须显式指定 `-Encoding UTF8`。

## 当前阶段主题

- 阶段：正式代码开发。
- 当前线程：P0 认证资料提交接口（已完成）。
- 当前目标：校内身份认证资料提交与状态查询后端最小闭环已实现并通过测试。

## Git 提交历史

- `447b0af` chore:-establish-project-baseline-and-backend-configuration
- `18e2a01` chore(repo): clean generated artifacts and record git hygiene
- `157abc0` feat(auth): persist authentication flow and introduce jwt
- `3be73bf` docs(codearts): record round04 handoff and round05 prompt
- `2ee5b68` feat(auth): add identity verification submission

## 已完成事项

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
- 非容器快速测试：39/39 通过（32 既有 + 7 新增）
- Testcontainers 测试：需要 Docker 可用才能运行

## 本轮边界说明

- 不处理附件二进制上传（学生证/校园卡 OBS 上传导留后续附件闭环）
- 不实现管理员审核接口
- 不实现 refresh token 轮换、logout、RBAC
- identity_verification_submission 表仅保存文本身份资料

## 配置边界摘要

可进入项目文档或模板的非敏感配置：
- `campus-buddy.campus-email.*`、`campus-buddy.object-storage.*`、`campus-buddy.security.jwt.*`（不含 secret）

不得进入项目文档、仓库、聊天或 Qt 客户端的敏感配置：
- `JWT_SECRET`（deploy profile）、`OBJECT_STORAGE_ACCESS_KEY_ID`、`OBJECT_STORAGE_SECRET_ACCESS_KEY`、`DB_PASSWORD`

## 推荐下一步

1. `提交 Round 05/06 纯文档留档`
   - 优先级：最高。
   - 目标：当前存在 Round 05 验证记录、交接文档更新和 Round 06 prompt 留档，应先形成纯文档提交，避免下一轮业务开发混入文档尾巴。

2. `管理员审核接口`
   - 优先级：高。
   - 目标：管理员审核认证资料，APPROVED → VERIFIED / REJECTED → rejectReason。
   - 推荐原因：先闭合认证状态流转，不接触真实 OBS 凭证，演示价值更直接。
   - 提示词：`docs/prompts/codearts/20260519_round_06_identity_verification_admin_review.md`。

3. `附件上传闭环（OBS 预签名 + 附件元数据表）`
   - 优先级：高。
   - 前置：管理员审核接口完成后再做更稳妥。
   - 目标：实现学生证/校园卡材料上传到 OBS，附件元数据关联到 identity_verification_submission。

4. `替换 no-op 邮件发送`
   - 优先级：中。
