# Latest Handoff

本文档服务于新线程快速接手。

## 2026-05-21 当前交接：CodeArts 代码实现 + Codex 文档兜底

### 当前分工

- CodeArts：承担老师要求的代码实现主体，并为每轮代码改动写一手技术留档，尤其是 `docs/validation/` 中的红灯、绿灯、回归、服务器 smoke、敏感信息检查和未覆盖风险。
- Codex：负责复核 CodeArts 的 Git 变更、测试与 validation，纠正边界问题，并把多轮技术结果整理为稳定项目文档、runbook 和下一轮 prompt。

### 最新工程状态

- 最新已复核提交：Round 28 代码待提交。
- Round 21：服务器 smoke test 已验证 P0 联系依赖 + P1 评价信用接口，Flyway V1-V7 通过，公网 health 通过。
- Round 22：deploy profile 已接入真实华为云 OBS，服务器 PUT / GET / SHA-256 / DELETE smoke 通过。
- Round 23：ObsClient 生命周期关闭接入 Spring `destroyMethod`，启动脚本不再通过进程命令行暴露 DB/JWT/OBS secret。
- Round 24：systemd 服务化完成——`campus-buddy-backend` 服务 active + enabled，开机自启；wrapper 脚本 source env + 映射 OBS 变量名；进程命令行仅 `--spring.profiles.active=deploy`；8 项 API smoke + OBS PUT/GET/DELETE 全部通过；146/146 本地回归通过。
- Round 25：Qt 客户端新增运行时 API base URL 配置，服务器联调验证 health、login、credit-summary。
- Round 26：Qt server smoke 移除硬编码测试邮箱/密码，改用 `CAMPUS_BUDDY_SMOKE_EMAIL` / `CAMPUS_BUDDY_SMOKE_PASSWORD`；扩展验证身份认证材料上传；本轮复核已将 validation 中旧测试密码字面量脱敏。
- Round 27：新增本人未引用认证材料删除接口，Qt server smoke 上传材料后调用 DELETE 清理；后端 153/153、Qt 27/27，全量 180/180 通过；服务器 systemd 重启成功，health 200，上传后删除返回 204。
- Round 28：PartnerPost 学生草稿 CRUD 后端 API 完成——Flyway V8 partner_post 表迁移成功；POST/PUT/GET/GET 分页端点实现；163/163 本地测试通过；服务器部署成功，V8 迁移到 v8；5/5 服务器 smoke test 通过（创建/列表/详情/更新/401）。
- 服务器私有配置入口：`/etc/campus-buddy/backend.env`，权限 `600`，owner `root:root`；不得读取、打印或记录真实值。
- 已新增稳定 runbook：`docs/25_server_deploy_and_obs_runbook_v1.md`。
- 已新增剩余功能路线图：`docs/26_remaining_function_completion_roadmap_v1.md`。

### 下一步候选

1. `P0-2 需求发布与审核 Batch 1B 后端学生侧提交与下架`：提交审核、撤回审核、主动下架和提交审核强校验。（推荐下一轮）
2. `P0-2 需求发布与审核 Batch 2 后端管理侧`：管理员审核队列、审核详情、通过/驳回。
3. `P0-2 需求发布与审核 Batch 1A+1B Qt 客户端联调`：Qt 客户端接入 PartnerPost 草稿 CRUD + 提交/下架。
4. `P0-3 广场发现与推荐后端`：已发布需求列表、筛选、搜索、详情和发布者公开摘要。
5. `P0-4 低压力联系后端`：基于已发布需求发起联系、站内短文本消息、轮询、未读数、联系方式卡片和解锁。
6. `Qt 学生侧发布/广场/联系 UI`：补齐课程演示主链路。
7. `HTTPS / 域名 / 备案`：真实试用或展示前处理，解决公网 HTTP 明文传输风险。

## 2026-05-20 临时线程交接：P0 低压力联系最小后端依赖规格

### 当前线程完成了什么

