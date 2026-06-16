# 后端接口完整测试用例

更新时间：2026-06-11  
适用环境：已部署云端后端，HTTP base URL 形如 `http://<host>/api`。  
安全约束：账号、密码、验证码、token、数据库密码、SMTP/OBS 密钥只通过私有环境变量传入，不写入仓库文档。

## 1. 测试账号与变量

建议在本机 PowerShell 中设置以下环境变量：

```powershell
$env:CAMPUS_BUDDY_API_BASE_URL="http://114.116.203.78/api"
$env:CAMPUS_BUDDY_SMOKE_EMAIL="<学生账号邮箱>"
$env:CAMPUS_BUDDY_SMOKE_PASSWORD="<学生账号密码>"
$env:CAMPUS_BUDDY_SMOKE_ADMIN_EMAIL="<管理员账号邮箱>"
$env:CAMPUS_BUDDY_SMOKE_ADMIN_PASSWORD="<管理员账号密码>"
```

执行自动化 smoke：

```powershell
.\scripts\server_api_full_smoke.ps1
```

脚本覆盖主链路和关键负向断言；完整注册正向链路和身份认证审核正向链路需要一次性邮箱验证码或受控待审用户，默认不自动执行破坏性审核。

## 2. 测试范围总览

| 模块 | 覆盖方式 |
| --- | --- |
| 系统健康 | 自动 |
| 登录鉴权 | 自动 |
| 邮箱验证码/注册 | 自动负向；正向需人工验证码 |
| 身份认证学生侧 | 自动查询、上传、删除未引用材料 |
| 身份认证管理员侧 | 自动查队列；下载/审核按数据条件或手工 |
| 学生发帖 | 自动创建、更新、列表、详情、提交审核、撤回 |
| 管理员帖子审核 | 自动队列、详情、通过、驳回 |
| 广场 | 自动列表、详情 |
| 联系会话 | 自动发起联系、列表、消息、增量、已读、关闭、重开 |
| 联系方式解锁 | 自动卡片 CRUD、双方确认、查看对方卡片 |
| 评价信用 | 自动创建/修改/列表/信用摘要；重复运行时创建评价可能 SKIP |
| 权限负向 | 自动未登录 401、学生访问管理员 403、字段校验 400 |

## 3. 详细测试用例

### 3.1 系统与鉴权

| ID | 接口 | 步骤 | 预期 |
| --- | --- | --- | --- |
| SYS-001 | `GET /api/health` | 无 token 请求 | 200，`status=UP` |
| SYS-002 | `GET /api/system/info` | 无 token 请求 | 200，包含 `serviceName` |
| SEC-001 | `GET /api/probe/secure` | 无 token 请求 | 401，`code=UNAUTHORIZED` |
| SEC-002 | `POST /api/auth/login` | 学生账号登录 | 200，返回 accessToken 和 user |
| SEC-003 | `POST /api/auth/login` | 管理员账号登录 | 200，`user.accountRole=ADMIN` |
| SEC-004 | `GET /api/probe/secure` | 带学生 token 请求 | 200，`authenticated=true` |
| SEC-005 | `GET /api/admin/partner-posts/review-queue` | 带学生 token 请求 | 403，`code=FORBIDDEN` |
| SEC-006 | `POST /api/auth/login` | 错误密码登录 | 401，`code=INVALID_LOGIN_CREDENTIALS` |

### 3.2 邮箱验证码与注册

| ID | 接口 | 步骤 | 预期 |
| --- | --- | --- | --- |
| AUTH-001 | `POST /api/auth/campus-email/verification-codes` | 使用非法邮箱域名 | 400，返回域名或校验错误 |
| AUTH-002 | `POST /api/auth/campus-email/verifications` | 使用错误验证码 | 400/409，返回验证码无效或过期 |
| AUTH-003 | `POST /api/auth/register` | 使用非法 ticket 或非法域名 | 400/409，不创建用户 |
| AUTH-004 | 邮箱验证码正向链路 | 用未注册邮箱发送验证码、人工读取验证码、校验、注册、登录 | 全部 200，登录返回 token |

说明：AUTH-004 需要真实可收信邮箱和验证码，不建议在通用自动 smoke 中硬编码执行。

### 3.3 身份认证

| ID | 接口 | 步骤 | 预期 |
| --- | --- | --- | --- |
| IDV-001 | `GET /api/auth/identity-verifications/me` | 学生 token 查询 | 200，返回 `authenticationStatus` |
| IDV-002 | `POST /api/auth/identity-verifications/materials` | 上传小型 PDF，字段名 `file` | 200，返回 `attachmentId` |
| IDV-003 | `DELETE /api/auth/identity-verifications/materials/{attachmentId}` | 删除刚上传且未引用材料 | 204 |
| IDV-004 | `POST /api/auth/identity-verifications` | 已认证用户再次提交 | 409 `IDENTITY_ALREADY_VERIFIED` 或按当前状态返回 |
| IDV-005 | `GET /api/admin/identity-verifications` | 管理员 token 查询待审 | 200，返回数组 |
| IDV-006 | `GET /api/admin/identity-verifications/{submissionId}/material` | 有待审且带材料时下载 | 200，返回二进制 |
| IDV-007 | `POST /api/admin/identity-verifications/{submissionId}/reviews` | 对受控待审用户审核 | 200，状态更新 |

说明：IDV-007 会改变用户认证状态，自动脚本默认不执行，除非你明确提供受控待审 `submissionId`。

### 3.4 帖子与审核

