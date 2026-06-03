# 校园搭子平台详细设计文档

文档日期：2026-05-30  
适用版本：课程作业正式提交版  
交付形态：本文为正式设计正文；提交 PDF 应使用同内容 LaTeX 源文件生成，子行采用编号列表，不使用前置黑点列表。

## 1. 引言

### 1.1 项目背景

校园搭子平台面向校内学生的结伴需求，覆盖饭搭子、学习搭子、运动搭子、课程组队和创新项目组队等场景。项目目标不是做一个普通聊天工具，而是提供“发布需求、审核、广场筛选、低压力联系、联系方式解锁、评价与信用摘要”的完整闭环，降低陌生同学之间建立合作或同行关系的成本。

### 1.2 建设目标

1. 支持学生通过校园邮箱注册登录，并通过身份认证进入核心业务。
2. 支持已认证学生发布结构化搭子需求，经管理员审核后进入广场。
3. 支持其他已认证学生浏览广场、查看发布者信用摘要并发起联系。
4. 支持会话中的低压力沟通、首条消息节制、未读状态、关闭会话和重新发起。
5. 支持联系方式卡片保存、双方确认后解锁联系方式。
6. 支持有效会话后的互评与信用摘要展示。
7. 支持管理员审核身份认证和帖子内容，保障平台基本秩序。
8. 支持后端服务器、数据库、对象存储、SMTP 邮件发送和 Windows 桌面端内测包的可运行交付。

### 1.3 适用范围

本文描述当前课程提交版已经实现或已经形成稳定接口的系统设计。Qt 桌面端是当前主要客户端；移动端目前具备 API 文档和接入边界，但正式移动客户端不属于当前实现范围。投诉申诉、通知治理、HTTPS 域名、刷新 token 接口等能力属于后续生产化改进。

### 1.4 术语说明

| 术语 | 含义 |
|---|---|
| 搭子 | 在某个校园场景中临时或阶段性结伴的人，例如一起吃饭、学习、运动或完成课程项目 |
| 广场 | 展示已审核通过搭子需求的列表页 |
| 低压力联系 | 不强制一开始交换手机号或微信，先通过站内会话沟通 |
| 信用摘要 | 基于评价和有效会话生成的可信度展示，也可面向用户称为靠谱分 |
| 身份认证 | 学生提交学号、学院、专业、年级和认证材料，由管理员审核 |
| OBS | 华为云对象存储，用于保存认证材料等文件 |
| SMTP | 邮件发送服务，用于发送校园邮箱验证码 |

## 2. 需求与设计目标

### 2.1 核心用户角色

| 角色 | 主要职责 |
|---|---|
| 学生用户 | 注册登录、提交身份认证、发布搭子需求、浏览广场、发起联系、交换联系方式、提交评价 |
| 管理员 | 审核身份认证、审核帖子、查看认证材料、批准或驳回申请 |

### 2.2 功能性需求概览

1. 账号与认证：校园邮箱验证码、注册、登录、JWT 登录态、身份资料提交与审核。
2. 帖子与广场：草稿、更新、提交审核、撤回、下架、管理员审核、广场列表与详情。
3. 联系会话：从帖子发起联系、消息列表、发送短文本、未读计数、标记已读、关闭会话。
4. 联系方式解锁：保存联系方式卡片、查询解锁状态、双方确认、解锁后查看对方卡片。
5. 评价信用：有效会话后评价、一次修改限制、查询我发出和收到的评价、查询信用摘要。
6. 管理后台：身份认证队列、认证材料下载、认证审核、帖子审核队列、帖子详情、帖子审核裁定。

### 2.3 非功能性需求

1. 安全性：核心接口使用 JWT；管理员接口要求 ADMIN 角色；广场、发帖、联系、评价要求身份认证通过。
2. 可维护性：后端采用 Spring Boot 分模块组织；数据库结构由 Flyway 迁移管理；客户端通过 API service 封装后端合同。
3. 可部署性：后端支持 deploy profile，服务器由 systemd 托管，Nginx 对外暴露 HTTP API。
4. 可演示性：Windows 桌面端已打包为内测 zip，后端公网 health 和关键 smoke 已形成验证记录。
5. 密钥隔离：数据库密码、JWT secret、OBS AK/SK、SMTP 授权码只放服务器或项目外私有环境变量，不写入仓库、文档和客户端。

