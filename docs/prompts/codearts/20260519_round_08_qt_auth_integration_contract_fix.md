# 2026-05-19 Round 08 CodeArts Prompt：Qt 认证集成契约审计与修正

以下提示词用于交给 CodeArts。目标是先修正 Qt 客户端与后端 P0 认证 API 的契约偏差和令牌存储边界，再继续扩展认证资料提交 UI。

```text
你将继续接手《校园搭子平台》开发。请默认使用中文沟通。

本轮名称：
Round 08：Qt 认证集成契约审计与修正

本轮背景：
上一轮你完成了：
- `8edd057 feat(auth): add identity material attachment upload`
- `ca058c3 feat(desktop): add login/register UI and auth API integration`
- `957c283 docs(handoff): update latest.md after Qt client auth integration`

本轮复核发现 Qt 客户端认证集成存在需要优先修正的风险：
1. Qt 注册请求体疑似不符合后端真实契约。
   - 后端 `POST /api/auth/campus-email/verification-codes` 需要 `{ campusEmail, purpose }`。
   - 后端 `POST /api/auth/campus-email/verifications` 需要 `{ campusEmail, code, purpose }`，成功后返回 `verificationTicket`。
   - 后端 `POST /api/auth/register` 需要 `{ campusEmail, verificationTicket, password, displayName }`。
   - 当前 Qt 注册流程似乎直接发送 `realName/studentNumber/verificationCode` 到 `/auth/register`，这会导致真实后端联调失败。
2. Qt 当前用 `QSettings` 保存 access token，和详细设计冲突。
   - `docs/13_detailed_design_v1.md` 明确：token 不得写入 QSettings 或本地明文文件。
   - 如果暂不实现 Windows Credential Manager，本轮应至少改为内存会话存储或 `SecureTokenStore` 抽象，不要持久化 token 到 QSettings。
3. `AuthResult.accessToken` 被复用来承载 `authenticationStatus`，语义混乱，后续 UI 会很容易出错。
4. 当前 Qt 认证资料提交方法只传 `realName/studentNumber`，而后端已需要更完整字段，并且附件上传闭环已存在；本轮不要扩新 UI，先修契约。

重要前置校验：
开始前必须运行：
- `git status --short --branch`
- `git log --oneline -10`

如果工作区不干净：
1. 若只包含 docs/、handoff/、docs/validation/、docs/prompts/ 下的 Round 07/08 留档文件，先复核这些文档，使用显式文件路径创建纯文档提交：
   `docs(codearts): record round07 handoff and round08 prompt`
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
- docs/validation/20260519_round07_identity_material_attachment_upload_record.md

本轮目标：
修正 Qt 客户端与后端认证 API 的契约，使登录、发送验证码、校验验证码、注册、查询认证状态能够按真实后端接口语义工作。

本轮允许做：
1. 修改 Qt `AuthApiService`、`RegisterWidget`、`HomePageWidget` 和相关测试。
2. 新增或调整 Qt 端 DTO/结果结构，使 `accessToken`、`verificationTicket`、`authenticationStatus` 分开表达。
3. 修改 token 存储实现：不得使用 QSettings 持久化 token。
4. 如果暂不实现 Windows Credential Manager，可先使用内存会话 token store，并在文档/validation 中明确这是临时实现。
5. 增加 Qt API client/service 测试，覆盖真实后端请求 JSON 字段。
6. 更新 validation、docs/03_current_plan.md、handoff/latest.md。
7. 门禁通过后创建一个开发提交。

本轮明确不做：
1. 不新增后端业务功能。
2. 不修改后端接口契约来迁就 Qt 的错误请求体。
3. 不实现新的大 UI 页面。
4. 不实现认证资料提交 UI 和附件上传 UI，除非只是修正已有 service 方法签名和测试。
5. 不实现管理员审核 UI。
6. 不接触真实 OBS、AK/SK、JWT_SECRET、DB_PASSWORD。
7. 不把 token、密码、验证码、verificationTicket 写入日志、QSettings、文档或测试输出。

必须修正的契约：

1. 发送验证码：
   - 路径：`POST /api/auth/campus-email/verification-codes`
   - 请求体：`{ "campusEmail": "...", "purpose": "REGISTER_OR_LOGIN" }`

2. 校验验证码：
   - 路径：`POST /api/auth/campus-email/verifications`
   - 请求体：`{ "campusEmail": "...", "code": "...", "purpose": "REGISTER_OR_LOGIN" }`
   - 响应中读取 `verificationTicket`

3. 注册：
   - 路径：`POST /api/auth/register`
   - 请求体：`{ "campusEmail": "...", "verificationTicket": "...", "password": "...", "displayName": "..." }`
   - 注册表单中不要把学号/真实姓名当成注册接口字段；真实身份资料属于认证资料提交，不属于账号注册。

4. 登录：
   - 路径：`POST /api/auth/login`
   - 成功后读取 `accessToken`，只保存到非持久化 token store 或符合设计的 SecureTokenStore。

5. 查询认证状态：
   - 路径：`GET /api/auth/identity-verifications/me`
   - 响应中的 `authenticationStatus` 应写入明确字段，例如 `AuthStatusResult.authenticationStatus`，不得塞进 `accessToken`。

Token 存储要求：
- 禁止继续使用 QSettings 保存 access token。
- 推荐本轮最小改法：
  - 将现有 `AuthTokenStore` 改为内存会话存储，进程退出即丢失。
  - 或新增 `SecureTokenStore` 抽象 + 内存实现，后续再补 Windows Credential Manager 实现。
- 如果选择 Windows Credential Manager，必须保持范围小、可测试、失败可回退；不要引入复杂安装或系统级依赖。
- Qt 普通配置仍可用 QSettings 保存 API base URL 等非敏感项。

测试先行要求：
必须先新增或修改测试，并确认修复前应失败。建议至少覆盖：
1. `sendVerificationCode` 请求体包含 `purpose=REGISTER_OR_LOGIN`。
2. `verifyCampusEmail` 读取 `verificationTicket`。
3. `registerAccount` 请求体包含 `verificationTicket`、`displayName`，不包含 `realName/studentNumber/verificationCode`。
4. 登录成功后 token 不写入 QSettings。
5. 查询认证状态读取 `authenticationStatus` 到专门字段，不复用 `accessToken`。
6. UI 层不直接使用 `QNetworkAccessManager`，仍通过 API/service 层调用。
7. 现有 Qt 测试全部通过。

建议验证命令：
- 按现有桌面端构建方式运行 Qt 测试，例如 `api_client_config_test`、`campus_api_client_test`、`auth_token_store_test`。
- 如果项目已有 CMake build 目录，请优先使用既有构建命令；不要把 build 产物提交到 Git。
- 后端非容器快速回归可不强制运行，除非你修改了后端代码；如果修改后端，必须运行：
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
`fix(desktop): align auth flow with backend contract`

完成后请输出：
1. 本轮实际完成了什么。
2. 修复前失败、修复后通过的测试记录。
3. Qt 测试命令与结果。
4. 是否运行后端测试；未运行则说明原因。
5. validation 记录路径。
6. 创建的提交哈希。
7. 当前 `git status --short --branch` 摘要。
8. 明确未完成事项，例如 Windows Credential Manager、认证资料提交 UI、附件上传 UI。
9. 下一轮建议。
```

