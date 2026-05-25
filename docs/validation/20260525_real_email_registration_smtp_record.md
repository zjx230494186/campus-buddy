# Real Email Registration SMTP Record

日期：2026-05-25

## 1. 本轮目标

补齐“真实邮箱收发验证码并注册”的后端发送能力。

本轮范围：

- 后端 SMTP 邮件发送适配。
- 后端配置项与测试。
- validation、current plan、handoff 和配置矩阵文档。

本轮不修改：

- Flyway migration。
- `deploy/**` 脚本。
- Qt UI。
- 服务器真实私有配置。
- 任何真实邮箱密码、授权码、token 或账号明文。

## 2. 实际修改文件

- `backend/pom.xml`
- `backend/src/main/java/com/campusbuddy/auth/NoopCampusEmailVerificationCodeSender.java`
- `backend/src/main/java/com/campusbuddy/auth/SmtpCampusEmailVerificationCodeSender.java`
- `backend/src/main/java/com/campusbuddy/config/CampusBuddyProperties.java`
- `backend/src/main/java/com/campusbuddy/config/MailSenderConfiguration.java`
- `backend/src/main/resources/application-local.properties`
- `backend/src/main/resources/application-local-h2.properties`
- `backend/src/main/resources/application-test.properties`
- `backend/src/main/resources/application-deploy.properties`
- `backend/src/test/java/com/campusbuddy/auth/SmtpCampusEmailVerificationCodeSenderTest.java`
- `docs/21_backend_configuration_matrix_v1.md`
- `docs/03_current_plan.md`
- `handoff/latest.md`
- `docs/validation/20260525_real_email_registration_smtp_record.md`

## 3. 实现摘要

- 新增 `spring-boot-starter-mail` 依赖。
- 新增 `campus-buddy.campus-email.delivery-mode`：
  - 默认值：`noop`
  - 真实发送：`smtp`
- `NoopCampusEmailVerificationCodeSender` 改为仅在 `delivery-mode=noop` 或未配置时启用。
- 新增 `SmtpCampusEmailVerificationCodeSender`：
  - 在 `delivery-mode=smtp` 时启用。
  - 使用 `JavaMailSender` 发送验证码邮件。
  - 邮件正文包含验证码、用途说明、有效期和安全提示。
  - SMTP 配置缺失时 fail fast，不静默假装发送。
- 新增 `MailSenderConfiguration`：
  - 根据 `campus-buddy.campus-email.smtp.*` 构造 `JavaMailSender`。
  - 支持 auth、STARTTLS、SSL、连接/读/写超时配置。

## 4. 配置项

非敏感配置或环境变量名：

- `CAMPUS_EMAIL_DELIVERY_MODE`
- `CAMPUS_EMAIL_ALLOWED_DOMAIN`
- `CAMPUS_EMAIL_SMTP_HOST`
- `CAMPUS_EMAIL_SMTP_PORT`
- `CAMPUS_EMAIL_SMTP_FROM`
- `CAMPUS_EMAIL_SMTP_FROM_NAME`
- `CAMPUS_EMAIL_SMTP_AUTH`
- `CAMPUS_EMAIL_SMTP_START_TLS`
- `CAMPUS_EMAIL_SMTP_SSL`

敏感配置，不得写入仓库、文档或聊天：

- `CAMPUS_EMAIL_SMTP_USERNAME`
- `CAMPUS_EMAIL_SMTP_PASSWORD`

开启真实发送的最小私有配置示例，只记录变量名，不记录真实值：

```properties
CAMPUS_EMAIL_DELIVERY_MODE=smtp
CAMPUS_EMAIL_ALLOWED_DOMAIN=<allowed recipient domain>
CAMPUS_EMAIL_SMTP_HOST=<smtp host>
CAMPUS_EMAIL_SMTP_PORT=587
CAMPUS_EMAIL_SMTP_USERNAME=<private username>
CAMPUS_EMAIL_SMTP_PASSWORD=<private password or app authorization code>
CAMPUS_EMAIL_SMTP_FROM=<sender email>
CAMPUS_EMAIL_SMTP_FROM_NAME=校园搭子平台
CAMPUS_EMAIL_SMTP_AUTH=true
CAMPUS_EMAIL_SMTP_START_TLS=true
CAMPUS_EMAIL_SMTP_SSL=false
```

## 5. 测试结果

目标测试命令：

```powershell
cd D:\big_homework\backend
.\mvnw.cmd "-Dtest=SmtpCampusEmailVerificationCodeSenderTest,CampusEmailVerificationEndpointTest" test
```

结果：11/11 PASS。

覆盖点：

- SMTP sender 会构造验证码邮件。
- 邮件包含收件人、发件人、主题、验证码、用途和有效期。
- SMTP host 缺失时 fail fast。
- 既有验证码发送/校验接口保持通过。

全量后端测试命令：

```powershell
cd D:\big_homework\backend
.\mvnw.cmd test
```

结果：251/251 PASS。

## 6. 真实邮件验证状态

本轮没有执行真实外部 SMTP 发信。

原因：

- 当前线程没有私有 SMTP 凭据。
- 按项目安全规则，SMTP 用户名、密码或邮箱授权码不得写入聊天、仓库或文档。

代码能力已具备；真实收发验证码需要在本机私有 shell 或服务器 `/etc/campus-buddy/backend.env` 注入 SMTP 环境变量后，再运行注册链路 smoke。

## 7. 敏感信息检查结论

- 未写入真实 SMTP 密码、授权码、邮箱密码、token、OBS AK/SK 或数据库密码。
- 文档只记录变量名和占位符。
- 新增测试中的 `test-password` 是单元测试假值，不是实际凭据。

## 8. Git status 摘要

本轮相关改动集中在后端邮件发送、后端配置、后端测试、validation 和项目交接文档。

仓库仍存在前置未跟踪 `deploy/*.sh`、`tmp_*`、历史 prompt/doc、Qt 功能演示截图等文件。本轮不修改、不暂存、不提交这些前置脏文件。

## 9. 提交哈希

- 提交前基线：`e25085e`
- 本轮提交哈希：见最终回复中的 Git commit 结果。

## 10. 未覆盖风险

- 未用真实 SMTP 服务完成端到端收信验证。
- 默认允许邮箱域名仍为 `campus.edu.cn`；真实演示若使用其它域名，需要通过 `CAMPUS_EMAIL_ALLOWED_DOMAIN` 配置。
- SMTP 服务商差异可能要求 `port/startTls/ssl/auth` 组合调整。
- 部署服务器还需要把 SMTP 环境变量加入 `/etc/campus-buddy/backend.env` 并重启服务。

## 11. 下一步建议

1. 在项目目录外的私有 env 文件或服务器私有 env 中配置 SMTP 变量。
2. 启动后端 deploy/local profile，调用发送验证码接口。
3. 在邮箱中读取验证码，完成 Qt 注册页：发送验证码 -> 校验验证码 -> 注册 -> 登录。
4. 将真实 SMTP smoke 结果另写 validation；不要记录真实验证码或 SMTP 密码。