## 3. 系统总体架构

### 3.1 整体架构说明

系统采用客户端到后端 API 的集中式架构。Qt 桌面端和未来移动端只访问 HTTP API，不直连数据库、不持有 OBS 或数据库密钥。后端负责鉴权、业务规则、数据持久化、文件上传和邮件发送。

| 层次 | 技术与职责 |
|---|---|
| 客户端层 | Qt Widgets 桌面端；未来移动端可复用同一 REST API |
| API 层 | Spring Boot Controller，统一接收 JSON 或 multipart 请求 |
| 业务层 | Auth、Post、Contact、Review、Storage 等 Service |
| 数据层 | PostgreSQL/H2 配合 JPA Repository；Flyway 管理迁移 |
| 文件层 | OBS 对象存储保存认证材料；测试环境可使用内存实现 |
| 外部服务 | SMTP 发送校园邮箱验证码；Nginx 提供公网入口 |

### 3.2 部署关系

| 组件 | 当前实现 |
|---|---|
| Qt 桌面端 | Windows 内测包，默认连接 `http://114.116.203.78/api` |
| 后端服务 | Ubuntu 24.04.4 ECS，systemd 服务 `campus-buddy-backend` |
| 反向代理 | Nginx，将公网 HTTP 请求转发到本机后端 |
| 数据库 | PostgreSQL 17.9，服务器本地 `127.0.0.1:5432` |
| 对象存储 | 华为云 OBS 私有桶 |
| 邮件服务 | SMTP 配置在服务器私有环境变量中 |

### 3.3 主流程概览

1. 学生输入校园邮箱并请求验证码。
2. 学生校验验证码后完成注册和登录，获得 Bearer token。
3. 学生提交身份认证材料，管理员审核通过后成为 VERIFIED。
4. 已认证学生创建帖子草稿，补齐字段后提交审核。
5. 管理员审核通过帖子，帖子进入广场。
6. 其他已认证学生浏览广场并发起联系。
7. 双方通过会话沟通，保存联系方式卡片并双向确认解锁。
8. 双方达到有效会话条件后提交评价，系统更新信用摘要。

## 4. 功能模块设计

### 4.1 注册登录与校园邮箱验证码

| 设计项 | 内容 |
|---|---|
| 模块职责 | 校园邮箱验证码发送、验证码校验、注册、登录、JWT 颁发 |
| 关键接口 | `/auth/campus-email/verification-codes`、`/auth/campus-email/verifications`、`/auth/register`、`/auth/login` |
| 主要数据 | user_account、verification code、verification ticket、refresh token |
| 关键规则 | 当前服务器只允许 `bjtu.edu.cn` 邮箱；注册需要已验证 ticket；登录返回 accessToken 和 refreshToken 字段 |
| 异常控制 | 邮箱域名非法、验证码错误或过期、发送过于频繁、账号已注册、密码错误 |

登录成功后客户端保存 accessToken，并在后续请求中通过 `Authorization: Bearer <accessToken>` 传递。当前实现中登录响应包含 refreshToken 字段，但尚未提供刷新 token 接口，因此客户端遇到 401 时应引导重新登录。

### 4.2 校园身份认证与管理员审核

| 设计项 | 内容 |
|---|---|
| 模块职责 | 学生上传认证材料、提交实名/学号资料；管理员审核通过或驳回 |
| 关键接口 | `/auth/identity-verifications/materials`、`/auth/identity-verifications`、`/auth/identity-verifications/me`、`/admin/identity-verifications` |
| 主要数据 | identity_material_attachment、identity_verification_submission、user_account.authentication_status |
| 状态流转 | UNVERIFIED -> PENDING_REVIEW -> VERIFIED 或 REJECTED |
| 关键规则 | PENDING_REVIEW 不允许重复提交；VERIFIED 不允许重复认证；认证材料归属必须属于当前用户 |

认证材料通过 multipart 上传。后端保存文件元数据和对象存储 key，管理员审核时可下载材料。正式文档和代码仓只记录配置项名称，不记录任何真实 OBS 密钥。

### 4.3 搭子帖子发布、草稿、审核与广场

