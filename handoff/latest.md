# Latest Handoff

## 2026-05-25 真实邮箱验证码收信与注册 smoke 已通过

### 本轮完成

- 使用项目目录外私有配置 `D:\big_homework_private\smtp-service.env` 启用 QQ SMTP 发信服务。
- 允许收件域名为 `bjtu.edu.cn`。
- 向测试收件邮箱发送验证码。
- 用户确认真实邮箱收到验证码。
- 使用真实验证码完成：
  - `/api/auth/campus-email/verifications`
  - `/api/auth/register`
  - `/api/auth/login`
- 新增 success validation：
  - `docs/validation/20260525_real_email_registration_smoke_success_record.md`
- 更新 `docs/03_current_plan.md`。

### 验证结果

本轮完成注册阶段输出：

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

### 边界确认

- 本轮使用本地 `local-h2` 后端验证。
- 未修改 Flyway migration。
- 未修改 deploy 脚本。
- 未修改 Qt UI。
- 未写入真实 SMTP 用户名、邮箱授权码、验证码、注册密码、token、OBS AK/SK 或数据库密码。
- 仓库仍有前置未跟踪 `deploy/*.sh`、`tmp_*`、历史 prompt/doc、Qt 功能演示截图等文件；本轮不纳入提交边界。

### 当前结论

- 真实 SMTP 验证码发信通过。
- 真实邮箱收信通过。
- 真实验证码校验通过。
- 注册通过。
- 注册后登录通过。

### 后续建议

1. **Qt 桌面端真实注册演示** — 建议复用当前演示线程；使用一个尚未注册过的新 `bjtu.edu.cn` 邮箱，打开 Qt 注册页手动输入验证码，完成 GUI 注册和登录截图。
2. **部署环境 SMTP 启用** — 建议新开部署线程；将同一组 SMTP 服务变量加入服务器私有 `/etc/campus-buddy/backend.env`，重启 systemd 服务并跑公网 smoke。
3. **答辩最终验收** — 建议新开验收线程；串起登录、认证、发布、广场、联系、评价、管理员审核和真实邮箱注册证据。

### 建议下一线程名称

`Qt 桌面端真实邮箱注册演示`

### 可复制启动 Prompt

```text
请读取 D:\big_homework\AGENTS.md、D:\big_homework\docs\03_current_plan.md、D:\big_homework\handoff\latest.md、D:\big_homework\docs\validation\20260525_real_email_registration_smoke_success_record.md。

当前任务是打开 Qt 桌面端，用一个尚未注册过的新 bjtu.edu.cn 邮箱走真实注册页演示。不要把 SMTP 用户名、邮箱授权码、验证码、注册密码、token 或真实联系方式写入聊天、仓库或项目文档。

已确认：后端真实 SMTP 发信、真实邮箱收信、验证码校验、注册、注册后登录已在 local-h2 后端 smoke 通过。

本轮范围：
- 可启动本地后端和 Qt 客户端。
- 可截图 Qt 注册、登录和登录后首页。
- 可写 validation 和 handoff。
- 不改 Flyway、不改 deploy 脚本、不扩大业务功能。

先确认本地后端 health=UP；如未运行，用项目外私有 SMTP 服务配置启动 local-h2 后端。然后打开 Qt 客户端，走注册页：输入新邮箱 -> 发送验证码 -> 等我从邮箱读取验证码 -> 输入验证码 -> 注册 -> 登录。最后输出 validation：命令、结果、截图路径、敏感信息检查和未覆盖风险。
```
