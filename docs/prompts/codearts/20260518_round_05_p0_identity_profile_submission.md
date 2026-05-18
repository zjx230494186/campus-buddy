# 2026-05-18 Round 05 CodeArts Prompt：P0 认证资料提交接口

以下提示词用于交给 CodeArts。目标是在 P0 账号认证已持久化、JWT 已可用的基础上，进入第一个正式业务闭环：校内身份认证资料提交与状态查询。

```text
你将继续接手《校园搭子平台》开发。请默认使用中文沟通。

本轮名称：
Round 05：P0 认证资料提交接口

本轮背景：
上一轮已经完成 Git 提交门禁与仓库污染清理：
- `18e2a01` chore(repo): clean generated artifacts and record git hygiene
- `157abc0` feat(auth): persist authentication flow and introduce jwt
- 非容器快速测试 32/32 通过
- 敏感信息复查通过
- `git ls-files` 对构建产物/缓存/临时文件模式匹配为空

重要前置校验：
开始前必须运行：
- `git status --short --branch`
- `git log --oneline -5`

如果工作区不干净：
1. 若只包含 docs/、handoff/、docs/validation/、docs/prompts/ 下的 Round 04/05 留档文件，先复核这些文档，使用显式文件路径创建一个文档提交：
   `docs(codearts): record round04 handoff and round05 prompt`
2. 若包含 backend/、desktop/ 或其他源码/配置改动，立即停止并报告，不要继续开发。
3. 禁止 `git add .` 和 `git add -A`，只能 `git add <明确文件路径>`。

请先阅读：
- AGENTS.md
- docs/03_current_plan.md
- handoff/latest.md
- docs/13_detailed_design_v1.md
- docs/06_execution_preparation_v1.md
- docs/12_code_generation_constraints_v1.md
- docs/21_codearts_prompt_review_workflow_v1.md
- docs/22_codearts_unattended_prompt_engineering_v1.md
- docs/validation/20260518_round04_git_commit_gate_and_pollution_audit_record.md

本轮目标：
实现“校内身份认证资料提交与状态查询”的后端最小闭环。

本轮允许做：
1. 新增认证资料相关数据库迁移。
2. 新增必要的 JPA Entity / Repository / Service / Controller / DTO。
3. 新增或修改认证资料提交与状态查询测试。
4. 修改必要的 Security 配置，使认证资料接口需要 JWT 登录。
5. 更新 docs/03_current_plan.md、handoff/latest.md 和本轮 validation 记录。
6. 在门禁通过后创建一个开发提交。

本轮明确不做：
1. 不实现 Qt 客户端页面。
2. 不实现管理员审核接口。
3. 不实现真实 OBS 附件上传/下载/删除接口。
4. 不让 Qt 客户端或用户侧持有 OBS 凭证。
5. 不新增对象存储真实密钥、AccessKey、SecretKey、临时凭证。
6. 不实现 refresh token 轮换、logout、RBAC 权限矩阵。
7. 不实现发布需求、私信、评价、投诉等后续模块。
8. 不修改公网 IP、域名、HTTPS、部署脚本。

业务口径：
1. 用户注册后默认 `authenticationStatus=UNVERIFIED`。
2. 用户必须登录后才能提交认证资料和查询自己的认证状态。
3. 提交认证资料后，用户认证状态从 `UNVERIFIED` 进入 `PENDING_REVIEW`。
4. `REJECTED` 用户允许重新提交，重新提交后回到 `PENDING_REVIEW`。
5. `PENDING_REVIEW` 用户重复提交应返回明确冲突错误，不能覆盖待审核资料。
6. `VERIFIED` 用户不允许重新提交普通认证资料，除非未来实现重新认证流程；本轮返回明确冲突错误。
7. 管理员审核结果本轮不实现，只保留未来管理端可扩展的数据结构。
8. 本轮不处理附件二进制上传。学生证/校园卡等材料原件的 OBS 上传和附件元数据表留到后续附件闭环；本轮可只实现文本身份资料与状态流转，并在 validation 中记录这个边界。

建议接口契约：
1. `POST /api/auth/identity-verifications`
   - 需要 `Authorization: Bearer <accessToken>`
   - 请求体建议包含：
     - `realName`
     - `studentNumber`
     - `college`
     - `major`
     - `grade`
   - 本轮不接收文件，不接收 OBS key，不接收 base64。
   - 成功返回：
     - `authenticationStatus = PENDING_REVIEW`
     - `submittedAt`
     - 当前资料摘要

2. `GET /api/auth/identity-verifications/me`
   - 需要 `Authorization: Bearer <accessToken>`
   - 返回：
     - `authenticationStatus`
     - `profileReviewStatus`
     - `submittedAt`
     - `reviewedAt`
     - `rejectReason`
     - `allowedActions`
     - 当前用户可见的资料摘要

如果你发现既有文档中已有更明确接口路径，应优先沿用既有文档；如果存在冲突，停止并列出冲突，不要自行扩展成多套接口。

测试先行要求：
必须先新增或修改测试，并确认实现前这些测试应失败。建议至少覆盖：
1. 未登录提交认证资料返回 401/未授权。
2. 已登录 `UNVERIFIED` 用户提交完整资料后返回 `PENDING_REVIEW`。
3. 提交后查询状态返回 `PENDING_REVIEW`、`submittedAt` 和资料摘要。
4. 缺少必填字段返回 `VALIDATION_FAILED`。
5. 同一用户处于 `PENDING_REVIEW` 时重复提交返回冲突错误。
6. `REJECTED` 用户重新提交后回到 `PENDING_REVIEW`（如当前没有制造 REJECTED 状态的公共接口，可在测试中通过 Repository/Entity 准备数据，不新增管理员接口）。
7. 非容器快速回归仍通过。

开发边界：
- 每个小闭环最多修改 8 个源代码文件。
- 如果需要新增超过 10 个源代码文件，停止并说明原因。
- 同一类测试失败最多修复 2 次；超过后停止并输出诊断报告。
- 不为了绿灯删除关键测试、降低断言质量或 mock 掉本轮核心路径。

数据库建议：
- 使用 Flyway 新增 `V3__create_identity_verification_tables.sql` 或等价命名。
- 表结构只保存本轮必要字段；避免保存学生证/校园卡原件、base64 或真实附件内容。
- 可以为用户当前认证申请建表，例如 `identity_verification_submission`。
- 注意与 `user_account.authentication_status` 保持一致更新。

验证命令：
优先运行本轮相关测试，例如：
- `.\mvnw.cmd test '-Dspring.testcontainers.enabled=false' '-Dtest=IdentityVerificationEndpointTest,AuthLoginEndpointTest,AuthRegistrationEndpointTest,SecurityProbeEndpointTest'`

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
`feat(auth): add identity verification submission`

完成后请输出：
1. 本轮实际完成了什么。
2. 测试先行的红灯/绿灯记录。
3. 最终测试命令与结果。
4. validation 记录路径。
5. 创建的提交哈希。
6. 当前 `git status --short --branch` 摘要。
7. 明确未完成事项，尤其是附件/OBS、管理员审核、Testcontainers 是否未完成。
8. 下一轮建议。
```