| 设计项 | 内容 |
|---|---|
| 模块职责 | 学生管理自己的帖子；管理员审核；已发布帖子进入广场 |
| 关键接口 | `/me/partner-posts`、`/me/partner-posts/{postId}/submit-review`、`/admin/partner-posts/review-queue`、`/partner-posts` |
| 主要数据 | partner_post、scenePayload、allowedActions |
| 状态流转 | DRAFT -> PENDING_REVIEW -> PUBLISHED 或 REJECTED；PUBLISHED 可下架 |
| 关键规则 | 提交审核时按场景校验字段；contactPreference 不能包含 11 位手机号；只有 VERIFIED 用户可发帖和看广场 |

五类场景使用统一帖子主表和 JSON 场景载荷。场景字段示例为：学习搭子要求 `studyGoal`，饭搭子要求 `canteen`，运动搭子要求 `sportType`，课程组队要求 `courseName`，创新项目要求 `projectDirection`。

### 4.4 低压力站内联系与会话

| 设计项 | 内容 |
|---|---|
| 模块职责 | 从广场帖子发起联系，创建或复用会话，发送消息，查询未读数，关闭会话 |
| 关键接口 | `/partner-posts/{postId}/contact-requests`、`/me/conversations`、`/me/conversations/{conversationId}/messages` |
| 主要数据 | conversation、conversation_message、last_read_message_id |
| 状态流转 | ACTIVE 可发送消息；CLOSED 禁止继续发送；重新发起联系可恢复 ACTIVE |
| 关键规则 | 不能联系自己的帖子；帖子必须 PUBLISHED；首条消息后必须等待对方回复才能继续发送 |

首条消息限制由后端返回 `CONTACT_REPLY_REQUIRED` 强制执行。客户端收到该错误后禁用发送入口并提示等待对方回复，避免单方连续轰炸式消息。

### 4.5 联系方式卡片与双方确认解锁

| 设计项 | 内容 |
|---|---|
| 模块职责 | 保存个人联系方式；查询会话解锁状态；双方确认后查看对方卡片 |
| 关键接口 | `/me/contact-card`、`/me/conversations/{conversationId}/contact-unlock`、`/peer-contact-card` |
| 主要数据 | contact_card、contact_unlock_confirm、contact_unlock_record |
| 状态流转 | LOCKED -> WAITING_FOR_PEER -> UNLOCKED |
| 关键规则 | 双方都保存卡片并确认后才解锁；未解锁前不能查看对方联系方式 |

该设计将联系方式交换从帖子正文中剥离出来，降低公开泄露手机号、微信号等私人信息的风险。

### 4.6 评价与信用摘要

| 设计项 | 内容 |
|---|---|
| 模块职责 | 对有效会话中的另一方提交评价，查询评价列表和信用摘要 |
| 关键接口 | `/me/reviews`、`/me/reviews/given`、`/me/reviews/received`、`/me/credit-summary`、`/users/{userId}/credit-summary` |
| 主要数据 | review、rating、reviewTags、credit summary 统计结果 |
| 关键规则 | 双方各至少 2 条 USER_TEXT 消息才可评价；同一关系只评价一次；24 小时内只可修改一次 |
| 分值规则 | 普通会话 rating 为 1 到 5；联系方式已解锁后允许 6 星评价 |

信用摘要在广场列表和帖子详情中展示给其他学生，帮助其判断沟通对象的可信度。摘要包含平均评分、有效会话数、评分样本数和高频标签。

### 4.7 管理员审核后台

| 设计项 | 内容 |
|---|---|
| 模块职责 | 统一承接身份认证审核和帖子审核 |
| 关键接口 | `/admin/identity-verifications`、`/admin/identity-verifications/{submissionId}/reviews`、`/admin/partner-posts/review-queue`、`/admin/partner-posts/{postId}/review` |
| 权限要求 | 登录且 accountRole 为 ADMIN |
| 关键规则 | 管理员通过认证会同步用户 authenticationStatus；帖子通过后进入广场，驳回后返回修改 |

当前管理员能力聚焦课程演示和核心闭环，没有实现完整用户封禁、投诉申诉处理和后台统计看板。

### 4.8 桌面端 UI 组织

