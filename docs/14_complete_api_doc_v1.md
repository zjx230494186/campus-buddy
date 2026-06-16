# 校园伙伴后端 API 接口文档

## 目录

1. [认证模块 (Auth)](#1-认证模块-auth)
2. [联系人模块 (Contact)](#2-联系人模块-contact)
3. [帖子模块 (Post)](#3-帖子模块-post)
4. [评价模块 (Review)](#4-评价模块-review)
5. [健康检查 (Health)](#5-健康检查-health)
6. [系统信息 (System)](#6-系统信息-system)

---

## 1. 认证模块 (Auth)

### 1.1 用户登录

| 属性 | 值 |
|------|-----|
| **路径** | `POST /api/auth/login` |
| **认证** | 无需认证 |
| **描述** | 用户登录接口 |

### 1.2 用户注册

| 属性 | 值 |
|------|-----|
| **路径** | `POST /api/auth/register` |
| **认证** | 无需认证 |
| **描述** | 用户注册接口 |

### 1.3 校园邮箱验证码

| 属性 | 值 |
|------|-----|
| **路径** | `POST /api/auth/campus-email/verification-codes` |
| **认证** | 无需认证 |
| **描述** | 发送校园邮箱验证码 |

| 属性 | 值 |
|------|-----|
| **路径** | `POST /api/auth/campus-email/verifications` |
| **认证** | 无需认证 |
| **描述** | 验证校园邮箱验证码 |

### 1.4 身份认证提交

| 属性 | 值 |
|------|-----|
| **路径** | `POST /api/auth/identity-verifications` |
| **认证** | 需要 JWT |
| **描述** | 提交身份认证申请 |

| 属性 | 值 |
|------|-----|
| **路径** | `GET /api/auth/identity-verifications/me` |
| **认证** | 需要 JWT |
| **描述** | 获取当前用户身份认证状态 |

### 1.5 身份认证材料

| 属性 | 值 |
|------|-----|
| **路径** | `POST /api/auth/identity-verifications/materials` |
| **认证** | 需要 JWT |
| **描述** | 上传身份认证材料（图片） |

| 属性 | 值 |
|------|-----|
| **路径** | `DELETE /api/auth/identity-verifications/materials/{attachmentId}` |
| **认证** | 需要 JWT |
| **描述** | 删除身份认证材料 |

### 1.6 身份认证管理（管理员）

| 属性 | 值 |
|------|-----|
| **路径** | `GET /api/admin/identity-verifications` |
| **认证** | 需要 JWT（管理员权限） |
| **描述** | 列出待审核的身份认证申请 |

| 属性 | 值 |
|------|-----|
| **路径** | `POST /api/admin/identity-verifications/{submissionId}/reviews` |
| **认证** | 需要 JWT（管理员权限） |
| **描述** | 审核身份认证申请 |

| 属性 | 值 |
|------|-----|
| **路径** | `GET /api/admin/identity-verifications/{submissionId}/material` |
| **认证** | 需要 JWT（管理员权限） |
| **描述** | 获取认证材料文件 |

---

## 2. 联系人模块 (Contact)

### 2.1 联系卡片

| 属性 | 值 |
|------|-----|
| **路径** | `GET /api/me/contact-card` |
| **认证** | 需要 JWT |
| **描述** | 获取当前用户联系卡片 |

| 属性 | 值 |
|------|-----|
| **路径** | `PUT /api/me/contact-card` |
| **认证** | 需要 JWT |
| **描述** | 更新联系卡片（微信、电话、QQ） |

### 2.2 联系请求

| 属性 | 值 |
|------|-----|
| **路径** | `POST /api/partner-posts/{postId}/contact-requests` |
| **认证** | 需要 JWT |
| **描述** | 向帖子发布者发起联系请求 |

### 2.3 私信会话

| 属性 | 值 |
|------|-----|
| **路径** | `GET /api/me/conversations` |
| **认证** | 需要 JWT |
| **描述** | 获取当前用户的会话列表 |

| 属性 | 值 |
|------|-----|
| **路径** | `GET /api/me/conversations/{conversationId}/messages` |
| **认证** | 需要 JWT |
| **描述** | 获取会话消息列表 |

| 属性 | 值 |
|------|-----|
| **路径** | `POST /api/me/conversations/{conversationId}/messages` |
| **认证** | 需要 JWT |
| **描述** | 发送会话消息 |

| 属性 | 值 |
|------|-----|
| **路径** | `POST /api/me/conversations/{conversationId}/close` |
| **认证** | 需要 JWT |
| **描述** | 关闭会话 |

| 属性 | 值 |
|------|-----|
| **路径** | `GET /api/me/conversations/{conversationId}/can-close` |
| **认证** | 需要 JWT |
| **描述** | 查询是否可以关闭会话 |

| 属性 | 值 |
|------|-----|
| **路径** | `POST /api/me/conversations/{conversationId}/read` |
| **认证** | 需要 JWT |
| **描述** | 标记会话已读 |

### 2.4 联系方式解锁

| 属性 | 值 |
|------|-----|
| **路径** | `GET /api/me/conversations/{conversationId}/contact-unlock` |
| **认证** | 需要 JWT |
| **描述** | 获取联系方式解锁状态 |

| 属性 | 值 |
|------|-----|
| **路径** | `POST /api/me/conversations/{conversationId}/contact-unlock/confirm` |
| **认证** | 需要 JWT |
| **描述** | 确认解锁联系方式 |

| 属性 | 值 |
|------|-----|
| **路径** | `GET /api/me/conversations/{conversationId}/peer-contact-card` |
| **认证** | 需要 JWT |
| **描述** | 获取对方联系卡片 |

### 2.5 群聊管理

| 属性 | 值 |
|------|-----|
| **路径** | `POST /api/me/group-chats` |
| **认证** | 需要 JWT |
| **描述** | 创建群聊 |

| 属性 | 值 |
|------|-----|
| **路径** | `GET /api/me/group-chats` |
| **认证** | 需要 JWT |
| **描述** | 获取当前用户的群聊列表 |

| 属性 | 值 |
|------|-----|
| **路径** | `GET /api/me/group-chats/{groupChatId}` |
| **认证** | 需要 JWT |
| **描述** | 获取群聊详情（含成员列表） |

| 属性 | 值 |
|------|-----|
| **路径** | `PUT /api/me/group-chats/{groupChatId}` |
| **认证** | 需要 JWT（管理员权限） |
| **描述** | 更新群聊信息 |

| 属性 | 值 |
|------|-----|
| **路径** | `DELETE /api/me/group-chats/{groupChatId}` |
| **认证** | 需要 JWT（管理员权限） |
| **描述** | 关闭群聊 |

### 2.6 群消息管理

| 属性 | 值 |
|------|-----|
| **路径** | `POST /api/me/group-chats/{groupChatId}/messages` |
| **认证** | 需要 JWT（群成员） |
| **描述** | 发送群消息 |

| 属性 | 值 |
|------|-----|
| **路径** | `GET /api/me/group-chats/{groupChatId}/messages` |
| **认证** | 需要 JWT（群成员） |
| **描述** | 获取群消息列表 |

| 属性 | 值 |
|------|-----|
| **路径** | `POST /api/me/group-chats/{groupChatId}/read` |
| **认证** | 需要 JWT（群成员） |
| **描述** | 标记群聊已读 |

### 2.7 群成员管理

| 属性 | 值 |
|------|-----|
| **路径** | `POST /api/me/group-chats/{groupChatId}/members` |
| **认证** | 需要 JWT（管理员权限） |
| **描述** | 添加群成员 |

| 属性 | 值 |
|------|-----|
| **路径** | `DELETE /api/me/group-chats/{groupChatId}/members/{userId}` |
| **认证** | 需要 JWT（管理员或自己退出） |
| **描述** | 移除群成员 / 退出群聊 |

---

## 3. 帖子模块 (Post)

### 3.1 帖子广场

| 属性 | 值 |
|------|-----|
| **路径** | `GET /api/partner-posts` |
| **认证** | 需要 JWT |
| **描述** | 获取帖子广场列表（可筛选场景类型、关键词） |

| 属性 | 值 |
|------|-----|
| **路径** | `GET /api/partner-posts/{postId}` |
| **认证** | 需要 JWT |
| **描述** | 获取帖子详情 |

### 3.2 我的帖子

| 属性 | 值 |
|------|-----|
| **路径** | `POST /api/me/partner-posts` |
| **认证** | 需要 JWT |
| **描述** | 创建帖子草稿 |

| 属性 | 值 |
|------|-----|
| **路径** | `PUT /api/me/partner-posts/{postId}` |
| **认证** | 需要 JWT |
| **描述** | 更新帖子草稿 |

| 属性 | 值 |
|------|-----|
| **路径** | `GET /api/me/partner-posts` |
| **认证** | 需要 JWT |
| **描述** | 获取我发布的帖子列表 |

| 属性 | 值 |
|------|-----|
| **路径** | `GET /api/me/partner-posts/{postId}` |
| **认证** | 需要 JWT |
| **描述** | 获取我的帖子详情 |

| 属性 | 值 |
|------|-----|
| **路径** | `POST /api/me/partner-posts/{postId}/submit-review` |
| **认证** | 需要 JWT |
| **描述** | 提交审核 |

| 属性 | 值 |
|------|-----|
| **路径** | `POST /api/me/partner-posts/{postId}/withdraw-review` |
| **认证** | 需要 JWT |
| **描述** | 撤回审核 |

| 属性 | 值 |
|------|-----|
| **路径** | `POST /api/me/partner-posts/{postId}/unpublish` |
| **认证** | 需要 JWT |
| **描述** | 下架帖子 |

### 3.3 帖子管理（管理员）

| 属性 | 值 |
|------|-----|
| **路径** | `GET /api/admin/partner-posts/review-queue` |
| **认证** | 需要 JWT（管理员权限） |
| **描述** | 获取待审核帖子队列 |

| 属性 | 值 |
|------|-----|
| **路径** | `GET /api/admin/partner-posts/{postId}` |
| **认证** | 需要 JWT（管理员权限） |
| **描述** | 获取帖子详情（管理员视角） |

| 属性 | 值 |
|------|-----|
| **路径** | `POST /api/admin/partner-posts/{postId}/review` |
| **认证** | 需要 JWT（管理员权限） |
| **描述** | 审核帖子 |

---

## 4. 评价模块 (Review)

### 4.1 评价管理

| 属性 | 值 |
|------|-----|
| **路径** | `POST /api/me/reviews` |
| **认证** | 需要 JWT |
| **描述** | 创建评价 |

| 属性 | 值 |
|------|-----|
| **路径** | `PUT /api/me/reviews/{reviewId}` |
| **认证** | 需要 JWT |
| **描述** | 更新评价 |

| 属性 | 值 |
|------|-----|
| **路径** | `GET /api/me/reviews/given` |
| **认证** | 需要 JWT |
| **描述** | 获取我发出的评价列表 |

| 属性 | 值 |
|------|-----|
| **路径** | `GET /api/me/reviews/received` |
| **认证** | 需要 JWT |
| **描述** | 获取我收到的评价列表 |

### 4.2 信誉积分

| 属性 | 值 |
|------|-----|
| **路径** | `GET /api/users/{userId}/credit-summary` |
| **认证** | 需要 JWT |
| **描述** | 获取用户公开信誉积分 |

| 属性 | 值 |
|------|-----|
| **路径** | `GET /api/me/credit-summary` |
| **认证** | 需要 JWT |
| **描述** | 获取我的完整信誉积分 |

---

## 5. 健康检查 (Health)

| 属性 | 值 |
|------|-----|
| **路径** | `GET /api/health` |
| **认证** | 无需认证 |
| **描述** | 健康检查接口 |

---

## 6. 系统信息 (System)

| 属性 | 值 |
|------|-----|
| **路径** | `GET /api/system/info` |
| **认证** | 无需认证 |
| **描述** | 获取系统信息（版本号等） |

---

## API 总览表

| 模块 | 接口数量 | 认证要求 |
|------|----------|----------|
| 认证 | 10 | 部分需要 |
| 联系人 | 16 | 需要 |
| 帖子 | 10 | 部分需要 |
| 评价 | 5 | 需要 |
| 健康检查 | 1 | 无需 |
| 系统信息 | 1 | 无需 |
| **总计** | **43** | - |

---

## 错误响应格式

所有 API 返回的错误响应统一格式：

```json
{
  "timestamp": "2024-01-01T12:00:00",
  "status": 404,
  "error": "Not Found",
  "message": "Resource not found",
  "path": "/api/me/group-chats"
}
```

## 认证方式

- **JWT Token**: 在请求头中携带 `Authorization: Bearer <token>`
- **匿名接口**: 无需认证的接口可直接访问

---

*文档生成时间: 2026-06-16*