- 已阅读用户指定上下文，包括 `AGENTS.md`、`docs/03_current_plan.md`、`handoff/latest.md`、`docs/13_detailed_design_v1.md`、`docs/06_execution_preparation_v1.md`、`docs/12_code_generation_constraints_v1.md`、`docs/21_codearts_prompt_review_workflow_v1.md`、`docs/22_codearts_unattended_prompt_engineering_v1.md`、P1-1 的 `.codeartsdoer` 规格与设计，以及 Round 14 prompt。
- 新增 `docs/24_p0_contact_min_dependency_spec_v1.md`，作为稳定项目规格文档。
- 文档明确该工作是 `P0 低压力联系最小后端依赖切片`，目标是解除 `P1-1 评价与信用摘要 Batch 1 后端核心` 的阻断。
- 文档只定义 `Conversation`、`ConversationMessage`、`ContactUnlockRecord` 的最小字段、状态、消息类型、查询能力、测试入口和门禁。
- 文档明确不做 Qt UI、完整聊天系统、WebSocket、图片消息、附件/OBS、联系方式明文保存、邀约完整状态机、举报、通知、管理端、P1-1 评价实现。
- 已更新 `docs/03_current_plan.md`，把当前阶段主题与推荐下一步调整为先做 P0 联系最小依赖实现 prompt。

### 下一步候选事项

1. `CodeArts P0 低压力联系最小依赖实现 prompt 设计` — 建议新开或切回 CodeArts prompt 设计线程，优先级最高；基于 `docs/24_p0_contact_min_dependency_spec_v1.md` 生成可复制给 CodeArts 的实现提示词。该线程本身仍不直接写业务代码、不调用 CodeArts。
2. `P0 低压力联系最小依赖 CodeArts 实现` — 建议由 CodeArts 执行；必须测试先行、红灯确认、最小实现、绿灯确认、必要回归、`docs/validation/` 留档、本地测试 + Ubuntu 24 服务器 smoke test 门禁、敏感信息检查、明确 Git 提交边界。
3. `P1-1 评价与信用摘要 Batch 1 后端核心` — 建议在 P0 最小依赖实现通过后再复用或新开 P1-1 实现线程。
4. `提交当前纯文档留档` — 建议在 prompt 设计交接确认后处理，不要和后续业务实现提交混在一起。

### 是否建议切回 CodeArts prompt 设计线程

- 建议：是。
- 理由：本线程已完成稳定规格补充；下一步不是继续扩大设计，而是把规格转成严格受边界约束的 CodeArts 实现 prompt。

### 下一线程建议名称

`CodeArts P0 联系最小依赖实现 prompt 设计与复核`

### 可直接复制的下一线程启动 prompt

```text
请先阅读：
- D:\big_homework\AGENTS.md
- D:\big_homework\docs\03_current_plan.md
- D:\big_homework\handoff\latest.md
- D:\big_homework\docs\12_code_generation_constraints_v1.md
- D:\big_homework\docs\21_codearts_prompt_review_workflow_v1.md
- D:\big_homework\docs\22_codearts_unattended_prompt_engineering_v1.md
- D:\big_homework\docs\24_p0_contact_min_dependency_spec_v1.md
- D:\big_homework\.codeartsdoer\specs\p1-review-credit-batch1\design.md

当前任务：切回 CodeArts prompt 设计与复核线程。现在不要直接写业务代码，不调用 CodeArts，不改后端实现，不创建 Git commit。

请基于 `docs/24_p0_contact_min_dependency_spec_v1.md` 设计下一轮可直接复制给 CodeArts 的实现提示词，目标为 `P0 低压力联系最小后端依赖 Batch 1`。提示词必须要求 CodeArts 只实现 Conversation、ConversationMessage、ContactUnlockRecord 及最小 Repository/Service 查询能力，测试先行、红灯确认、绿灯确认、必要回归、docs/validation 留档、本地测试 + Ubuntu 24 服务器 smoke test 门禁、敏感信息检查和明确 Git 提交边界。

明确禁止 CodeArts 扩大为完整 IM、Qt UI、WebSocket、图片消息、附件/OBS、联系方式明文保存、邀约完整状态机、举报、通知、管理端或 P1-1 评价实现。
```

## 当前状态

- 项目名称：校园搭子平台。
- 当前阶段：P1-1 评价与信用摘要 Batch 1 后端核心已实现。
- 当前线程：Round 17 — P1-1 review/credit batch1 backend core。
- Git 分支：main，HEAD: `193a35e feat(review): add review submission/modification and credit summary APIs`。
- 后端测试：109/109 通过。
- Qt 测试：3/3 通过。

