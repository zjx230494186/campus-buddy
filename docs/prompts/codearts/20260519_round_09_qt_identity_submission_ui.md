# 2026-05-19 Round 09 CodeArts Prompt：Qt 认证资料提交 UI

以下提示词用于交给 CodeArts。目标是在 Qt 认证契约已修正后，补齐学生侧认证资料提交界面，并通过后端已有 API 完成文本资料 + 认证材料附件上传的最小可演示闭环。

```text
你将继续接手《校园搭子平台》开发。请默认使用中文沟通。

本轮名称：
Round 09：Qt 认证资料提交 UI

本轮背景：
当前已完成：
- `e53f11a fix(desktop): align auth flow with backend contract`
- Qt 认证流程已对齐后端契约：
  - 验证码接口带 `purpose=REGISTER_OR_LOGIN`
  - 校验验证码后读取 `verificationTicket`
  - 注册使用 `campusEmail/verificationTicket/password/displayName`
  - token 不写入 QSettings，当前使用内存会话存储
- 后端已完成：
  - 认证资料提交接口
  - 认证材料附件后端中转上传
  - 管理员审核接口
- 后端非容器测试 56/56 通过
- Qt 测试 3/3 通过

本轮目标：
实现 Qt 学生侧认证资料提交 UI 最小闭环：
1. 登录后在主页能看到认证状态。
2. 未认证或已驳回用户可以填写认证资料。
3. 用户可以选择学生证/校园卡等认证材料文件，并通过后端中转上传。
4. 用户提交认证资料时关联 `materialAttachmentId`。
5. 提交成功后界面显示 `PENDING_REVIEW`。

重要前置校验：
开始前必须运行：
- `git status --short --branch`
- `git log --oneline -12`

如果工作区不干净：
1. 若只包含 docs/、handoff/、docs/validation/、docs/prompts/ 下的 Round 08/09 留档文件，先复核这些文档，使用显式文件路径创建纯文档提交：
   `docs(codearts): record round08 handoff and round09 prompt`
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
- docs/validation/20260519_round08_qt_auth_contract_fix_record.md
- docs/validation/20260519_round07_identity_material_attachment_upload_record.md

本轮允许做：
1. 修改 `CampusApiClient`，增加 multipart/form-data 上传能力。
2. 修改 `AuthApiService`，增加认证材料上传和完整认证资料提交方法。
3. 新增或修改 Qt UI 组件，例如 `IdentityVerificationWidget`，或在 `HomePageWidget` 中添加简洁认证资料表单。
4. 新增/修改 Qt 测试，验证请求路径、认证头、multipart 上传、提交 JSON 字段。
5. 更新 validation、docs/03_current_plan.md、handoff/latest.md。
6. 门禁通过后创建一个开发提交。

本轮明确不做：
1. 不修改后端业务代码。
2. 不实现管理员审核 UI。
3. 不实现 Windows Credential Manager；继续使用当前内存 token store。
4. 不连接真实 OBS，不让 Qt 直传 OBS，不写 AK/SK。
5. 不实现 P1 需求发布、私信、评价、投诉等模块。
6. 不把 token、密码、验证码、verificationTicket、附件内容写入日志、文档或测试输出。

后端接口契约：

1. 上传认证材料：
   - `POST /api/auth/identity-verifications/materials`
   - 需要 `Authorization: Bearer <accessToken>`
   - `multipart/form-data`
   - 字段：`file`
   - 成功返回至少包含 `attachmentId`

2. 提交认证资料：
   - `POST /api/auth/identity-verifications`
   - 需要 `Authorization: Bearer <accessToken>`
   - JSON 字段至少包含：
     - `realName`
     - `studentNumber`
     - `college`
     - `major`
     - `grade`
     - `materialAttachmentId`

3. 查询认证状态：
   - `GET /api/auth/identity-verifications/me`
   - 需要 `Authorization: Bearer <accessToken>`
   - 读取 `authenticationStatus`、`reviewStatus`、`rejectReason`、`allowedActions`

UI 建议：
- 在主页或单独 widget 中提供表单：
  - 真实姓名
  - 学号
  - 学院
  - 专业
  - 年级
  - 选择认证材料文件
  - 上传材料
  - 提交认证
- 上传成功前禁用提交按钮，避免没有 `materialAttachmentId` 就提交。
- 支持 jpg/jpeg/png/pdf 文件选择过滤。
- 界面只显示文件名、大小、上传状态，不显示对象 key 或任何 OBS URL。
- 若认证状态为 `PENDING_REVIEW` 或 `VERIFIED`，应避免继续展示可提交表单，显示状态即可。
- 若状态为 `REJECTED`，显示驳回原因并允许重新提交。

测试先行要求：
必须先新增或修改测试，并确认修复前应失败。建议至少覆盖：
1. `CampusApiClient` multipart 上传会带 Bearer token。
2. 认证材料上传请求路径为 `/auth/identity-verifications/materials`，字段名为 `file`。
3. 上传成功后 `AuthApiService` 读取 `attachmentId`。
4. 提交认证资料 JSON 包含 `realName/studentNumber/college/major/grade/materialAttachmentId`。
5. 状态查询读取 `authenticationStatus/reviewStatus/rejectReason/allowedActions`。
6. UI 层不直接使用 `QNetworkAccessManager`。
7. Qt 全部既有测试通过。

验证命令：
- 按当前桌面端构建方式运行 Qt 测试：
  - `api_client_config_test`
  - `campus_api_client_test`
  - `auth_token_store_test`
- 如新增测试可执行文件，必须一并运行。
- 不提交 `desktop/build*/`、`.ninja_*` 或任何构建产物。
- 未修改后端代码时不强制运行后端测试；如修改后端，必须运行：
  `.\mvnw.cmd test '-Dspring.testcontainers.enabled=false' '-Dtest=!DatabaseMigrationTest'`

Git 纪律：
开始前、提交前、提交后都必须运行：
- `git status --short --branch`

提交前必须运行：
- `git diff --name-only`
- `git diff --stat`

禁止：
- `git add .`
- `git add -A`
- 提交 `desktop/build*/`
- 提交 `.ninja_*`
- 提交 `backend/target/`
- 提交测试输出日志
- 提交真实密钥、密码、token、AccessKey、SecretKey、SSH 私钥

提交策略：
本轮最多创建一个开发提交。
推荐提交信息：
`feat(desktop): add identity verification submission UI`

完成后请输出：
1. 本轮实际完成了什么。
2. 修复前失败、修复后通过的测试记录。
3. Qt 测试命令与结果。
4. 是否运行后端测试；未运行则说明原因。
5. validation 记录路径。
6. 创建的提交哈希。
7. 当前 `git status --short --branch` 摘要。
8. 明确未完成事项，例如 Windows Credential Manager、管理员审核 UI、真实 OBS SDK。
9. 下一轮建议。
```

