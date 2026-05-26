# Latest Handoff

## 2026-05-26 广场访问认证状态缺口修复并上线

### 本轮完成

- 修复未完成身份认证用户仍可访问广场列表和帖子详情的问题。
- `PartnerPostPlazaService.listPosts(...)` 和 `getPostDetail(...)` 入口增加 `VERIFIED` 状态校验。
- 未认证用户现在返回：
  - HTTP 403
  - `AUTHENTICATION_STATUS_REQUIRED`
- 新增 service 单测：
  - `PartnerPostPlazaServiceTest`
- 修正 endpoint 合同测试：
  - `unverifiedUserCanViewPlazaList` 改为 `unverifiedUserCannotViewPlazaList`
  - 新增 `unverifiedUserCannotViewPlazaDetail`
- 已重新构建 jar 并部署到华为云服务器。
- 新增验证记录：
  - `docs/validation/20260526_plaza_requires_verified_server_fix_record.md`

### 验证结果

- 红灯：`PartnerPostPlazaServiceTest` 初始 2/2 failed，失败原因为未抛权限异常。
- 绿灯：`PartnerPostPlazaServiceTest` 2/2 passed。
- 轻量回归：`PartnerPostPlazaServiceTest,SmtpCampusEmailVerificationCodeSenderTest` 4/4 passed。
- 构建：`.\mvnw.cmd -DskipTests package` 通过。
- 服务器 health：`GET http://114.116.203.78/api/health` 返回 `{"status":"UP"}`。
- 服务器公网 smoke：
  - 未认证 smoke 账号访问广场列表：403 `AUTHENTICATION_STATUS_REQUIRED`
  - 已认证初始学生账号访问广场列表：200
  - 未认证 smoke 账号访问帖子详情：403 `AUTHENTICATION_STATUS_REQUIRED`
  - 已认证初始学生账号访问同一帖子详情：200

### 服务器备份

- jar 备份：
  - `/srv/campus-buddy/campus-buddy-backend-0.0.1-SNAPSHOT.jar.backup.20260526_234539`

### 边界

- 未修改 Flyway。
- 未修改 Qt 客户端。
- 未修改 deploy 脚本。
- 未写入真实用户密码、验证码、token、SMTP 授权码、数据库密码、OBS AK/SK 或服务器私钥。

## 2026-05-26 服务器真实邮箱验证码发送上线完成

### 本轮完成

- 将本机项目目录外私有 SMTP 配置同步到服务器 `/etc/campus-buddy/backend.env`。
- 设置服务器允许收件域名为 `bjtu.edu.cn`。
- 发现服务器旧 jar 只包含 Noop sender，重新构建并上线包含 SMTP sender 的后端 jar。
- 重启 `campus-buddy-backend` systemd 服务。
- 新增验证记录：
  - `docs/validation/20260526_server_smtp_bjtu_deploy_record.md`

### 备份

- 配置备份：
  - `/etc/campus-buddy/backend.env.backup.20260526_222031`
- jar 备份：
  - `/srv/campus-buddy/campus-buddy-backend-0.0.1-SNAPSHOT.jar.backup.20260526_222339`

### 验证结果

- 服务器 health：`GET http://114.116.203.78/api/health` 返回 `{"status":"UP"}`。
- 非 `bjtu.edu.cn` 邮箱请求验证码返回 `INVALID_CAMPUS_EMAIL_DOMAIN`。
- `24301082@bjtu.edu.cn` 请求验证码返回 `CODE_SENT`。
- 初始学生账号和管理员账号登录回归通过。

### 安全边界

- 未把 SMTP 授权码、服务器私钥、数据库密码、OBS AK/SK、JWT secret、验证码或 token 写入仓库、文档或聊天。
- 文档只记录变量 present/missing、备份路径、接口状态码和脱敏邮箱。

### 后续建议

