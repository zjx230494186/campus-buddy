# Latest Handoff

## 2026-06-16 AI 帖子自动预审规划

### 当前线程完成了什么

1. 查看了当前后端帖子审核基线：学生提交后进入 `PENDING_REVIEW`，管理员通过 `/api/admin/partner-posts/review-queue` 和 `/api/admin/partner-posts/{postId}/review` 手动审核。
2. 新增设计文档：`docs/31_ai_post_moderation_design_v1.md`。
3. 在设计中确定 V1 采用“AI 自动预审 + 人工兜底”，不做无兜底全自动审核。
4. 明确 V1 只审核 `PartnerPost` 文本字段，不发送身份认证材料、邮箱、学号、真实姓名、附件原文或对象存储地址给大模型。
5. 明确状态流转：帖子主状态仍为 `DRAFT / PENDING_REVIEW / PUBLISHED / REJECTED`；V1 不修改数据库、不新增 Flyway、不新增审计表。
6. 明确配置策略：默认关闭，`provider=noop`；真实 `POST_MODERATION_API_KEY` 只能放服务器私有环境变量。
7. 明确测试先行批次：配置与 noop 替身、预审编排与自动动作、真实 HTTP 客户端、V2 审计表和管理员展示评估。

### 下一步候选事项

1. 复用当前线程：对 `docs/31_ai_post_moderation_design_v1.md` 做小修，例如调整阈值、风险类别、是否允许自动通过/自动驳回。
2. 新开线程：实现 Batch 1，也就是配置类、`PostModerationClient` 接口和 noop moderation client；不改数据库。
3. 新开线程：如果要先做更保守方案，可把设计改成“只给 AI 建议，不自动改帖子状态”。

### 建议归档当前线程

当前线程已经完成规划闭环，建议归档。下一线程名称：

```text
AI 帖子自动预审 Batch 1 配置与 noop 替身
```

可直接复制的启动 prompt：

```text
请读取 `AGENTS.md`、`docs/31_ai_post_moderation_design_v1.md`、`docs/03_current_plan.md`、`handoff/latest.md`。

当前任务是实现 AI 帖子自动预审 Batch 1：新增配置类、`PostModerationClient` 接口和 `NoopPostModerationClient`。严格测试先行：先写配置与 noop client 相关测试并确认红灯，再实现代码，再跑本批测试和必要回归测试。

范围边界：本线程不接入真实大模型 HTTP API，不需要真实 API key，不修改移动端，不改变现有管理员审核接口语义，不新增 Flyway、不新增表、不新增 JPA 实体。敏感配置只能使用环境变量名，不写入真实值。完成后更新 `docs/validation/` 测试记录、`docs/03_current_plan.md` 和 `handoff/latest.md`。
```

## 2026-06-11 Android CLOSED 会话交换联系方式错误修复

### 问题原因

用户在双方保存联系方式后点击确认交换，弹出：

```text
Cannot confirm unlock on a non-ACTIVE conversation
```

该错误来自后端 `ContactUnlockService.confirmUnlock`。后端规则要求联系方式确认只能发生在 `ACTIVE` 会话中；如果会话已关闭 `CLOSED`，后端会拒绝确认交换。

### 本线程完成

1. 移动端会话列表进入聊天页时，现在会把后端返回的会话 `status` 一并传入。
2. 聊天页使用真实会话状态初始化，不再默认所有会话都是 `ACTIVE`。
3. 非 `ACTIVE` 会话会禁用“确认交换”，并显示中文说明：当前会话已关闭，不能交换联系方式，可从原帖子重新发起联系后再交换。
4. 增加 `CONVERSATION_CLOSED` 中文错误提示，避免再直接展示后端英文 message。

### 验证结果

1. 已在 `D:\Github\mobile-client` 执行：

```powershell
.\gradlew.bat :app:assembleDebug
```

2. 构建结果：`BUILD SUCCESSFUL`。
3. 本轮未调用云端真实接口，未写入真实账号、密码、验证码或 token。

### 使用建议

如果要对同一个帖子重新交换联系方式，需要从原帖子重新发起联系，让后端恢复/创建 `ACTIVE` 会话后再操作；已关闭会话不能直接确认交换。

## 2026-06-11 Android 登录门禁与联系方式交换修复

### 本线程完成

1. 修复移动端启动后未登录却显示主界面/个人页的问题。
   1. `NavGraph` 现在固定以登录页为起点。
   2. App 启动时清理旧本地 token，避免残留 token 造成“假登录”。
   3. 受保护路由增加未登录拦截，未登录访问主功能会回到登录页。
