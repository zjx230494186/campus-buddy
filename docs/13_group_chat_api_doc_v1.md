# 多人群聊后端 API 文档

## 概述

本文档描述了 CampusBuddy 项目中多人群聊功能的后端 REST API 接口规范。

## 基础信息

| 属性 | 值 |
|------|-----|
| 基础路径 | `/api/me/group-chats` |
| 认证方式 | JWT Bearer Token |
| 内容类型 | `application/json` |

---

## 接口列表

### 1. 创建群聊

**请求**

| 属性 | 值 |
|------|-----|
| HTTP 方法 | POST |
| 端点 | `/api/me/group-chats` |

**请求体**

```json
{
  "name": "string (必填，群名称，最大100字符)",
  "description": "string (可选，群描述，最大500字符)",
  "relatedPostUuid": "UUID (可选，关联帖子UUID)",
  "maxMembers": "integer (可选，最大成员数，默认20)",
  "initialMemberIds": ["UUID (可选，初始成员ID列表)"]
}
```

**成功响应** (HTTP 200)

```json
{
  "groupChatId": "number (群聊ID)",
  "name": "string (群名称)",
  "status": "string (状态: ACTIVE)"
}
```

**错误响应**

| 错误码 | 错误类型 | 说明 |
|--------|----------|------|
| 400 | VALIDATION_FAILED | 请求参数验证失败 |
| 403 | USER_NOT_VERIFIED | 用户未完成身份验证 |

---

### 2. 获取群聊列表

**请求**

| 属性 | 值 |
|------|-----|
| HTTP 方法 | GET |
| 端点 | `/api/me/group-chats` |

**查询参数**

| 参数 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| page | int | 否 | 0 | 页码（从0开始） |
| size | int | 否 | 20 | 每页数量（最大50） |

**成功响应** (HTTP 200)

```json
{
  "items": [
    {
      "groupChatId": "number",
      "name": "string",
      "description": "string",
      "status": "string",
      "memberCount": "number",
      "maxMembers": "number",
      "currentUserRole": "string (ADMIN/MEMBER)",
      "lastMessagePreview": "string",
      "lastMessageAt": "string (ISO时间戳)",
      "createdAt": "string (ISO时间戳)",
      "updatedAt": "string (ISO时间戳)",
      "unreadCount": "number"
    }
  ],
  "page": "number",
  "size": "number",
  "totalElements": "number",
  "totalPages": "number"
}
```

---

### 3. 获取群聊详情

**请求**

| 属性 | 值 |
|------|-----|
| HTTP 方法 | GET |
| 端点 | `/api/me/group-chats/{groupChatId}` |

**路径参数**

| 参数 | 类型 | 说明 |
|------|------|------|
| groupChatId | Long | 群聊ID |

**成功响应** (HTTP 200)

```json
{
  "groupChatId": "number",
  "name": "string",
  "description": "string",
  "status": "string",
  "maxMembers": "number",
  "memberCount": "number",
  "currentUserRole": "string",
  "lastMessagePreview": "string",
  "lastMessageAt": "string",
  "createdAt": "string",
  "updatedAt": "string",
  "members": [
    {
      "userId": "string",
      "displayName": "string",
      "role": "string",
      "status": "string",
      "joinedAt": "string"
    }
  ],
  "unreadCount": "number"
}
```

**错误响应**

| 错误码 | 错误类型 | 说明 |
|--------|----------|------|
| 404 | GROUP_CHAT_NOT_FOUND | 群聊不存在 |
| 403 | NOT_MEMBER | 当前用户不是群成员 |

---

### 4. 更新群聊信息

**请求**

| 属性 | 值 |
|------|-----|
| HTTP 方法 | PUT |
| 端点 | `/api/me/group-chats/{groupChatId}` |

**路径参数**

| 参数 | 类型 | 说明 |
|------|------|------|
| groupChatId | Long | 群聊ID |

**请求体**

```json
{
  "name": "string (可选，群名称)",
  "description": "string (可选，群描述)",
  "maxMembers": "integer (可选，最大成员数)"
}
```

**成功响应** (HTTP 200)

```json
{
  "groupChatId": "number",
  "name": "string",
  "description": "string",
  "maxMembers": "number"
}
```

**错误响应**

