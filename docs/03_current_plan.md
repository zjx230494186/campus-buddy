# 03 Current Plan

本文只维护当前阶段最重要事项，不承载全部历史。

## 文档编码规则

- 项目 Markdown 文档统一使用 UTF-8 编码保存。
- 不使用 GBK/ANSI 保存长期维护文档。
- 使用 PowerShell 写入中文文档时必须显式指定 `-Encoding UTF8`。

## 当前阶段主题

- 阶段：CodeArts 代码实现复核 + 服务器部署闭环治理。
- 当前线程：CodeArts P1 开发 prompt 设计、结果复核与项目文档兜底。
- 当前目标：CodeArts 继续承担老师要求的代码实现主体；Codex 负责复核 CodeArts 的 Git 变更、测试、validation、敏感信息风险，并把多轮技术结果整理为可交接的稳定项目文档。
- 当前推进策略：在 Codex 严格质量控制下，允许 CodeArts 的实现批次适度放大，以尽可能提升推进速度；但不得放宽测试、构建、server smoke、敏感信息检查、Git 边界和 validation 留档门禁。

## 2026-05-21 当前部署状态

- P0 联系最小依赖、P1-1 评价与信用摘要、评价列表 API 已完成后端实现。
- Ubuntu 24.04.4 服务器 smoke test 已通过，公网 Nginx health check 返回 `200 UP`。
- deploy profile 已接入真实华为云 OBS，服务器侧 PUT / GET / SHA-256 / DELETE smoke test 已通过。
- 私有配置入口为服务器 `/etc/campus-buddy/backend.env`，权限要求 `600`、`root:root`；仓库和文档只记录变量名，不记录真实值。
- 后端已完成 systemd 服务化：`campus-buddy-backend` 为 active + enabled，开机自启；进程命令行不暴露 DB/JWT/OBS secret。
- Qt 已具备运行时 API base URL 配置；服务器 smoke 已覆盖 health、login、credit-summary 和身份认证材料上传。
- Round 26 已移除 Qt server smoke 中的硬编码测试邮箱/密码，改为 `CAMPUS_BUDDY_SMOKE_EMAIL` / `CAMPUS_BUDDY_SMOKE_PASSWORD` 私有环境变量。
- Round 27 已补齐本人未引用认证材料删除接口；Qt server smoke 上传材料后会调用 DELETE 清理，API 层面返回 204。
- Round 28 已补齐 `PartnerPost` 学生侧草稿地基：Flyway V8、草稿创建、更新、列表、详情；后端 163/163 通过，服务器 smoke 5/5 通过。
- Round 29 已补齐 `PartnerPost` 学生侧提交审核、撤回审核、主动下架和提交审核强校验；后端 177/177 通过，服务器提交/撤回 smoke 通过。
- Round 30 已补齐 `PartnerPost` 管理员审核队列、审核详情、通过/驳回裁定；后端 190/190 通过，服务器审核队列、详情和审核通过 smoke 通过。
- Round 31 已补齐 `PartnerPost` 广场列表、筛选、搜索、详情和详情页发布者公开信用摘要；后端 202/202 通过，服务器列表/详情 smoke 通过。
- Round 32 已补齐 `PartnerPost` 广场列表项发布者认证状态和公开信用摘要；后端 205/205 通过，服务器列表公开摘要 smoke 通过。
- Round 33 已补齐低压力联系基础会话后端：发起联系、创建/复用会话、初始邀约消息、普通短文本消息、会话列表和消息查询；新增 V9 additive migration；后端 222/222 通过，服务器 smoke 通过。
- Round 34 已补齐低压力联系查询合同：会话列表严格 `updatedAt DESC`，`afterMessageId` 增量拉取受 size/MAX_PAGE_SIZE 限制；后端 225/225 通过，服务器 smoke 通过。
- Round 35 已提交 Qt 广场与基础会话 API service/model，但 Qt 构建、Qt 测试和 Qt server smoke 因当轮 shell 未找到 cmake/Qt6 工具链而未形成完整绿灯闭环；当前只能视为“代码提交已完成，验证待收口”。
- Qt 工具链实际仍在本机：`G:\Qt\6.10.3\mingw_64`，CMake 为 `G:\Qt\Tools\CMake_64\bin\cmake.exe`，Ninja 为 `G:\Qt\Tools\Ninja\ninja.exe`，MinGW 为 `G:\Qt\Tools\mingw1310_64\bin`。
- Round 36 已收口 Qt API client 构建验证与合同修正：提交 `cc40539`，Qt `ctest` 6/6 通过，Qt server integration smoke 10/10 通过，修复 `CampusApiClient::buildUrl` query string 处理、Qt API service URL encoding、`queryMessages(page,size)` 忽略 page 等问题。
- Round 37 已补齐 Qt 学生侧“我的发布”API client：提交 `be9077c`，Qt `ctest` 7/7 通过，Qt server integration smoke 16/16 通过；但复核发现 `MyPartnerPostApiServiceTest` 多个用例名称声称校验 path/header/body，实际 mock server 未捕获原始请求，合同测试需要补强。
- Round 38 已补强 Qt 我的发布 API client 合同测试：提交 `098902c`，只重写 `MyPartnerPostApiServiceTest` 和 validation，Qt `ctest` 7/7 通过，server smoke 16/16 保持通过。
- Round 39 已完成 Qt 学生侧发布/广场/联系入口 UI 一期：提交 `f45e352`，新增 `PostEditorWidget`、`MyPostsWidget`、`PlazaWidget`，Qt `ctest` 8/8 通过，`campus_buddy_desktop --smoke-test` 通过，server smoke 16/16 通过；残余风险为 Widget 交互未自动化、广场详情未调用单独 GET、场景筛选枚举需修正。
- Round 40 已完成 Qt 低压力联系会话 UI 与广场合同小修：提交 `f4b6555`，新增 `ConversationsWidget`，修正广场/发布场景枚举，广场详情接入 GET；Qt `ctest` 8/8、`campus_buddy_desktop --smoke-test` 和 server smoke 16/16 均通过。
- Round 41 已完成 Qt 评价与信用摘要 API client + UI：提交 `d6519b3`，Qt build、`ctest` 9/9 和 server smoke 21/21 通过；但复核发现评价提交 smoke 实际返回 `CONVERSATION_NOT_REVIEWABLE`（NOTE 而非真实 PASS），且信用摘要 topTags 应解析后端 `tagName` 字段，需要 Round 42 收口。
- Round 42 已完成 Qt 评价信用合同与 smoke 补强：提交 `f0145a9`，修复 topTags `tagName` 解析、删除公开摘要永真断言，Qt `ctest` 9/9 通过，server smoke 22/22 通过，评价提交真实创建成功。
- Round 43 已完成 Qt 管理员审核 UI 主体：提交 `de4f9f5`，新增 `AdminReviewWidget`、`AdminReviewApiService` 和 10/10 Qt 测试；但复核发现身份认证 `submissionId` 被错误建模为数字而后端实际为 UUID，且发布审核驳回 server smoke 未真实通过，需要 Round 44 收口。
- Round 44 已完成 Qt 管理员审核合同与 smoke 收口：提交 `fa7346e`，修复身份认证 `submissionId` UUID 字符串合同、认证审核 UI 字段显示和 STUDY `scenePayload.studyGoal` smoke 数据；Qt `ctest` 10/10、`campus_buddy_desktop --smoke-test` 和 server smoke 27/27 通过。
- Round 45 已完成低压力联系后端会话状态与未读数：提交 `ffa1d71`，新增 V10 迁移，补会话关闭、CLOSED 发送阻断、重新发起恢复 ACTIVE、unreadCount 和标记已读；后端 236/236 通过，服务器 V1-V10 validated，close/read/recontact smoke 通过。
- Round 46 已完成 Qt 桌面端适配会话状态与未读数：提交 `39cfac3`，新增 unreadCount 解析、closeConversation/markConversationRead API 方法、ConversationsWidget UI 适配（未读数显示、标记已读/关闭按钮、CLOSED 禁用发送、CONVERSATION_CLOSED 错误处理）；Qt ctest 10/10、server smoke 33/33 全部通过。
- Round 47 已完成后端联系方式卡片与双方解锁：提交 `62cd1a3`，新增 V11 迁移（contact_card + contact_unlock_confirm），ContactCard CRUD、单方确认→WAITING_FOR_PEER、双方确认+有卡片→UNLOCKED、查看对方联系方式、CLOSED 阻断、6 星评价解锁；后端 249/249 通过，服务器 smoke 9/10 PASS。
- Round 48 已完成 Qt 联系方式卡片与解锁 UI 适配：提交 `309fa94`，5 个 API 方法实现、5 个合同测试、ConversationsWidget 联系方式编辑区+解锁状态+确认交换+查看对方卡片、smoke 步骤 34-38；Qt ctest 10/10、server smoke 38/38 全部通过。
- Round 49 已完成课程演示准备、交付清单与敏感留档修正：提交 `4087f5d`，修正 Round 48 validation 中联系方式测试值，新增 `docs/27_course_demo_and_delivery_checklist_v1.md`，明确 14 步演示主线和交付清单。
- 当前残余风险：OBS 物理对象删除尚未独立复核；公网仍为 HTTP。
- 当前最大功能缺口：投诉申诉/治理/通知仍未实现，但不建议在课程演示收尾前继续扩大业务范围。
- 后续收敛路线见 `docs/26_remaining_function_completion_roadmap_v1.md`。
- 课程演示与交付清单见 `docs/27_course_demo_and_delivery_checklist_v1.md`。
- Round 50 已完成答辩演示彩排与最终验收证据包（人工试用），发现 VALIDATION_FAILED 模糊提示、场景字段缺失、防重复提交缺失等问题。
- Round 51 已完成用户行为负向测试矩阵与错误处理修复：新增 200+ 负向测试案例文档、修复 Qt VALIDATION_FAILED 字段级错误展示、场景字段动态 UI、防重复提交、8 个新自动化测试；Qt ctest 10/10、server smoke 38/38 通过。
- Round 52 已完成角色隔离 UI 与切换账号数据清理：后端登录响应增加 accountRole 字段；Qt 根据角色动态构建 tab（STUDENT 只显示学生功能，ADMIN 只显示审核功能）；退出登录时清空所有子 widget 缓存数据；后端 249/249、Qt ctest 10/10、server smoke 38/38 通过。
- Qt UI 视觉打磨第一批已完成：新增 AppStyles/UiHelpers，打磨登录/注册、首页、发布表单、广场、会话和评价信用页；Qt build、ctest 10/10、desktop smoke 通过；server smoke 因当前 shell 未注入私有 smoke 账号环境变量而阻塞，未伪装为通过。
- Qt UI 视觉打磨第二批已完成：打磨认证资料、我的发布、管理员审核页面；补管理员认证审核队列选择处理；Qt build、ctest 10/10、desktop smoke 通过；server smoke 仍因当前 shell 未注入私有 smoke 账号环境变量而阻塞。
- Qt UI 视觉打磨第三批已完成：统一主链路空状态、加载态和防重复点击；新增 `UiHelpers::setButtonBusy` / `emptyStateText` 并接入认证、我的发布、广场、会话、管理员审核；Qt build 通过，逐项 `ctest -R` 10/10 通过，desktop smoke 通过；同一 `ctest` 批量进程下出现 `0xc0000135` DLL 搜索异常，server smoke 仍因当前 shell 未注入私有 smoke 账号环境变量而阻塞。
- Qt UI 视觉打磨第四批已完成：发布表单增加字段级错误标签，保存/更新/提交接入忙碌态；评价信用页改为滚动内容区，并补刷新、提交、修改、列表刷新忙碌态和评价列表空状态；Qt build、ctest 10/10、desktop smoke 通过；server smoke 仍因当前 shell 未注入私有 smoke 账号环境变量而阻塞。
- 真实邮箱验证码发送能力已补齐：新增 SMTP delivery mode、JavaMailSender 配置、SMTP 验证码 sender 和测试；后端全量测试 251/251 通过。
- 真实注册收信 smoke 辅助脚本已补齐：`scripts/real_email_registration_smoke.ps1` 可从项目外私有 env 启动 local-h2 后端、发送验证码、读取私有 env 中的验证码并完成注册/登录验证。
- 真实注册收信 smoke 已通过：使用项目目录外 `D:\big_homework_private\smtp-service.env`、QQ SMTP 发信服务和 `bjtu.edu.cn` 收件域名完成真实收信、验证码校验、注册和登录验证；validation 见 `docs/validation/20260525_real_email_registration_smoke_success_record.md`。
- 下一步优先级：如需演示 Qt 桌面端注册页，使用一个尚未注册过的新 `bjtu.edu.cn` 邮箱重新走 GUI 注册链路；然后进入答辩现场演示或按需做 Round 53 生产化收口。