2. 修复会话页联系方式交换不可用的问题。
   1. 后端已实现联系方式交换，但 App 之前是半接入状态。
   2. App 端原先按 `myConfirmed` 解析，后端实际返回 `currentUserConfirmed`。
   3. App 端原先没有解析 `currentUserHasContactCard` / `peerHasContactCard`。
   4. 会话页联系方式输入框原先是只读，导致无法真正保存卡片。
   5. 现在已改为可编辑，并按后端规则启用“保存我的卡片 / 确认交换 / 查看对方联系方式”。
3. 增加联系方式交换相关错误提示：
   1. `CONTACT_CARD_REQUIRED`
   2. `CONTACT_NOT_UNLOCKED`
   3. `PEER_CONTACT_CARD_NOT_FOUND`
   4. `CONTACT_UNLOCK_NOT_AVAILABLE`

### 验证结果

1. 已在 `D:\Github\mobile-client` 执行：

```powershell
.\gradlew.bat :app:assembleDebug
```

2. 构建结果：`BUILD SUCCESSFUL`。
3. 本轮未调用云端真实接口，未写入真实账号、密码、验证码或 token。

### 使用规则说明

联系方式交换不是单方点击即能查看。后端规则是：

1. 当前用户先保存自己的联系方式卡片。
2. 当前用户点击确认交换。
3. 对方也需要保存联系方式卡片并点击确认交换。
4. 双方都确认且双方都有卡片后，状态变为 `UNLOCKED`，此时才能查看对方联系方式。

## 2026-06-11 Android 移动端接口对齐

### 本线程完成

1. 检查 `D:\Github\mobile-client` 的 Retrofit API、Repository、数据模型和主要页面，按当前后端接口设计做对齐。
2. 补齐移动端缺失接口：
   1. `POST /api/me/conversations/{conversationId}/close`
   2. `GET /api/me/contact-card`
   3. `PUT /api/me/contact-card`
   4. `GET /api/me/conversations/{conversationId}/contact-unlock`
   5. `POST /api/me/conversations/{conversationId}/contact-unlock/confirm`
   6. `GET /api/me/conversations/{conversationId}/peer-contact-card`
3. 调整响应体模型空值兼容，覆盖帖子时间/标签、会话对方信息、消息内容、评价列表和信用摘要等字段。
4. 移除 App 端后端不支持的 `TRAVEL` 场景入口。
5. 页面展示补齐：
   1. 会话列表展示状态和更新时间。
   2. 聊天页展示会话状态、关闭会话、联系方式卡片、联系方式解锁状态、双方确认、对方联系方式和评价入口。
   3. 身份认证页展示审核状态、提交/审核时间、驳回原因和允许动作，并禁用已认证/审核中重复提交。
   4. 我的评价页从模拟数据改为真实 `given/received` 接口列表，展示评分、标签、状态、修改标记和时间。

### 验证结果

1. 已在 `D:\Github\mobile-client` 执行：

```powershell
.\gradlew.bat :app:assembleDebug
```

2. 构建结果：`BUILD SUCCESSFUL`。
3. 构建仅输出既有 deprecated warning，未发现阻塞编译问题。
4. 本轮未调用云端真实接口，未写入真实账号、密码、验证码或 token。

### 仍需人工/真机验证

1. 在 Android 真机或模拟器登录已认证学生账号，验证广场、发起会话、聊天、联系方式解锁和评价入口是否符合预期。
2. 评价页当前已支持显示 6 颗星记录；写评价页是否允许选择 6 星仍需结合联系方式解锁状态进一步完善。
3. 当前移动端仍没有管理员专属 GUI；管理员身份认证审核、帖子审核如需移动端操作，需要新增独立管理员页面和导航入口。

### 下一步候选

1. 复用当前线程：安装 Debug APK 到模拟器/真机，使用脱敏测试账号做移动端云端接口人工 smoke。
2. 新开线程：实现 Android 管理员审核 GUI。
3. 新开线程：补写评价页 6 星选择与联系方式解锁状态联动。

## 2026-06-11 云端接口完整测试用例与 smoke 脚本

### 本线程完成

1. 新增完整接口测试用例文档：`docs/api_full_test_cases_20260611.md`。
2. 新增云端接口自动化 smoke 脚本：`scripts/server_api_full_smoke.ps1`。
3. 脚本通过环境变量读取：
   1. `CAMPUS_BUDDY_API_BASE_URL`
   2. `CAMPUS_BUDDY_SMOKE_EMAIL`
   3. `CAMPUS_BUDDY_SMOKE_PASSWORD`
   4. `CAMPUS_BUDDY_SMOKE_ADMIN_EMAIL`
   5. `CAMPUS_BUDDY_SMOKE_ADMIN_PASSWORD`
4. 覆盖系统健康、登录鉴权、验证码/注册负向、身份认证查询与附件上传删除、学生发帖、管理员帖子审核、广场、会话、联系方式解锁、评价信用和关键权限负向测试。