| 错误码 | 错误类型 | 说明 |
|--------|----------|------|
| 404 | GROUP_CHAT_NOT_FOUND | 群聊不存在 |
| 403 | NOT_ADMIN | 当前用户不是管理员 |

---

### 5. 关闭群聊

**请求**

| 属性 | 值 |
|------|-----|
| HTTP 方法 | DELETE |
| 端点 | `/api/me/group-chats/{groupChatId}` |

**路径参数**

| 参数 | 类型 | 说明 |
|------|------|------|
| groupChatId | Long | 群聊ID |

**成功响应** (HTTP 200)

```json
{
  "groupChatId": "number",
  "status": "string (CLOSED)",
  "closedAt": "string (ISO时间戳)"
}
```

**错误响应**

| 错误码 | 错误类型 | 说明 |
|--------|----------|------|
| 404 | GROUP_CHAT_NOT_FOUND | 群聊不存在 |
| 403 | NOT_ADMIN | 当前用户不是管理员 |

---

### 6. 发送群消息

**请求**

| 属性 | 值 |
|------|-----|
| HTTP 方法 | POST |
| 端点 | `/api/me/group-chats/{groupChatId}/messages` |

**路径参数**

| 参数 | 类型 | 说明 |
|------|------|------|
| groupChatId | Long | 群聊ID |

**请求体**

```json
{
  "message": "string (必填，消息内容，最大500字符)"
}
```

**成功响应** (HTTP 200)

```json
{
  "messageId": "number (消息ID)",
  "groupChatId": "number (群聊ID)"
}
```

**错误响应**

| 错误码 | 错误类型 | 说明 |
|--------|----------|------|
| 400 | VALIDATION_FAILED | 消息内容为空或过长 |
| 404 | GROUP_CHAT_NOT_FOUND | 群聊不存在 |
| 403 | NOT_MEMBER | 当前用户不是群成员 |

---

### 7. 获取群消息列表

**请求**

| 属性 | 值 |
|------|-----|
| HTTP 方法 | GET |
| 端点 | `/api/me/group-chats/{groupChatId}/messages` |

**路径参数**

| 参数 | 类型 | 说明 |
|------|------|------|
| groupChatId | Long | 群聊ID |

**查询参数**

| 参数 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| afterMessageId | Long | 否 | - | 获取此消息ID之后的消息 |
| page | int | 否 | 0 | 页码 |
| size | int | 否 | 20 | 每页数量（最大50） |

**成功响应** (HTTP 200)

```json
{
  "items": [
    {
      "messageId": "number",
      "senderId": "string",
      "senderName": "string",
      "messageType": "string",
      "content": "string",
      "createdAt": "string"
    }
  ],
  "page": "number",
  "size": "number",
  "totalElements": "number",
  "totalPages": "number"
}
```

**错误响应**

| 错误码 | 错误类型 | 说明 |
|--------|----------|------|
| 404 | GROUP_CHAT_NOT_FOUND | 群聊不存在 |
| 403 | NOT_MEMBER | 当前用户不是群成员 |

---

### 8. 标记群聊已读

**请求**

| 属性 | 值 |
|------|-----|
| HTTP 方法 | POST |
| 端点 | `/api/me/group-chats/{groupChatId}/read` |

**路径参数**

| 参数 | 类型 | 说明 |
|------|------|------|
| groupChatId | Long | 群聊ID |

**成功响应** (HTTP 200)

无响应体

**错误响应**

| 错误码 | 错误类型 | 说明 |
|--------|----------|------|
| 404 | GROUP_CHAT_NOT_FOUND | 群聊不存在 |
| 403 | NOT_MEMBER | 当前用户不是群成员 |

---

### 9. 添加群成员

**请求**

| 属性 | 值 |
|------|-----|
| HTTP 方法 | POST |
| 端点 | `/api/me/group-chats/{groupChatId}/members` |

**路径参数**

| 参数 | 类型 | 说明 |
|------|------|------|
| groupChatId | Long | 群聊ID |

**请求体**

```json
{
  "userIds": ["UUID (要添加的用户ID列表)"]
}
```

**成功响应** (HTTP 200)

```json
{
  "addedUserIds": ["string (成功添加的用户ID列表)"],
  "alreadyMemberUserIds": ["string (已是成员的用户ID列表)"]
}
```