## 本轮设计进展

- 已阅读 `AGENTS.md`、`docs/03_current_plan.md`、`handoff/latest.md`、`docs/13_detailed_design_v1.md`、`docs/06_execution_preparation_v1.md`、`docs/09_codearts_requirement_tables_v1.md`、`docs/12_code_generation_constraints_v1.md`。
- 当前任务明确为继续完善《校园搭子平台》详细设计文档；不写代码，不调用 CodeArts，不生成实现 prompt。
- 已确认 `PartnerPost` 采用“公共字段 + 场景扩展字段”的单对象模型。
- 已在 `docs/13_detailed_design_v1.md` 的 P0-2 需求发布与审核模块中补充 `PartnerPost` 字段模型、公共字段清单、五类场景扩展字段口径、联系方式不写入发布物等规则。
- 已确认并写入“发布者撤回审核”作为正式业务动作：`PENDING_REVIEW -> DRAFT`，并补充撤回与管理员审核并发时的 `POST_STATUS_CONFLICT` 边界。
- 已确认并写入接口拆分：学生侧“我的发布接口”7 个，管理侧“审核接口”3 个，并补充基础错误码与 Qt API Client 分层约束。
- 已确认并写入权限边界：未认证但已登录用户不得创建或保存服务端 `PartnerPost` 草稿。
- 已确认并写入异常分支：公共字段或场景字段校验失败统一返回 `VALIDATION_FAILED`，通过 `details` 提供字段路径到错误信息的映射。
- 已确认并写入后续实现批次：Batch 1 后端学生侧我的发布、Batch 2 后端管理侧需求审核、Batch 3 Qt 学生侧我的发布 UI、Batch 4 Qt 管理侧审核 UI，并补充每批测试入口。
- 已确认并写入 P0 必需接口 DTO 字段：创建草稿、更新草稿、学生侧详情、我的发布列表项、管理审核详情、管理员审核请求。
- 已确认并写入最小枚举和值域约束：`sceneType`、`status`、`timeMode`、`decision`、`allowedActions`，以及标题、正文、标签、附件数量、驳回原因等限制。
- 已补充 P0-2 需求发布与审核追踪矩阵，覆盖 `IR3-IR8` 与 `US1-US5/US8` 到设计元素、接口和测试入口的映射。
- 已补充 P0-2 用例图前说明和两条顺序图前说明：发布需求并提交审核、管理员审核需求。
- P1 已开始进入评价与信用摘要模块；已确认评价为用户主动发起，不依赖关闭会话自动生成任务。
- 已写入信用摘要初版计算口径：无显式评价的有效对话默认 4 星，普通显式评价最高 5 星，联系方式解锁后的显式评价最高 6 星，新账号初始视为 3 次 3 星对话 + 3 次 4 星对话，用户信用卡片展示平均星数和总对话次数。
- 已确认有效对话定义：同一 `Conversation` 中双方各自至少发送 2 条用户消息才计入有效对话；信用卡片总对话次数只统计真实有效对话，不包含初始虚拟评分基线。
- 已确认评价字段：星级必填、预设标签可选、不开放自由短评；信用卡片按 `标签名*次数` 展示高频标签，例如 `配合度高*3`。
- 已确认评价修改规则：同一评价提交后 24 小时内允许修改一次，修改后按最新有效评价重算平均星数和标签计数。
- 已确认评价争议挂接：评价申诉进入投诉申诉案件，`Review.status` 支持 `ACTIVE`、`DISPUTED`、`INVALIDATED`，管理员裁定后重算信用摘要。
- 已确认评价与信用摘要接口拆分：评价操作接口 `POST/PUT/GET /api/me/reviews...`，信用摘要查询接口 `GET /api/users/{userId}/credit-summary` 和 `GET /api/me/credit-summary`。
- 已确认评价与信用摘要后续实现批次：Batch 1 后端核心，Batch 2 Qt 信用卡片与评价入口，Batch 3 评价争议挂接到投诉申诉。
- 已补充 P1-1 评价与信用摘要追踪矩阵，覆盖 `IR12-IR16` 与 `US6/US9/US10` 到设计元素、接口和测试入口的映射。
- P1-2 已开始进入投诉申诉与案件模块；已确认投诉、申诉、举报和处理结果争议统一由 `CaseTicket` 承载，`targetType` 支持 `REVIEW`、`CONVERSATION`、`PARTNER_POST`、`USER`、`ADMIN_DECISION`。
- 已确认案件状态机采用三态：`SUBMITTED`、`WAITING_FOR_UPDATE`、`CLOSED`；补证据和对方回应通过 `waitingForRole/updateReason` 区分。
- 已确认案件创建权限边界，并补充恶意举报治理挂接：捏造证据、批量针对同一用户举报、反复提交无事实依据案件等可被标记为治理线索，后续由管理端按情节处罚。
- 已确认证据附件规则：支持 jpg/jpeg/png/pdf，单文件不超过 10 MB，每个案件最多 10 个证据附件，后端中转上传并做权限校验。
- 已确认回应规则：被投诉方或相关方可回应但不强制，`WAITING_FOR_UPDATE` 后提供 72 小时窗口，超时后管理员可按现有材料裁定，未回应本身不等于违规。
- 已确认案件裁定结果：`REJECT_CASE`、`KEEP_ORIGINAL`、`INVALIDATE_REVIEW`、`HIDE_REVIEW_TAGS`、`REMOVE_POST`、`CLOSE_CONVERSATION`、`MARK_MALICIOUS`、`ESCALATE_GOVERNANCE`；具体账号处罚和审计留给管理端治理模块。
- 已确认投诉申诉与案件接口拆分：用户侧案件接口负责创建、补证据、回应和查看进度；管理侧案件接口负责队列、详情、要求补充和裁定结案。
- 已确认投诉申诉与案件后续实现批次：Batch 1 后端用户侧案件创建/证据/回应/我的案件，Batch 2 后端管理侧案件队列/要求补充/裁定结案，Batch 3 Qt 用户侧案件页 + Qt 管理端案件队列。
- 已补充 P1-2 投诉申诉与案件追踪矩阵，覆盖 `IR11/IR14/IR15/IR16` 与 `US10/US11` 到设计元素、接口和测试入口的映射。
- P1-3 已开始进入管理端治理模块；已确认账号处置、信用干预、恶意举报处罚等治理动作统一由 `GovernanceAction` 承载，动作类型包括 `WARNING`、`MUTE`、`SUSPEND_ACCOUNT`、`RESTORE_ACCOUNT`、`ADJUST_CREDIT`、`LIMIT_REPORTING`。
- 已确认治理动作功能限制边界：`WARNING` 不限制功能，`MUTE` 限制表达类能力，`SUSPEND_ACCOUNT` 阻断关键互动但保留登录和申诉入口，`LIMIT_REPORTING` 只限制举报/申诉，`ADJUST_CREDIT` 只影响信用摘要。
- 已确认管理员角色采用 `ADMIN / SUPER_ADMIN` 两级：普通管理员处理日常审核、案件和轻度治理，超级管理员执行封禁、恢复账号、信用干预、撤销治理动作和管理员账号管理。
- 已确认信用干预边界：`ADJUST_CREDIT` 仅允许结构化调整，不允许手填最终平均星数；单次人工分值调整建议限制在 `-0.5` 到 `+0.5`。
- 已确认恶意举报处罚梯度：第一次轻微恶意举报执行 `WARNING`；重复恶意举报或明显捏造执行 `LIMIT_REPORTING` 7 天；批量组织恶意举报或伪造证据执行 `MUTE` 7 天 + `LIMIT_REPORTING` 30 天；严重伪造证据、持续攻击他人或规避限制执行 `SUSPEND_ACCOUNT`，由 `SUPER_ADMIN` 执行。
- 已确认管理端审计日志统一由 `AdminAuditLog` 承载，字段包括操作者、角色、动作、目标、来源、原因摘要、前后快照引用、`traceId`、来源 IP 和时间；审计日志记录引用与摘要，不复制敏感原文；关键管理写操作若审计失败，应回滚业务操作。
- 已确认 P1-3 管理端治理接口按“治理动作接口 + 审计查询接口”拆分，不设计万能管理接口；治理动作接口包括创建、撤销、列表和详情，审计查询接口包括审计列表和详情。
- 已确认 P1-3 后续实现批次拆为 3 批：Batch 1 后端治理动作创建/撤销/查询和权限校验；Batch 2 后端审计日志自动写入、查询和事务一致性；Batch 3 Qt 管理端治理页面与审计日志查看页面。
- 已确认 P1-3 管理端治理 V1 核心用例限定为 6 个：查看用户治理档案、创建治理动作、撤销治理动作、查看审计日志、从案件升级治理、查看受限用户申诉入口。
- 已确认 P1-3 图类说明优先覆盖两条高风险主流程：从案件升级治理、撤销治理动作；已补充活动流程、失败分支和顺序图参与者/主干说明。
- 已确认 P1-3 治理关键对象收束为 4 个：`GovernanceAction`、`AdminAuditLog`、`RestrictionPolicy`、`CreditAdjustmentRecord`；其中 `RestrictionPolicy` 可作为从生效治理动作计算出的权限策略视图，不强制独立建表。
- 已确认信用摘要变化只允许由 `ADJUST_CREDIT` 下的结构化 `CreditAdjustmentRecord` 触发；`WARNING`、`MUTE`、`SUSPEND_ACCOUNT`、`RESTORE_ACCOUNT`、`LIMIT_REPORTING` 本身不直接改变信用分或标签计数。
- 已补充 P1-3 管理端治理追踪矩阵，覆盖 `IR16`、`US10`、`US11` 到 `GovernanceAction`、`AdminAuditLog`、`RestrictionPolicy`、`CreditAdjustmentRecord`、治理动作接口、审计查询接口和 Batch 1/2/3 测试入口的映射。
- P1-3 管理端治理模块已阶段性收束；下一步建议进入 P1 通知与留痕模块。
- P1-4 通知与留痕模块已开始；已确认 V1 只做站内通知和系统事件留痕，不做实时推送、短信、邮件、移动端 Push 或 WebSocket；核心对象为 `Notification` 与 `SystemEventLog`。
- 已确认 P1-4 通知触发点限定为 8 类：认证审核结果、需求审核结果、收到新会话/邀约、评价提醒、评价争议处理结果、案件状态变化、治理动作生效/撤销、信用摘要人工调整；不做点赞、浏览、推荐曝光、营销公告和批量广播。
- 已确认 `Notification` 只保留 `UNREAD` 与 `READ` 两态，不做删除、归档、撤回和过期；字段包括接收人、类型、标题、内容、来源对象、跳转目标、状态、创建时间和已读时间。
- 已确认 `SystemEventLog` 只做业务事件留痕和通知派生追踪，字段包括事件类型、来源对象、触发用户、受影响用户、派生通知、摘要、时间和 `traceId`；不保存认证材料、聊天原文、证据原文、联系方式等敏感正文。
- 已确认 P1-4 接口拆分：用户侧提供 `GET /api/me/notifications`、`GET /api/me/notifications/unread-count`、`POST /api/me/notifications/{notificationId}/read`、`POST /api/me/notifications/read-all`；管理侧只提供 `GET /api/admin/system-events` 和 `GET /api/admin/system-events/{eventId}`；不提供人工发通知或群发接口。
- 已确认 P1-4 实现批次拆为 3 批：Batch 1 后端通知核心；Batch 2 后端系统事件与触发挂接；Batch 3 Qt 通知中心与管理端系统事件查询页面。
- 用户补充确认：后续实现希望不局限于 Win11 本地环境，应进入 Ubuntu 24 服务器实战部署验证，包括数据库、对象存储、后端服务、Nginx/公网 IP 等；真实凭据不得进入仓库、文档或 Qt 客户端。
- 已确认后续涉及后端、数据库迁移、对象存储、部署配置或公网访问的实现批次，完成门禁为“本地测试通过 + Ubuntu 24 服务器 smoke test 通过”；服务器 smoke test 至少记录部署/重启、健康检查、关键接口、数据库、对象存储和私有配置就绪情况。
- 已补充 P1-4 通知与留痕追踪矩阵，覆盖 8 类通知触发点、`Notification`、`SystemEventLog`、用户侧通知接口、管理侧系统事件接口和 Batch 1/2/3 测试入口。
- P1-1、P1-2、P1-3、P1-4 四个 P1 支撑模块均已阶段性收束。
- 已开始补齐第 5-9 章全局设计视图；第 5 章已补充总体用例图边界、参与者、模块用例拆分、跨图关系和需求编号映射口径。
- 第 6 章已按账号、发布、联系、评价、案件、治理、通知、附件 8 个对象簇补齐核心领域对象与类图前说明，包括对象职责、关系、聚合边界、类图拆分建议和扩展点。
- 第 7 章已收束 8 个高风险状态对象：`User.authenticationStatus`、`PartnerPost.status`、`Conversation.status`、`ContactUnlockRecord` 解锁状态、`Review.status`、`CaseTicket.status`、`GovernanceAction` 生效/撤销视图、`Notification.status`；已补齐状态集合、合法流转和一致性约束。
- 第 8 章已按 6 条主流程补齐活动图前说明：认证闭环、需求发布审核、广场联系解锁、评价信用、投诉申诉案件、治理通知闭环。
- 第 9 章已按 8 条关键交互补齐顺序图前说明：提交认证资料、审核认证、发布并提交审核、审核需求、发起联系并解锁、提交评价并重算信用、创建案件并裁定、治理动作并通知。
- 已新增 `docs/23_server_smoke_test_readiness_checklist_v1.md`，作为后续 Ubuntu 24 服务器实战部署准备和 smoke test 留档清单，覆盖环境基线、私有配置、数据库、对象存储、Nginx/公网 IP、每批 smoke test 步骤和验证记录模板。
- 用户最新要求：下一步需要设计提供给 CodeArts 的 P1 开发 prompt，并准备好工作交接。下一线程应先做 prompt 设计与归档，不直接写业务代码。
- 用户补充要求：下一线程将临时作为 CodeArts prompt 设计与复核线程。用户每次对话会返回 CodeArts 的报告，该线程需要检查 CodeArts 完成情况、Git 变更、测试与留档，并针对性给出下一步提示词。
- 推荐首轮 CodeArts P1 开发 prompt 从 `P1-1 评价与信用摘要 Batch 1 后端核心` 开始；如果用户在下一线程指定其他 P1 模块，应以用户最新指令为准。

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