### 验证结果

1. 已执行 PowerShell 静态语法检查：`syntax=ok`。
2. 已扫描新增用例文档和脚本，未写入用户提供的真实账号、密码或 token。
3. 本轮未实际调用云端接口。

### 下一步候选

1. 复用当前线程：用户在本机设置私有环境变量后，运行 `.\scripts\server_api_full_smoke.ps1`，根据输出定位失败接口。
2. 新开线程：根据 smoke 输出生成正式 validation 记录，并修复发现的后端/部署问题。

## 2026-06-11 本地 PostgreSQL 初始化脚本整理

### 本线程完成

1. 阅读 `backend/src/main/resources/db/migration/` 下 Flyway V1-V11。
2. 确认当前数据库目标为 PostgreSQL，表结构依赖 `gen_random_uuid()`，本地库需要 `pgcrypto` 扩展。
3. 新增本地初始化脚本：
   1. `deploy/local-postgres/00_create_database.sql`
   2. `deploy/local-postgres/01_schema.sql`
   3. `deploy/local-postgres/README.md`

### 验证结果

1. 本轮为 SQL 整理，未连接云端数据库，未导出生产数据。
2. 未执行本地 `psql` 导入验证。
3. 未写入真实数据库密码、云端连接串、token 或其他密钥。

### 下一步候选

1. 复用当前线程：在本机 PostgreSQL 执行 `00_create_database.sql` 与 `01_schema.sql`，再启动后端连接本地库做 health/login smoke。
2. 新开线程：整理本地部署运行配置，包括本地 env、后端启动命令与数据库 smoke 记录。

## 2026-06-11 后端接口测试索引整理

### 本线程完成

1. 快速阅读项目关键文档与后端源码，确认后端为 Spring Boot 4 + Java 21，接口集中在 `backend/src/main/java/com/campusbuddy/**Controller.java`。
2. 核对 `SecurityConfiguration.java`，整理无需登录、需要登录、需要管理员和需要已认证学生状态的接口边界。
3. 新增接口测试索引：`docs/backend_api_test_index_20260611.md`，覆盖系统、注册登录、身份认证、帖子、广场、会话、联系方式、评价信用和管理员审核接口。

### 验证结果

1. 本轮为源码阅读和文档整理，未修改后端业务代码。
2. 未运行后端测试或公网 smoke。
3. 未写入真实账号、密码、验证码、token、SMTP、数据库或 OBS 密钥。

### 下一步候选

1. 复用当前线程：基于 `docs/backend_api_test_index_20260611.md` 生成 Postman/Apifox 集合或 PowerShell 自动化 smoke 脚本。
2. 新开线程：执行真实接口测试，建议线程名为 `后端接口 smoke 测试执行`。

可复制启动 prompt：

```text
请读取 AGENTS.md、docs/backend_api_test_index_20260611.md、docs/03_current_plan.md、handoff/latest.md。

当前任务是执行后端接口 smoke 测试。优先从 GET /api/health 和 GET /api/system/info 开始，再按接口索引中的建议顺序推进。不要把真实账号密码、验证码、token、SMTP、数据库或 OBS 密钥写入仓库或聊天记录；如需使用私有环境变量，只记录 present/missing 与脱敏结果。
```


## 2026-06-16 AI 帖子自动审核云端部署交接

### 本轮完成

1. 本地确认无 Git 合并冲突：无 unmerged 文件，无 `<<<<<<<` / `=======` / `>>>>>>>` 冲突标记。
2. 本地执行 `mvn -DskipTests package` 成功生成后端 jar。
3. 用户通过 `scp` 将 jar 上传到云端，并替换 `/srv/campus-buddy/campus-buddy-backend-0.0.1-SNAPSHOT.jar` 后重启 `campus-buddy-backend`。
4. 云端私有配置 `/etc/campus-buddy/backend.env` 已加入 Qwen OpenAI-compatible 审核变量；真实 `POST_MODERATION_API_KEY` 不记录在仓库或文档中。
5. 公网 API 验证成功：`GET /api/health` 返回 `UP`。
6. 真实账号 smoke 成功：学生账号创建学习帖并提交审核后直接返回 `PUBLISHED`，管理员审核队列不包含该帖子。

### 后续注意

- 安卓端公开接口无需改 URL 或请求体，但提交审核后状态可能直接为 `PUBLISHED` 或 `REJECTED`，不要写死只显示“待审核”。
- 若某个帖子没有自动通过，优先检查帖子内容是否低置信、用户已发布帖子是否达到上限、以及服务器日志是否有 provider 调用失败。
- 若轮换 DashScope key，只修改服务器 `/etc/campus-buddy/backend.env`，然后执行 `sudo systemctl restart campus-buddy-backend`。