| 页面 | 职责 |
|---|---|
| 登录/注册页 | 邮箱验证码、注册、登录 |
| 首页 | 根据角色展示学生功能或管理员功能 |
| 认证资料页 | 上传认证材料并提交身份资料 |
| 我的发布页 | 查看自己的帖子状态，提交审核、撤回、下架 |
| 帖子编辑页 | 创建/更新草稿，显示字段级错误和忙碌态 |
| 广场页 | 浏览已发布帖子、查看详情、发起联系 |
| 会话与联系方式页 | 查看会话、发送消息、标记已读、关闭会话、保存卡片、确认交换 |
| 评价信用页 | 提交评价、查看自己信用摘要和评价列表 |
| 管理员审核页 | 审核身份认证和帖子 |

## 5. 关键业务流程设计

### 5.1 新用户注册、验证码与登录

1. 用户输入 `bjtu.edu.cn` 校园邮箱。
2. 客户端调用发送验证码接口，按钮进入倒计时状态。
3. 用户输入验证码，客户端调用校验接口并获得 verificationTicket。
4. 用户填写密码和显示名，调用注册接口。
5. 用户调用登录接口获得 accessToken。
6. 客户端根据返回的 authenticationStatus 判断是否进入身份认证页。

### 5.2 身份认证审核

1. 学生上传认证材料，获得 attachmentId。
2. 学生填写真实姓名、学号、学院、专业、年级并提交认证。
3. 认证状态变为 PENDING_REVIEW。
4. 管理员在审核页查看队列和材料摘要。
5. 管理员通过后，用户 authenticationStatus 变为 VERIFIED。
6. 管理员驳回后，用户可查看原因并重新提交。

### 5.3 发帖、审核和广场发布

1. VERIFIED 学生创建帖子草稿。
2. 学生选择场景并填写公共字段与场景字段。
3. 学生提交审核，后端执行必填、长度、枚举、时间和联系方式泄露校验。
4. 管理员审核帖子。
5. 审核通过后帖子状态变为 PUBLISHED，并出现在广场。
6. 审核驳回后帖子状态变为 REJECTED，学生可编辑后再次提交。

### 5.4 浏览广场并发起联系

1. VERIFIED 学生访问广场列表。
2. 用户按场景或关键词筛选帖子。
3. 用户进入帖子详情，查看发布者认证状态和信用摘要。
4. 用户输入首条短消息并发起联系。
5. 后端创建或复用双方围绕该帖子的会话。
6. 当前用户等待对方回复；对方回复前继续发送会返回 `CONTACT_REPLY_REQUIRED`。

### 5.5 联系方式解锁

1. 双方在站内会话中确认有进一步沟通意愿。
2. 双方分别保存自己的联系方式卡片。
3. 任一方点击确认交换后，状态进入 WAITING_FOR_PEER。
4. 另一方也确认后，状态变为 UNLOCKED。
5. 双方可以查看对方联系方式卡片。

### 5.6 有效会话与评价

1. 双方在同一会话中各自至少发送 2 条 USER_TEXT。
2. 用户从会话进入评价页，使用会话中的对方 ID 作为 revieweeId。
3. 用户选择评分和固定标签并提交评价。
4. 后端校验评价资格、重复评价和评分范围。
5. 评价写入后，信用摘要统计随之更新。
6. 用户可查询自己发出和收到的评价。

## 6. 数据库设计

当前数据库由 Flyway V1 到 V11 管理。以下只列正式设计需要理解的主要表和关系，不机械复制全部字段。

| 表 | 作用 | 主键 | 关键字段与关系 |
|---|---|---|---|
| user_account | 用户账号与身份状态 | user_id | campus_email、password_hash、display_name、authentication_status、account_role |
| campus_email_verification_code | 邮箱验证码 | id | campus_email、code_hash、purpose、expires_at、consumed_at |
| campus_email_verification_ticket | 验证通过后的注册凭证 | id | campus_email、ticket、expires_at、used_at |
| refresh_token | 登录刷新 token 记录 | id | user_id、token_hash、expires_at；当前无刷新接口 |
| identity_material_attachment | 认证材料附件 | attachment_id | owner_user_id、object_key、content_type、size_bytes、sha256 |
| identity_verification_submission | 身份认证申请 | submission_id | user_id、real_name、student_number、college、major、grade、review_status、material_attachment_id |
| partner_post | 搭子帖子 | post_id | publisher_id、scene_type、status、title、description、time_mode、location_text、scene_payload、reject_reason |
| conversation | 会话 | conversation_id | initiator_id、receiver_id、related_post_id、status、updated_at |
| conversation_message | 会话消息 | message_id | conversation_id、sender_id、message_type、content、created_at |
| conversation_read_state | 已读状态 | id | conversation_id、user_id、last_read_message_id |
| contact_card | 个人联系方式卡片 | id | user_id、wechat_id、phone_number、qq_number |
| contact_unlock_confirm | 单方确认记录 | id | conversation_id、user_id、confirmed_at |
| contact_unlock_record | 双方解锁事实 | id | conversation_id、unlocked_at |
| review | 评价 | review_id | conversation_id、reviewer_id、reviewee_id、rating、review_tags、status、modified_once |

