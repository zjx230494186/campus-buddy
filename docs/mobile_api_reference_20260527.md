# 校园搭子平台移动端接口文档

更新时间：2026-05-27  
适用对象：Android / iOS 移动端 UI 与客户端开发  
数据来源：当前后端 Controller / Service 源码与已上线服务器行为

## 1. 接入总览

### 1.1 服务地址

当前内测服务器 API 基地址：

```text
http://114.116.203.78/api/
```

移动端使用 Retrofit 时，`baseUrl` 必须以 `/` 结尾。推荐配置：

```kotlin
Retrofit.Builder()
    .baseUrl("http://114.116.203.78/api/")
    .addConverterFactory(GsonConverterFactory.create())
    .client(okHttpClient)
    .build()
```

如果写成 `http://114.116.203.78/api`，Retrofit 会报：

```text
IllegalArgumentException: baseUrl must end in /
```

移动端不连接 H2 数据库，也不需要数据库驱动。H2 是后端内部/本地调试用数据库；移动端只通过 HTTP API 访问后端。

### 1.2 HTTP 与 HTTPS

当前内测服务是 HTTP：

```text
http://114.116.203.78/api/
```

Android 临时测试需要允许明文 HTTP。可以先用全局开关：

```xml
<application
    android:usesCleartextTraffic="true"
    ...>
</application>
```

更稳妥的做法是只放行测试 IP：

```xml
<!-- res/xml/network_security_config.xml -->
<network-security-config>
    <domain-config cleartextTrafficPermitted="true">
        <domain includeSubdomains="false">114.116.203.78</domain>
    </domain-config>
</network-security-config>
```

```xml
<application
    android:networkSecurityConfig="@xml/network_security_config"
    ...>
</application>
```

上线正式版本前建议切换到 HTTPS 域名。

### 1.3 通用请求规则

- 普通 JSON 请求使用 `Content-Type: application/json`。
- 文件上传使用 `multipart/form-data`。
- 除注册、登录、验证码、健康检查外，大部分接口需要登录态。
- 登录态通过请求头传递：

```text
Authorization: Bearer <accessToken>
```

OkHttp 拦截器示例：

```kotlin
val authClient = OkHttpClient.Builder()
    .addInterceptor { chain ->
        val token = tokenStore.accessToken
        val request = if (token.isNullOrBlank()) {
            chain.request()
        } else {
            chain.request().newBuilder()
                .addHeader("Authorization", "Bearer $token")
                .build()
        }
        chain.proceed(request)
    }
    .build()
```

### 1.4 时间与分页

时间字段使用 ISO-8601 字符串，例如：

```text
2026-05-27T08:30:00Z
```

分页统一从 `page=0` 开始：

```json
{
  "items": [],
  "page": 0,
  "size": 20,
  "totalElements": 0,
  "totalPages": 0
}
```

### 1.5 通用错误体

错误响应统一形状：

```json
{
  "code": "VALIDATION_FAILED",
  "message": "Validation failed",
  "details": {
    "title": "is required"
  },
  "traceId": "..."
}
```

常见错误：

| HTTP | code | 含义 | 移动端建议 |
| --- | --- | --- | --- |
| 400 | `VALIDATION_FAILED` | 请求字段不合法 | 展示 `message`，字段级错误优先展示 `details` |
| 401 | `UNAUTHORIZED` | 未登录或 token 无效 | 清理登录态，引导重新登录 |
| 403 | `FORBIDDEN` | 角色不足 | 提示无权限 |
| 403 | `AUTHENTICATION_STATUS_REQUIRED` | 未完成身份认证 | 引导去身份认证页 |
| 403 | `CONTACT_REPLY_REQUIRED` | 发起对话后，对方尚未回复，不能继续发送 | 禁用发送框，提示等待对方回复 |
| 404 | `RESOURCE_NOT_FOUND` | 路径不存在 | 检查接口路径 |
| 409 | `*_ALREADY_*` / `*_PENDING` | 状态冲突 | 刷新当前页面状态 |
| 429 | `EMAIL_VERIFICATION_TOO_FREQUENT` | 验证码发送过于频繁 | 按倒计时禁用发送按钮 |

字段校验错误的 `details` 可能是对象，也可能是字符串。客户端解析时建议按 `Any?` 接收。

## 2. 状态、角色与核心枚举

### 2.1 账号与认证状态

`authenticationStatus`：