## Git 提交历史

- `447b0af` chore:-establish-project-baseline-and-backend-configuration
- `18e2a01` chore(repo): clean generated artifacts and record git hygiene
- `157abc0` feat(auth): persist authentication flow and introduce jwt
- `3be73bf` docs(codearts): record round04 handoff and round05 prompt
- `2ee5b68` feat(auth): add identity verification submission
- `d5c78fa` docs(codearts): record round05 handoff and round06 prompt
- `4955bee` feat(auth): add identity verification admin review
- `8edd057` feat(auth): add identity material attachment upload
- `b850d37` docs(codearts): record round06/07 validation and update handoff
- `ca058c3` feat(desktop): add login/register UI and auth API integration
- `957c283` docs(handoff): update latest.md after Qt client auth integration
- `8bb54bd` docs(codearts): record round07 handoff and round08 prompt
- `e53f11a` fix(desktop): align auth flow with backend contract
- `b44f519` docs(codearts): record round08 validation and update handoff
- `bc0ae9e` docs(codearts): record round08 handoff and round09 prompt
- `c59a138` feat(desktop): add identity verification submission UI
- `bb8b4a7` docs(codearts): record round09 validation and update handoff

## 已完成事项

### P0 低压力联系最小后端依赖规格（2026-05-20）

