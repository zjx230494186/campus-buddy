# 26 Remaining Function Completion Roadmap V1

本文用于把 `docs/13_detailed_design_v1.md` 中尚未实现的功能收束成后续 CodeArts 批次。它不是新需求文档，只记录从当前代码状态到课程答辩可演示闭环的实现顺序。

## 0. 当前基线

截至 2026-05-24，已完成并验证的主要能力：

- 账号注册、登录、JWT 认证。
- 身份认证资料提交、管理员审核、认证材料上传与本人未引用材料删除。
- 后端对象存储抽象、华为云 OBS deploy profile、服务器 OBS smoke。
- Ubuntu 24.04.4 服务器部署、Nginx 公网入口、systemd 服务化。
- P0 低压力联系最小依赖对象：`Conversation`、`ConversationMessage`、`ContactUnlockRecord`。
- P1 评价与信用摘要：评价提交、修改、信用摘要、公开/本人摘要差异、评价列表。
- Qt 登录注册、认证资料提交、学生发布/广场/联系会话 UI、联系方式卡片/解锁 UI、评价与信用摘要 UI、API base URL 运行时配置、服务器 smoke。

重要未完成事实：

- `PartnerPost` 需求发布与审核后端主闭环已实现：学生草稿、提交审核、撤回审核、主动下架、管理员审核队列、审核详情、通过/驳回。
- 广场发现后端已具备已发布需求列表、筛选、搜索、详情和列表/详情发布者公开摘要。
- 低压力联系已具备基础站内会话、未读状态、关闭/重开、联系方式卡片与双方解锁；实时 WebSocket 和复杂 IM 能力未实现。
- 投诉申诉、管理端治理、通知与留痕尚未实现。
- Qt 端已具备学生主链路和管理员审核 UI；通知/案件/治理 UI 尚未实现。
- HTTPS、真实邮件发送、Windows Credential Manager、OBS IAM 委托、CI/CD、监控备份仍属于生产化缺口。

## 1. 实现顺序原则

后续按依赖链推进，不跳到下游 UI：

1. 先补 `PartnerPost` 发布物生命周期。
2. 再补广场发现，让用户能找到已发布需求。
3. 再补低压力联系，让用户能基于需求发起会话和站内消息。
4. 再补 Qt 对应 UI 与服务器模式验证。
5. 再补投诉申诉、治理、通知等 P1 支撑功能。
6. 最后做 HTTPS、邮件、凭据托管、演示脚本和总文档收尾。

## 2. 后续 CodeArts 批次

### Round 28：P0-2 需求发布与审核 Batch 1A 后端学生侧草稿地基

目标：

- 新增 `PartnerPost` 持久化模型、Flyway V8。
- 支持已认证学生显式创建草稿、更新自己的草稿、查询我的发布列表和详情。
- 只处理 `DRAFT` 草稿地基。
- 不实现提交审核、撤回审核、下架、管理侧审核、广场、低压力联系或 Qt UI。

完成门槛：

- 后端新增测试红灯后绿灯。
- 后端全量回归通过。
- deploy 到服务器后至少验证 health、Flyway V1-V8、创建草稿、更新草稿、查询我的发布列表和详情。

状态：

- 已完成。提交：`cd6e0d6 feat(post): add PartnerPost student draft CRUD API with Flyway V8 migration`。
- 后端 163/163 通过；服务器 V8 迁移和 5/5 smoke 通过。

### Round 29：P0-2 需求发布与审核 Batch 1B 后端学生侧提交与下架

目标：

- 提交审核：`DRAFT -> PENDING_REVIEW`。
- 撤回审核：`PENDING_REVIEW -> DRAFT`。
- 主动下架：`PUBLISHED -> DRAFT`。
- 提交审核强校验公共字段和最小场景字段。
- 仍不实现管理员审核、广场或 Qt UI。

状态：

- 已完成。提交：`2113d29 feat(post): add student submit and unpublish lifecycle`。
- 后端 177/177 通过；服务器创建草稿、提交审核、详情、撤回审核 smoke 通过。

### Round 30：P0-2 需求发布与审核 Batch 2 后端管理侧

目标：

