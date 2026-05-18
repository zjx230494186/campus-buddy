# Latest Handoff

本文档服务于新线程快速接手。

## 当前状态

- 项目名称：校园搭子平台。
- 当前阶段：正式代码开发。
- 当前线程：后端配置体系与环境差异（已完成）。
- 后端配置属性类 `CampusBuddyProperties` 已实现。
- Spring Profile（local/test/deploy）已创建。
- 校园邮箱域名白名单和验证码超时已从硬编码提取为配置项。
- 对象存储非敏感配置已纳入属性类。
- 配置矩阵文档：`D:\big_homework\docs\21_backend_configuration_matrix_v1.md`。
- 验证记录：`D:\big_homework\docs\validation\20260518_backend_configuration_properties_test_record.md`。

## 当前线程完成了什么

- 新增 `CampusBuddyProperties` 配置属性类（前缀 `campus-buddy`），包含 `CampusEmail` 和 `ObjectStorage` 内部类。
- 新增三个 Spring Profile 配置文件：`application-local.properties`、`application-test.properties`、`application-deploy.properties`。
- 修改 `application.properties` 设置默认 profile 为 `local`。
- 修改 `CampusBuddyBackendApplication` 启用 `CampusBuddyProperties`。
- 修改 `CampusEmailVerificationService` 从配置读取域名白名单、验证码过期秒数和重发冷却秒数，移除硬编码。
- 修改 `AuthRegistrationService` 从配置读取域名白名单，移除硬编码。
- 新增配置属性测试 7 个用例，全部通过。
- 非容器后端全量回归 31 个用例，全部通过。
- 形成配置矩阵文档和验证记录。

## 关键结论

- 校园邮箱域名白名单、验证码超时和对象存储非敏感配置已可配置化。
- 敏感配置（数据库密码、OBS AK/SK、JWT 密钥）仅通过环境变量引用，不写入仓库。
- `deploy` profile 已预留数据库连接环境变量模板。
- 当前邮件发送仍为 no-op 替身，JWT 仍为占位，后续需替换。
- `DatabaseMigrationTest` 因 Docker 不可用失败，是当前完整回归的唯一阻塞。

## 本线程没有做什么

- 没有连接真实数据库。
- 没有实现真实 JWT 签发验签。
- 没有替换 no-op 邮件发送。
- 没有实现新业务接口。
- 没有修改 Qt 客户端。
- 没有写入任何敏感凭据。
- 没有创建首个 Git 提交。

## 下一步候选事项

1. `Docker/Testcontainers + PostgreSQL/Flyway 环境验证`
   - 建议：新开线程。
   - 优先级：最高。
   - 目标：解决 `DatabaseMigrationTest` 的 Docker 阻塞。

2. `真实 JWT 最小安全链路验证`
   - 建议：新开线程。
   - 优先级：高。
   - 目标：使用 Spring Security 实现真实 JWT 签发验签。

3. `P0 认证资料提交接口`
   - 建议：新开线程，前置数据库迁移和 JWT。
   - 优先级：高。
   - 目标：实现认证资料提交和状态查询最小闭环。

## 建议归档与下一线程

- 建议归档当前线程：是。
- 下一线程名称：`Docker/Testcontainers + PostgreSQL/Flyway 环境验证`

## 2026-05-18 CodeArts 提示词设计与提交复核工作流

- 后续本线程用于配合 CodeArts 正式开发。
- 用户每次告知 CodeArts 已完成工作后，本线程先检查 Git 提交、变更范围、测试结果、验证记录和敏感信息风险。
- 检查后再与用户商量下一轮 CodeArts 提示词，并将关键提示词留档。
- 工作流文档：`D:\big_homework\docs\21_codearts_prompt_review_workflow_v1.md`。
- CodeArts 初始启动 prompt：`D:\big_homework\docs\20_codearts_formal_development_start_prompt_v1.md`。
