# 后端接口测试索引

更新时间：2026-06-11  
来源：`backend/src/main/java/com/campusbuddy/**Controller.java`、`SecurityConfiguration.java`、现有移动端接口文档。  
用途：手工接口测试、Postman/Apifox 用例整理、移动端/桌面端联调前核对。

## 1. 项目快速理解

后端是 Spring Boot 4 + Java 21 项目，核心业务是“校园搭子平台”：

- 注册登录：校园邮箱验证码、注册、登录、JWT Bearer 鉴权。
- 身份认证：学生提交认证资料，管理员审核。
- 搭子帖子：学生创建草稿、提交审核、管理员审核、广场浏览。
- 站内联系：从帖子发起联系、会话消息、已读、关闭、联系方式解锁。
- 评价信用：会话完成后评价，形成信用摘要。

后端主目录：

- `backend/src/main/java/com/campusbuddy/auth`：注册登录、邮箱验证码、身份认证。
- `backend/src/main/java/com/campusbuddy/post`：帖子、广场、帖子审核。
- `backend/src/main/java/com/campusbuddy/contact`：会话、消息、联系方式卡片、联系方式解锁。
- `backend/src/main/java/com/campusbuddy/review`：评价与信用摘要。
- `backend/src/main/java/com/campusbuddy/security`：JWT 与接口鉴权规则。
- `backend/src/main/java/com/campusbuddy/health`、`system`、`probe`：健康检查、系统信息、安全探针。

## 2. 测试入口与通用规则

公网内测服务文档中记录的 base URL：

```text
http://114.116.203.78/api
```

本地默认 Spring Boot 端口通常是：

```text
http://localhost:8080/api
```

鉴权请求头：

```text
Authorization: Bearer <accessToken>
```

通用错误体：

```json
{
  "code": "VALIDATION_FAILED",
  "message": "Validation failed",
  "details": {},
  "traceId": "..."
}
```

安全规则摘要：

- 无需登录：`/api/health`、`/api/system/info`、邮箱验证码、注册、登录。
- 需要登录：`/api/probe/secure`、`/api/auth/identity-verifications/**`、`/api/me/**`、`/api/partner-posts/**`、`/api/users/*/credit-summary`。
- 需要管理员：`/api/admin/**`，要求 JWT 中角色为 `ADMIN`。
- 部分学生业务除登录外还要求 `authenticationStatus=VERIFIED`，否则返回 403 `AUTHENTICATION_STATUS_REQUIRED`。

## 3. 所有可用接口总表

### 系统与探针

| 方法 | 路径 | 鉴权 | 说明 |
| --- | --- | --- | --- |
| GET | `/api/health` | 否 | 健康检查，返回 `{"status":"UP"}` |
| GET | `/api/system/info` | 否 | 系统信息 |
| GET | `/api/probe/secure` | 登录 | JWT 安全探针 |

### 注册登录与邮箱验证码

| 方法 | 路径 | 鉴权 | 请求体/参数 |
| --- | --- | --- | --- |
| POST | `/api/auth/campus-email/verification-codes` | 否 | JSON：`campusEmail`, `purpose` |
| POST | `/api/auth/campus-email/verifications` | 否 | JSON：`campusEmail`, `code`, `purpose` |
| POST | `/api/auth/register` | 否 | JSON：`campusEmail`, `verificationTicket`, `password`, `displayName`, `college`, `grade` |
| POST | `/api/auth/login` | 否 | JSON：`campusEmail`, `password`, `clientName`, `deviceId` |

常用 `purpose`：

```text
REGISTER_OR_LOGIN
```

### 身份认证：学生侧

| 方法 | 路径 | 鉴权 | 请求体/参数 |
| --- | --- | --- | --- |
| POST | `/api/auth/identity-verifications/materials` | 登录 | multipart，文件字段名 `file` |
| DELETE | `/api/auth/identity-verifications/materials/{attachmentId}` | 登录 | path：`attachmentId` UUID |
| POST | `/api/auth/identity-verifications` | 登录 | JSON：`realName`, `studentNumber`, `college`, `major`, `grade`, `materialAttachmentId` |
| GET | `/api/auth/identity-verifications/me` | 登录 | 查询我的认证状态 |

### 身份认证：管理员侧

| 方法 | 路径 | 鉴权 | 请求体/参数 |
| --- | --- | --- | --- |
| GET | `/api/admin/identity-verifications?status=PENDING_REVIEW` | 管理员 | 认证待审列表；非 `PENDING_REVIEW` 返回空列表 |
| GET | `/api/admin/identity-verifications/{submissionId}/material` | 管理员 | 下载认证材料，`submissionId` 是 UUID |
| POST | `/api/admin/identity-verifications/{submissionId}/reviews` | 管理员 | JSON：`decision`, `rejectReason` |

`decision` 可选：

```text
APPROVED
REJECTED
```

拒绝时 `rejectReason` 必填。

### 我的帖子：学生侧