## 当前线程完成了什么

- CampusApiClient：新增 `uploadMultipart` 方法，支持 multipart/form-data + Bearer 认证头
- AuthApiService：新增 `uploadIdentityMaterial`（文件上传）、`submitIdentityVerification`（含 college/major/grade/materialAttachmentId）、扩展 `getIdentityVerificationStatus`（读取 reviewStatus/rejectReason/allowedActions）
- AuthResult：新增 attachmentId/reviewStatus/rejectReason/allowedActions 字段
- IdentityVerificationWidget：完整认证资料提交表单 + 文件选择/上传 + 状态感知
- HomePageWidget：嵌入 IdentityVerificationWidget，查询状态后传递给认证组件
- Qt 3/3 测试通过，后端 56/56 不变

## 关键结论

- Qt 客户端 P0 认证全链路闭环：注册→登录→查询状态→上传材料→提交认证→查看审核结果
- 认证资料提交包含完整字段：realName/studentNumber/college/major/grade/materialAttachmentId
- 附件上传采用后端中转，不直传 OBS

## 本线程没有做什么

- 没有实现 Windows Credential Manager 适配器
- 没有实现管理员审核 UI
- 没有适配真实 OBS SDK
- 没有替换 no-op 邮件发送
- 没有开始 P1 需求发布与审核模块