| 值 | 含义 |
| --- | --- |
| `UNVERIFIED` | 已注册，但未提交身份认证 |
| `PENDING_REVIEW` | 身份认证待管理员审核 |
| `VERIFIED` | 已通过身份认证 |
| `REJECTED` | 身份认证被拒，可重新提交 |

`campusEmailVerificationStatus`：

| 值 | 含义 |
| --- | --- |
| `VERIFIED` | 校园邮箱已验证 |

`accountRole`：

| 值 | 含义 |
| --- | --- |
| `STUDENT` | 普通学生用户 |
| `ADMIN` | 管理员 |

业务规则：查看广场、发帖、联系发帖人、评价等核心业务要求用户完成身份认证，即 `authenticationStatus=VERIFIED`。

### 2.2 帖子枚举

`sceneType`：

| 值 | 场景 | 提交审核必填的 `scenePayload` key |
| --- | --- | --- |
| `STUDY` | 学习搭子 | `studyGoal` |
| `MEAL` | 饭搭子 | `canteen` |
| `SPORT` | 运动搭子 | `sportType` |
| `COURSE_TEAM` | 课程组队 | `courseName` |
| `INNOVATION_PROJECT` | 创新项目 | `projectDirection` |

`timeMode`：

| 值 | 含义 | 提交审核要求 |
| --- | --- | --- |
| `EXACT_TIME` | 精确时间 | `startAt` 必填 |
| `TIME_RANGE` | 时间范围 | `startAt`、`endAt` 必填，且 `endAt > startAt` |
| `TEXT_PREFERENCE` | 文字偏好 | `timeText` 必填，最长 60 字符 |

`post.status`：

| 值 | 含义 |
| --- | --- |
| `DRAFT` | 草稿 |
| `PENDING_REVIEW` | 待审核 |
| `PUBLISHED` | 已发布 |
| `REJECTED` | 审核拒绝 |

帖子响应中的 `allowedActions` 用于驱动按钮：

| status | allowedActions |
| --- | --- |
| `DRAFT` | `EDIT`, `SUBMIT_REVIEW` |
| `PENDING_REVIEW` | `WITHDRAW_REVIEW` |
| `PUBLISHED` | `UNPUBLISH` |
| `REJECTED` | `EDIT`, `VIEW_REJECT_REASON` |

### 2.3 对话与评价枚举

对话 `status`：

| 值 | 含义 |
| --- | --- |
| `ACTIVE` | 可对话 |
| `CLOSED` | 已关闭 |

消息 `messageType`：

| 值 | 含义 |
| --- | --- |
| `USER_TEXT` | 用户文字消息 |
| 其他系统消息 | 后端可能扩展，移动端应兼容未知值 |

联系方式解锁 `status`：

| 值 | 含义 |
| --- | --- |
| `LOCKED` | 未解锁 |
| `WAITING_FOR_PEER` | 当前用户已确认，等待对方确认 |
| `UNLOCKED` | 双方确认，已解锁 |

评价标签只能使用固定值：

```text
守时, 沟通顺畅, 配合度高, 认真负责, 体验很好,
迟到, 失联, 临时变卦, 配合度低, 体验不佳
```

## 3. 系统接口

### 3.1 健康检查

```http
GET /health
```

鉴权：不需要。

响应：

```json
{
  "status": "UP"
}
```

### 3.2 系统信息

```http
GET /system/info
```

鉴权：不需要。

用于确认后端版本和环境信息。移动端一般只在调试页使用。

## 4. 注册、登录与邮箱验证码

### 4.1 发送校园邮箱验证码

```http
POST /auth/campus-email/verification-codes
```

鉴权：不需要。

请求：

```json
{
  "campusEmail": "demo.student@bjtu.edu.cn",
  "purpose": "REGISTER_OR_LOGIN"
}
```

响应：

```json
{
  "campusEmail": "demo.student@bjtu.edu.cn",
  "verificationStatus": "PENDING",
  "expiresInSeconds": 600,
  "resendAfterSeconds": 60
}
```

当前服务器只允许 `@bjtu.edu.cn` 邮箱收验证码。移动端应在输入层先做域名提示，但以后端错误为准。

常见错误：

- `INVALID_CAMPUS_EMAIL_DOMAIN`：邮箱域名不允许。
- `EMAIL_VERIFICATION_TOO_FREQUENT`：发送过于频繁。
- `VALIDATION_FAILED`：邮箱或用途为空/格式不对。

### 4.2 校验验证码

```http
POST /auth/campus-email/verifications
```

鉴权：不需要。

请求：

```json
{
  "campusEmail": "demo.student@bjtu.edu.cn",
  "code": "123456",
  "purpose": "REGISTER_OR_LOGIN"
}
```