1. **用户收件确认** — 请检查 `24301082@bjtu.edu.cn` 是否收到最新验证码。
2. **Qt 注册页演示** — 使用尚未注册的新 `bjtu.edu.cn` 邮箱，从桌面端走发送验证码、输入验证码、注册和登录。
3. **HTTPS 收口** — 公网仍为 HTTP，正式外发前建议补 HTTPS。

## 2026-05-26 Windows 内测版桌面端打包完成

### 本轮完成

- 将 Qt 桌面端默认 API 地址从本机 `localhost` 切换为服务器 `http://114.116.203.78/api`。
- 使用 Qt 6.10.3 MinGW Release 构建桌面端，并通过 `windeployqt` 收集运行时依赖。
- 生成 Windows 内测 zip 包：
  - `D:\big_homework\deliverables\internal_beta\CampusBuddyInternalBeta_20260526.zip`
  - 解压后入口程序：`CampusBuddyInternalBeta_20260526\campus_buddy_desktop.exe`
- 新增打包验证记录：
  - `docs/validation/20260526_windows_internal_beta_package_record.md`
- 已提交本轮源码与验证文档：
  - `103d476 build(desktop): package internal beta for server runtime`

### 验证结果

- 服务器 health：`GET http://114.116.203.78/api/health` 返回 `{"status":"UP"}`。
- Qt API 配置测试通过。
- Release 桌面端 `--smoke-test` 通过。
- 在剥离 Qt/CMake/MinGW PATH 后，打包目录内 exe `--smoke-test` 通过。
- zip 内容包含 exe、Qt Core/Gui/Widgets/Network DLL、MinGW runtime、platforms/qwindows.dll、tls backend 和 README。

### 服务器密钥依赖检查

- 客户端只依赖公开 HTTP API 地址和运行时可选覆盖项 `CAMPUS_BUDDY_API_BASE_URL` / `--api-base-url=`。
- 未发现客户端包依赖本机 SSH 私钥、服务器私钥、数据库密码、OBS AK/SK、SMTP 授权码或项目外私有 env 文件。
- 包内 Qt 网络/TLS DLL 出现的 `BEGIN PRIVATE KEY` 等字符串属于 Qt 官方 TLS/证书解析器内置文本，不是项目密钥材料。

### 边界与残余风险

- 本轮不修改后端业务逻辑、不修改 Flyway、不修改 deploy 脚本。
- 公网接口当前仍是 HTTP；用于内测可以接受，正式外发前建议补 HTTPS/Nginx 证书。
- `windeployqt` 报告未找到 `dxcompiler.dll` / `dxil.dll`，当前 Qt Widgets 主链路 smoke 通过，低风险；若后续引入 Qt Quick 或 shader 相关能力需重新验证。
- `deliverables/internal_beta/` 为本地交付物目录，未纳入 Git 提交。

### 后续建议

1. **内测分发** — 可直接分发 `CampusBuddyInternalBeta_20260526.zip`，让测试者整包解压后双击 exe。
2. **HTTPS 收口** — 建议新开部署线程，为服务器接入 HTTPS，再重新打包或改默认 API 地址。
3. **现场演示前复核** — 演示前重新确认服务器服务 `UP`，并用包内 exe 做一次登录/注册/发布主链路人工 smoke。

### 建议下一线程名称

`Windows 内测包分发与现场演示复核`

### 可复制启动 Prompt

```text
请读取 D:\big_homework\AGENTS.md、D:\big_homework\docs\03_current_plan.md、D:\big_homework\handoff\latest.md、D:\big_homework\docs\validation\20260526_windows_internal_beta_package_record.md。

当前任务是对 Windows 内测包做最终分发前复核。交付物为 D:\big_homework\deliverables\internal_beta\CampusBuddyInternalBeta_20260526.zip，解压后双击 campus_buddy_desktop.exe 应连接到 http://114.116.203.78/api。

本轮范围：检查服务器 health、检查 zip 内容、运行包内 exe smoke，必要时做一次主链路人工演示。不要修改后端、Flyway 或 deploy；不要写入或泄露服务器 SSH 私钥、DB 密码、OBS AK/SK、SMTP 授权码、验证码、token 或真实联系方式。
```

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
