# 2026-06-17 后端群聊热修复审查、冒烟与上线记录

## 范围

- 审查并上线后端远端主分支提交：`95709e9 liuyi 修复多人群聊功能`。
- 重点关注：
  - V12 会话关闭字段迁移。
  - V13 群聊三张表迁移。
  - V14 群聊外键修复迁移。
  - 最新 jar 在 PostgreSQL 上的启动、迁移和真实服务冒烟。

## 本地验证

- `.\mvnw.cmd -DskipTests package`
  - 结果：通过。
- `.\mvnw.cmd "-Dtest=PostModerationServiceTest,OpenAiCompatiblePostModerationClientTest" test`
  - 结果：通过。
  - 测试数：10，失败：0，错误：0。

当前 Windows 环境没有可用 Docker，因此未在本地运行 Testcontainers 相关回归。

## 上线前服务器只读检查

- 公网健康检查：
  - `GET http://114.116.203.78/api/health`
  - 结果：`{"status":"UP"}`。
- systemd 服务：
  - `campus-buddy-backend`
  - 结果：active。
- 上线前生产库 Flyway 状态：
  - 已应用到 V11。
  - V12/V13/V14 尚未应用。
- 上线前群聊表状态：
  - `group_chat`、`group_chat_member`、`group_chat_message` 尚不存在。
  - 因此本服务器不存在历史群聊孤儿数据阻塞 V14 的问题。

## 迁移干跑

在服务器 PostgreSQL 容器内创建临时数据库，按顺序执行 V1 到 V14 所有迁移 SQL。

结果：通过。

确认临时库内存在：

- `conversation`
- `group_chat`
- `group_chat_member`
- `group_chat_message`

临时迁移数据库已删除。

## 最新 Jar 临时库启动冒烟

将最新构建 jar 上传到服务器 `/tmp`，在端口 `18080` 使用临时 PostgreSQL 数据库启动，让 Flyway 从空库迁移到 V14。

检查：

- `GET http://127.0.0.1:18080/api/health`
  - 结果：`{"status":"UP"}`。
- `GET http://127.0.0.1:18080/api/system/info`
  - 结果包含 `serviceName=campus-buddy-backend`。

结果：通过。

临时进程已停止，临时数据库已删除，生产服务端口 `8080` 在该阶段保持运行。

## 正式上线记录

- 上线提交：`95709e9 liuyi 修复多人群聊功能`。
- 构建产物：
  - `campus-buddy-backend-0.0.1-SNAPSHOT.jar`
  - SHA256：`83AFC4C28F95F99844E41BBFABD168FEECAA2DF0165F071DA80F74F9D30E33E4`
- 服务器备份目录：
  - `/srv/campus-buddy/backups/deploy_20260617_175551`
- 备份内容：
  - 上线前 jar 备份。
  - 上线前 PostgreSQL `pg_dump` 备份。
- 上线方式：
  - 上传新 jar 到服务器临时目录。
  - 校验远端 SHA256 与本地构建一致。
  - 替换 `/srv/campus-buddy/campus-buddy-backend-0.0.1-SNAPSHOT.jar`。
  - 重启 `campus-buddy-backend` systemd 服务。

## 上线后验证

- systemd：
  - `campus-buddy-backend` active。
- 公网健康检查：
  - `GET http://114.116.203.78/api/health`
  - 结果：`{"status":"UP"}`。
- 公网系统信息：
  - `GET http://114.116.203.78/api/system/info`
  - 结果包含 `serviceName=campus-buddy-backend` 和 `version=0.0.1-SNAPSHOT`。
- 未登录访问安全接口：
  - `GET /api/probe/secure`
  - 结果：401。
- 未登录访问群聊列表：
  - `GET /api/me/group-chats?page=0&size=5`
  - 结果：401。
- 已认证用户短时 JWT 冒烟：
  - `GET /api/probe/secure`
  - 结果：200，`authenticated=true`。
  - `GET /api/me/credit-summary`
  - 结果：200。
  - `GET /api/me/group-chats?page=0&size=5`
  - 结果：200，响应包含 `items/page/size/totalElements/totalPages`。
- Flyway：
  - V12、V13、V14 均成功应用。
  - 当前生产库版本：V14。
- 群聊表：
  - `group_chat`
  - `group_chat_member`
  - `group_chat_message`
- 群聊外键孤儿数据：
  - `group_chat.creator_user_id` 孤儿数：0。
  - `group_chat.related_post_id` 孤儿数：0。
  - `group_chat_member.user_id` 孤儿数：0。
  - `group_chat_message.sender_user_id` 孤儿数：0。
- 上线后日志：
  - 未发现新的 `ERROR`、异常堆栈或 Flyway 失败。

## 已知残余风险

- 本轮上线后只做了群聊读取接口的鉴权冒烟，没有在生产环境创建真实群聊、发送群聊消息或增删成员，以避免污染生产数据。
- `GroupChatController` 仍直接解析 UUID 字符串；如果移动端传入格式错误的 UUID，可能需要后续补更友好的 400 校验。
- 服务器上的旧 `/srv/campus-buddy/api_smoke_test.sh` 登录步骤仍失败，疑似脚本内旧测试账号不可用；本轮已用短时 JWT 完成不依赖测试账号密码的鉴权冒烟，但后续建议更新正式 smoke 脚本的测试账号来源。

## 结论

后端最新主分支提交已上线到华为云服务器。服务启动、Flyway V12-V14 迁移、公开接口、鉴权保护、已认证用户读取接口和群聊列表接口均通过最小冒烟。当前状态可以继续给 Qt 端和移动端联调使用。