- 管理员审核队列、审核详情、通过/驳回裁定。
- 审核通过后进入 `PUBLISHED`。
- 驳回原因、审核时间、审核管理员记录。
- 发布者撤回与管理员审核状态冲突处理。

状态：

- 已完成。提交：`509f7d7 feat(post): add admin partner post review APIs`。
- 后端 190/190 通过；服务器审核队列、详情和审核通过 smoke 通过。
- 残余风险：审核队列排序未显式固定；并发审核冲突未处理；驳回后学生重编辑再提交缺端到端测试；服务器 smoke 管理员账号仍依赖临时 SQL 初始化。

### Round 31：P0-3 广场发现与推荐后端

目标：

- 查询 `PUBLISHED` 需求列表、筛选、搜索、详情。
- 列表和详情返回发布者公开摘要与信用摘要，不泄露真实姓名、学号、校园邮箱或联系方式。
- 自己发布的需求可查看但不可发起联系。

状态：

- 已完成主体。提交：`0d4ad38 feat(post): add plaza discovery list and detail endpoints`。
- 后端 202/202 通过；服务器列表、详情和未登录 401 smoke 通过。
- 复核发现列表项只返回 `publisherDisplayName`，缺少 `publisherAuthenticationStatus` 与公开信用摘要；详情已包含公开信用摘要。进入低压力联系前先做 Round 32 合同补强。

### Round 32：P0-3 广场发现后端 Batch 1B 合同补强

目标：

- 补齐 `GET /api/partner-posts` 列表项中的发布者公开摘要。
- 列表项返回 `publisherAuthenticationStatus` 和公开 `publisherCreditSummary`。
- 保持不返回 `disputedReviewCount`、联系方式、真实姓名、学号、校园邮箱、认证材料 objectKey、审核内部字段。
- 不新增推荐算法、低压力联系、Qt UI 或 Flyway 迁移。

状态：

- 已完成。提交：`8eae3ff feat(post): add publisher summary to plaza list items`。
- 后端 205/205 通过；服务器广场列表公开摘要 smoke 通过。
- 残余风险：列表逐项查信用摘要性能待优化；信用摘要异常时可能返回 null。

### Round 33：P0-4 低压力联系后端 Batch 1A 发起联系与基础消息

目标：

- 基于 `PUBLISHED PartnerPost` 发起联系。
- 创建或复用 `Conversation`。
- 写入初始邀约消息。
- 发送普通短文本消息。
- 会话列表、会话详情、消息分页和 `afterMessageId` 增量拉取。
- 不做 WebSocket、图片消息、完整 IM、拉黑、举报、联系方式卡片解锁、未读数和标记已读。

状态：

- 已完成主体。提交：`8656480 feat(contact): add contact request and conversation messaging endpoints`。
- 新增 V9 additive migration：`conversation.related_post_uuid UUID`。
- 后端 222/222 通过；服务器发起联系、会话列表、发送消息、查询消息 smoke 通过。
- 残余风险：会话列表排序未严格 `updatedAt DESC`；`afterMessageId` 增量拉取未做分页保护；CLOSED 会话重开未实现；Qt 尚未对接。

### Round 34：P0-4 低压力联系后端 Batch 1B 查询合同补强

目标：

- 会话列表严格按 `updatedAt DESC` 排序，不能只在未赋值的 stream 上排序。
- `afterMessageId` 增量拉取受 `size` 上限保护，避免一次性返回无限消息。
- 保持 Round 33 发起联系、发送消息、会话列表和消息查询既有合同不破坏。
- 不做 Qt、会话关闭、未读数、联系方式解锁或通知。

状态：

- 已完成。提交：`ea78231 fix(contact): harden conversation list sorting and afterMessageId pagination`。
- 后端 225/225 通过；服务器会话列表排序和 afterMessageId 分页保护 smoke 通过。
- 残余风险：CLOSED 会话重开未实现；消息长度上限 30 较保守；Qt 尚未对接；WebSocket 未实现。

### Round 35：Qt 广场与基础会话 API Client 对接

目标：

