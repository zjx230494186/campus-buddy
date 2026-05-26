# Server SMTP Bjtu Deploy Record

日期：2026-05-26

## 1. 目标

将真实校园邮箱验证码发送能力上线到华为云服务器，使公网注册验证码接口：

- 只允许 `bjtu.edu.cn` 域名邮箱请求验证码。
- 使用服务器私有 SMTP 配置真实发送验证码邮件。
- 不把 SMTP 授权码、服务器私钥、数据库密码、OBS AK/SK、JWT secret、验证码或 token 写入仓库和文档。

## 2. 前置发现

上线前公网接口验证结果：

```text
non_bjtu_reject_check=INVALID_CAMPUS_EMAIL_DOMAIN
bjtu_send_check=INVALID_CAMPUS_EMAIL_DOMAIN
```

服务器 `/etc/campus-buddy/backend.env` 中邮件相关变量缺失，且服务器旧 jar 只包含 `NoopCampusEmailVerificationCodeSender`，不包含 SMTP sender。

## 3. 配置上线

使用本机项目目录外私有配置：

```text
D:\big_homework_private\smtp-service.env
```

同步到服务器私有配置：

```text
/etc/campus-buddy/backend.env
```

服务器配置写入前已备份：

```text
/etc/campus-buddy/backend.env.backup.20260526_222031
```

写入后变量存在性检查：

```text
allowed_domain=bjtu.edu.cn
CAMPUS_EMAIL_DELIVERY_MODE=present
CAMPUS_EMAIL_SMTP_HOST=present
CAMPUS_EMAIL_SMTP_PORT=present
CAMPUS_EMAIL_SMTP_USERNAME=present
CAMPUS_EMAIL_SMTP_PASSWORD=present
CAMPUS_EMAIL_SMTP_FROM=present
CAMPUS_EMAIL_SMTP_FROM_NAME=present
CAMPUS_EMAIL_SMTP_AUTH=present
CAMPUS_EMAIL_SMTP_START_TLS=present
CAMPUS_EMAIL_SMTP_SSL=present
```

## 4. 后端 jar 上线

本地构建：

```powershell
cd D:\big_homework\backend
.\mvnw.cmd -DskipTests package
```

本地 jar 内容检查确认包含：

```text
BOOT-INF/classes/com/campusbuddy/auth/SmtpCampusEmailVerificationCodeSender.class
BOOT-INF/classes/com/campusbuddy/config/MailSenderConfiguration.class
BOOT-INF/classes/application-deploy.properties
```

服务器旧 jar 替换前已备份：

```text
/srv/campus-buddy/campus-buddy-backend-0.0.1-SNAPSHOT.jar.backup.20260526_222339
```

上线后服务状态：

```text
systemd service=active
GET http://114.116.203.78/api/health
{"status":"UP"}
```

## 5. 验证结果

非白名单域名验证：

```text
POST http://114.116.203.78/api/auth/campus-email/verification-codes
campusEmail=codex-domain-check@example.com

status=400
code=INVALID_CAMPUS_EMAIL_DOMAIN
```

`bjtu.edu.cn` 域名发信验证：

```text
POST http://114.116.203.78/api/auth/campus-email/verification-codes
campusEmail=24******@bjtu.edu.cn

status=200
verificationStatus=CODE_SENT
expiresInSeconds=600
resendAfterSeconds=60
```

说明：后端 SMTP sender 在发送成功后才返回 `CODE_SENT`；如果 SMTP 认证或投递调用失败，请求会失败而不是静默成功。

初始账号回归：

```text
smoketest@campus.edu.cn -> role=STUDENT, authenticationStatus=VERIFIED
smokeadmin@campus.edu.cn -> role=ADMIN, authenticationStatus=VERIFIED
```

## 6. 测试说明

通过：

```powershell
.\mvnw.cmd "-Dtest=SmtpCampusEmailVerificationCodeSenderTest" test
```

结果：

```text
Tests run: 2, Failures: 0, Errors: 0, Skipped: 0
```

未通过但原因明确：

```powershell
.\mvnw.cmd "-Dtest=SmtpCampusEmailVerificationCodeSenderTest,CampusEmailVerificationEndpointTest" test
```

`CampusEmailVerificationEndpointTest` 需要 Docker/Testcontainers，本机当前 Docker 不可用，因此该组合测试在本机失败；这不是 SMTP sender 单测失败。

## 7. 边界与风险

- 本轮没有修改 Flyway migration。
- 本轮没有修改 Qt 客户端。
- 本轮没有把任何 SMTP 授权码、服务器 SSH 私钥、数据库密码、OBS AK/SK、JWT secret、验证码或 token 写入仓库、文档或聊天。
- 公网仍为 HTTP，正式外发前建议补 HTTPS。
- 已触发一次向测试邮箱的验证码发送请求；验证码内容不记录。