| ID | 接口 | 步骤 | 预期 |
| --- | --- | --- | --- |
| POST-001 | `POST /api/me/partner-posts` | 创建字段不完整草稿 | 200，`status=DRAFT` |
| POST-002 | `POST /api/me/partner-posts/{id}/submit-review` | 提交不完整草稿 | 400 `VALIDATION_FAILED` |
| POST-003 | `POST /api/me/partner-posts` | 创建完整草稿 | 200，返回 `postId` |
| POST-004 | `PUT /api/me/partner-posts/{id}` | 更新自己的草稿 | 200，字段更新 |
| POST-005 | `GET /api/me/partner-posts` | 查询我的帖子列表 | 200，分页结构 |
| POST-006 | `GET /api/me/partner-posts/{id}` | 查询我的帖子详情 | 200，`postId` 一致 |
| POST-007 | `POST /api/me/partner-posts/{id}/submit-review` | 提交审核 | 200，`status=PENDING_REVIEW` |
| POST-008 | `POST /api/me/partner-posts/{id}/withdraw-review` | 撤回审核 | 200，回到 `DRAFT` |
| ADMPOST-001 | `GET /api/admin/partner-posts/review-queue` | 管理员查询审核队列 | 200 |
| ADMPOST-002 | `GET /api/admin/partner-posts/{id}` | 管理员查审核详情 | 200 |
| ADMPOST-003 | `POST /api/admin/partner-posts/{id}/review` | `decision=REJECT` 且给 reason | 200，`status=REJECTED` |
| ADMPOST-004 | `POST /api/admin/partner-posts/{id}/review` | `decision=APPROVE` | 200，`status=PUBLISHED` |
| POST-009 | `POST /api/me/partner-posts/{id}/unpublish` | 对已发布帖子下架 | 200，回到 `DRAFT` |

注意：帖子审核接口使用 `APPROVE` / `REJECT`，不是身份认证审核的 `APPROVED` / `REJECTED`。

### 3.5 广场、会话、联系方式

| ID | 接口 | 步骤 | 预期 |
| --- | --- | --- | --- |
| PLAZA-001 | `GET /api/partner-posts` | 学生查询广场 | 200，分页结构 |
| PLAZA-002 | `GET /api/partner-posts/{id}` | 查询已发布帖子详情 | 200，`status=PUBLISHED` |
| CHAT-001 | `POST /api/partner-posts/{id}/contact-requests` | 另一账号发起联系 | 200，返回 `conversationId`；若重复运行可能触发 `CONTACT_REPLY_REQUIRED` |
| CHAT-002 | `GET /api/me/conversations` | 双方分别查询会话 | 200，含会话 |
| CHAT-003 | `GET /api/me/conversations/{id}/messages` | 查询消息列表 | 200 |
| CHAT-004 | `POST /api/me/conversations/{id}/messages` | 双方交替发送消息 | 200，返回 `messageId` |
| CHAT-005 | `GET /api/me/conversations/{id}/messages?afterMessageId=` | 增量拉取 | 200 |
| CHAT-006 | `POST /api/me/conversations/{id}/read` | 标记已读 | 200 |
| CARD-001 | `GET /api/me/contact-card` | 查询联系方式卡片 | 200 |
| CARD-002 | `PUT /api/me/contact-card` | 保存联系方式卡片 | 200 |
| UNLOCK-001 | `GET /api/me/conversations/{id}/contact-unlock` | 查询解锁状态 | 200 |
| UNLOCK-002 | `POST /api/me/conversations/{id}/contact-unlock/confirm` | 双方确认 | 200，最终 `UNLOCKED` |
| UNLOCK-003 | `GET /api/me/conversations/{id}/peer-contact-card` | 解锁后查看对方卡片 | 200 |
| CHAT-007 | `POST /api/me/conversations/{id}/close` | 关闭会话 | 200，`status=CLOSED` |
| CHAT-008 | `POST /api/me/conversations/{id}/messages` | 关闭后继续发送 | 403 `CONVERSATION_CLOSED` |
| CHAT-009 | `POST /api/partner-posts/{id}/contact-requests` | 关闭后重新发起联系 | 200，恢复 `ACTIVE` |

### 3.6 评价与信用

| ID | 接口 | 步骤 | 预期 |
| --- | --- | --- | --- |
| REV-001 | `POST /api/me/reviews` | 有效会话后创建评价 | 201，返回 `reviewId`；重复运行可能 409 `REVIEW_ALREADY_EXISTS` |
| REV-002 | `PUT /api/me/reviews/{reviewId}` | 24 小时内首次修改 | 200，`modifiedOnce=true` |
| REV-003 | `GET /api/me/reviews/given` | 查询我发出的评价 | 200 |
| REV-004 | `GET /api/me/reviews/received` | 查询我收到的评价 | 200 |
| CREDIT-001 | `GET /api/me/credit-summary` | 查询我的信用摘要 | 200 |
| CREDIT-002 | `GET /api/users/{userId}/credit-summary` | 查询对方公开信用摘要 | 200 |

## 4. 结果判定

- `PASS`：接口按预期返回状态码和关键字段。
- `SKIP`：缺少安全测试数据，或重复运行导致业务状态不可再次创建，例如评价已存在。
- `FAIL`：状态码、错误码或关键字段不符合预期。

一次 smoke 合格标准：

1. 健康检查、登录、鉴权、发帖、管理员审核、广场、会话、联系方式、信用摘要主链路均 PASS。
2. 负向测试至少覆盖 401、403、400。
3. SKIP 项只允许出现在需要一次性验证码、受控待审身份认证、重复评价等状态依赖接口。