## 当前线程完成了什么

- 辅助用户评估 CodeArts 每轮输出、检查 Git 提交、发现并纠正实现偏差。
- 形成并归档多轮 CodeArts prompt。
- 推动 P0 认证后端链路完成：注册、登录、JWT、认证资料提交、材料附件上传、管理员审核。
- 推动 Qt 学生端 P0 认证链路完成：登录、注册、认证资料提交 UI、材料上传、状态回显。
- 建立后续 CodeArts 使用规则：短批次、测试先行、提交门禁、禁止 `git add .` / `git add -A`。

## 关键结论

- P0 认证学生端演示链路已经可作为后续主业务开发的地基。
- P0-2 需求发布与审核模块详细设计已经形成阶段性闭环，可支撑后续测试先行实现拆分。
- P0 主链路详细设计已具备阶段性基线，后续可按软件工程过程模型进入 P1 支撑能力文档工作。
- P1 推进顺序已确认为“评价与信用摘要 → 投诉申诉与案件 → 管理端治理 → 通知与留痕”。评价与信用摘要先行，因为它承接低压力联系结束后的评价入口，并为投诉申诉、管理端治理和通知留痕提供上游对象。

## 本线程没有做什么

- 没有实现 Windows Credential Manager 适配器。
- 没有实现管理员审核 UI。
- 没有适配真实 OBS SDK。
- 没有替换 no-op 邮件发送。
- 没有开始 P0-2 需求发布与审核模块编码。
- 没有运行 Testcontainers/Docker 测试。

