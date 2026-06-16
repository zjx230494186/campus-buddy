# Debug Session: group-chat-resource-not-found

**Status**: [OPEN]
**Session ID**: group-chat-resource-not-found
**Created**: 2026-06-16

## Symptom
- 用户在移动端帖子详情页发起"多人群聊"，弹出"Resource not found"错误
- 该错误来自后端 `GlobalExceptionHandler` 中 `NoResourceFoundException` 的处理
- 此前已尝试在 `SecurityConfiguration.java` 中添加 `/api/me/group-chats/**` 的认证规则，但问题仍然存在

## 关键发现

### 后端代码已存在的接口
- `GroupChatController` 上有 `@RequestMapping("/api")` + `@PostMapping("/me/group-chats")` → POST `/api/me/group-chats`
- `SecurityConfiguration` 中已经添加了 `/api/me/group-chats/**` 认证规则

### 真正的根因候选
- **本仓库中没有移动端代码** —— 截图中的"发起多人群聊"页面在仓库中找不到任何对应的调用代码
- 移动端可能：
  1. **在另一个仓库**（不是这个 `campus-buddy` 后端仓库）
  2. **请求路径不对**（例如请求了 `/api/me/groupChat`、`/api/me/group-chats` 之外的不存在路径）
  3. **请求方法不对**（比如用了 PUT/GET/PATCH 而非 POST）
  4. **请求体字段名不对**（如 `maxMember` 而非 `maxMembers`）

## Hypotheses

### H1: 前端请求URL错误
- 前端可能使用了错误的 API 路径
- **状态**: 待验证
- **验证方法**: 需要查看移动端代码

### H2: HTTP方法不匹配
- 前端可能使用了错误的 HTTP 方法
- **状态**: 待验证
- **验证方法**: 抓包或查看移动端代码

### H3: 请求体格式问题
- 前端发送的 JSON 请求体字段名与后端 record 字段名不匹配
- **状态**: 待验证

### H4: 移动端代码不在此仓库
- 移动端代码可能在另一个仓库中
- **状态**: 高度可能
- **验证方法**: 通过 `Glob` 搜索所有 `*.java`/ `*.kt`/ `*.swift`/ `*.dart` 文件均未找到群聊调用代码

### H5: 后端服务没有重启
- 用户修改了后端代码但服务还在运行旧的代码
- **状态**: 可能
- **验证方法**: 检查后端进程是否重启

## 下一步行动
1. **确认后端服务已重启并加载了新配置**
2. **检查移动端代码**（如果可访问）：
   - 确认实际请求的 URL
   - 确认 HTTP 方法
   - 确认请求体字段名
3. **用 curl 或 Postman 直接测试** `POST /api/me/group-chats` 验证后端是否正常
4. **抓包**查看移动端实际发起的请求
