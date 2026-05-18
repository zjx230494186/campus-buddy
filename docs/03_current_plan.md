# 03 Current Plan

本文只维护当前阶段最重要事项，不承载全部历史。

## 文档编码规则

- 项目 Markdown 文档统一使用 UTF-8 编码保存。
- 不使用 GBK/ANSI 保存长期维护文档。
- 使用 PowerShell 写入中文文档时必须显式指定 `-Encoding UTF8`。
- 若发现乱码，先备份原文件，再修复为 UTF-8。

## 当前阶段主题

- 阶段：正式代码开发。
- 当前线程：后端配置体系与环境差异（已完成）。
- 当前目标：已完成第一轮最小闭环——后端配置属性类 + Spring Profile + 消除硬编码；下一轮进入 P0 认证最小闭环的正式实现。

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

- 后端配置属性类 `CampusBuddyProperties` 已实现并启用。
- Spring Profile 配置文件已创建：`application-local.properties`、`application-test.properties`、`application-deploy.properties`。
- 默认 profile 设为 `local`。
- 校园邮箱域名白名单已从硬编码提取为配置项。
- 验证码过期和重发冷却已从硬编码提取为配置项。
- 对象存储非敏感配置已纳入 `CampusBuddyProperties`。
- `CampusEmailVerificationService` 和 `AuthRegistrationService` 已从配置读取域名白名单，不再硬编码。
- 配置属性测试 7 个用例全部通过。
- 非容器后端全量回归 31 个用例全部通过。
- 已形成配置矩阵文档：`D:\big_homework\docs\21_backend_configuration_matrix_v1.md`。
- 已形成验证记录：`D:\big_homework\docs\validation\20260518_backend_configuration_properties_test_record.md`。
- 本轮未写入任何敏感凭据。

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

不得进入项目文档、仓库、聊天或 Qt 客户端的敏感配置：

- `OBJECT_STORAGE_ACCESS_KEY_ID`、`OBJECT_STORAGE_SECRET_ACCESS_KEY`、`OBJECT_STORAGE_SESSION_TOKEN`
- `DB_PASSWORD`、JWT 签名密钥、SSH 私钥、云账号密码、IAM 用户密码。

## 当前明确不做

- 不实现业务附件上传接口。
- 不创建对象存储业务表。
- 不修改 Qt 客户端直连 OBS。
- 不创建真实 IAM 用户、访问密钥、委托或权限策略。
- 不申请域名、HTTPS 证书、备案。
- 不把任何敏感凭据写入项目文档、仓库、聊天或 Qt 客户端。
- 不把公网 IP 写死在深层业务逻辑。

## 推荐下一步

1. `Docker/Testcontainers + PostgreSQL/Flyway 环境验证`
   - 优先级：最高。
   - 目标：解决 `DatabaseMigrationTest` 的 Docker 阻塞，为后续数据库迁移铺路。

2. `真实 JWT 最小安全链路验证`
   - 优先级：高。
   - 目标：使用 Spring Security 实现真实 JWT 签发验签，替换占位 token。

3. `P0 认证资料提交接口`
   - 优先级：高。
   - 前置：数据库迁移和 JWT 链路完成。
   - 目标：实现认证资料提交和状态查询的最小闭环。

## 2026-05-18 CodeArts 提示词设计与提交复核工作流

- 当前线程后续用于配合 CodeArts 正式开发：用户告知 CodeArts 已完成的工作后，本线程负责检查 Git 提交、变更范围、测试结果、验证记录和敏感信息风险。
- 后续每轮将先复核事实，再与用户商量下一轮 CodeArts 提示词。
- 已新增工作流文档：`D:\big_homework\docs\21_codearts_prompt_review_workflow_v1.md`。
- 每轮给 CodeArts 的提示词应留档，优先归档到 `docs/prompts/codearts/`，或在工作流文档中索引。
- 复核默认检查项：`git status --short --branch`、最近提交、变更范围、测试命令、`docs/validation/` 留档、敏感信息风险。