### 常用命令

```bash
sudo systemctl status campus-buddy-backend --no-pager
sudo journalctl -u campus-buddy-backend -n 100 --no-pager
sudo nano /etc/campus-buddy/backend.env
sudo systemctl restart campus-buddy-backend
```## 2026-05-30 作业正式提交版详细设计文档与 LaTeX 源

### 本轮完成

1. 新增正式详细设计正文：
   1. `docs/30_submission_detailed_design_v1.md`
   2. 面向课程老师、助教和小组成员，不写成工作记忆或执行流水账。
   3. 覆盖引言、需求目标、总体架构、功能模块、关键流程、数据库、接口、权限安全、前端、部署、测试、限制和总结。
2. 新增 LaTeX / PDF 交付源：
   1. `deliverables/submission/submission_detailed_design_20260530.tex`
   2. 使用 `ctexart`，便于后续用 XeLaTeX 生成规范 PDF。
   3. 正文列表使用编号结构，已检查未生成 `itemize` 黑点列表。
   4. 已用 Conda 环境 `campus_latex` 中的 Tectonic 生成 `deliverables/submission/submission_detailed_design_20260530.pdf`。
3. 更新项目索引和当前计划：
   1. `docs/00_project_map.md`
   2. `docs/03_current_plan.md`
4. 新增验证记录：
   1. `docs/validation/20260530_submission_detailed_design_latex_record.md`

### 依据材料

1. `AGENTS.md`
2. `docs/00_project_map.md`
3. `docs/01_project_brief.md`
4. `docs/03_current_plan.md`
5. `docs/04_product_design_v1.md`
6. `docs/07_course_requirement_alignment_v1.md`
7. `docs/25_server_deploy_and_obs_runbook_v1.md`
8. `docs/27_course_demo_and_delivery_checklist_v1.md`
9. `docs/mobile_api_reference_20260527.md`
10. `handoff/latest.md`
11. 必要的后端 Controller、Service、Repository 和 Flyway 迁移清单。

### 验证结果

1. Markdown 正文未发现以 `- ` 开头的黑点列表。
2. LaTeX 源未发现 `\begin{itemize}`。
3. 当前本机 PATH 未发现 `xelatex/pdflatex/lualatex`，但 Conda 环境 `campus_latex` 中存在 `tectonic 0.16.9` 和 `pandoc 3.9.0.2`。
4. 使用 `tectonic.exe` 已生成 16 页 PDF。
5. Tectonic 日志仍有多处 `Overfull hbox`，主要来自长 API 路径、长表名和宽表格；当前没有 `pdftoppm/pdfinfo`，尚未完成 PNG 渲染版式检查。

### 边界

1. 未修改后端。
2. 未修改 Qt。
3. 未修改 Flyway。
4. 未修改 deploy 脚本。
5. 未重新打包内测 zip。
6. 未写入真实密码、验证码、token、SMTP 授权码、数据库密码、OBS AK/SK 或服务器私钥。

### 下一步建议

1. 建议新开线程：`详细设计 PDF 编译与版式验收`
2. 启动 prompt：

```text
请读取 D:\big_homework\AGENTS.md、D:\big_homework\docs\30_submission_detailed_design_v1.md、D:\big_homework\deliverables\submission\submission_detailed_design_20260530.tex、D:\big_homework\docs\validation\20260530_submission_detailed_design_latex_record.md。

当前任务是对已生成的正式详细设计 PDF 做版式验收与必要排版修正。PDF 路径为 D:\big_homework\deliverables\submission\submission_detailed_design_20260530.pdf，LaTeX 源为 D:\big_homework\deliverables\submission\submission_detailed_design_20260530.tex。当前可用的 LaTeX 环境是 Conda 环境 campus_latex 中的 Tectonic：C:\Users\zjx230494186\anaconda3\envs\campus_latex\Library\bin\tectonic.exe。要求：PDF 使用 LaTeX 生成；中文可正常显示；子行使用编号，不使用黑点列表；检查页眉页脚、目录、表格、分页和过宽文本。重点处理 Tectonic 日志中的 Overfull hbox，尤其是长 API 路径、长表名和宽表格。

本轮不要修改后端、Qt、Flyway、deploy、服务器配置或密钥。不要写入真实密码、验证码、token、SMTP 授权码、数据库密码、OBS AK/SK 或服务器私钥。
```

## 2026-05-27 移动端接口文档补齐

### 本轮完成

- 新增移动端接口文档：
  - `docs/mobile_api_reference_20260527.md`
