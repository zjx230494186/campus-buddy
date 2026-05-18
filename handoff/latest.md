# Latest Handoff

本文档服务于新线程快速接手。

## 当前状态

- 项目名称：校园搭子平台。
- 当前阶段：正式代码开发。
- 当前线程：CodeArts 下一轮提示词设计与提交门禁准备。
- Git 分支：main，基线提交 `447b0af` 之后尚未提交新改动。
- 非容器快速测试：32/32 通过。

## 当前线程完成了什么

### Flyway 迁移修复
- 诊断并修复 Flyway V2 迁移在 `@SpringBootTest` + Testcontainers 环境不执行问题。
- 创建 `FlywayConfiguration.java` 手动注册 Flyway bean。

### P0 认证持久化 / JWT
- JPA 实体与 Repository 已创建。
- 认证服务重写为 JPA 持久化。
- 真实 JWT 链路：JwtService + JwtAuthenticationFilter + JwtProperties。
- Security 配置已切换到真实 JWT。
- `SecureProbeController.authenticationMode` 更新为 `"jwt"`。
- `AuthRegistrationService` 逻辑顺序修正：先检查邮箱已注册再消费 ticket。

### 测试修复
- 修复 `AuthPersistenceIntegrationTest`、`AuthRegistrationEndpointTest`、`AuthLoginEndpointTest`、`SecurityProbeEndpointTest`。
- 创建 `src/test/resources/application.properties`。
- 非容器快速测试 32/32 通过。

### 仓库审计
- 补充 `.gitignore`：新增 Qt 构建产物、LaTeX 构建产物、.arts/、调试输出文件等忽略规则。
- 清理 Git 跟踪污染：`git rm --cached` 移除 29 个不应跟踪的文件。
- 敏感信息审计：deploy profile JWT secret 通过 `${JWT_SECRET}` 环境变量注入；未发现真实密钥泄露。

## 关键结论

- P0 认证核心链路（注册→验证→登录→JWT签发→受保护端点访问）已通过集成测试验证。
- 仓库已清理构建产物污染，.gitignore 已补全。
- 敏感信息通过环境变量引用，未泄露到仓库。
- 当前所有改动尚未提交到 Git。

## 本线程没有做什么

- 没有提交 Git 改动（需用户确认）。
- 没有运行 Testcontainers/Docker 测试（需 Docker 可用）。
- 没有替换 no-op 邮件发送。
- 没有实现认证资料提交接口。
- 没有修改 Qt 客户端。
- 没有写入任何敏感凭据。

## 下一步候选事项

1. `Git 提交门禁与剩余污染复核`
   - 建议：复用当前线程或新开线程。
   - 优先级：最高。
   - 目标：复查剩余 Git 跟踪污染、测试、敏感信息与提交边界，在满足门禁条件后创建仓库卫生提交和 P0 认证/JWT 提交。
   - CodeArts 提示词：`D:\big_homework\docs\prompts\codearts\20260518_round_04_git_commit_gate_and_remaining_pollution_audit.md`。

2. `P0 认证资料提交接口`
   - 建议：新开线程。
   - 优先级：高。
   - 目标：实现认证资料提交和状态查询最小闭环。

3. `替换 no-op 邮件发送`
   - 建议：新开线程。
   - 优先级：中。
   - 目标：接入真实邮件发送服务。

## 建议归档与下一线程

- 建议归档当前线程：是。
- 下一线程名称：`Git 提交门禁与剩余污染复核`

## 2026-05-18 CodeArts 提示词设计与提交复核工作流

- 后续本线程用于配合 CodeArts 正式开发。
- 用户每次告知 CodeArts 已完成工作后，本线程先检查 Git 提交、变更范围、测试结果、验证记录和敏感信息风险。
- 工作流文档：`D:\big_homework\docs\21_codearts_prompt_review_workflow_v1.md`。
