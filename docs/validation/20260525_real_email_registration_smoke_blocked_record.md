# Real Email Registration Smoke Blocked Record

日期：2026-05-25

## 1. 本轮目标

推进“真实邮箱接收验证码，并随后完成注册”的端到端 smoke。

## 2. 本轮新增

- 新增本地辅助脚本：`scripts/real_email_registration_smoke.ps1`
- 脚本用途：
  - 从项目目录外私有 env 文件加载 SMTP 和注册 smoke 变量。
  - 可启动 `local-h2` 后端。
  - 调用发送验证码接口。
  - 在收到验证码并写入私有 env 后，继续调用验证码校验、注册和登录接口。
  - 输出只包含 present/missing 与阶段结果，不输出 SMTP 密码、邮箱授权码、验证码、注册密码或 access token。

## 3. 需要的私有 env 文件

默认路径：

```text
D:\big_homework_private\smtp.env
```

该文件不得提交到仓库，不得把真实值写入聊天或项目文档。

需要变量：

```properties
CAMPUS_EMAIL_DELIVERY_MODE=smtp
CAMPUS_EMAIL_ALLOWED_DOMAIN=<recipient domain>
CAMPUS_EMAIL_SMTP_HOST=<smtp host>
CAMPUS_EMAIL_SMTP_PORT=587
CAMPUS_EMAIL_SMTP_USERNAME=<private smtp username>
CAMPUS_EMAIL_SMTP_PASSWORD=<private smtp password or app authorization code>
CAMPUS_EMAIL_SMTP_FROM=<sender email>
CAMPUS_EMAIL_SMTP_FROM_NAME=校园搭子平台
CAMPUS_EMAIL_SMTP_AUTH=true
CAMPUS_EMAIL_SMTP_START_TLS=true
CAMPUS_EMAIL_SMTP_SSL=false

CAMPUS_BUDDY_REAL_REGISTER_EMAIL=<recipient email>
CAMPUS_BUDDY_REAL_REGISTER_PASSWORD=<private test account password>
CAMPUS_BUDDY_REAL_REGISTER_DISPLAY_NAME=<display name>
```

收到邮件后，再追加短期验证码变量：

```properties
CAMPUS_BUDDY_REAL_REGISTER_CODE=<received code>
```

验证码短期有效，完成 smoke 后建议从私有 env 中删除。

## 4. 本轮实际验证

检查当前环境变量：

```powershell
CAMPUS_EMAIL_DELIVERY_MODE=missing
CAMPUS_EMAIL_ALLOWED_DOMAIN=missing
CAMPUS_EMAIL_SMTP_HOST=missing
CAMPUS_EMAIL_SMTP_PORT=missing
CAMPUS_EMAIL_SMTP_USERNAME=missing
CAMPUS_EMAIL_SMTP_PASSWORD=missing
CAMPUS_EMAIL_SMTP_FROM=missing
CAMPUS_EMAIL_SMTP_FROM_NAME=missing
CAMPUS_EMAIL_SMTP_AUTH=missing
CAMPUS_EMAIL_SMTP_START_TLS=missing
CAMPUS_EMAIL_SMTP_SSL=missing
```

查找项目目录外私有配置：

```powershell
D:\big_homework_private missing
```

运行脚本缺配置安全失败：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\real_email_registration_smoke.ps1 -EnvFile D:\big_homework_private\smtp.env -SendOnly
```

结果：

```text
Private env file not found: D:\big_homework_private\smtp.env
```

## 5. 当前结论

- SMTP 代码能力已经在上一轮提交中补齐。
- 本轮新增了真实注册 smoke 辅助脚本。
- 当前没有真实外部 SMTP 收信验证通过。
- 阻塞原因：本机没有 `D:\big_homework_private\smtp.env`，当前 shell 也没有 SMTP 环境变量。

## 6. 后续执行命令

私有 env 就绪后，先发送验证码：

```powershell
cd D:\big_homework
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\real_email_registration_smoke.ps1 -EnvFile D:\big_homework_private\smtp.env -StartBackend -SendOnly
```

收到邮箱验证码后，把验证码写入私有 env 的 `CAMPUS_BUDDY_REAL_REGISTER_CODE`，再完成注册和登录验证：

```powershell
cd D:\big_homework
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\real_email_registration_smoke.ps1 -EnvFile D:\big_homework_private\smtp.env -CompleteRegistration
```

## 7. 敏感信息检查

- 本轮没有读取或写入真实 SMTP 用户名、密码、邮箱授权码、验证码、注册密码或 access token。
- 文档只记录变量名、占位符和安全失败结果。
- 脚本日志默认写入项目目录外私有目录。
