# 2026-05-19 Round 07 CodeArts Prompt：认证材料附件上传闭环

以下提示词用于交给 CodeArts。目标是在学生认证资料提交和管理员审核已完成的基础上，实现认证材料附件的后端中转上传、元数据关联和受控读取最小闭环。

```text
你将继续接手《校园搭子平台》开发。请默认使用中文沟通。

本轮名称：
Round 07：认证材料附件上传闭环

本轮背景：
当前已完成：
- `2ee5b68 feat(auth): add identity verification submission`
- `4955bee feat(auth): add identity verification admin review`
- 学生侧认证资料提交、状态查询已完成
- 管理员审核 APPROVED/REJECTED 已完成
- 非容器快速测试 46/46 通过

重要纠偏：
上一轮建议中出现“OBS 预签名”说法，但项目详细设计与对象存储底座已经确认：
- V1 不让 Qt 客户端直传对象存储。
- V1 采用后端中转上传对象存储。
- 附件访问控制由后端业务权限校验完成。
- 不返回 OBS 裸 URL，不让 Qt 客户端持有 OBS AK/SK、临时凭证或桶直连地址。

因此本轮不要实现预签名 URL，不要实现客户端直传。本轮实现“后端中转上传 + 附件元数据表 + ObjectStorageService 抽象 + 测试替身”。

重要前置校验：
开始前必须运行：
- `git status --short --branch`
- `git log --oneline -8`

如果工作区不干净：
1. 若只包含 docs/、handoff/、docs/validation/、docs/prompts/ 下的 Round 06/07 留档文件，先复核这些文档，使用显式文件路径创建一个纯文档提交：
   `docs(codearts): record round06 handoff and round07 prompt`
2. 若包含 backend/、desktop/ 或其他源码/配置改动，立即停止并报告，不要继续开发。
3. 禁止 `git add .` 和 `git add -A`，只能 `git add <明确文件路径>`。

请先阅读：
- AGENTS.md
- docs/03_current_plan.md
- handoff/latest.md
- docs/13_detailed_design_v1.md
- docs/17_object_storage_purchase_and_configuration_base_v1.md
- docs/12_code_generation_constraints_v1.md
- docs/21_codearts_prompt_review_workflow_v1.md
- docs/22_codearts_unattended_prompt_engineering_v1.md
- docs/validation/20260519_round06_identity_verification_admin_review_record.md

本轮目标：
实现认证材料附件的后端最小闭环：
1. 已登录且处于 `UNVERIFIED` 或 `REJECTED` 的用户，可以为自己的认证申请上传 1 个认证材料附件。
2. 后端校验 MIME、文件大小、业务状态和用户归属。
3. 后端生成对象 key，并通过 `ObjectStorageService` 抽象写入对象存储。
4. PostgreSQL 保存附件元数据、对象 key、sha256、大小、MIME、业务归属和状态。
5. 认证资料提交接口能关联已上传的附件。
6. 管理员查看待审核列表时能看到附件元数据 ID，但不返回 OBS 裸 URL。
7. 管理员可通过后端受控接口读取附件内容用于审核。

本轮允许做：
1. 新增附件元数据 Flyway 迁移。
2. 新增附件元数据 Entity / Repository。
3. 新增 `ObjectStorageService` 接口和测试替身实现。
4. 新增认证材料附件上传 Controller / Service。
5. 修改认证资料提交请求，使其可接收 `materialAttachmentId` 或等价字段。
6. 修改管理员待审核列表/详情，使其返回附件元数据摘要。
7. 新增必要测试。
8. 更新 validation、docs/03_current_plan.md、handoff/latest.md。
9. 门禁通过后创建一个开发提交。

本轮明确不做：
1. 不连接真实华为云 OBS。
2. 不新增或粘贴任何 AccessKey、SecretKey、临时凭证、JWT_SECRET、DB_PASSWORD。
3. 不实现预签名 URL。
4. 不让 Qt 客户端直传对象存储。
5. 不返回 OBS 裸 URL 或桶直连地址。
6. 不修改 Qt 客户端。
7. 不实现完整附件系统，不覆盖帖子图片、投诉证据等其他业务附件。
8. 不实现真实云对象删除策略的生产任务；如需删除，只在服务接口中预留或用测试替身验证。
9. 不实现完整 RBAC、管理员 UI、需求发布、私信、评价、投诉等其他模块。

附件业务规则：
- 认证材料支持：`image/jpeg`、`image/png`、`application/pdf`。
- 单文件大小上限：10 MB。
- 禁止可执行文件、压缩包、脚本文件和未知 MIME。
- 对象 key 必须由后端生成，不能信任客户端文件名。
- 对象 key 不得包含真实姓名、学号、邮箱、手机号等个人信息。
- 建议 key 前缀：`auth-materials/yyyy/MM/dd/<uuid>`。
- 数据库保存 sha256、size、contentType、originalFilename、objectKey、businessType、ownerUserId、status。
- originalFilename 仅作为展示辅助，应限制长度并清理路径分隔符。

建议接口契约：

1. `POST /api/auth/identity-verifications/materials`
   - 需要 `Authorization: Bearer <accessToken>`
   - `multipart/form-data`
   - 字段：`file`
   - 成功返回：
     - `attachmentId`
     - `contentType`
     - `sizeBytes`
     - `sha256`
     - `status`
   - 不返回 objectKey，不返回 OBS URL。

2. `POST /api/auth/identity-verifications`
   - 在既有文本字段基础上增加：
     - `materialAttachmentId`
   - 后端必须校验该附件属于当前用户、业务类型为认证材料、状态可用。

3. `GET /api/admin/identity-verifications?status=PENDING_REVIEW`
   - 返回附件元数据摘要，例如 `materialAttachmentId`、`materialContentType`、`materialSizeBytes`。
   - 不返回对象 key 或 OBS URL。

4. `GET /api/admin/identity-verifications/{submissionId}/material`
   - 需要 ADMIN。
   - 后端校验管理员权限和 submission 关联。
   - 通过 ObjectStorageService 读取对象流并返回文件内容。
   - 本轮测试使用测试替身，不访问真实 OBS。

如果发现既有接口或文档已有更明确路径，应优先沿用；如果存在冲突，停止并列出冲突，不要自行扩展多套接口。

对象存储抽象建议：
- 新增接口 `ObjectStorageService`。
- 方法最少包括：`putObject(key, contentType, bytes/stream)`、`getObject(key)`。
- 测试 profile 使用内存实现或测试替身。
- 生产 OBS SDK 适配器本轮可以不实现；如确需占位，只保留接口和配置，不连接真实云。
- 不在日志中输出对象内容、token、密钥或敏感个人信息。

测试先行要求：
必须先新增或修改测试，并确认实现前这些测试应失败。建议至少覆盖：
1. 未登录上传认证材料返回 401。
2. 已登录用户上传合法 PNG/JPEG/PDF 成功，返回 attachmentId、sha256、size、contentType。
3. 超过 10 MB 返回 `ATTACHMENT_TOO_LARGE`。
4. 非允许 MIME 返回 `ATTACHMENT_TYPE_NOT_ALLOWED`。
5. 提交认证资料时引用不属于自己的 attachmentId 返回 403 或冲突错误。
6. 提交认证资料时引用自己的 attachmentId 成功进入 `PENDING_REVIEW`。
7. 管理员待审核列表能看到附件摘要但不含 objectKey/OBS URL。
8. 管理员可通过后端接口读取附件内容。
9. 普通学生读取管理员材料接口返回 403。
10. 非容器快速回归仍通过。

开发预算：
- 本轮最多完成 3 个小闭环：附件元数据、上传接口、管理员受控读取。
- 如果需要新增超过 14 个源代码文件，停止并说明原因。
- 同一类测试失败最多修复 2 次；超过后停止并输出诊断报告。
- 不为了绿灯删除关键测试、弱化断言、跳过对象存储抽象或绕过权限校验。

验证命令：
优先运行本轮相关测试，例如：
- `.\mvnw.cmd test '-Dspring.testcontainers.enabled=false' '-Dtest=IdentityVerificationMaterialAttachmentEndpointTest,IdentityVerificationEndpointTest,IdentityVerificationAdminReviewEndpointTest'`

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
`feat(auth): add identity material attachment upload`

完成后请输出：
1. 本轮实际完成了什么。
2. 测试先行的红灯/绿灯记录。
3. 最终测试命令与结果。
4. validation 记录路径。
5. 创建的提交哈希。
6. 当前 `git status --short --branch` 摘要。
7. 明确未完成事项，尤其是真实 OBS SDK 适配、真实云连通、Testcontainers 是否未完成。
8. 下一轮建议。
```