- 新增 `docs/24_p0_contact_min_dependency_spec_v1.md`。
- 明确该切片只服务于解除 P1-1 评价与信用摘要阻断，不扩展为完整 IM。
- 定义 `Conversation`、`ConversationMessage`、`ContactUnlockRecord` 的最小字段、状态、枚举和值域。
- 明确用户消息与系统消息的区分：有效对话只统计用户消息，系统消息必须排除。
- 明确有效对话规则：同一会话双方各自至少 2 条用户消息。
- 明确联系方式解锁判定：只通过 `ContactUnlockRecord` 判断解锁事实，不保存外部联系方式明文。
- 明确 Repository / Service 查询能力、测试先行入口、红灯/绿灯/留档要求、服务器 smoke test 门禁和 Git 提交边界。
- 明确与 P1-1 阻断项的追溯关系，供下一轮 CodeArts 实现 prompt 直接引用。

### Qt 认证资料提交 UI（提交 `c59a138`）
- `CampusApiClient` 新增 multipart 上传能力。
- `AuthApiService` 新增认证材料上传、完整认证资料提交、扩展认证状态读取。
- `IdentityVerificationWidget` 支持真实姓名、学号、学院、专业、年级、材料选择、上传和提交。
- `HomePageWidget` 嵌入认证资料组件并传递认证状态。
- Qt 测试 3/3 通过。