- Qt 新增 PartnerPost/Plaza API service 或等价领域 API 封装。
- Qt 新增 ContactConversation API service 或等价领域 API 封装。
- 支持调用广场列表/详情、发起联系、会话列表、发送消息、查询消息。
- 扩展 Qt server integration smoke，验证真实服务器 health、login、广场列表、发起联系、会话列表、发送消息、查询消息。
- 不做 Qt UI 页面，不做联系方式解锁，不做未读数或会话关闭。

状态：

- 已提交但未完成验证闭环。提交：`4fab1c4 feat(desktop): add PartnerPost and ContactConversation API services`。
- 本轮新增 Qt PartnerPost/ContactConversation API service、domain models、测试和 server smoke 扩展。
- Qt 构建、Qt 测试、`campus_buddy_desktop` 构建和 Qt server smoke 因当轮 shell 未找到 cmake/Qt6 工具链而阻塞；后端 225/225 通过不能替代 Qt 绿灯。
- 复核确认本机 Qt 工具链仍存在：`G:\Qt\6.10.3\mingw_64`、`G:\Qt\Tools\CMake_64\bin\cmake.exe`、`G:\Qt\Tools\Ninja\ninja.exe`、`G:\Qt\Tools\mingw1310_64\bin`。
- 进入 Round 36 收口，不直接进入 UI。

### Round 36：Qt API Client 构建验证与合同修正

目标：

- 显式使用 `G:\Qt\6.10.3` 工具链完成 Qt configure、build、ctest。
- 修复 Round 35 暴露的编译、链接、测试或 API client 合同问题。
- 补强请求 path、query、Authorization header、request body、URL encoding、`queryMessages` page/afterMessageId 分支的测试。
- 完成 `campus_buddy_desktop` 构建验证。
- 尽量完成 Qt server integration smoke；若服务器数据无可联系 `ownPost=false` post，则如实记录数据阻塞。
- 不做 Qt UI、不改后端、不改 Flyway、不改 deploy。

状态：

- 已完成。提交：`cc40539 fix(desktop): correct buildUrl query string handling and smoke test admin contact`。
- Qt `ctest` 6/6 通过；Qt server integration smoke 10/10 通过。
- 修复 `CampusApiClient::buildUrl` 对 path 中 query string 的处理、Qt API service URL encoding、`queryMessages(page,size)` 忽略 page、server smoke 自己联系自己 post 的问题。
- 残余风险：server smoke 仍依赖服务器上已有可用测试账号和已发布 post，后续 UI/演示 smoke 应进一步固定测试数据选择。

### Round 37：Qt 学生侧发布 API Client 对接

目标：

- 新增 Qt 学生侧“我的发布”API service/model。
- 覆盖 `POST/PUT/GET /api/me/partner-posts`、详情、提交审核、撤回审核、主动下架端点。
- 补齐 Qt 测试和最小服务器 smoke：创建草稿、更新草稿、我的发布列表、详情、提交审核、撤回审核。
- 不做 Qt UI，不改后端，不改 Flyway，不改 deploy。

状态：

- 已完成主体。提交：`be9077c feat(desktop): add my partner post api client`。
- Qt `ctest` 7/7 通过；Qt server integration smoke 16/16 通过。
- 残余问题：`MyPartnerPostApiServiceTest` 中多个合同测试没有捕获原始 HTTP 请求，method/path/query/header/body 断言不实；进入 UI 前需做 Round 38 测试补强。

### Round 38：Qt 我的发布 API Client 合同测试补强

目标：

- 补实 `MyPartnerPostApiServiceTest` 的 mock HTTP request 捕获。
- 真实断言 method、path、query、Authorization header、Content-Type 和 JSON body。
- 必要时最小修复 `MyPartnerPostApiService` / `CampusApiClient::putJson`。
- 复跑 Qt `ctest`、`campus_buddy_desktop` 构建和 Qt server smoke 16/16。
- 不做 Qt UI，不改后端，不改 Flyway，不改 deploy。

状态：

- 已完成。提交：`098902c test(desktop): harden my partner post api client contract tests`。
- Qt `ctest` 7/7 通过；Qt server smoke 16/16 保持通过。

### Round 39：Qt 学生侧发布/广场/联系入口 UI

目标：

- 新增学生侧发布草稿、我的发布列表、广场列表/详情和发起联系入口 UI。
- 集成到 `HomePageWidget` Tab。
- 保持 Qt 构建、启动 smoke 和 server smoke 通过。