- 文档按移动端接入视角整理：
  - 服务器 base URL：`http://114.116.203.78/api/`
  - Retrofit `baseUrl` 必须以 `/` 结尾
  - 移动端不连接 H2，只通过 HTTP API 调后端
  - Android HTTP 明文测试放行方式
  - Bearer Token 鉴权与统一错误体
  - 注册、验证码、登录、身份认证、帖子、广场、会话、联系方式解锁、评价信用、管理员审核接口
  - `CONTACT_REPLY_REQUIRED`、`AUTHENTICATION_STATUS_REQUIRED`、字段级 `VALIDATION_FAILED` 等移动端必须处理的业务错误
- 更新项目文档索引：
  - `docs/00_project_map.md`
  - `docs/03_current_plan.md`

### 验证结果

- 文档内容基于当前后端 Controller / Service 源码核对。
- 未运行后端测试或服务器 smoke；本轮是接口文档整理，无代码行为变更。

### 边界

- 未修改后端。
- 未修改 Qt。
- 未修改 Flyway。
- 未修改 deploy 脚本。
- 未写入真实用户密码、验证码、token、SMTP 授权码、数据库密码、OBS AK/SK 或服务器私钥。

## 2026-05-27 初始邀约消息限制与评价信息提示修复

### 本轮完成

- 修复“发起联系后，对方未回复前仍可继续发送多条消息”的合同缺口。
- 后端 `requestContact(...)` 和 `sendMessage(...)` 均增加统一门禁：
  - 当前用户已发送过 `USER_TEXT`
  - 对方尚未发送过 `USER_TEXT`
  - 则返回 HTTP 403 `CONTACT_REPLY_REQUIRED`
- `ConversationMessageRepository` 增加 `countUserTextFromOther(...)`，只用对方真实文本回复作为放行条件。
- Qt 会话页选中会话后显示：
  - `评价用会话ID`
  - `被评价者ID`
- Qt 会话页在等待对方回复时禁用发送按钮，并对 `CONTACT_REPLY_REQUIRED` 显示中文提示。
- Qt 评价页补充提示，告诉用户到“会话与联系方式”页选中会话复制所需 ID。
- 已重新生成内测 zip：
  - `D:\big_homework\deliverables\internal_beta\CampusBuddyInternalBeta_20260526.zip`
- 新增验证记录：
  - `docs/validation/20260527_contact_reply_gate_and_review_hint_fix_record.md`

### 验证结果

- 后端红灯：新增 service 测试后，旧实现未能拒绝第二条消息/重复联系。
- 后端绿灯：`ContactConversationServiceTest,SmtpCampusEmailVerificationCodeSenderTest` 5/5 passed。
- 后端 jar 构建：`.\mvnw.cmd -DskipTests package` passed。
- Qt 红灯：新增源码测试后，旧 UI 缺少评价信息来源提示。
- Qt 绿灯：`ctest` 11/11 passed。
- 桌面端 smoke：`campus_buddy_desktop.exe --smoke-test` passed。
- 包内 exe 剥离 Qt/CMake/MinGW PATH 后 `--smoke-test` passed。
- 服务器部署：systemd `campus-buddy-backend` active，公网 health 返回 `{"status":"UP"}`。

### 备份

- 服务器 jar 备份：
  - `/srv/campus-buddy/campus-buddy-backend-0.0.1-SNAPSHOT.jar.backup.20260527_151353`
- 旧 Windows 内测包和旧解压目录备份：
  - `D:\big_homework\deliverables\internal_beta\backups\contact_reply_review_hint_20260527_144806`

### 未完成验证

- 尝试执行完整服务器公网行为 smoke，但 Windows PowerShell 到 Linux bash 的脚本转义/CRLF 问题导致输出不可采信。
- 已确认没有残留 `codex-contact-*` 临时 smoke 用户或 `Codex contact smoke` 临时帖子。
- 不把该行为级公网 smoke 伪装为已通过；后续如需更强线上证据，建议写成项目内稳定脚本，而不是临时跨 shell here-string。

### 边界

- 未修改 Flyway。
- 未修改 deploy 脚本。
- 未写入真实用户密码、验证码、token、SMTP 授权码、数据库密码、OBS AK/SK 或服务器私钥。

## 2026-05-27 Qt 发布编辑页校验失败后按钮恢复修复

### 本轮完成

- 修复发布草稿提交审核返回字段缺失后，补齐字段仍无法继续更新草稿或提交审核的问题。
- `PostEditorWidget` 现在统一缓存当前草稿状态和 `allowedActions`。
- 草稿编辑动作兼容后端当前返回的 `EDIT`，同时保留历史 `UPDATE_DRAFT` 兼容。
- 保存、更新、提交审核成功或失败后，统一恢复“保存草稿 / 更新草稿 / 提交审核”按钮状态。
- 新增 `post_editor_widget_test`，覆盖：
  - `EDIT + SUBMIT_REVIEW` 动作下加载草稿后按钮可用。
  - `VALIDATION_FAILED` 后更新草稿和提交审核按钮恢复可用。