**错误响应**

| 错误码 | 错误类型 | 说明 |
|--------|----------|------|
| 404 | GROUP_CHAT_NOT_FOUND | 群聊不存在 |
| 403 | NOT_ADMIN | 当前用户不是管理员 |
| 403 | MAX_MEMBERS_EXCEEDED | 添加成员后将超过最大成员数 |

---

### 10. 移除群成员

**请求**

| 属性 | 值 |
|------|-----|
| HTTP 方法 | DELETE |
| 端点 | `/api/me/group-chats/{groupChatId}/members/{userId}` |

**路径参数**

| 参数 | 类型 | 说明 |
|------|------|------|
| groupChatId | Long | 群聊ID |
| userId | UUID | 要移除的用户ID |

**成功响应** (HTTP 200)

```json
{
  "userId": "string",
  "success": "boolean"
}
```

**错误响应**

| 错误码 | 错误类型 | 说明 |
|--------|----------|------|
| 404 | GROUP_CHAT_NOT_FOUND | 群聊不存在 |
| 404 | MEMBER_NOT_FOUND | 成员不存在 |
| 403 | NOT_ADMIN | 当前用户不是管理员（非自己退出时） |
| 403 | LAST_ADMIN_CANNOT_LEAVE | 不能移除最后一个管理员 |

---

## 数据模型

### GroupChat（群聊实体）

| 字段 | 类型 | 约束 | 说明 |
|------|------|------|------|
| id | Long | PRIMARY KEY | 群聊ID |
| name | String | NOT NULL, max 100 | 群名称 |
| description | String | max 500 | 群描述 |
| creatorId | UUID | NOT NULL | 创建者ID |
| relatedPostUuid | UUID | - | 关联帖子UUID |
| status | String | NOT NULL, default 'ACTIVE' | 状态: ACTIVE/CLOSED |
| maxMembers | Integer | NOT NULL, default 20 | 最大成员数 |
| createdAt | Timestamp | NOT NULL | 创建时间 |
| updatedAt | Timestamp | NOT NULL | 更新时间 |
| lastMessageAt | Timestamp | - | 最后消息时间 |

### GroupChatMember（群成员实体）

| 字段 | 类型 | 约束 | 说明 |
|------|------|------|------|
| id | Long | PRIMARY KEY | 成员记录ID |
| groupChatId | Long | NOT NULL, FOREIGN KEY | 所属群聊ID |
| userId | UUID | NOT NULL | 用户ID |
| role | String | NOT NULL, default 'MEMBER' | 角色: ADMIN/MEMBER |
| status | String | NOT NULL, default 'JOINED' | 状态: JOINED/LEFT |
| joinedAt | Timestamp | NOT NULL | 加入时间 |
| lastReadMessageId | Long | - | 最后已读消息ID |
| leftAt | Timestamp | - | 离开时间 |

### GroupChatMessage（群消息实体）

| 字段 | 类型 | 约束 | 说明 |
|------|------|------|------|
| id | Long | PRIMARY KEY | 消息ID |
| groupChatId | Long | NOT NULL, FOREIGN KEY | 所属群聊ID |
| senderId | UUID | NOT NULL | 发送者ID |
| messageType | String | NOT NULL | 消息类型 |
| content | String | max 1000 | 消息内容 |
| createdAt | Timestamp | NOT NULL | 发送时间 |

---

## 权限说明

| 操作 | 所需权限 |
|------|----------|
| 创建群聊 | 已验证用户 |
| 查看群聊列表 | 仅查看自己加入的群聊 |
| 查看群聊详情 | 群成员 |
| 更新群聊信息 | 群管理员 |
| 关闭群聊 | 群管理员 |
| 发送消息 | 群成员 |
| 查看消息 | 群成员 |
| 标记已读 | 群成员 |
| 添加成员 | 群管理员 |
| 移除成员 | 群管理员（或自己退出） |

---

## 错误响应格式

所有错误响应统一格式：

```json
{
  "code": "string (错误码)",
  "message": "string (错误信息)",
  "details": "object (可选，详细信息)",
  "timestamp": "string (ISO时间戳)",
  "traceId": "string (追踪ID)"
}
```
