# Real Email Registration Smoke Success Record

日期：2026-05-25

## 1. 本轮目标

验证真实 SMTP 发信、真实邮箱收信、验证码校验、注册和注册后登录。

## 2. 前置条件

- 私有 SMTP 服务配置文件已由用户创建在项目目录外：
  - `D:\big_homework_private\smtp-service.env`
- 文件内 SMTP 服务变量完整。
- 允许接收验证码的邮箱域名配置为：
  - `bjtu.edu.cn`
- 本轮测试收件邮箱：
  - `24******@bjtu.edu.cn`

## 3. 执行命令

发送验证码：

```powershell
cd D:\big_homework
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\real_email_registration_smoke.ps1 -EnvFile D:\big_homework_private\smtp-service.env -StartBackend -SendOnly
```

完成验证码校验、注册和登录：

```powershell
cd D:\big_homework
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\real_email_registration_smoke.ps1 -EnvFile D:\big_homework_private\smtp-service.env -CompleteRegistration
```

## 4. 结果

发送验证码：

- SMTP 服务配置文件存在。
- 必需 SMTP 变量全部 present。
- 本地 `local-h2` 后端启动成功。
- `/api/health` 返回 `UP`。
- 验证码发送请求返回成功。
- 用户确认真实邮箱收到验证码。

完成注册：

```text
private_env=D:\big_homework_private\smtp-service.env
CAMPUS_EMAIL_DELIVERY_MODE=present
CAMPUS_EMAIL_ALLOWED_DOMAIN=present
CAMPUS_EMAIL_SMTP_HOST=present
CAMPUS_EMAIL_SMTP_PORT=present
CAMPUS_EMAIL_SMTP_USERNAME=present
CAMPUS_EMAIL_SMTP_PASSWORD=present
CAMPUS_EMAIL_SMTP_FROM=present
CAMPUS_EMAIL_SMTP_FROM_NAME=present
CAMPUS_EMAIL_SMTP_AUTH=present
CAMPUS_EMAIL_SMTP_START_TLS=present
CAMPUS_EMAIL_SMTP_SSL=present
CAMPUS_BUDDY_REAL_REGISTER_EMAIL=present
CAMPUS_BUDDY_REAL_REGISTER_PASSWORD=present
CAMPUS_BUDDY_REAL_REGISTER_DISPLAY_NAME=present
CAMPUS_BUDDY_REAL_REGISTER_CODE=present
backend=already-running
verify_code=ok
register=ok
login=ok
```

## 5. 结论

- 真实 SMTP 验证码发信通过。
- 真实邮箱收信通过。
- 验证码校验通过。
- 使用真实验证码完成注册通过。
- 注册后登录通过。

## 6. 边界与残余风险

- 本轮使用 `local-h2` 本地后端验证；数据不持久化到生产数据库。
- 本轮没有修改 Flyway migration。
- 本轮没有修改 deploy 脚本。
- 本轮没有修改 Qt UI。
- 下一步若要演示 Qt 桌面端注册页，需要用一个尚未注册过的新 `bjtu.edu.cn` 邮箱重新走发送验证码、手动输入验证码、注册和登录。

## 7. 敏感信息处理

- 未将 SMTP 用户名、授权码、注册密码、验证码或 access token 写入本文档。
- 私有 SMTP 配置仍位于项目目录外。
- 本文档只记录变量 present/missing 和阶段结果。