响应：

```json
{
  "campusEmail": "demo.student@bjtu.edu.cn",
  "verificationStatus": "VERIFIED",
  "verifiedAt": "2026-05-27T08:30:00Z",
  "verificationTicket": "..."
}
```

`verificationTicket` 用于注册。客户端不需要解析其内容，只需保存并传给注册接口。

常见错误：

- `EMAIL_VERIFICATION_CODE_INVALID`
- `EMAIL_VERIFICATION_CODE_EXPIRED`
- `VALIDATION_FAILED`

### 4.3 注册

```http
POST /auth/register
```

鉴权：不需要。

请求：

```json
{
  "campusEmail": "demo.student@bjtu.edu.cn",
  "verificationTicket": "...",
  "password": "your-password",
  "displayName": "张同学",
  "college": "软件学院",
  "grade": "2024"
}
```

响应：

```json
{
  "userId": "9fe2bdce-....",
  "campusEmail": "demo.student@bjtu.edu.cn",
  "displayName": "张同学",
  "authenticationStatus": "UNVERIFIED",
  "campusEmailVerificationStatus": "VERIFIED",
  "createdAt": "2026-05-27T08:30:00Z"
}
```

注册成功后仍需要登录获取 token，并继续提交身份认证。

常见错误：

- `EMAIL_ALREADY_REGISTERED`
- `CAMPUS_EMAIL_NOT_VERIFIED`
- `INVALID_CAMPUS_EMAIL_DOMAIN`
- `VALIDATION_FAILED`

### 4.4 登录

```http
POST /auth/login
```

鉴权：不需要。

请求：

```json
{
  "campusEmail": "demo.student@bjtu.edu.cn",
  "password": "your-password",
  "clientName": "android",
  "deviceId": "optional-device-id"
}
```

响应：

```json
{
  "accessToken": "...",
  "accessTokenExpiresInSeconds": 7200,
  "refreshToken": "...",
  "refreshTokenExpiresInSeconds": 2592000,
  "tokenType": "Bearer",
  "user": {
    "userId": "9fe2bdce-....",
    "displayName": "张同学",
    "authenticationStatus": "VERIFIED",
    "campusEmailVerificationStatus": "VERIFIED",
    "accountRole": "STUDENT"
  }
}
```

当前有 `refreshToken` 字段，但后端尚未提供刷新 token 接口。移动端可以先保存 accessToken；遇到 401 时引导重新登录。

常见错误：

- `INVALID_LOGIN_CREDENTIALS`
- `VALIDATION_FAILED`

## 5. 身份认证接口

### 5.1 上传认证材料

```http
POST /auth/identity-verifications/materials
```

鉴权：需要登录。

请求类型：`multipart/form-data`  
文件字段名：`file`

限制：

- 支持 `image/jpeg`、`image/png`、`application/pdf`。
- 最大 10MB。
- 认证材料按当前产品约定应包含学生证和身份证照；如果只上传一个文件，建议前端引导用户合成一张图片或一个 PDF。

响应：

```json
{
  "attachmentId": "0d3f...",
  "contentType": "image/png",
  "sizeBytes": 123456,
  "sha256": "...",
  "status": "UPLOADED"
}
```

### 5.2 删除未引用的认证材料

```http
DELETE /auth/identity-verifications/materials/{attachmentId}
```

鉴权：需要登录。

成功响应：`204 No Content`

如果材料已经被身份认证提交引用，不能删除。

### 5.3 提交身份认证

```http
POST /auth/identity-verifications
```

鉴权：需要登录。

请求：

```json
{
  "realName": "张敏轩",
  "studentNumber": "20240001",
  "college": "软件学院",
  "major": "软件工程",
  "grade": "2024",
  "materialAttachmentId": "0d3f..."
}
```

响应：

```json
{
  "authenticationStatus": "PENDING_REVIEW",
  "submittedAt": "2026-05-27T08:30:00Z",
  "realName": "张敏轩",
  "studentNumber": "20240001",
  "college": "软件学院",
  "major": "软件工程",
  "grade": "2024"
}
```

状态规则：

- `UNVERIFIED` / `REJECTED`：可提交。
- `PENDING_REVIEW`：不可重复提交，返回 `IDENTITY_VERIFICATION_PENDING`。
- `VERIFIED`：不可重复认证。

### 5.4 查询我的身份认证状态

```http
GET /auth/identity-verifications/me
```

鉴权：需要登录。

响应：

