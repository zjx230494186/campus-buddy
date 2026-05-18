# 2026-05-19 Round 06 CodeArts Prompt：认证资料管理员审核接口

以下提示词用于交给 CodeArts。目标是在 Round 05 已完成学生侧认证资料提交与状态查询的基础上，补齐管理员审核认证资料的后端最小闭环。

```text
你将继续接手《校园搭子平台》开发。请默认使用中文沟通。

本轮名称：
Round 06：认证资料管理员审核接口

本轮背景：
当前已完成：
- `2ee5b68 feat(auth): add identity verification submission`
- 学生侧 `POST /api/auth/identity-verifications`
- 学生侧 `GET /api/auth/identity-verifications/me`
- 非容器快速测试 39/39 通过
- Round 05 明确未完成：附件上传、管理员审核、Testcontainers/Docker 测试

本轮优先做管理员审核，不做附件上传。原因：
- 审核接口能先闭合 `PENDING_REVIEW -> VERIFIED / REJECTED` 状态流转。
- 不需要 OBS 凭证，不接触真实云资源。
- 对演示和后续发布权限控制更直接。

重要前置校验：
开始前必须运行：
- `git status --short --branch`
- `git log --oneline -6`

如果工作区不干净：
1. 若只包含 docs/、handoff/、docs/validation/、docs/prompts/ 下的 Round 05/06 留档文件，先复核这些文档，使用显式文件路径创建一个纯文档提交：
   `docs(codearts): record round05 handoff and round06 prompt`
2. 若包含 backend/、desktop/ 或其他源码/配置改动，立即停止并报告，不要继续开发。
3. 禁止 `git add .` 和 `git add -A`，只能 `git add <明确文件路径>`。

请先阅读：
- AGENTS.md
- docs/03_current_plan.md
- handoff/latest.md
- docs/13_detailed_design_v1.md
- docs/12_code_generation_constraints_v1.md
- docs/21_codearts_prompt_review_workflow_v1.md
- docs/22_codearts_unattended_prompt_engineering_v1.md
- docs/validation/20260519_round05_identity_verification_submission_record.md

本轮目标：
实现认证资料管理员审核后端最小闭环：
1. 管理员可以查看待审核认证资料列表。
2. 管理员可以审核通过，用户 `authenticationStatus` 从 `PENDING_REVIEW` 变为 `VERIFIED`，提交记录 `reviewStatus` 变为 `APPROVED`。
3. 管理员可以审核驳回，用户 `authenticationStatus` 从 `PENDING_REVIEW` 变为 `REJECTED`，提交记录 `reviewStatus` 变为 `REJECTED`，并保存 `rejectReason`。
4. 学生侧 `GET /api/auth/identity-verifications/me` 能看到审核后的状态和驳回原因。

本轮允许做：
1. 新增最小 ADMIN 角色字段或等价的最小管理员授权机制。
2. 必要时新增 Flyway 迁移，例如给 `user_account` 增加 `account_role`，默认 `STUDENT`。
3. 修改 JWT 签发与过滤器，使 ADMIN 用户能获得 `ROLE_ADMIN` 权限。
4. 新增管理员审核 Controller / Service / DTO。
5. 新增管理员审核相关测试。
6. 更新验证记录、docs/03_current_plan.md、handoff/latest.md。
7. 门禁通过后创建一个开发提交。

本轮明确不做：
1. 不做完整 RBAC 权限矩阵。
2. 不做管理员登录后台 UI。
3. 不做管理员账号创建页面。
4. 不做多角色、多权限、权限菜单、审计日志系统。
5. 不做附件上传、OBS 读写、预签名 URL、附件元数据表。
6. 不接触真实 AccessKey、SecretKey、JWT_SECRET、DB_PASSWORD。
7. 不修改 Qt 客户端。
8. 不实现发布需求、私信、评价、投诉、邮件发送等其他模块。

建议接口契约：

1. `GET /api/admin/identity-verifications?status=PENDING_REVIEW`
   - 需要 `Authorization: Bearer <adminAccessToken>`
   - 仅 ADMIN 可访问。
   - 返回待审核列表，每项至少包含：
     - `submissionId`
     - `userId`
     - `realName`
     - `studentNumber`
     - `college`
     - `major`
     - `grade`
     - `reviewStatus`
     - `submittedAt`

2. `POST /api/admin/identity-verifications/{submissionId}/reviews`
   - 需要 `Authorization: Bearer <adminAccessToken>`
   - 请求体：
     - `decision`: `APPROVED` 或 `REJECTED`
     - `rejectReason`: 驳回时必填，通过时应为空或忽略
   - 通过成功返回：
     - `reviewStatus=APPROVED`
     - `authenticationStatus=VERIFIED`
     - `reviewedAt`
   - 驳回成功返回：
     - `reviewStatus=REJECTED`
     - `authenticationStatus=REJECTED`
     - `rejectReason`
     - `reviewedAt`

如既有文档中已有更明确接口路径，应优先沿用既有路径；如果发现冲突，停止并列出冲突，不要自行扩展多套接口。

最小管理员授权建议：
- 可以给 `user_account` 增加 `accountRole` 字段，默认 `STUDENT`。
- 测试中可通过 Repository 把某个账号设置为 `ADMIN`，然后重新登录获取带管理员权限的 JWT。
- JWT 中可增加 `accountRole` claim。
- `JwtAuthenticationFilter` 应同时保留现有认证状态角色，并在 `accountRole=ADMIN` 时授予 `ROLE_ADMIN`。
- Security 配置中 `/api/admin/**` 需要 `ROLE_ADMIN`。
- 这只是最小管理员授权，不是完整 RBAC。

测试先行要求：
必须先新增或修改测试，并确认实现前这些测试应失败。建议至少覆盖：
1. 普通学生访问管理员待审核列表返回 403。
2. 未登录访问管理员接口返回 401。
3. ADMIN 可以查询 `PENDING_REVIEW` 认证资料列表。
4. ADMIN 审核通过后：
   - submission.reviewStatus = `APPROVED`
   - user.authenticationStatus = `VERIFIED`
   - 学生重新登录或查询状态能看到 `VERIFIED`
5. ADMIN 审核驳回后：
   - submission.reviewStatus = `REJECTED`
   - user.authenticationStatus = `REJECTED`
   - rejectReason 保存并能在学生状态查询中看到
6. `REJECTED` 决策缺少 rejectReason 返回 `VALIDATION_FAILED`
7. 非 `PENDING_REVIEW` 的 submission 重复审核返回冲突错误
8. 非容器快速回归仍通过

开发预算：
- 本轮最多完成 2 个小闭环：最小 ADMIN 授权、审核接口。
- 如果需要新增超过 12 个源代码文件，停止并说明原因。
- 同一类测试失败最多修复 2 次；超过后停止并输出诊断报告。
- 不为了绿灯删除关键测试、弱化断言或绕过真实状态流转。

验证命令：
优先运行本轮相关测试，例如：
- `.\mvnw.cmd test '-Dspring.testcontainers.enabled=false' '-Dtest=IdentityVerificationAdminReviewEndpointTest,IdentityVerificationEndpointTest,AuthLoginEndpointTest,SecurityProbeEndpointTest'`

最后运行非容器快速回归：
- `.\mvnw.cmd test '-Dspring.testcontainers.enabled=false' '-Dtest=!DatabaseMigrationTest'`

如果 Docker/Testcontainers 不可用：
- 不要声称数据库迁移容器测试已完成。
- 可以完成非容器测试和代码级验证。
- 在 validation 中明确 Testcontainers/Docker 未验证。

Git 纪律：
开始前、提交前、提交后都必须运行：
- `git status --short --branch`

提交前必须运行：
- `git diff --name-only`
- `git diff --stat`

禁止：
- `git add .`
- `git add -A`
- 提交 `backend/target/`
- 提交 `desktop/build*/`
- 提交 `.ninja_*`
- 提交 `.arts/`
- 提交测试输出日志
- 提交真实密钥、密码、token、AccessKey、SecretKey、SSH 私钥

提交策略：
本轮最多创建一个开发提交。
推荐提交信息：
`feat(auth): add identity verification admin review`

完成后请输出：
1. 本轮实际完成了什么。
2. 测试先行的红灯/绿灯记录。
3. 最终测试命令与结果。
4. validation 记录路径。
5. 创建的提交哈希。
6. 当前 `git status --short --branch` 摘要。
7. 明确未完成事项，尤其是附件/OBS、完整 RBAC、Testcontainers 是否未完成。
8. 下一轮建议。
```

