# 21 后端配置矩阵 V1

本文记录校园搭子平台后端的配置体系边界，区分 local / test / deploy 三个 Spring Profile，并明确敏感与非敏感配置的分界。

## 1. Spring Profile 定义

| Profile | 用途 | 数据库 | 邮件发送 | 状态存储 |
|---------|------|--------|----------|----------|
| `local` | 开发者本机开发 | 不连接 | 默认 no-op，可用 SMTP 环境变量开启真实发送 | 进程内内存 |
| `test` | 自动化测试 | 不连接 | no-op 替身 | 进程内内存 |
| `deploy` | ECS 部署/演示 | PostgreSQL（环境变量） | 默认 no-op，可用 SMTP 环境变量开启真实发送 | PostgreSQL |

默认 profile：`local`（通过 `application.properties` 中 `spring.profiles.active=local` 设置）。

## 2. 非敏感配置矩阵

以下配置可写入项目仓库和文档：

| 配置键 | local | test | deploy | 说明 |
|--------|-------|------|--------|------|
| `campus-buddy.campus-email.allowed-domains[0]` | `${CAMPUS_EMAIL_ALLOWED_DOMAIN:campus.edu.cn}` | `campus.edu.cn` | `${CAMPUS_EMAIL_ALLOWED_DOMAIN:campus.edu.cn}` | 校园邮箱域名白名单；deploy 需替换为真实学校域名 |
| `campus-buddy.campus-email.code-expires-in-seconds` | `600` | `600` | `600` | 验证码有效秒数 |
| `campus-buddy.campus-email.resend-after-seconds` | `60` | `60` | `60` | 重发冷却秒数 |
| `campus-buddy.campus-email.delivery-mode` | `${CAMPUS_EMAIL_DELIVERY_MODE:noop}` | `noop` | `${CAMPUS_EMAIL_DELIVERY_MODE:noop}` | 邮件发送模式：`noop` 或 `smtp` |
| `campus-buddy.campus-email.smtp.host` | `${CAMPUS_EMAIL_SMTP_HOST:}` | 未配置 | `${CAMPUS_EMAIL_SMTP_HOST:}` | SMTP 主机 |
| `campus-buddy.campus-email.smtp.port` | `${CAMPUS_EMAIL_SMTP_PORT:587}` | 未配置 | `${CAMPUS_EMAIL_SMTP_PORT:587}` | SMTP 端口 |
| `campus-buddy.campus-email.smtp.from` | `${CAMPUS_EMAIL_SMTP_FROM:}` | 未配置 | `${CAMPUS_EMAIL_SMTP_FROM:}` | 发件人邮箱 |
| `campus-buddy.campus-email.smtp.from-name` | `${CAMPUS_EMAIL_SMTP_FROM_NAME:校园搭子平台}` | 未配置 | `${CAMPUS_EMAIL_SMTP_FROM_NAME:校园搭子平台}` | 发件人展示名 |
| `campus-buddy.campus-email.smtp.auth` | `${CAMPUS_EMAIL_SMTP_AUTH:true}` | 未配置 | `${CAMPUS_EMAIL_SMTP_AUTH:true}` | 是否启用 SMTP 认证 |
| `campus-buddy.campus-email.smtp.start-tls` | `${CAMPUS_EMAIL_SMTP_START_TLS:true}` | 未配置 | `${CAMPUS_EMAIL_SMTP_START_TLS:true}` | 是否启用 STARTTLS |
| `campus-buddy.campus-email.smtp.ssl` | `${CAMPUS_EMAIL_SMTP_SSL:false}` | 未配置 | `${CAMPUS_EMAIL_SMTP_SSL:false}` | 是否启用 SSL |
| `campus-buddy.object-storage.provider` | `huaweicloud-obs` | `huaweicloud-obs` | `huaweicloud-obs` | 对象存储供应商 |
| `campus-buddy.object-storage.endpoint` | `obs.cn-north-4.myhuaweicloud.com` | `obs.cn-north-4.myhuaweicloud.com` | `obs.cn-north-4.myhuaweicloud.com` | OBS endpoint |
| `campus-buddy.object-storage.region` | `cn-north-4` | `cn-north-4` | `cn-north-4` | OBS 区域 |
| `campus-buddy.object-storage.bucket` | `20260518-bighomework` | `20260518-bighomework` | `20260518-bighomework` | OBS 桶名 |
| `campus-buddy.object-storage.access-mode` | `backend-proxy` | `backend-proxy` | `backend-proxy` | 访问方式：后端中转 |
| `campus-buddy.object-storage.public-read` | `false` | `false` | `false` | 桶公共读 |
| `campus-buddy.object-storage.cors-enabled` | `false` | `false` | `false` | CORS 是否开启 |
| `campus-buddy.security.jwt-placeholder.test-token` | `technical-spike-test-token` | `technical-spike-test-token` | `technical-spike-test-token` | 占位 token，后续替换为真实 JWT |
| `campus-buddy.security.jwt-placeholder.principal` | `technical-spike-user` | `technical-spike-user` | `technical-spike-user` | 占位 principal |
| `logging.level.com.campusbuddy` | `DEBUG` | `DEBUG` | `INFO` | 日志级别 |