## 下一步候选事项

1. `CodeArts P1 开发 prompt 设计与复核` — 建议新开 prompt 设计线程，优先级最高；每轮先复核用户带回的 CodeArts 报告、Git 变更、测试和留档，再产出下一轮可复制提示词并归档到 `docs/prompts/codearts/`。
2. `提交当前纯文档留档` — CodeArts P1 prompt 设计交接确认后处理，优先级高。
3. `服务器基线复核` — 建议新开实现准备线程，按 `docs/23_server_smoke_test_readiness_checklist_v1.md` 逐项确认 Ubuntu 24、SSH、安全组、Nginx、公网 IP、PostgreSQL、OBS 和私有配置。
4. `P1 投诉申诉与案件模块小修` — 仅限发现字段、编号或表述小问题时处理。
5. `P1 评价与信用摘要模块小修` — 仅限发现字段、编号或表述小问题时处理。
6. `P1 管理端治理模块小修` — 仅限发现字段、编号或表述小问题时处理。
7. `Windows Credential Manager 适配器` — 后续实现线程，优先级中高。
8. `真实 OBS SDK 适配器` — 后续实现线程，优先级中，需要凭证私有配置方案就位。

## 建议归档与下一线程

- 建议归档当前线程：是。
- 下一线程名称：`CodeArts P1 开发 prompt 设计与复核`
- 下一线程启动 prompt：