状态：

- 已完成一期。提交：`f45e352 feat(desktop): add student post and plaza widgets`。
- 新增 `PostEditorWidget`、`MyPostsWidget`、`PlazaWidget`；Qt `ctest` 8/8 通过；`campus_buddy_desktop --smoke-test` 通过；Qt server smoke 16/16 通过。
- 残余风险：Widget 交互未自动化；`PlazaWidget` 未单独 GET 详情；sceneType 筛选枚举需修正；无完整聊天 UI。

### Round 40：Qt 低压力联系会话 UI + 广场合同小修

目标：

- 修正广场 sceneType 筛选枚举为后端有效值。
- 广场选择条目后调用详情 GET。
- 新增会话列表、会话详情和发送消息 UI。
- 集成到 `HomePageWidget` Tab。
- 不做 WebSocket、未读数、会话关闭、联系方式解锁或评价 UI。

状态：

- 已完成。提交：`f4b6555 feat(desktop): add contact conversation widget`。
- 新增 `ConversationsWidget`；修正广场 sceneType 枚举和详情 GET；Qt `ctest` 8/8、`campus_buddy_desktop --smoke-test`、Qt server smoke 16/16 通过。
- 残余风险：无自动刷新/轮询，无 WebSocket，Widget 交互未自动化。

### Round 41：Qt 评价与信用摘要 API Client + UI

目标：

- 新增 Qt 评价与信用摘要 API service/model。
- 支持提交评价、修改评价、已发出/已收到评价列表、我的信用摘要、公开信用摘要。
- 新增 `ReviewCreditWidget` 并集成到 `HomePageWidget`。
- 扩展 Qt server smoke，尽量验证有效会话评价提交、信用摘要和评价列表。
- 不改后端，不做投诉申诉、治理、通知或管理员 UI。

状态：

- 已完成主体。提交：`d6519b3 feat(desktop): add review & credit summary API client and UI`。
- Qt build 92/92、`ctest` 9/9、server smoke 21/21 通过。
- 复核保留问题：server smoke create review 为 `CONVERSATION_NOT_REVIEWABLE` NOTE，不是真实 PASS；topTags 应解析后端 `tagName` 字段；公开摘要测试有永真断言。

### Round 42：Qt 评价信用合同与 Smoke 补强

目标：

- 修复信用摘要 topTags `tagName` 解析。
- 修复公开摘要测试中的无意义永真断言。
- 构造真实可评价会话，让 Qt server smoke 中 create review 成功，或明确记录不可重复评价阻塞。
- 不改后端，不新增 UI 页面，不做管理员 UI。

状态：

- 已完成。提交：`f0145a9 fix(desktop): harden review credit parsing and smoke`。
- 修复信用摘要 topTags `tagName` 解析，保留 `tag` fallback。
- 删除公开摘要测试中的永真断言。
- Qt server smoke 构造双方各 2 条 USER_TEXT 的真实可评价会话，`POST /me/reviews` 真实 PASS。
- Qt `ctest` 9/9 通过；server smoke 22/22 通过。
- 残余风险：server smoke 每次创建评价会累积测试数据，后续断言需避免依赖固定 items 数量。

### Round 43：Qt 管理员审核 UI 最小闭环

目标：

- 新增管理员审核入口。
- 覆盖 `PartnerPost` 管理员审核队列、详情、通过/驳回。
- 覆盖身份认证待审核列表、通过/驳回。
- 服务器 smoke 只主动审核本轮创建的测试发布；身份认证列表只查 count，不误审真实待审申请。
- 不改后端、不改 Flyway、不改 deploy、不做材料内容下载/预览。

当前 prompt：

- `docs/prompts/codearts/20260524_round_43_qt_admin_review_ui.md`

状态：

- 主体已完成但需收口。提交：`de4f9f5 feat(desktop): add admin review UI`。
- 新增 `AdminReviewWidget`、`AdminReviewApiService`、`AdminReviewModels` 和 `AdminReviewApiServiceTest`；Qt `ctest` 10/10 通过。
- 复核发现身份认证审核 `submissionId` 类型错误：Qt 侧使用 `long long`，后端实际为 UUID 字符串。
- 复核发现管理员发布驳回 server smoke 未真实通过：测试发布提交审核返回 `VALIDATION_FAILED`，后续驳回被跳过。
- 进入 Round 44 做合同与 smoke 收口，不直接进入打包或新功能。