### Qt 登录/注册最小 UI（提交 `ca058c3`）
- 新增 LoginWidget、RegisterWidget、HomePageWidget。
- 新增 AuthApiService、AuthTokenStore。
- CampusApiClient 支持 Bearer 认证头。
- Qt 测试 3/3 通过。

### Qt 认证契约修正（提交 `e53f11a`）
- 新增 `SecureTokenStore` 抽象和 `InMemorySessionTokenStore` 内存实现。
- token 不再写入 QSettings。
- `AuthResult` 拆分 `accessToken`、`authenticationStatus`、`verificationTicket`。
- 验证码接口统一传 `purpose=REGISTER_OR_LOGIN`。
- 校验验证码后读取 `verificationTicket`。
- 注册请求体改为 `campusEmail/verificationTicket/password/displayName`。
- 注册 UI 改为 email → 发送验证码 → 校验验证码 → displayName + 密码 → 注册。
- Qt 测试 3/3 通过。

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
- 真实邮件发送已支持 SMTP 配置开关；未配置 SMTP 时默认 no-op，真实收信需私有环境变量
- 不实现预签名 URL，不让 Qt 客户端直传

## 配置边界摘要

可进入项目文档或模板的非敏感配置：
- `campus-buddy.campus-email.*`、`campus-buddy.object-storage.*`、`campus-buddy.security.jwt.*`（不含 secret）