关系上，user_account 是身份、帖子、会话、联系方式和评价的根实体；partner_post 由发布者创建；conversation 绑定双方用户和可选 related_post；review 绑定 conversation 并要求 reviewer/reviewee 是会话参与者。

## 7. 接口设计

### 7.1 通用规则

| 项目 | 设计 |
|---|---|
| 当前 API 基地址 | `http://114.116.203.78/api/` |
| 鉴权方式 | `Authorization: Bearer <accessToken>` |
| 请求体 | JSON；文件上传使用 multipart/form-data |
| 分页 | `page` 从 0 开始，返回 items、page、size、totalElements、totalPages |
| 错误体 | code、message、details、traceId |

常见业务错误包括 `VALIDATION_FAILED`、`UNAUTHORIZED`、`FORBIDDEN`、`AUTHENTICATION_STATUS_REQUIRED`、`CONTACT_REPLY_REQUIRED`、`RESOURCE_NOT_FOUND`、`REVIEW_ALREADY_EXISTS` 等。

### 7.2 账号认证接口

| 方法 | 路径 | 鉴权 | 说明 |
|---|---|---|---|
| POST | `/auth/campus-email/verification-codes` | 否 | 发送校园邮箱验证码 |
| POST | `/auth/campus-email/verifications` | 否 | 校验验证码并返回 verificationTicket |
| POST | `/auth/register` | 否 | 使用 ticket 注册账号 |
| POST | `/auth/login` | 否 | 登录并返回 token 与用户摘要 |

### 7.3 身份认证接口

| 方法 | 路径 | 鉴权 | 说明 |
|---|---|---|---|
| POST | `/auth/identity-verifications/materials` | 是 | 上传认证材料 |
| DELETE | `/auth/identity-verifications/materials/{attachmentId}` | 是 | 删除未引用材料 |
| POST | `/auth/identity-verifications` | 是 | 提交身份认证 |
| GET | `/auth/identity-verifications/me` | 是 | 查询我的认证状态 |
| GET | `/admin/identity-verifications` | ADMIN | 查询认证审核队列 |
| GET | `/admin/identity-verifications/{submissionId}/material` | ADMIN | 下载认证材料 |
| POST | `/admin/identity-verifications/{submissionId}/reviews` | ADMIN | 审核认证 |

### 7.4 帖子接口

| 方法 | 路径 | 鉴权 | 说明 |
|---|---|---|---|
| POST | `/me/partner-posts` | VERIFIED | 新建草稿 |
| PUT | `/me/partner-posts/{postId}` | VERIFIED | 更新草稿或可编辑帖子 |
| GET | `/me/partner-posts` | VERIFIED | 查询我的帖子列表 |
| GET | `/me/partner-posts/{postId}` | VERIFIED | 查询我的帖子详情 |
| POST | `/me/partner-posts/{postId}/submit-review` | VERIFIED | 提交审核 |
| POST | `/me/partner-posts/{postId}/withdraw-review` | VERIFIED | 撤回审核 |
| POST | `/me/partner-posts/{postId}/unpublish` | VERIFIED | 下架已发布帖子 |
| GET | `/partner-posts` | VERIFIED | 广场列表 |
| GET | `/partner-posts/{postId}` | VERIFIED | 广场详情 |
| GET | `/admin/partner-posts/review-queue` | ADMIN | 帖子审核队列 |
| POST | `/admin/partner-posts/{postId}/review` | ADMIN | 审核帖子 |

### 7.5 会话与联系方式接口

