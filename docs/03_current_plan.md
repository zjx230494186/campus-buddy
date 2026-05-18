# 03 Current Plan

本文只维护当前阶段最重要事项，不承载全部历史。

## 文档编码规则

- 项目 Markdown 文档统一使用 UTF-8 编码保存。
- 不使用 GBK/ANSI 保存长期维护文档。
- 使用 PowerShell 写入中文文档时必须显式指定 `-Encoding UTF8`。
- 若发现乱码，先备份原文件，再修复为 UTF-8。

## 当前阶段主题

- 阶段：正式代码开发。
- 当前线程：CodeArts 下一轮提示词设计与提交门禁准备。
- 当前目标：先让 CodeArts 执行 Git 提交门禁与剩余污染复核，将仓库清理和 P0 认证/JWT 改动安全固定为可追溯提交；通过后再进入下一业务闭环。

## 当前对象存储状态

- 云厂商：华为云。
- 产品：对象存储服务 OBS。
- 桶名称：`20260518-bighomework`。
- 区域：华北-北京四，`cn-north-4`。
- Endpoint：`obs.cn-north-4.myhuaweicloud.com`。
- 访问域名：`20260518-bighomework.obs.cn-north-4.myhuaweicloud.com`。
- 桶策略：私有桶，不开启公共读或公共读写。
- CORS：当前未配置。
- 计费：按需使用。

## 已完成事项

### 后端配置体系（前序线程）
- 后端配置属性类 `CampusBuddyProperties` 已实现并启用。
- Spring Profile 配置文件已创建：`application-local.properties`、`application-test.properties`、`application-deploy.properties`。
- 默认 profile 设为 `local`。
- 校园邮箱域名白名单已从硬编码提取为配置项。
- 验证码过期和重发冷却已从硬编码提取为配置项。
- 对象存储非敏感配置已纳入 `CampusBuddyProperties`。
- 配置属性测试 7 个用例全部通过。

### Flyway 迁移修复
- 诊断并修复 Flyway V2 迁移在 `@SpringBootTest` + Testcontainers 环境不执行问题。
- 创建 `FlywayConfiguration.java` 手动注册 Flyway bean，绕过 Spring Boot 4.0.6 自动配置失效。

### P0 认证持久化 / JWT
- JPA 实体：`UserAccount`、`CampusEmailVerificationCodeEntity`、`CampusEmailVerificationTicketEntity`。
- JPA Repository：`UserAccountRepository`、`VerificationCodeRepository`、`VerificationTicketRepository`。
- 服务重写：`CampusEmailVerificationService`、`AuthRegistrationService`、`AuthLoginService` 改为 JPA 持久化。
- 真实 JWT：`JwtService`（JJWT 签发验签）、`JwtProperties`、`JwtAuthenticationFilter`。
- Security 配置已从占位 token 切换到真实 JWT。
- `SecureProbeController.authenticationMode` 已从 `"jwt-placeholder"` 更新为 `"jwt"`。

### 测试修复
- 修复 `AuthPersistenceIntegrationTest` 编译错误。
- 修复 `AuthRegistrationEndpointTest` 反射→JPA Repository。
- 修复 `AuthLoginEndpointTest` 断言（真实 JWT 无固定前缀）。
- 修复 `SecurityProbeEndpointTest` 使用 JwtService 生成真实 JWT token。
- 修复 `AuthRegistrationService` 逻辑顺序：先检查邮箱已注册再消费 ticket。
- 创建 `src/test/resources/application.properties`（激活 test profile）。

### 仓库审计
- 审计并补充 `.gitignore`：新增 `desktop/build-*/`、`latex_work/build_*/`、`.arts/`、`test_output*.txt`、`test_error*.txt`、`test_flyway_out*.txt`、`ppt_extracted.txt`、ninja 文件、autogen 目录。
- 清理 Git 跟踪污染：`git rm --cached` 移除 24 个 Qt 构建产物、3 个 LaTeX 构建产物、1 个 `.arts/settings.json`、1 个 `ppt_extracted.txt`。
- 敏感信息审计：local/test profile JWT secret 为非敏感测试密钥；deploy profile JWT secret 通过 `${JWT_SECRET}` 环境变量注入；数据库密码通过环境变量引用；未发现真实密钥泄露。

### 当前测试状态
- 非容器快速测试：32/32 通过。
- Testcontainers 测试：需要 Docker 可用才能运行。

## 配置边界摘要

可进入项目文档或模板的非敏感配置：

- `campus-buddy.campus-email.allowed-domains` — 校园邮箱域名白名单
- `campus-buddy.campus-email.code-expires-in-seconds` — 验证码有效秒数
- `campus-buddy.campus-email.resend-after-seconds` — 重发冷却秒数
- `campus-buddy.object-storage.provider` — 对象存储供应商
- `campus-buddy.object-storage.endpoint` — OBS endpoint
- `campus-buddy.object-storage.region` — OBS 区域
- `campus-buddy.object-storage.bucket` — OBS 桶名
- `campus-buddy.object-storage.access-mode` — 访问方式
- `campus-buddy.object-storage.public-read` — 桶公共读
- `campus-buddy.object-storage.cors-enabled` — CORS 开启
- `campus-buddy.security.jwt.access-token-expires-in-seconds` — access token 有效秒数
- `campus-buddy.security.jwt.refresh-token-expires-in-seconds` — refresh token 有效秒数

不得进入项目文档、仓库、聊天或 Qt 客户端的敏感配置：

- `JWT_SECRET`（deploy profile）、`OBJECT_STORAGE_ACCESS_KEY_ID`、`OBJECT_STORAGE_SECRET_ACCESS_KEY`、`OBJECT_STORAGE_SESSION_TOKEN`
- `DB_PASSWORD`、SSH 私钥、云账号密码、IAM 用户密码。

## 当前明确不做

- 不实现业务附件上传接口。
- 不创建对象存储业务表。
- 不修改 Qt 客户端直连 OBS。
- 不创建真实 IAM 用户、访问密钥、委托或权限策略。
- 不申请域名、HTTPS 证书、备案。
- 不把任何敏感凭据写入项目文档、仓库、聊天或 Qt 客户端。
- 不把公网 IP 写死在深层业务逻辑。

## 推荐下一步

1. `Git 提交门禁与剩余污染复核`
   - 优先级：最高。
   - 目标：复查当前工作区、剩余历史跟踪污染、测试和敏感信息，在满足门禁条件后拆分创建 Git 提交。
   - 提示词：`docs/prompts/codearts/20260518_round_04_git_commit_gate_and_remaining_pollution_audit.md`。

2. `P0 认证资料提交接口`
   - 优先级：高。
   - 前置：Git 提交门禁完成，当前改动已形成干净提交。
   - 目标：实现认证资料提交和状态查询的最小闭环。

3. `替换 no-op 邮件发送`
   - 优先级：中。
   - 目标：接入真实邮件发送服务。
