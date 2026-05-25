# Latest Handoff

## 2026-05-25 真实注册收信 smoke 准备完成但缺私有 SMTP 配置

### 本轮完成

- 延续上一轮 SMTP sender 能力，继续推进“真实邮箱收到验证码，并随后完成注册”。
- 检查当前 shell、用户级和机器级环境变量，SMTP 相关变量全部 missing。
- 检查 `D:\big_homework_private`，当前不存在。
- 新增本地 smoke 辅助脚本：`scripts/real_email_registration_smoke.ps1`
- 新增 validation：`docs/validation/20260525_real_email_registration_smoke_blocked_record.md`
- 更新 `docs/03_current_plan.md`。

### 脚本能力

`scripts/real_email_registration_smoke.ps1` 可以：

- 从项目目录外私有 env 文件加载 SMTP 和注册 smoke 变量。
- 必要时启动 `local-h2` 后端。
- 调用 `/api/auth/campus-email/verification-codes` 发送验证码。
- 收到验证码后，从私有 env 的 `CAMPUS_BUDDY_REAL_REGISTER_CODE` 读取验证码。
- 调用 `/api/auth/campus-email/verifications`、`/api/auth/register`、`/api/auth/login` 完成验证码校验、注册和登录验证。
- 只输出阶段状态，不输出 SMTP 密码、邮箱授权码、验证码、注册密码或 token。

### 当前阻塞

- 真实外部邮箱收信 smoke 尚未通过。
- 阻塞原因：没有私有 SMTP 配置文件，也没有当前 shell 环境变量。
- 默认期望路径：`D:\big_homework_private\smtp.env`

### 需要的私有变量

变量名可以记录，真实值不能写入聊天、仓库或项目文档：

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
- `CAMPUS_BUDDY_REAL_REGISTER_EMAIL`
- `CAMPUS_BUDDY_REAL_REGISTER_PASSWORD`
- `CAMPUS_BUDDY_REAL_REGISTER_DISPLAY_NAME`
- `CAMPUS_BUDDY_REAL_REGISTER_CODE`（收到验证码后临时写入，完成后建议删除）

### 后续命令

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

### 边界确认

- 未修改 Flyway migration。
- 未修改 deploy 脚本。
- 未修改 Qt UI。
- 未写入真实 SMTP 用户名、密码、邮箱授权码、验证码、注册密码、token、OBS AK/SK 或数据库密码。
- 仓库仍有前置未跟踪 `deploy/*.sh`、`tmp_*`、历史 prompt/doc、Qt 功能演示截图等文件；本轮不纳入提交边界。

### 建议下一线程名称

`真实 SMTP 收信 smoke 执行与 Qt 注册演示`

### 可复制启动 Prompt

```text
请读取 D:\big_homework\AGENTS.md、D:\big_homework\docs\03_current_plan.md、D:\big_homework\handoff\latest.md、D:\big_homework\docs\validation\20260525_real_email_registration_smoke_blocked_record.md。

当前任务是执行真实 SMTP 收信 smoke，并在通过后用 Qt 注册页演示注册。不要把 SMTP 用户名、密码、邮箱授权码、验证码、token 或真实账号密码写入聊天、仓库或项目文档。

先确认 D:\big_homework_private\smtp.env 是否存在。只输出变量 present/missing，不输出真实值。

若私有 env 就绪：
1. 运行 scripts\real_email_registration_smoke.ps1 -StartBackend -SendOnly。
2. 等我在邮箱中确认收到验证码，并把验证码写入私有 env 的 CAMPUS_BUDDY_REAL_REGISTER_CODE。
3. 运行 scripts\real_email_registration_smoke.ps1 -CompleteRegistration。
4. 通过后再打开 Qt 客户端，走注册页真实链路演示。
5. 输出 validation：命令、结果、截图路径、敏感信息检查、未覆盖风险。
```
