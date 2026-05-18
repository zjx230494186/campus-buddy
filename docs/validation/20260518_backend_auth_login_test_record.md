# 测试记录：后端 P0-1 / 登录接口

## 基本信息

- 日期：2026-05-18
- 线程：P0-1 登录接口测试先行实现
- 模块：账号准入与身份认证
- 功能：`POST /api/auth/login`
- 对应需求编号：`SF1`、`IR1`
- 对应契约：`D:\big_homework\docs\15_p0_auth_api_contract_v1.md` 第 4.4 节
- 测试文件：`D:\big_homework\backend\src\test\java\com\campusbuddy\auth\AuthLoginEndpointTest.java`
- 实现文件：
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\auth\AuthLoginController.java`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\auth\AuthLoginService.java`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\auth\AuthRegistrationService.java`

## 测试设计

- 测试类型：Spring Boot Test + MockMvc 接口测试。
- 成功路径：
  - 先通过邮箱验证码发送、验证码校验、注册接口创建账号。
  - 再调用 `POST /api/auth/login`。
  - 断言返回占位 `accessToken`、`refreshToken`、`accessTokenExpiresInSeconds=900`、`refreshTokenExpiresInSeconds=2592000`、`tokenType=Bearer` 和当前用户摘要。
- 异常路径：
  - 已注册账号使用错误密码返回 `INVALID_LOGIN_CREDENTIALS`。
  - 缺失或格式错误字段返回 `VALIDATION_FAILED`。
- 安全约束：
  - 登录密码校验必须使用注册阶段保存的 BCrypt 哈希。
  - 不保存或比较明文密码。
  - 不把密码或 token 原文写入日志。

## 红灯记录

- 运行命令：`.\mvnw.cmd test -Dtest=AuthLoginEndpointTest`
- 失败测试：
  - `loginReturnsPlaceholderTokensForRegisteredAccount`
  - `loginRejectsInvalidCredentials`
  - `loginRejectsMissingOrMalformedFields`
- 失败原因：`POST /api/auth/login` 尚未实现，三个登录相关用例均命中既有 `404 RESOURCE_NOT_FOUND`。
- 是否符合预期：符合。该结果证明登录接口在实现前确实不存在。

## 实现摘要

- 新增 `AuthLoginController`，只暴露 `POST /api/auth/login`。
- 新增 `AuthLoginService`：
  - 校验 `campusEmail` 与 `password`。
  - 通过 `AuthRegistrationService.authenticateForLogin` 查找注册账号。
  - 错误邮箱或错误密码统一返回 `INVALID_LOGIN_CREDENTIALS`。
  - 成功返回占位 access token、refresh token、过期秒数、`Bearer` token 类型和当前用户摘要。
  - refresh token 仅以 SHA-256 哈希作为内存会话占位保存，不保存 token 原文。
- 扩展 `AuthRegistrationService`：
  - 新增包内登录认证入口。
  - 使用 `BCryptPasswordEncoder.matches` 校验已保存的 BCrypt 密码哈希。
- 本批没有实现真实 JWT 签发验签、refresh token 轮换、logout、RBAC、认证资料提交、数据库业务表、JPA Entity/Repository 或 Qt 页面。

## 绿灯与回归记录

- 本批测试命令：`.\mvnw.cmd test -Dtest=AuthLoginEndpointTest`
- 本批测试结果：通过，`Tests run: 3, Failures: 0, Errors: 0, Skipped: 0`
- 认证相关回归命令：`.\mvnw.cmd test "-Dtest=AuthLoginEndpointTest,AuthRegistrationEndpointTest,CampusEmailVerificationEndpointTest"`
- 认证相关回归结果：通过，`Tests run: 18, Failures: 0, Errors: 0, Skipped: 0`
- 非容器后端回归命令：`.\mvnw.cmd test "-Dtest=AuthLoginEndpointTest,AuthRegistrationEndpointTest,CampusEmailVerificationEndpointTest,HealthEndpointTest,SecurityProbeEndpointTest,SystemInfoEndpointTest"`
- 非容器后端回归结果：通过，`Tests run: 24, Failures: 0, Errors: 0, Skipped: 0`
- 完整后端回归命令：`.\mvnw.cmd test`
- 完整后端回归结果：未完全通过，`Tests run: 25, Failures: 0, Errors: 1, Skipped: 0`。
- 完整回归阻塞原因：`DatabaseMigrationTest` 启动 Testcontainers 前找不到可用 Docker 环境，错误为 `Could not find a valid Docker environment`。

## 后续风险

- 当前账号、验证码、ticket 和 refresh token 会话仍为进程内内存实现，服务重启后丢失。
- 当前 access token 和 refresh token 为占位格式，不是真实 JWT 和可验证 refresh token 体系。
- 当前登录接口不实现 refresh token 轮换、logout 或 RBAC。
- 当前白名单域名仍为最小闭环测试值 `campus.edu.cn`，后续需要配置化。
- 完整数据库迁移回归需要在 Docker daemon 可用后补跑 `.\mvnw.cmd test`。

## 下一步建议

- 下一项真实业务接口建议新开线程实现 `POST /api/auth/token/refresh`。
- 也可先复用当前后端认证上下文做账号准入内存状态持久化小修，但不建议和 refresh token 轮换混在同一线程。