## 3. 敏感配置矩阵

以下配置不得写入项目仓库、文档或聊天：

| 配置键 | 获取方式 | 说明 |
|--------|----------|------|
| `OBJECT_STORAGE_ACCESS_KEY_ID` | 服务器环境变量或 IAM 委托 | OBS 访问密钥 ID |
| `OBJECT_STORAGE_SECRET_ACCESS_KEY` | 服务器环境变量或 IAM 委托 | OBS 访问密钥 |
| `OBJECT_STORAGE_SESSION_TOKEN` | IAM 委托临时凭证 | 临时会话 token |
| `DB_HOST` / `DB_PORT` / `DB_NAME` | 服务器环境变量 | 数据库连接参数 |
| `DB_USERNAME` | 服务器环境变量 | 数据库用户名 |
| `DB_PASSWORD` | 服务器环境变量 | 数据库密码 |
| JWT 签名密钥 | 服务器环境变量 | 后续实现真实 JWT 时配置 |
| `CAMPUS_EMAIL_SMTP_USERNAME` | 服务器环境变量或项目目录外私有 env | SMTP 用户名 |
| `CAMPUS_EMAIL_SMTP_PASSWORD` | 服务器环境变量或项目目录外私有 env | SMTP 密码或邮箱授权码 |

## 4. 配置属性类

- `CampusBuddyProperties`（前缀 `campus-buddy`）
  - `CampusEmail`：`allowedDomains`、`codeExpiresInSeconds`、`resendAfterSeconds`、`deliveryMode`、`smtp`
  - `ObjectStorage`：`provider`、`endpoint`、`region`、`bucket`、`accessMode`、`publicRead`、`corsEnabled`
- `JwtPlaceholderProperties`（前缀 `campus-buddy.security.jwt-placeholder`）— 占位，后续替换

## 5. 已消除的硬编码

本轮修改前以下值在 Java 代码中为硬编码常量，现已从 `CampusBuddyProperties` 读取：

- `CampusEmailVerificationService`：`ALLOWED_DOMAINS`、`CODE_EXPIRES_IN_SECONDS`、`RESEND_AFTER_SECONDS`
- `AuthRegistrationService`：`ALLOWED_DOMAINS`

## 6. 当前已知待配置化项

- 邮件发送：已支持 `noop` / `smtp` 切换；真实收发需在私有环境中配置 SMTP 变量并做端到端 smoke
- JWT 密钥：当前为占位，后续需真实 JWT 配置
- 数据库连接：deploy profile 已预留环境变量模板，但尚未接入
- 校园邮箱真实域名：deploy 需替换 `campus.edu.cn` 为真实学校域名