### Round 44：Qt 管理员审核合同修复与 Smoke 收口

目标：

- 修复身份认证审核 `submissionId` UUID 字符串合同。
- 补强 `AdminReviewApiServiceTest`，使用 UUID mock 验证路径和解析。
- 补真实管理员发布审核驳回 smoke：本轮创建测试发布、提交审核、admin 详情、admin REJECT、验证 `REJECTED/reviewedAt/rejectReason`。
- 补跑 `campus_buddy_desktop --smoke-test`。
- 不改后端、不改 Flyway、不改 deploy、不新增新功能。

当前 prompt：

- `docs/prompts/codearts/20260524_round_44_qt_admin_review_contract_and_smoke_fix.md`

状态：

- 已完成。提交：`fa7346e fix(desktop): fix admin review submissionId UUID type and smoke scenePayload`。
- 修复身份认证审核 `submissionId` UUID 字符串合同。
- 修复认证审核 UI 字段显示。
- 修复 STUDY smoke draft 缺少 `scenePayload.studyGoal` 导致的提交审核 `VALIDATION_FAILED`。
- Qt `ctest` 10/10 通过；`campus_buddy_desktop --smoke-test` 通过；server smoke 27/27 通过，admin REJECT 真实 PASS。
- 残余风险：身份认证提交 + 审核完整 smoke 仍未覆盖，当前待审列表 items=0；公网仍为 HTTP。

### Round 45：P0-4 低压力联系后端 Batch 1C 会话关闭与未读状态

目标：

- 会话关闭。
- 会话重新发起时恢复 ACTIVE 并追加新邀约消息。
- 未读数和标记已读。

当前 prompt：

- `docs/prompts/codearts/20260524_round_45_contact_conversation_state_and_unread_backend.md`

状态：

- 已完成。提交：`ffa1d71 feat(contact): add conversation close and unread state`。
- 新增 V10 additive migration：`participant1_last_read_message_id`、`participant2_last_read_message_id`。
- 后端新增会话关闭、标记已读、CLOSED 发送阻断、重新发起恢复 ACTIVE、会话列表 `unreadCount`。
- 新增 11 个测试，后端 236/236 通过；服务器 V1-V10 validated，close/read/recontact smoke 通过。
- 残余风险：Qt 尚未适配新增端点和 `unreadCount`。

### Round 46：Qt 会话状态与未读数适配

目标：

- Qt `ConversationListItem` 解析 `unreadCount`。
- `ContactConversationApiService` 新增 `closeConversation` 与 `markConversationRead`。
- `ConversationsWidget` 显示未读数、支持标记已读、支持关闭会话、`CLOSED` 禁用发送。
- 扩展 Qt server smoke 覆盖 unread/read/close/recontact。
- 不改后端、不改 Flyway、不做联系方式解锁。

当前 prompt：

- `docs/prompts/codearts/20260524_round_46_qt_contact_state_and_unread_ui.md`

状态：

- 已完成。提交：`39cfac3 feat(desktop): add conversation unread and close controls`。
- Qt `ConversationListItem` 已解析 `unreadCount`。
- `ContactConversationApiService` 已新增 `closeConversation` 与 `markConversationRead`。
- `ConversationsWidget` 已支持未读数显示、标记已读、关闭会话、`CLOSED` 禁用发送和 `CONVERSATION_CLOSED` 错误处理。
- Qt `ctest` 10/10 通过；`campus_buddy_desktop --smoke-test` 通过；server smoke 33/33 通过。
- 残余风险：Widget 交互未自动化；无自动刷新/WebSocket；公网仍为 HTTP。

### Round 47：P0-4 低压力联系后端 Batch 2 联系方式卡片与解锁

目标：

- `ContactCard` 维护。
- 双方确认解锁联系方式。
- 查看已解锁联系方式。
- 关闭会话后阻断联系方式查看。
- 解锁后联动 6 星评价上限。