```text
请先阅读：
- D:\big_homework\AGENTS.md
- D:\big_homework\docs\03_current_plan.md
- D:\big_homework\handoff\latest.md
- D:\big_homework\docs\13_detailed_design_v1.md
- D:\big_homework\docs\06_execution_preparation_v1.md
- D:\big_homework\docs\09_codearts_requirement_tables_v1.md
- D:\big_homework\docs\12_code_generation_constraints_v1.md
- D:\big_homework\docs\21_codearts_prompt_review_workflow_v1.md
- D:\big_homework\docs\22_codearts_unattended_prompt_engineering_v1.md
- D:\big_homework\docs\23_server_smoke_test_readiness_checklist_v1.md

当前任务：临时作为 CodeArts P1 开发 prompt 设计与复核线程运行。现在不要直接写业务代码，不调用 CodeArts，不改后端实现。用户每轮会带回 CodeArts 的报告或提交信息；请先检查 CodeArts 完成情况、Git 变更范围、测试结果、验证留档和敏感信息风险，再针对性设计下一轮可直接复制给 CodeArts 的提示词，并将 prompt 归档到 `docs/prompts/codearts/`。

推荐首轮 CodeArts P1 开发目标：`P1-1 评价与信用摘要 Batch 1 后端核心`。要求 prompt 必须遵守测试先行、红灯确认、最小实现、绿灯确认、必要回归、`docs/validation/` 留档、本地测试 + 服务器 smoke test 门禁、敏感信息禁止项和 Git 提交边界。若发现 P1-1 依赖尚未实现的 P0 会话/联系对象导致无法直接实现，应在 prompt 中明确前置依赖和停止条件，不允许 CodeArts 自行扩大到整个 P1 或补完整系统。
```