- 修正 `MyPartnerPostApiServiceTest` 里的草稿样例动作，避免过期 `UPDATE_DRAFT` 掩盖真实合同。
- 已重新生成内测 zip：
  - `D:\big_homework\deliverables\internal_beta\CampusBuddyInternalBeta_20260526.zip`
- 新增验证记录：
  - `docs/validation/20260527_qt_post_editor_validation_recovery_fix_record.md`

### 验证结果

- Qt Release 构建通过。
- `ctest`：11/11 passed。
- `post_editor_widget_test.exe`：passed。
- `my_partner_post_api_service_test.exe`：passed。
- `campus_buddy_desktop.exe --smoke-test`：passed。
- 剥离 Qt/CMake/MinGW PATH 后运行包内 exe `--smoke-test`：passed。
- 服务器 health：`GET http://114.116.203.78/api/health` 返回 `{"status":"UP"}`。

### 备份

- 旧 zip 与旧解压目录已备份：
  - `D:\big_homework\deliverables\internal_beta\backups\post_editor_fix_20260527_135738`

### 边界

- 本轮只修改 Qt 桌面端和桌面端测试。
- 未修改后端。
- 未修改 Flyway。
- 未修改 deploy 脚本。
- 未写入真实用户密码、验证码、token、SMTP 授权码、数据库密码、OBS AK/SK 或服务器私钥。
- 这是客户端 bug，服务器后端 jar 不需要重新部署；内测用户需要使用更新后的 zip。

## 2026-05-26 广场访问认证状态缺口修复并上线

### 本轮完成

- 修复未完成身份认证用户仍可访问广场列表和帖子详情的问题。
- `PartnerPostPlazaService.listPosts(...)` 和 `getPostDetail(...)` 入口增加 `VERIFIED` 状态校验。
- 未认证用户现在返回：
  - HTTP 403
  - `AUTHENTICATION_STATUS_REQUIRED`
- 新增 service 单测：
  - `PartnerPostPlazaServiceTest`
- 修正 endpoint 合同测试：
  - `unverifiedUserCanViewPlazaList` 改为 `unverifiedUserCannotViewPlazaList`
  - 新增 `unverifiedUserCannotViewPlazaDetail`
- 已重新构建 jar 并部署到华为云服务器。
- 新增验证记录：
  - `docs/validation/20260526_plaza_requires_verified_server_fix_record.md`

### 验证结果

- 红灯：`PartnerPostPlazaServiceTest` 初始 2/2 failed，失败原因为未抛权限异常。
- 绿灯：`PartnerPostPlazaServiceTest` 2/2 passed。
- 轻量回归：`PartnerPostPlazaServiceTest,SmtpCampusEmailVerificationCodeSenderTest` 4/4 passed。
- 构建：`.\mvnw.cmd -DskipTests package` 通过。
- 服务器 health：`GET http://114.116.203.78/api/health` 返回 `{"status":"UP"}`。
- 服务器公网 smoke：
  - 未认证 smoke 账号访问广场列表：403 `AUTHENTICATION_STATUS_REQUIRED`
  - 已认证初始学生账号访问广场列表：200
  - 未认证 smoke 账号访问帖子详情：403 `AUTHENTICATION_STATUS_REQUIRED`
  - 已认证初始学生账号访问同一帖子详情：200

### 服务器备份

- jar 备份：
  - `/srv/campus-buddy/campus-buddy-backend-0.0.1-SNAPSHOT.jar.backup.20260526_234539`

### 边界

- 未修改 Flyway。
- 未修改 Qt 客户端。
- 未修改 deploy 脚本。
- 未写入真实用户密码、验证码、token、SMTP 授权码、数据库密码、OBS AK/SK 或服务器私钥。

## 2026-05-26 服务器真实邮箱验证码发送上线完成

### 本轮完成

- 将本机项目目录外私有 SMTP 配置同步到服务器 `/etc/campus-buddy/backend.env`。
- 设置服务器允许收件域名为 `bjtu.edu.cn`。
- 发现服务器旧 jar 只包含 Noop sender，重新构建并上线包含 SMTP sender 的后端 jar。
- 重启 `campus-buddy-backend` systemd 服务。
- 新增验证记录：
  - `docs/validation/20260526_server_smtp_bjtu_deploy_record.md`

### 备份

- 配置备份：
  - `/etc/campus-buddy/backend.env.backup.20260526_222031`
- jar 备份：
  - `/srv/campus-buddy/campus-buddy-backend-0.0.1-SNAPSHOT.jar.backup.20260526_222339`

### 验证结果