| 方法 | 路径 | 鉴权 | 请求体/参数 |
| --- | --- | --- | --- |
| POST | `/api/me/partner-posts` | 登录 + 已认证 | 创建草稿 |
| PUT | `/api/me/partner-posts/{postId}` | 登录 + 已认证 | 更新自己的草稿 |
| GET | `/api/me/partner-posts?status=&page=0&size=20` | 登录 | 我的帖子列表 |
| GET | `/api/me/partner-posts/{postId}` | 登录 | 我的帖子详情 |
| POST | `/api/me/partner-posts/{postId}/submit-review` | 登录 + 已认证 | 提交审核，空 body |
| POST | `/api/me/partner-posts/{postId}/withdraw-review` | 登录 + 已认证 | 撤回审核，空 body |
| POST | `/api/me/partner-posts/{postId}/unpublish` | 登录 + 已认证 | 下架已发布帖子，空 body |

创建/更新草稿 JSON 字段：

```json
{
  "sceneType": "INNOVATION_PROJECT",
  "title": "创新项目招募前端同学",
  "description": "希望一起完成课程项目和演示。",
  "timeMode": "TEXT_PREFERENCE",
  "timeText": "本周晚上可沟通",
  "startAt": null,
  "endAt": null,
  "locationText": "线上或逸夫楼",
  "participantCount": 3,
  "targetRequirement": "熟悉 Android 或 Qt",
  "contactPreference": "先站内沟通",
  "tags": ["课程项目", "前端"],
  "attachmentIds": [],
  "scenePayload": {
    "projectDirection": "校园服务应用"
  }
}
```

帖子枚举：

- `sceneType`：`MEAL`、`STUDY`、`SPORT`、`COURSE_TEAM`、`INNOVATION_PROJECT`
- `timeMode`：`EXACT_TIME`、`TIME_RANGE`、`TEXT_PREFERENCE`
- `status`：`DRAFT`、`PENDING_REVIEW`、`PUBLISHED`、`REJECTED`

提交审核时场景必填 key：

| sceneType | scenePayload 必填 key |
| --- | --- |
| `MEAL` | `canteen` |
| `STUDY` | `studyGoal` |
| `SPORT` | `sportType` |
| `COURSE_TEAM` | `courseName` |
| `INNOVATION_PROJECT` | `projectDirection` |

### 广场

| 方法 | 路径 | 鉴权 | 请求体/参数 |
| --- | --- | --- | --- |
| GET | `/api/partner-posts?sceneType=&keyword=&page=0&size=20` | 登录 + 已认证 | 广场列表 |
| GET | `/api/partner-posts/{postId}` | 登录 + 已认证 | 广场帖子详情 |
| POST | `/api/partner-posts/{postId}/contact-requests` | 登录 + 已认证 | JSON：`message` |

发起联系请求体：

```json
{
  "message": "你好，我对这个项目感兴趣。"
}
```

### 会话与消息

| 方法 | 路径 | 鉴权 | 请求体/参数 |
| --- | --- | --- | --- |
| GET | `/api/me/conversations?page=0&size=20` | 登录 | 我的会话列表 |
| GET | `/api/me/conversations/{conversationId}/messages?page=0&size=20` | 登录 | 会话消息分页 |
| GET | `/api/me/conversations/{conversationId}/messages?afterMessageId=123&size=50` | 登录 | 增量拉取消息 |
| POST | `/api/me/conversations/{conversationId}/messages` | 登录 | JSON：`message` |
| POST | `/api/me/conversations/{conversationId}/close` | 登录 | 关闭会话，空 body |
| POST | `/api/me/conversations/{conversationId}/read` | 登录 | 标记已读，空 body |

发送消息请求体：

```json
{
  "message": "可以约时间聊一下吗？"
}
```

业务限制：发起联系后，若对方尚未回复，当前用户继续发送会返回 403 `CONTACT_REPLY_REQUIRED`。

### 联系方式卡片与解锁

| 方法 | 路径 | 鉴权 | 请求体/参数 |
| --- | --- | --- | --- |
| GET | `/api/me/contact-card` | 登录 | 查询我的联系方式卡片 |
| PUT | `/api/me/contact-card` | 登录 | JSON：`wechatId`, `phoneNumber`, `qqNumber` |
| GET | `/api/me/conversations/{conversationId}/contact-unlock` | 登录 | 查询联系方式解锁状态 |
| POST | `/api/me/conversations/{conversationId}/contact-unlock/confirm` | 登录 | 确认解锁，空 body |
| GET | `/api/me/conversations/{conversationId}/peer-contact-card` | 登录 | 查看对方联系方式，要求已解锁 |

保存联系方式请求体：

```json
{
  "wechatId": "wechat_xxx",
  "phoneNumber": "13800000000",
  "qqNumber": "123456789"
}
```

### 评价与信用