不得进入项目文档、仓库、聊天或 Qt 客户端的敏感配置：
- `JWT_SECRET`（deploy profile）、`OBJECT_STORAGE_ACCESS_KEY_ID`、`OBJECT_STORAGE_SECRET_ACCESS_KEY`、`DB_PASSWORD`
- `CAMPUS_EMAIL_SMTP_USERNAME`、`CAMPUS_EMAIL_SMTP_PASSWORD`

## 推荐下一步

1. `CodeArts P0 低压力联系最小依赖实现 prompt 设计`
   - 优先级：最高。
   - 目标：下一线程切回 CodeArts prompt 设计与复核线程，基于 `docs/24_p0_contact_min_dependency_spec_v1.md` 生成实现提示词，先让 CodeArts 补 `Conversation`、`ConversationMessage`、`ContactUnlockRecord` 及最小查询能力；线程本身不直接写业务代码、不调用 CodeArts。

2. `P1-1 评价与信用摘要 Batch 1 后端核心`
   - 优先级：高。
   - 目标：只有在 P0 低压力联系最小依赖实现、测试和留档通过后，才重新激活 P1-1 后端核心实现。

3. `提交当前纯文档留档`
   - 优先级：高。
   - 目标：CodeArts prompt 设计交接确认后，提交当前纯文档改动，避免后续真实服务器复核或实现准备混入设计尾巴。

4. `服务器基线复核`
   - 优先级：高。
   - 目标：按 `docs/23_server_smoke_test_readiness_checklist_v1.md` 逐项确认 Ubuntu 24、SSH、安全组、Nginx、公网 IP、PostgreSQL、OBS 和私有配置。

5. `P1 投诉申诉与案件模块小修`
   - 优先级：高。
   - 目标：仅限发现字段、编号或表述小问题时处理。

6. `P1 评价与信用摘要模块小修`
   - 优先级：高。
   - 目标：仅限发现字段、编号或表述小问题时处理。

7. `P1 管理端治理模块小修`
   - 优先级：中。
   - 目标：仅限发现字段、编号或表述小问题时处理；P1-3 主体已阶段性收束。

8. `Windows Credential Manager 适配器`
   - 优先级：高。
   - 目标：替换当前内存 token store，使 token 可跨进程安全保存。

9. `真实 OBS SDK 适配器`
   - 优先级：中。
   - 目标：实现 HuaweiOBSObjectStorageService 替换 InMemoryObjectStorageService。
   - 需要华为云 OBS 凭据。

10. `替换 no-op 邮件发送`
   - 优先级：中。
   - 目标：接入 SMTP 或华为云邮件服务发送校园邮箱验证码。

11. `Testcontainers 集成测试`
   - 优先级：低。
   - 目标：在 Docker 可用环境运行完整集成测试。

12. `完整 RBAC 权限矩阵`
   - 优先级：低。
   - 目标：STUDENT / ADMIN / SUPER_ADMIN 角色权限细化。

## 后续实现环境原则

- 后续实现不再只以 Win11 本地环境通过为闭环，应尽早引入 Ubuntu 24 服务器实战部署验证。
- 后端、数据库、对象存储适配、Nginx/公网 IP 访问和运行配置应在服务器环境中验证；Win11 本地主要承担 Qt 客户端开发、基础单元测试和快速调试。
- 对象存储后续应从 `InMemoryObjectStorageService` 测试替身推进到真实 OBS/对象存储适配器；凭据只放服务器私有环境变量或私有配置，不写入仓库、文档或 Qt 客户端。
- 后续每个实现批次除本地测试外，应尽量增加服务器 smoke test 或部署验证记录，避免 Win11 与 Ubuntu 24 环境差异在最后集中暴露。
- 涉及后端、数据库迁移、对象存储、部署配置或公网访问的批次，完成门禁调整为“本地测试通过 + Ubuntu 24 服务器 smoke test 通过”；只在 Win11 本地通过不视为完整闭环。
- 服务器 smoke test 至少记录部署/重启结果、健康检查、关键接口最小调用、数据库迁移或连通性、对象存储连通性和私有配置就绪情况。