| 方法 | 路径 | 鉴权 | 说明 |
|---|---|---|---|
| POST | `/partner-posts/{postId}/contact-requests` | VERIFIED | 从帖子发起联系 |
| GET | `/me/conversations` | VERIFIED | 查询我的会话 |
| GET | `/me/conversations/{conversationId}/messages` | VERIFIED | 查询消息 |
| POST | `/me/conversations/{conversationId}/messages` | VERIFIED | 发送消息 |
| POST | `/me/conversations/{conversationId}/read` | VERIFIED | 标记已读 |
| POST | `/me/conversations/{conversationId}/close` | VERIFIED | 关闭会话 |
| GET | `/me/contact-card` | 是 | 查询我的联系方式卡片 |
| PUT | `/me/contact-card` | 是 | 保存我的联系方式卡片 |
| GET | `/me/conversations/{conversationId}/contact-unlock` | VERIFIED | 查询解锁状态 |
| POST | `/me/conversations/{conversationId}/contact-unlock/confirm` | VERIFIED | 确认解锁 |
| GET | `/me/conversations/{conversationId}/peer-contact-card` | VERIFIED | 查看对方卡片 |

### 7.6 评价与信用接口

| 方法 | 路径 | 鉴权 | 说明 |
|---|---|---|---|
| POST | `/me/reviews` | VERIFIED | 创建评价 |
| PUT | `/me/reviews/{reviewId}` | VERIFIED | 修改评价一次 |
| GET | `/me/reviews/given` | 是 | 查询我发出的评价 |
| GET | `/me/reviews/received` | 是 | 查询我收到的评价 |
| GET | `/me/credit-summary` | 是 | 查询我的信用摘要 |
| GET | `/users/{userId}/credit-summary` | 是 | 查询其他用户信用摘要 |

## 8. 权限与安全设计

### 8.1 登录态与角色

1. 用户登录后获得 JWT accessToken。
2. 客户端在后续请求中带 Bearer Token。
3. 后端从 token 中识别用户和 accountRole。
4. STUDENT 只能访问学生业务接口。
5. ADMIN 才能访问 `/admin/**` 接口。

### 8.2 身份认证状态门禁

1. 未登录用户只能访问健康检查、验证码、注册和登录。
2. 已登录但未 VERIFIED 的用户不能访问广场、发帖、联系和评价等核心业务。
3. 未认证用户访问广场列表或详情时返回 `AUTHENTICATION_STATUS_REQUIRED`。
4. 管理员审核通过身份认证后，学生才可进入主业务闭环。

### 8.3 密钥与配置边界

1. 数据库密码、JWT secret、OBS AK/SK、SMTP 授权码只存放在服务器私有 `/etc/campus-buddy/backend.env` 或项目外私有 env。
2. systemd 启动脚本通过环境变量加载配置，不把 secret 放入 Java 命令行。
3. 仓库、Markdown 文档、Qt 客户端和内测 zip 不包含真实密钥。
4. 客户端只持有公开 API base URL，不直连数据库、不直接访问 OBS。

### 8.4 当前安全限制

当前公网 API 仍为 HTTP，适合课程演示和内测，不适合承载真实生产数据。正式生产应接入域名、HTTPS、证书自动续期、密钥轮换、日志脱敏、备份恢复和更细权限审计。

## 9. 前端设计

### 9.1 Qt 桌面端结构

Qt 桌面端采用页面组件和 API service 分层。页面负责交互状态、表单校验提示和用户反馈；API service 负责构造请求、解析响应、处理业务错误；token store 负责保存当前登录态。

### 9.2 主要体验设计

1. 登录注册页按“发送验证码、校验验证码、注册、登录”的自然顺序组织。
2. 首页根据 accountRole 动态展示学生功能或管理员功能，避免普通用户看到后台入口。
3. 发帖页在提交审核失败时展示字段级错误，不清空用户已填内容。
4. 广场页展示发布者认证状态和信用摘要，帮助用户判断是否联系。
5. 会话页展示未读数、关闭状态、等待对方回复状态和联系方式解锁状态。
6. 评价页提示 conversationId 和 revieweeId 的来源，减少人工填写错误。
7. 关键按钮在请求中进入忙碌态，避免重复提交。
8. 空列表、加载中、错误信息使用统一辅助样式展示。

## 10. 部署与运行设计

### 10.1 本地开发环境

后端使用 Maven/Spring Boot；数据库可在本地使用 H2 或测试 profile。Qt 使用本机 Qt 6.10.3 MinGW、CMake 和 Ninja 构建。开发环境用于快速测试，最终部署验证以 Ubuntu 服务器和公网 API 为准。

### 10.2 服务器部署