```json
{
  "authenticationStatus": "PENDING_REVIEW",
  "reviewStatus": "PENDING_REVIEW",
  "submittedAt": "2026-05-27T08:30:00Z",
  "reviewedAt": null,
  "rejectReason": null,
  "realName": "张敏轩",
  "studentNumber": "20240001",
  "college": "软件学院",
  "major": "软件工程",
  "grade": "2024",
  "allowedActions": ["RESUBMIT"]
}
```

`allowedActions`：

| authenticationStatus | allowedActions |
| --- | --- |
| `UNVERIFIED` | `SUBMIT` |
| `REJECTED` | `SUBMIT` |
| `PENDING_REVIEW` | `RESUBMIT` |
| `VERIFIED` | `VIEW` |

注意：当前后端在 `PENDING_REVIEW` 状态下会阻止重复提交，所以移动端看到 `RESUBMIT` 时建议只作为“查看/等待审核”的 UI 状态，不要直接提交第二次。

## 6. 我的帖子接口

以下接口都需要登录并完成身份认证。

### 6.1 新建草稿

```http
POST /me/partner-posts
```

请求：

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
  "contactPreference": "先站内沟通，通过后再交换联系方式",
  "tags": ["课程项目", "前端"],
  "attachmentIds": [],
  "scenePayload": {
    "projectDirection": "校园服务应用"
  }
}
```

响应：见“帖子详情模型”。

草稿阶段允许字段不完整，但字段长度和枚举仍会校验。

### 6.2 更新草稿

```http
PUT /me/partner-posts/{postId}
```

请求字段同新建草稿。

只能更新自己的 `DRAFT` 或 `REJECTED` 后可编辑状态的帖子。

### 6.3 查询我的帖子列表

```http
GET /me/partner-posts?status=DRAFT&page=0&size=20
```

`status` 可不传。

响应：

```json
{
  "items": [],
  "page": 0,
  "size": 20,
  "totalElements": 0,
  "totalPages": 0
}
```

### 6.4 查询我的帖子详情

```http
GET /me/partner-posts/{postId}
```

### 6.5 提交审核

```http
POST /me/partner-posts/{postId}/submit-review
```

请求体：空。

提交审核时必填校验：

- `sceneType`
- `title`，最长 40 字符。
- `description`，最长 500 字符。
- `timeMode`
- `locationText`，最长 80 字符。
- `participantCount`，1 到 20。
- `targetRequirement`，最长 120 字符。
- `contactPreference`，最长 80 字符，不能包含 11 位手机号。
- `scenePayload` 对应场景必填 key。
- 时间字段按 `timeMode` 规则校验。

字段级错误示例：

```json
{
  "code": "VALIDATION_FAILED",
  "message": "Validation failed",
  "details": {
    "title": "is required",
    "description": "is required",
    "scenePayload.projectDirection": "is required for scene INNOVATION_PROJECT"
  },
  "traceId": "..."
}
```

移动端建议：保存草稿与提交审核使用同一个编辑表单；提交失败后不要清空表单，按 `details` 高亮对应字段。

### 6.6 撤回审核

```http
POST /me/partner-posts/{postId}/withdraw-review
```

请求体：空。通常从 `PENDING_REVIEW` 回到草稿态。

### 6.7 下架帖子

```http
POST /me/partner-posts/{postId}/unpublish
```

请求体：空。用于下架已发布帖子。

### 6.8 帖子详情模型

```json
{
  "postId": "9fe2bdce-....",
  "publisherId": "7c13...",
  "sceneType": "INNOVATION_PROJECT",
  "status": "DRAFT",
  "title": "创新项目招募前端同学",
  "description": "希望一起完成课程项目和演示。",
  "timeMode": "TEXT_PREFERENCE",
  "timeText": "本周晚上可沟通",
  "startAt": null,
  "endAt": null,
  "locationText": "线上或逸夫楼",
  "participantCount": 3,
  "targetRequirement": "熟悉 Android 或 Qt",
  "contactPreference": "先站内沟通，通过后再交换联系方式",
  "tags": ["课程项目", "前端"],
  "attachmentIds": [],
  "scenePayload": {
    "projectDirection": "校园服务应用"
  },
  "rejectReason": null,
  "publishedAt": null,
  "createdAt": "2026-05-27T08:30:00Z",
  "updatedAt": "2026-05-27T08:30:00Z",
  "allowedActions": ["EDIT", "SUBMIT_REVIEW"]
}
```

## 7. 广场接口

以下接口需要登录并完成身份认证。未认证用户访问会返回 `AUTHENTICATION_STATUS_REQUIRED`。

### 7.1 广场列表

```http
GET /partner-posts?sceneType=SPORT&keyword=篮球&page=0&size=20
```

查询参数：

| 参数 | 必填 | 说明 |
| --- | --- | --- |
| `sceneType` | 否 | 按场景筛选 |
| `keyword` | 否 | 按标题/描述等搜索 |
| `page` | 否 | 默认 0 |
| `size` | 否 | 默认 20 |

响应：

```json
{
  "items": [
    {
      "postId": "9fe2bdce-....",
      "publisherId": "7c13...",
      "publisherDisplayName": "张同学",
      "publisherAuthenticationStatus": "VERIFIED",
      "publisherCreditSummary": {
        "userId": "7c13...",
        "averageRating": 5.0,
        "realConversationCount": 3,
        "ratingSampleCount": 2,
        "topTags": [
          { "tagName": "沟通顺畅", "count": 2 }
        ],
        "updatedAt": "2026-05-27T08:30:00Z"
      },
      "sceneType": "SPORT",
      "status": "PUBLISHED",
      "title": "周末打羽毛球",
      "description": "找一位同学一起练习。",
      "tags": ["羽毛球"],
      "timeText": "周末下午",
      "locationText": "体育馆",
      "scenePayload": {
        "sportType": "羽毛球"
      },
      "publishedAt": "2026-05-27T08:30:00Z",
      "updatedAt": "2026-05-27T08:30:00Z",
      "ownPost": false
    }
  ],
  "page": 0,
  "size": 20,
  "totalElements": 1,
  "totalPages": 1
}
```

### 7.2 广场帖子详情

```http
GET /partner-posts/{postId}
```

响应字段比列表多：

- `timeMode`
- `startAt`
- `endAt`
- `participantCount`
- `targetRequirement`

移动端建议：列表页展示摘要，详情页展示完整场景字段、发布者信用摘要和“联系 TA”按钮。

## 8. 站内联系与对话接口

以下接口需要登录并完成身份认证。

### 8.1 从帖子发起联系

```http
POST /partner-posts/{postId}/contact-requests
```

请求：

```json
{
  "message": "你好，我对这个项目感兴趣。"
}
```

限制：

- `message` 不能为空。
- 最长 30 字符。
- 不能联系自己的帖子。
- 帖子必须是 `PUBLISHED`。
- 发起联系后，在对方回复前，当前用户不能继续发送第二条用户消息。

响应：

```json
{
  "conversationId": 12,
  "status": "ACTIVE"
}
```

如果用户已经发过首条消息且对方还没有回复，再调用会返回：

```json
{
  "code": "CONTACT_REPLY_REQUIRED",
  "message": "Please wait for the other participant to reply before sending another message",
  "details": null,
  "traceId": "..."
}
```

### 8.2 我的会话列表

```http
GET /me/conversations?page=0&size=20
```

响应：

```json
{
  "items": [
    {
      "conversationId": 12,
      "status": "ACTIVE",
      "otherParticipantId": "7c13...",
      "otherParticipantDisplayName": "李同学",
      "relatedPostUuid": "9fe2bdce-....",
      "relatedPostTitle": "周末打羽毛球",
      "lastMessagePreview": "你好，我对这个项目感兴趣。",
      "lastMessageAt": "2026-05-27T08:30:00Z",
      "updatedAt": "2026-05-27T08:30:00Z",
      "unreadCount": 1
    }
  ],
  "page": 0,
  "size": 20,
  "totalElements": 1,
  "totalPages": 1
}
```

### 8.3 查询会话消息

```http
GET /me/conversations/{conversationId}/messages?page=0&size=20
```

增量拉取：

```http
GET /me/conversations/{conversationId}/messages?afterMessageId=123&size=50
```

当 `afterMessageId > 0` 时，后端返回该消息之后的新消息，按时间升序；`size` 最大 50。

响应：

```json
{
  "items": [
    {
      "messageId": 123,
      "senderId": "9fe2bdce-....",
      "messageType": "USER_TEXT",
      "content": "你好，我对这个项目感兴趣。",
      "createdAt": "2026-05-27T08:30:00Z"
    }
  ],
  "page": 0,
  "size": 20,
  "totalElements": 1,
  "totalPages": 1
}
```

### 8.4 发送消息

```http
POST /me/conversations/{conversationId}/messages
```

请求：

```json
{
  "message": "可以约时间聊一下吗？"
}
```

响应：

```json
{
  "messageId": 124
}
```

移动端必须处理 `CONTACT_REPLY_REQUIRED`：如果当前用户已经发过用户消息，而对方没有发过用户消息，应禁用发送按钮并提示“请等待对方回复后再继续发送”。

常见错误：

- `CONTACT_REPLY_REQUIRED`
- `CONVERSATION_CLOSED`
- `CONVERSATION_NOT_FOUND`
- `NOT_PARTICIPANT`
- `VALIDATION_FAILED`

### 8.5 标记已读

```http
POST /me/conversations/{conversationId}/read
```

成功响应：`200 OK`，空 body。

### 8.6 关闭会话

```http
POST /me/conversations/{conversationId}/close
```

响应：

```json
{
  "conversationId": 12,
  "status": "CLOSED"
}
```

## 9. 联系方式卡片与解锁

### 9.1 查询我的联系方式卡片

```http
GET /me/contact-card
```

鉴权：需要登录。

响应：

```json
{
  "hasCard": true,
  "wechatId": "wechat_xxx",
  "phoneNumber": "13800000000",
  "qqNumber": "123456789",
  "updatedAt": "2026-05-27T08:30:00Z"
}
```

### 9.2 保存我的联系方式卡片

```http
PUT /me/contact-card
```

请求：

```json
{
  "wechatId": "wechat_xxx",
  "phoneNumber": "13800000000",
  "qqNumber": "123456789"
}
```

响应同查询接口。

### 9.3 查询会话联系方式解锁状态

```http
GET /me/conversations/{conversationId}/contact-unlock
```

响应：

```json
{
  "conversationId": 12,
  "status": "WAITING_FOR_PEER",
  "currentUserConfirmed": true,
  "peerConfirmed": false,
  "currentUserHasContactCard": true,
  "peerHasContactCard": true,
  "unlockedAt": null
}
```

### 9.4 确认解锁联系方式

```http
POST /me/conversations/{conversationId}/contact-unlock/confirm
```

请求体：空。

响应同查询接口。双方都确认后，`status=UNLOCKED`。

### 9.5 查看对方联系方式

```http
GET /me/conversations/{conversationId}/peer-contact-card
```

要求：联系方式已解锁。

响应：

```json
{
  "wechatId": "peer_wechat",
  "phoneNumber": "13900000000",
  "qqNumber": "987654321"
}
```

## 10. 评价与信用接口

### 10.1 查询我的信用摘要

```http
GET /me/credit-summary
```

鉴权：需要登录。

响应：

```json
{
  "userId": "9fe2bdce-....",
  "averageRating": 5.0,
  "realConversationCount": 3,
  "ratingSampleCount": 2,
  "topTags": [
    { "tagName": "沟通顺畅", "count": 2 }
  ],
  "disputedReviewCount": 0,
  "updatedAt": "2026-05-27T08:30:00Z"
}
```

### 10.2 查询其他用户信用摘要

```http
GET /users/{userId}/credit-summary
```

鉴权：需要登录。

响应：

```json
{
  "userId": "7c13...",
  "averageRating": 5.0,
  "realConversationCount": 3,
  "ratingSampleCount": 2,
  "topTags": [
    { "tagName": "守时", "count": 1 }
  ],
  "updatedAt": "2026-05-27T08:30:00Z"
}
```

### 10.3 创建评价

```http
POST /me/reviews
```

鉴权：需要登录并完成身份认证。

请求：

```json
{
  "conversationId": 12,
  "revieweeId": "7c13...",
  "rating": 5,
  "reviewTags": ["守时", "沟通顺畅"]
}
```

响应：

```json
{
  "id": 5,
  "conversationId": 12,
  "reviewerId": "9fe2bdce-....",
  "revieweeId": "7c13...",
  "rating": 5,
  "reviewTags": ["守时", "沟通顺畅"],
  "status": "NORMAL",
  "modifiedOnce": false,
  "createdAt": "2026-05-27T08:30:00Z",
  "updatedAt": "2026-05-27T08:30:00Z"
}
```

评价前置条件：

- 当前用户必须是该会话参与者。
- `revieweeId` 必须是会话中的另一方。
- 同一会话中双方各自至少发送过 2 条 `USER_TEXT` 消息，才算有效会话。
- 同一评价关系只能创建一次。
- `rating` 默认范围 1 到 5。
- 如果该会话已经完成联系方式解锁，`rating` 允许 1 到 6。
- `reviewTags` 必须来自固定标签列表。

移动端 UI 建议：

- 评价入口应从会话详情进入，自动带入 `conversationId`。
- `revieweeId` 使用会话列表或消息页中的 `otherParticipantId`，不要让用户手填。
- 如果返回 `CONVERSATION_NOT_REVIEWABLE`，提示“双方互动不足，需双方各发送至少 2 条消息后才能评价”。
- 如果联系方式已解锁，可以显示 6 分选项；否则最多 5 分。

### 10.4 修改评价

```http
PUT /me/reviews/{reviewId}
```

请求：

```json
{
  "rating": 4,
  "reviewTags": ["沟通顺畅"]
}
```

限制：

- 只能修改自己发出的评价。
- 只能修改一次。
- 必须在创建后 24 小时内修改。

### 10.5 查询我发出的评价

```http
GET /me/reviews/given?page=0&size=20
```

响应为分页的 `ReviewResponse`。

### 10.6 查询我收到的评价

```http
GET /me/reviews/received?page=0&size=20
```

响应为分页的 `ReviewResponse`。

## 11. 管理员接口

管理员接口需要登录，并且 `accountRole=ADMIN`。

### 11.1 身份认证待审列表

```http
GET /admin/identity-verifications?status=PENDING_REVIEW
```

响应：

```json
[
  {
    "submissionId": 1,
    "userId": "9fe2bdce-....",
    "realName": "张敏轩",
    "studentNumber": "20240001",
    "college": "软件学院",
    "major": "软件工程",
    "grade": "2024",
    "reviewStatus": "PENDING_REVIEW",
    "submittedAt": "2026-05-27T08:30:00Z",
    "materialAttachmentId": "0d3f...",
    "materialContentType": "image/png",
    "materialSizeBytes": 123456
  }
]
```

### 11.2 下载认证材料

```http
GET /admin/identity-verifications/{submissionId}/material
```

响应：二进制文件流。响应头包含：

```text
Content-Type: image/png
X-Original-Filename: demo_identity.png
```

### 11.3 审核身份认证

```http
POST /admin/identity-verifications/{submissionId}/reviews
```

请求：

```json
{
  "decision": "APPROVED",
  "rejectReason": null
}
```

拒绝时：

```json
{
  "decision": "REJECTED",
  "rejectReason": "材料不清晰，请重新上传。"
}
```

响应：

```json
{
  "reviewStatus": "APPROVED",
  "authenticationStatus": "VERIFIED",
  "reviewedAt": "2026-05-27T08:30:00Z",
  "rejectReason": null
}
```

### 11.4 帖子审核队列

```http
GET /admin/partner-posts/review-queue?page=0&size=20
```

返回待审核帖子分页列表。

### 11.5 帖子审核详情

```http
GET /admin/partner-posts/{postId}
```

返回管理员视角的帖子详情。

### 11.6 审核帖子

```http
POST /admin/partner-posts/{postId}/review
```

请求：

```json
{
  "decision": "APPROVED",
  "reason": null
}
```

拒绝时：

```json
{
  "decision": "REJECTED",
  "reason": "标题或正文信息不完整。"
}
```

## 12. 推荐移动端页面流程

### 12.1 注册登录链路

1. 输入 `@bjtu.edu.cn` 邮箱。
2. 调用发送验证码接口。
3. 输入验证码，调用校验验证码接口，拿到 `verificationTicket`。
4. 调用注册接口。
5. 调用登录接口，保存 `accessToken`。
6. 查询身份认证状态。
7. 如果不是 `VERIFIED`，引导到身份认证页。

### 12.2 身份认证链路

1. 上传认证材料，拿到 `attachmentId`。
2. 填写姓名、学号、学院、专业、年级。
3. 调用提交身份认证接口。
4. 进入等待审核状态。
5. 管理员通过后，用户重新登录或刷新身份认证状态，即可访问广场。

### 12.3 发帖链路

1. 进入编辑页，先调用新建草稿或加载已有草稿。
2. 用户填写字段。
3. 保存草稿：调用 `PUT /me/partner-posts/{postId}`。
4. 提交审核：调用 `POST /me/partner-posts/{postId}/submit-review`。
5. 如 `VALIDATION_FAILED`，根据 `details` 高亮字段并保留用户输入。
6. 管理员审核通过后，帖子进入广场。

### 12.4 广场联系链路

1. 查询广场列表。
2. 进入帖子详情。
3. 点击联系，输入首条消息。
4. 调用发起联系接口，进入会话页。
5. 若当前用户已发首条且对方未回复，后端返回 `CONTACT_REPLY_REQUIRED`，移动端禁用发送。
6. 对方回复后，双方可继续正常对话。

### 12.5 评价链路

1. 从会话详情进入评价页。
2. 使用会话中的 `otherParticipantId` 作为 `revieweeId`。
3. 确认双方各自至少有 2 条用户消息；客户端可先粗略判断，最终以后端为准。
4. 调用创建评价接口。
5. 若联系方式已解锁，允许 1 到 6 分；否则 1 到 5 分。

## 13. 移动端 FAQ

### 13.1 为什么不能直接连 H2？

H2 是后端内部数据库或本地调试数据库，不对移动端开放。移动端连接数据库会绕过鉴权、业务校验和安全边界。移动端只需要连接：

```text
http://114.116.203.78/api/
```

### 13.2 为什么我能登录但看不了广场？

广场要求用户已完成身份认证。登录只代表账号密码正确；业务权限还要看 `authenticationStatus`。如果是 `UNVERIFIED`、`PENDING_REVIEW` 或 `REJECTED`，应引导用户去认证页。

### 13.3 为什么发起对话后不能继续发第二条？

产品规则要求：用户对帖子发布人开始对话时，只能先发一条消息；对方回复后才正式解锁继续对话。后端会用 `CONTACT_REPLY_REQUIRED` 强制限制。

### 13.4 评价页里的 conversationId 和 revieweeId 从哪里来？

从会话列表或会话详情来：

- `conversationId`：会话 ID。
- `revieweeId`：会话中的 `otherParticipantId`。

不要让用户手动填写这些 ID。

### 13.5 手机端发帖能不能填手机号或微信？

不建议。帖子字段里的 `contactPreference` 不能包含 11 位手机号。联系方式应通过站内会话和联系方式解锁流程交换。

### 13.6 当前 token 怎么刷新？

登录响应里有 `refreshToken` 字段，但当前后端没有刷新 token 接口。内测阶段可以先在 401 时重新登录。

## 14. Retrofit 接口骨架示例

```kotlin
interface CampusBuddyApi {
    @POST("auth/campus-email/verification-codes")
    suspend fun sendEmailCode(@Body body: SendEmailCodeRequest): EmailCodeResponse

