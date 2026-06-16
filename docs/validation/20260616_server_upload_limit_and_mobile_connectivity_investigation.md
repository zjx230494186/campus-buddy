# 2026-06-16 服务器上传限制与移动端连通性排查记录

## 现象

- Qt 身份认证页选择约 2.4MB PNG 后上传显示 `Unable to write`。
- 移动端同事反馈进入应用后卡退，并描述为服务器连接失败。

## 根因证据

1. 公网 `GET /api/health` 返回 200 `{"status":"UP"}`。
2. Nginx、`campus-buddy-backend`、PostgreSQL 容器均为 active/running，磁盘使用约 18%。
3. Nginx error log 对 2,497,332 和 2,941,457 字节上传记录：
   - `client intended to send too large body`
   - 对应公网响应为 HTTP 413。
4. Nginx 放宽后，后端日志进一步记录：
   - `MaxUploadSizeExceededException: Maximum upload size exceeded`
5. OBS 日志在同一时段存在 `putObject` HTTP 200，说明 OBS 凭据和写入链路可用。

结论：业务代码允许 10MB，但 Nginx 和 Spring multipart 均保留约 1MB 默认限制，形成两层拒绝。

## 修复

- 服务器 Nginx `server` 配置增加 `client_max_body_size 12m;`。
- `application-deploy.properties` 增加：
  - `spring.servlet.multipart.max-file-size=10MB`
  - `spring.servlet.multipart.max-request-size=12MB`
- 新增 `DeployMultipartConfigurationTest` 防止 deploy 配置回退。
- 重新构建并部署后端 jar，重启 systemd 服务。

## 备份

- Nginx：`/etc/nginx/sites-available/campus-buddy.backup.20260616_001709`
- 后端 jar：`/srv/campus-buddy/campus-buddy-backend-0.0.1-SNAPSHOT.jar.backup.20260616_002017`

## 验证

- 配置测试红灯：未配置时预期 `10MB`，实际为 `null`。
- 配置测试绿灯：1/1 passed。
- jar 构建：`mvnw.cmd -DskipTests package` passed。
- 公网 health：UP。
- 使用空白测试账号上传 2,560,000 字节 PNG：HTTP 200。
- OBS 返回附件元数据，随后 DELETE 清理测试附件：HTTP 204。
- 本机 Docker 不可用，依赖 Testcontainers 的上传端点集成测试未能运行；真实服务器行为验证已通过。

## 移动端判断

- Nginx access log 已收到 `okhttp/4.12.0` 请求，包含登录 200、登录 401 和广场 200，证明移动端能够访问服务器。
- 空白账号登录返回 `authenticationStatus=UNVERIFIED`；随后访问广场按业务规则返回 403 `AUTHENTICATION_STATUS_REQUIRED`。
- 移动端卡退不是服务器整体不可达的证据。高概率客户端没有正确处理 401/403 或错误响应模型，但必须结合 Android Logcat 才能定位具体崩溃代码。
