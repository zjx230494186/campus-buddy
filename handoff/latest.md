# Latest Handoff

## 2026-05-25 真实邮箱验证码 SMTP 能力完成

### 本轮完成

- 目标：补齐“真实邮箱收发验证码并注册”的后端发信能力。
- 保留现有验证码生成、哈希存储、校验、ticket 和注册消费流程。
- 新增 `spring-boot-starter-mail`。
- 新增 `campus-buddy.campus-email.delivery-mode`：
  - 默认 `noop`，本地/测试不需要 SMTP 凭据。
  - 设置为 `smtp` 后启用真实邮件发送。
- `NoopCampusEmailVerificationCodeSender` 改为仅在 `delivery-mode=noop` 或未配置时启用。
- 新增 `SmtpCampusEmailVerificationCodeSender`，通过 SMTP 发送验证码邮件。
- 新增 `MailSenderConfiguration`，根据 `campus-buddy.campus-email.smtp.*` 构造 `JavaMailSender`。
- local/local-h2/deploy profile 已预留 SMTP 环境变量模板。
- test profile 固定 `delivery-mode=noop`，避免自动化测试依赖外部邮箱。
- 新增 `SmtpCampusEmailVerificationCodeSenderTest`。
- Validation 留档：`docs/validation/20260525_real_email_registration_smtp_record.md`

### 验证结果

- 目标测试：
  - 命令：`.\mvnw.cmd "-Dtest=SmtpCampusEmailVerificationCodeSenderTest,CampusEmailVerificationEndpointTest" test`
  - 结果：11/11 PASS。
- 后端全量测试：
  - 命令：`.\mvnw.cmd test`
  - 结果：251/251 PASS。

### 边界确认

- 未修改 Flyway migration。
- 未修改 `deploy/**` 脚本。
- 未修改 Qt UI。
- 未写入真实 SMTP 用户名、密码、邮箱授权码、token、OBS AK/SK 或数据库密码。
- 文档只记录变量名和占位符。
- 仓库中仍有前置未跟踪 `deploy/*.sh`、`tmp_*`、历史 prompt/doc、Qt 功能演示截图等文件；本轮不纳入提交边界。

### 真实收信状态

- 代码能力已具备。
- 本轮没有真实外部 SMTP 收信验证。
- 阻塞原因：当前线程没有私有 SMTP 凭据；按项目安全边界，凭据不得写进聊天、仓库或文档。

### 需要配置的私有环境变量

变量名可记录，真实值不能写入聊天或项目文档：

- `CAMPUS_EMAIL_DELIVERY_MODE=smtp`
- `CAMPUS_EMAIL_ALLOWED_DOMAIN`
- `CAMPUS_EMAIL_SMTP_HOST`
- `CAMPUS_EMAIL_SMTP_PORT`
- `CAMPUS_EMAIL_SMTP_USERNAME`
- `CAMPUS_EMAIL_SMTP_PASSWORD`
- `CAMPUS_EMAIL_SMTP_FROM`
- `CAMPUS_EMAIL_SMTP_FROM_NAME`
- `CAMPUS_EMAIL_SMTP_AUTH`
- `CAMPUS_EMAIL_SMTP_START_TLS`
- `CAMPUS_EMAIL_SMTP_SSL`

### 当前代码基线

- SMTP 邮件发送提交：待当前线程提交后以最终回复为准。
- 后端：全量测试 251/251 通过。
- Qt：本轮未修改，上一批 UI polish 基线仍为 `e25085e`。

### 下一步候选

1. **真实 SMTP 收信 smoke** — 建议复用当前后端验证线程；在项目目录外私有 env 或服务器 `/etc/campus-buddy/backend.env` 注入 SMTP 变量，启动后端，调用发送验证码接口，实际邮箱收信后完成注册。
2. **Qt 注册真实链路演示** — 建议在 SMTP smoke 通过后执行；打开 Qt，发送验证码、读取邮箱验证码、校验验证码、注册、登录。
3. **答辩演示前最终验证** — 建议新开演示/验收线程；在 smoke 账号和 SMTP 都就绪后跑完整演示链路并截图。

### 建议下一线程名称

`真实 SMTP 收信 smoke 与 Qt 注册链路演示`

### 可复制启动 Prompt

```text
请读取 D:\big_homework\AGENTS.md、D:\big_homework\docs\03_current_plan.md、D:\big_homework\handoff\latest.md、D:\big_homework\docs\validation\20260525_real_email_registration_smtp_record.md。

当前任务是验证真实 SMTP 收信与 Qt 注册链路。不要把 SMTP 用户名、密码、邮箱授权码、验证码、token 或真实账号密码写入聊天或项目文档。

范围：
- 可运行后端和 Qt 客户端。
- 可读取项目目录外的私有 env 文件，前提是我明确给出路径。
- 可写 validation 和 handoff。
- 不改 Flyway、不改 deploy 脚本、不扩大业务功能。

先检查以下环境变量是否存在，只输出 present/missing，不输出真实值：
- CAMPUS_EMAIL_DELIVERY_MODE
- CAMPUS_EMAIL_ALLOWED_DOMAIN
- CAMPUS_EMAIL_SMTP_HOST
- CAMPUS_EMAIL_SMTP_PORT
- CAMPUS_EMAIL_SMTP_USERNAME
- CAMPUS_EMAIL_SMTP_PASSWORD
- CAMPUS_EMAIL_SMTP_FROM
- CAMPUS_EMAIL_SMTP_FROM_NAME
- CAMPUS_EMAIL_SMTP_AUTH
- CAMPUS_EMAIL_SMTP_START_TLS
- CAMPUS_EMAIL_SMTP_SSL

若变量齐全：
1. 启动后端。
2. 调用 /api/auth/campus-email/verification-codes 发送验证码。
3. 等我在邮箱中确认收到验证码。
4. 使用验证码完成 /api/auth/campus-email/verifications 和 /api/auth/register，或通过 Qt 注册页完成。
5. 输出 validation：命令、结果、阻塞、敏感信息检查、未覆盖风险。
```