当前 prompt：

- `docs/prompts/codearts/20260524_round_47_contact_card_unlock_backend.md`

状态：

- 已完成。提交：`62cd1a3 feat(contact): add contact card and unlock flow`。
- 新增 V11 additive migration：`contact_card` 与 `contact_unlock_confirm`。
- 采用独立确认表记录单方确认；只有双方确认且双方都有卡片时才写入 `ContactUnlockRecord`，`isContactUnlocked()` 语义不提前解锁。
- 新增 ContactCard GET/PUT、解锁状态查询、确认解锁、查看对方联系方式。
- 后端 249/249 通过；服务器 V1-V11 validated；ContactCard upsert、WAITING_FOR_PEER、UNLOCKED、CLOSED 阻断和 6 星评价解锁 smoke 通过。
- 残余风险：Qt 尚未适配；联系方式变更通知未做；批量解锁未做；公网仍为 HTTP。

### Round 48：Qt 联系方式卡片与解锁 UI 适配

目标：

- Qt API client 接入 `GET/PUT /api/me/contact-card`。
- Qt API client 接入解锁状态、确认解锁和查看对方联系方式端点。
- `ConversationsWidget` 增加联系方式卡片编辑、解锁状态展示、确认交换和查看对方联系方式。
- 扩展 Qt server smoke，验证卡片保存、确认解锁和查看对方联系方式；不得打印联系方式明文。
- 不改后端、不改 Flyway、不改 deploy。

当前 prompt：

- `docs/prompts/codearts/20260524_round_48_qt_contact_card_unlock_ui.md`

状态：

- 已完成。提交：`309fa94 feat(desktop): add contact card and unlock UI adaptation`。
- 新增 5 个 API 方法实现和 5 个合同测试。
- `ConversationsWidget` 已支持联系方式编辑、解锁状态显示、确认交换和查看对方卡片。
- Qt `ctest` 10/10 通过；server smoke 38/38 通过。
- 复核发现 Round 48 validation 曾记录联系方式测试值，应在 Round 49 做敏感留档修正。

### Round 49：课程演示准备、交付清单与敏感留档修正

目标：

- 修正 Round 48 validation 中联系方式明文测试值，只保留字段存在性。
- 新增课程演示与交付清单文档。
- 更新当前计划、handoff 和 roadmap，明确主演示链路已闭合，投诉/治理/通知/HTTPS/移动端为后续扩展。
- 不改后端、不改 Qt、不改 Flyway、不改 deploy。

当前 prompt：

- `docs/prompts/codearts/20260524_round_49_course_demo_delivery_and_sensitive_doc_cleanup.md`

状态：

- 已完成。Round 48 validation 敏感留档已修正；课程演示与交付清单已新增（`docs/27_course_demo_and_delivery_checklist_v1.md`）；状态文档已更新。
- 无业务代码变更；文档级敏感信息搜索通过。

### Round 50：P1 投诉申诉与案件后端最小闭环

目标：

- 用户针对评价、会话、需求、用户或处理结果创建案件。
- 补充说明和证据。
- 管理员查看、要求补充、裁定。

### Round 51：P1 管理端治理与通知留痕

目标：

- 治理动作、轻度账号处置、系统事件留痕。
- 站内通知列表、未读数、标记已读。

### Round 52：生产化与答辩收尾

目标：

- HTTPS / 域名或可信访问方案。
- 邮件真实发送或答辩可说明的 Noop 边界。
- Windows Credential Manager 或明确的演示期 token 策略。
- OBS IAM 委托或长期 AK/SK 风险说明。
- 总验证记录、部署说明、演示脚本、功能覆盖矩阵。

## 3. 当前下一步

立即进入 Round 49：`课程演示准备、交付清单与敏感留档修正`。

原因：

- Round 48 已把 Qt 联系方式卡片与解锁 UI 接入完成。
- “广场发起联系 → 会话消息 → 双方解锁联系方式 → 评价与信用摘要”的课程演示主链路已闭合。
- 继续开投诉申诉、治理或通知会扩大范围；当前更高收益是先修正敏感留档、形成演示脚本和交付清单，把已完成能力稳定交付。