| 项目 | 设计 |
|---|---|
| 操作系统 | Ubuntu 24.04.4 LTS |
| 运行目录 | `/srv/campus-buddy` |
| 服务名 | `campus-buddy-backend` |
| 配置文件 | `/etc/campus-buddy/backend.env` |
| 公网入口 | `http://114.116.203.78/api` |
| 健康检查 | `GET /api/health` 返回 `{"status":"UP"}` |

后端 jar 由 systemd 管理，服务 active 且 enabled。Nginx 负责把公网 HTTP 请求转发到后端。PostgreSQL 仅绑定本机地址，避免客户端或公网直接访问数据库。

### 10.3 对象存储与邮件

1. OBS 用于保存认证材料等文件，后端通过服务器私有 AK/SK 访问。
2. OBS smoke 覆盖 PUT、GET、SHA-256 校验、DELETE 和删除后 404。
3. SMTP 用于发送校园邮箱验证码，服务器只记录配置变量名和存在性。
4. 真实验证码、SMTP 授权码和邮件账号不得写入提交材料。

### 10.4 Windows 内测包

桌面端内测 zip 位于 `deliverables/internal_beta/`，解压后运行 `campus_buddy_desktop.exe`。内测包默认 API 地址为公网后端，也支持通过环境变量或启动参数覆盖 API base URL。

## 11. 测试与验收设计

### 11.1 自动化测试

| 类型 | 覆盖内容 |
|---|---|
| 后端单元/集成测试 | 注册登录、身份认证、帖子、广场、会话、联系方式、评价、管理员审核等 |
| Qt 自动化测试 | API client 合同、页面行为、按钮状态、字段级错误、角色隔离 |
| 数据库迁移验证 | Flyway V1 到 V11 在服务器环境可执行 |

当前项目记录中，后端测试、Qt ctest、desktop smoke 和 server smoke 多次通过；正式文档只保留验证类型和关键结论，不把历史轮次流水账放入正文。

### 11.2 Smoke 与人工验收

1. 后端 health：公网 `/api/health` 返回 UP。
2. 服务器 smoke：覆盖登录、认证、广场、联系、会话、联系方式解锁、评价和管理员审核。
3. 桌面端 smoke：内测包在剥离本机 Qt/CMake/MinGW PATH 后仍能启动。
4. 邮箱 smoke：真实 SMTP 发信、收信、验证码校验、注册和登录已形成验证记录。
5. 打包验证：zip 内容包含 exe、Qt DLL、MinGW runtime、platforms/qwindows.dll 和必要 TLS 后端。

### 11.3 关键负向测试

1. 未认证用户访问广场返回 `AUTHENTICATION_STATUS_REQUIRED`。
2. 帖子提交审核缺字段返回 `VALIDATION_FAILED` 和字段级 details。
3. 发起联系后对方未回复时继续发送返回 `CONTACT_REPLY_REQUIRED`。
4. CLOSED 会话禁止继续发送消息。
5. 非管理员访问管理员接口返回权限错误。
6. 重复评价返回冲突错误。
7. 未解锁联系方式时不能查看对方联系方式卡片。

## 12. 当前限制与后续改进

1. 当前公网仍为 HTTP，正式生产应接入 HTTPS、域名和证书自动续期。
2. 投诉申诉、通知治理、用户封禁、信用人工干预等完整治理模块尚未实现。
3. 移动端目前只有 API 支持和接入文档，正式 Android/iOS 客户端需单独开发。
4. 登录响应中存在 refreshToken 字段，但当前后端尚未提供刷新 token 接口。
5. 当前会话不是 WebSocket 实时聊天，主要依赖查询和刷新。
6. 内测包适合课程演示，不等同完整商业生产系统。
7. 服务器备份、监控告警、日志审计、密钥轮换和高并发压测仍需补齐。

## 13. 总结

校园搭子平台当前实现已经形成从校园邮箱注册、身份认证、发帖、管理员审核、广场展示、低压力联系、联系方式解锁、互评信用到服务器部署和桌面端打包的最小闭环。设计重点放在校内身份可信、联系方式保护、先审后发、评价可追溯和密钥隔离上。当前版本足以支撑课程答辩和内测演示；若进入真实生产，需要继续补齐 HTTPS、治理、通知、移动端、刷新 token、监控备份和安全审计等生产化能力。