    @POST("auth/campus-email/verifications")
    suspend fun verifyEmailCode(@Body body: VerifyEmailCodeRequest): EmailVerifyResponse

    @POST("auth/register")
    suspend fun register(@Body body: RegisterRequest): RegisterResponse

    @POST("auth/login")
    suspend fun login(@Body body: LoginRequest): LoginResponse

    @Multipart
    @POST("auth/identity-verifications/materials")
    suspend fun uploadIdentityMaterial(@Part file: MultipartBody.Part): UploadMaterialResponse

    @POST("auth/identity-verifications")
    suspend fun submitIdentity(@Body body: IdentitySubmitRequest): IdentitySubmitResponse

    @GET("auth/identity-verifications/me")
    suspend fun getMyIdentity(): IdentityStatusResponse

    @GET("partner-posts")
    suspend fun listPlazaPosts(
        @Query("sceneType") sceneType: String?,
        @Query("keyword") keyword: String?,
        @Query("page") page: Int = 0,
        @Query("size") size: Int = 20
    ): PlazaListResponse

    @GET("partner-posts/{postId}")
    suspend fun getPlazaPost(@Path("postId") postId: String): PlazaPostDetailResponse

    @POST("partner-posts/{postId}/contact-requests")
    suspend fun requestContact(
        @Path("postId") postId: String,
        @Body body: ContactRequest
    ): ContactRequestResponse

    @GET("me/conversations")
    suspend fun listConversations(
        @Query("page") page: Int = 0,
        @Query("size") size: Int = 20
    ): ConversationListResponse

    @GET("me/conversations/{conversationId}/messages")
    suspend fun listMessages(
        @Path("conversationId") conversationId: Long,
        @Query("afterMessageId") afterMessageId: Long? = null,
        @Query("page") page: Int = 0,
        @Query("size") size: Int = 20
    ): MessageListResponse

    @POST("me/conversations/{conversationId}/messages")
    suspend fun sendMessage(
        @Path("conversationId") conversationId: Long,
        @Body body: SendMessageRequest
    ): SendMessageResponse

    @POST("me/reviews")
    suspend fun createReview(@Body body: CreateReviewRequest): ReviewResponse
}
```

注意：这里的路径不以 `/` 开头，因为 Retrofit 已经使用 `baseUrl("http://114.116.203.78/api/")`。

