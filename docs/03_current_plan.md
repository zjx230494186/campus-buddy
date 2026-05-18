# 03 Current Plan

本文只维护当前阶段最重要事项，不承载全部历史。

## 文档编码规则

- 项目 Markdown 文档统一使用 UTF-8 编码保存。
- 不使用 GBK/ANSI 保存长期维护文档。
- 使用 PowerShell 写入中文文档时必须显式指定 `-Encoding UTF8`。
- 若发现乱码，先备份原文件，再修复为 UTF-8。

## 当前阶段主题

- 阶段：正式代码开发。
- 当前线程：Git 提交门禁与剩余污染复核（已完成）。
- 当前目标：仓库卫生提交和 P0 认证/JWT 提交已完成，工作区干净，可进入下一业务闭环。

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

## Git 提交历史

- `447b0af` chore:-establish-project-baseline-and-backend-configuration
- `18e2a01` chore(repo): clean generated artifacts and record git hygiene
- `157abc0` feat(auth): persist authentication flow and introduce jwt

## 已完成事项

### 后端配置体系（前序线程）
- 后端配置属性类 `CampusBuddyProperties` 已实现并启用。
- Spring Profile 配置文件已创建。
- 校园邮箱域名白名单和验证码超时已从硬编码提取为配置项。
- 对象存储非敏感配置已纳入 `CampusBuddyProperties`。

### 仓库卫生（提交 `18e2a01`）
- 补充 `.gitignore`：新增 Qt 构建目录、ninja 文件、autogen、LaTeX 构建目录、.arts/、调试输出文件等忽略规则。
- 清理 Git 跟踪污染：`git rm --cached` 移除 29+14=43 个不应跟踪的文件（Qt 构建产物、LaTeX 构建产物、.arts/、ppt_extracted.txt）。
- 添加审计文档和 prompt 记录。

### P0 认证持久化 / JWT（提交 `157abc0`）
- JPA 实体：`UserAccount`、`CampusEmailVerificationCodeEntity`、`CampusEmailVerificationTicketEntity`。
- JPA Repository：`UserAccountRepository`、`VerificationCodeRepository`、`VerificationTicketRepository`。
- 服务重写：`CampusEmailVerificationService`、`AuthRegistrationService`、`AuthLoginService` 改为 JPA 持久化。
- 真实 JWT：`JwtService`（JJWT 签发验签）、`JwtProperties`、`JwtAuthenticationFilter`。
- Security 配置已从占位 token 切换到真实 JWT。
- `SecureProbeController.authenticationMode` 更新为 `"jwt"`。
- Flyway 手动配置：`FlywayConfiguration.java` 绕过自动配置失效。
- `AuthRegistrationService` 逻辑顺序修正：先检查邮箱已注册再消费 ticket。
- 所有测试更新为真实 JWT + JPA。

### 当前测试状态
- 非容器快速测试：32/32 通过。
- Testcontainers 测试：需要 Docker 可用才能运行。

## 配置边界摘要

可进入项目文档或模板的非敏感配置：

- `campus-buddy.campus-email.allowed-domains` — 校园邮箱域名白名单
- `campus-buddy.campus-email.code-expires-in-seconds` — 验证码有效秒数
- `campus-buddy.campus-email.resend-after-seconds` — 重发冷却秒数
- `campus-buddy.object-storage.*` — 对象存储非敏感配置
- `campus-buddy.security.jwt.access-token-expires-in-seconds` — access token 有效秒数
- `campus-buddy.security.jwt.refresh-token-expires-in-seconds` — refresh token 有效秒数

不得进入项目文档、仓库、聊天或 Qt 客户端的敏感配置：

- `JWT_SECRET`（deploy profile）、`OBJECT_STORAGE_ACCESS_KEY_ID`、`OBJECT_STORAGE_SECRET_ACCESS_KEY`
- `DB_PASSWORD`、SSH 私钥、云账号密码、IAM 用户密码。

## 当前明确不做

- 不实现业务附件上传接口。
- 不创建对象存储业务表。
- 不修改 Qt 客户端直连 OBS。
- 不创建真实 IAM 用户、访问密钥、委托或权限策略。
- 不申请域名、HTTPS 证书、备案。
- 不把任何敏感凭据写入项目文档、仓库、聊天或 Qt 客户端。

## 推荐下一步

1. `提交 Round 04/05 纯文档留档`
   - 优先级：最高。
   - 目标：当前存在 Round 04 后续交接文档、验证记录和 Round 05 prompt 留档改动，应先形成纯文档提交，避免下一轮业务开发混入文档尾巴。

2. `P0 认证资料提交接口`
   - 优先级：高。
   - 前置：纯文档留档已提交，工作区无源码或配置脏改动。
   - 目标：实现认证资料提交和状态查询的最小闭环。
   - 提示词：`docs/prompts/codearts/20260518_round_05_p0_identity_profile_submission.md`。

3. `替换 no-op 邮件发送`
   - 优先级：中。
   - 目标：接入真实邮件发送服务。