- 服务器 health：`GET http://114.116.203.78/api/health` 返回 `{"status":"UP"}`。
- 非 `bjtu.edu.cn` 邮箱请求验证码返回 `INVALID_CAMPUS_EMAIL_DOMAIN`。
- `24301082@bjtu.edu.cn` 请求验证码返回 `CODE_SENT`。
- 初始学生账号和管理员账号登录回归通过。

### 安全边界

- 未把 SMTP 授权码、服务器私钥、数据库密码、OBS AK/SK、JWT secret、验证码或 token 写入仓库、文档或聊天。
- 文档只记录变量 present/missing、备份路径、接口状态码和脱敏邮箱。

### 后续建议

1. **用户收件确认** — 请检查 `24301082@bjtu.edu.cn` 是否收到最新验证码。
2. **Qt 注册页演示** — 使用尚未注册的新 `bjtu.edu.cn` 邮箱，从桌面端走发送验证码、输入验证码、注册和登录。
3. **HTTPS 收口** — 公网仍为 HTTP，正式外发前建议补 HTTPS。

## 2026-05-26 Windows 内测版桌面端打包完成

### 本轮完成

- 将 Qt 桌面端默认 API 地址从本机 `localhost` 切换为服务器 `http://114.116.203.78/api`。
- 使用 Qt 6.10.3 MinGW Release 构建桌面端，并通过 `windeployqt` 收集运行时依赖。
- 生成 Windows 内测 zip 包：
  - `D:\big_homework\deliverables\internal_beta\CampusBuddyInternalBeta_20260526.zip`
  - 解压后入口程序：`CampusBuddyInternalBeta_20260526\campus_buddy_desktop.exe`
- 新增打包验证记录：
  - `docs/validation/20260526_windows_internal_beta_package_record.md`
- 已提交本轮源码与验证文档：
  - `103d476 build(desktop): package internal beta for server runtime`

### 验证结果

- 服务器 health：`GET http://114.116.203.78/api/health` 返回 `{"status":"UP"}`。
- Qt API 配置测试通过。
- Release 桌面端 `--smoke-test` 通过。
- 在剥离 Qt/CMake/MinGW PATH 后，打包目录内 exe `--smoke-test` 通过。
- zip 内容包含 exe、Qt Core/Gui/Widgets/Network DLL、MinGW runtime、platforms/qwindows.dll、tls backend 和 README。

### 服务器密钥依赖检查

- 客户端只依赖公开 HTTP API 地址和运行时可选覆盖项 `CAMPUS_BUDDY_API_BASE_URL` / `--api-base-url=`。
- 未发现客户端包依赖本机 SSH 私钥、服务器私钥、数据库密码、OBS AK/SK、SMTP 授权码或项目外私有 env 文件。
- 包内 Qt 网络/TLS DLL 出现的 `BEGIN PRIVATE KEY` 等字符串属于 Qt 官方 TLS/证书解析器内置文本，不是项目密钥材料。

### 边界与残余风险

- 本轮不修改后端业务逻辑、不修改 Flyway、不修改 deploy 脚本。
- 公网接口当前仍是 HTTP；用于内测可以接受，正式外发前建议补 HTTPS/Nginx 证书。
- `windeployqt` 报告未找到 `dxcompiler.dll` / `dxil.dll`，当前 Qt Widgets 主链路 smoke 通过，低风险；若后续引入 Qt Quick 或 shader 相关能力需重新验证。
- `deliverables/internal_beta/` 为本地交付物目录，未纳入 Git 提交。

### 后续建议

1. **内测分发** — 可直接分发 `CampusBuddyInternalBeta_20260526.zip`，让测试者整包解压后双击 exe。
2. **HTTPS 收口** — 建议新开部署线程，为服务器接入 HTTPS，再重新打包或改默认 API 地址。
3. **现场演示前复核** — 演示前重新确认服务器服务 `UP`，并用包内 exe 做一次登录/注册/发布主链路人工 smoke。

### 建议下一线程名称

`Windows 内测包分发与现场演示复核`

### 可复制启动 Prompt

```text
请读取 D:\big_homework\AGENTS.md、D:\big_homework\docs\03_current_plan.md、D:\big_homework\handoff\latest.md、D:\big_homework\docs\validation\20260526_windows_internal_beta_package_record.md。

当前任务是对 Windows 内测包做最终分发前复核。交付物为 D:\big_homework\deliverables\internal_beta\CampusBuddyInternalBeta_20260526.zip，解压后双击 campus_buddy_desktop.exe 应连接到 http://114.116.203.78/api。

本轮范围：检查服务器 health、检查 zip 内容、运行包内 exe smoke，必要时做一次主链路人工演示。不要修改后端、Flyway 或 deploy；不要写入或泄露服务器 SSH 私钥、DB 密码、OBS AK/SK、SMTP 授权码、验证码、token 或真实联系方式。
```