| 方法 | 路径 | 鉴权 | 请求体/参数 |
| --- | --- | --- | --- |
| GET | `/api/me/credit-summary` | 登录 | 查询我的信用摘要 |
| GET | `/api/users/{userId}/credit-summary` | 登录 | 查询其他用户公开信用摘要 |
| POST | `/api/me/reviews` | 登录 + 已认证 | 创建评价 |
| PUT | `/api/me/reviews/{reviewId}` | 登录 | 修改评价 |
| GET | `/api/me/reviews/given?page=0&size=20` | 登录 | 我发出的评价 |
| GET | `/api/me/reviews/received?page=0&size=20` | 登录 | 我收到的评价 |

创建评价请求体：

```json
{
  "conversationId": 12,
  "revieweeId": "7c13c000-0000-0000-0000-000000000000",
  "rating": 5,
  "reviewTags": ["守时", "沟通顺畅"]
}
```

修改评价请求体：

```json
{
  "rating": 4,
  "reviewTags": ["沟通顺畅"]
}
```

评价标签固定值：

```text
守时, 沟通顺畅, 配合度高, 认真负责, 体验很好,
迟到, 失联, 临时变卦, 配合度低, 体验不佳
```

### 帖子审核：管理员侧

| 方法 | 路径 | 鉴权 | 请求体/参数 |
| --- | --- | --- | --- |
| GET | `/api/admin/partner-posts/review-queue?page=0&size=20` | 管理员 | 待审核帖子队列 |
| GET | `/api/admin/partner-posts/{postId}` | 管理员 | 帖子审核详情 |
| POST | `/api/admin/partner-posts/{postId}/review` | 管理员 | JSON：`decision`, `reason` |

帖子审核请求体：

```json
{
  "decision": "APPROVED",
  "reason": null
}
```

拒绝示例：

```json
{
  "decision": "REJECTED",
  "reason": "标题或正文信息不完整。"
}
```

## 4. 建议接口测试顺序

1. 基础连通性：`GET /api/health`、`GET /api/system/info`。
2. 未登录保护：直接请求 `/api/probe/secure`，应返回 401 `UNAUTHORIZED`。
3. 注册登录链路：发送验证码、校验验证码、注册、登录，拿 `accessToken`。
4. 鉴权验证：带 Bearer token 请求 `/api/probe/secure`。
5. 学生身份认证：上传材料、提交认证、查我的认证状态。
6. 管理员审核身份认证：管理员登录后查待审、下载材料、审核通过。
7. 学生发帖：创建草稿、字段校验失败、补齐字段、提交审核。
8. 管理员审核帖子：查队列、查详情、审核通过。
9. 广场：学生查列表、查详情、发起联系。
10. 会话：查会话、查消息、发送消息、标记已读、关闭。
11. 联系方式：保存卡片、双方确认解锁、查看对方卡片。
12. 评价信用：双方足量消息后创建评价、查发出/收到评价、查信用摘要。

## 5. PowerShell 调用示例

健康检查：

```powershell
$base = "http://localhost:8080/api"
Invoke-RestMethod -Method Get -Uri "$base/health"
```

登录并保存 token：

```powershell
$base = "http://localhost:8080/api"
$loginBody = @{
  campusEmail = "demo.student@bjtu.edu.cn"
  password = "your-password"
  clientName = "powershell"
  deviceId = "manual-test"
} | ConvertTo-Json

$login = Invoke-RestMethod -Method Post -Uri "$base/auth/login" -ContentType "application/json" -Body $loginBody
$headers = @{ Authorization = "Bearer $($login.accessToken)" }
Invoke-RestMethod -Method Get -Uri "$base/probe/secure" -Headers $headers
```

创建帖子草稿：

```powershell
$postBody = @{
  sceneType = "INNOVATION_PROJECT"
  title = "创新项目招募前端同学"
  description = "希望一起完成课程项目和演示。"
  timeMode = "TEXT_PREFERENCE"
  timeText = "本周晚上可沟通"
  locationText = "线上或逸夫楼"
  participantCount = 3
  targetRequirement = "熟悉 Android 或 Qt"
  contactPreference = "先站内沟通"
  tags = @("课程项目", "前端")
  attachmentIds = @()
  scenePayload = @{ projectDirection = "校园服务应用" }
} | ConvertTo-Json -Depth 5

Invoke-RestMethod -Method Post -Uri "$base/me/partner-posts" -Headers $headers -ContentType "application/json" -Body $postBody
```

## 6. 当前注意事项

- 登录响应有 `refreshToken` 字段，但源码中未发现刷新 token 接口。
- `GET /api/admin/identity-verifications/{submissionId}/material` 的 `submissionId` 是 UUID 字符串。
- `GET /api/admin/identity-verifications?status=` 目前只有 `PENDING_REVIEW` 会返回真实列表，其他值返回空列表。
- 广场与发帖等核心学生业务要求用户完成身份认证。
- 认证材料上传使用 multipart 字段名 `file`。
- 本文只记录接口形状和测试顺序，不记录真实账号、密码、验证码、token、SMTP、数据库或 OBS 密钥。