## 2026-05-25 真实邮箱验证码收信与注册 smoke 已通过

### 本轮完成

- 使用项目目录外私有配置 `D:\big_homework_private\smtp-service.env` 启用 QQ SMTP 发信服务。
- 允许收件域名为 `bjtu.edu.cn`。
- 向测试收件邮箱发送验证码。
- 用户确认真实邮箱收到验证码。
- 使用真实验证码完成：
  - `/api/auth/campus-email/verifications`
  - `/api/auth/register`
  - `/api/auth/login`
- 新增 success validation：
  - `docs/validation/20260525_real_email_registration_smoke_success_record.md`
- 更新 `docs/03_current_plan.md`。

### 验证结果

本轮完成注册阶段输出：

```text
private_env=D:\big_homework_private\smtp-service.env
CAMPUS_EMAIL_DELIVERY_MODE=present
CAMPUS_EMAIL_ALLOWED_DOMAIN=present
CAMPUS_EMAIL_SMTP_HOST=present
CAMPUS_EMAIL_SMTP_PORT=present
CAMPUS_EMAIL_SMTP_USERNAME=present
CAMPUS_EMAIL_SMTP_PASSWORD=present
CAMPUS_EMAIL_SMTP_FROM=present
CAMPUS_EMAIL_SMTP_FROM_NAME=present
CAMPUS_EMAIL_SMTP_AUTH=present
CAMPUS_EMAIL_SMTP_START_TLS=present
CAMPUS_EMAIL_SMTP_SSL=present
CAMPUS_BUDDY_REAL_REGISTER_EMAIL=present
CAMPUS_BUDDY_REAL_REGISTER_PASSWORD=present
CAMPUS_BUDDY_REAL_REGISTER_DISPLAY_NAME=present
CAMPUS_BUDDY_REAL_REGISTER_CODE=present
backend=already-running
verify_code=ok
register=ok
login=ok
```

### 边界确认

- 本轮使用本地 `local-h2` 后端验证。
- 未修改 Flyway migration。
- 未修改 deploy 脚本。
- 未修改 Qt UI。
- 未写入真实 SMTP 用户名、邮箱授权码、验证码、注册密码、token、OBS AK/SK 或数据库密码。
- 仓库仍有前置未跟踪 `deploy/*.sh`、`tmp_*`、历史 prompt/doc、Qt 功能演示截图等文件；本轮不纳入提交边界。

### 当前结论

- 真实 SMTP 验证码发信通过。
- 真实邮箱收信通过。
- 真实验证码校验通过。
- 注册通过。
- 注册后登录通过。

### 后续建议

1. **Qt 桌面端真实注册演示** — 建议复用当前演示线程；使用一个尚未注册过的新 `bjtu.edu.cn` 邮箱，打开 Qt 注册页手动输入验证码，完成 GUI 注册和登录截图。
2. **部署环境 SMTP 启用** — 建议新开部署线程；将同一组 SMTP 服务变量加入服务器私有 `/etc/campus-buddy/backend.env`，重启 systemd 服务并跑公网 smoke。
3. **答辩最终验收** — 建议新开验收线程；串起登录、认证、发布、广场、联系、评价、管理员审核和真实邮箱注册证据。

### 建议下一线程名称

`Qt 桌面端真实邮箱注册演示`

### 可复制启动 Prompt

```text
请读取 D:\big_homework\AGENTS.md、D:\big_homework\docs\03_current_plan.md、D:\big_homework\handoff\latest.md、D:\big_homework\docs\validation\20260525_real_email_registration_smoke_success_record.md。

当前任务是打开 Qt 桌面端，用一个尚未注册过的新 bjtu.edu.cn 邮箱走真实注册页演示。不要把 SMTP 用户名、邮箱授权码、验证码、注册密码、token 或真实联系方式写入聊天、仓库或项目文档。

已确认：后端真实 SMTP 发信、真实邮箱收信、验证码校验、注册、注册后登录已在 local-h2 后端 smoke 通过。

本轮范围：
- 可启动本地后端和 Qt 客户端。
- 可截图 Qt 注册、登录和登录后首页。
- 可写 validation 和 handoff。
- 不改 Flyway、不改 deploy 脚本、不扩大业务功能。

先确认本地后端 health=UP；如未运行，用项目外私有 SMTP 服务配置启动 local-h2 后端。然后打开 Qt 客户端，走注册页：输入新邮箱 -> 发送验证码 -> 等我从邮箱读取验证码 -> 输入验证码 -> 注册 -> 登录。最后输出 validation：命令、结果、截图路径、敏感信息检查和未覆盖风险。
```

