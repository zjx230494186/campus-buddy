# 测试记录：后端 P0-1 / 校验校园邮箱验证码接口

## 基本信息

- 日期：2026-05-18
- 线程：P0-1 校验校园邮箱验证码接口测试先行实现
- 模块：账号准入与身份认证
- 功能：`POST /api/auth/campus-email/verifications`
- 对应需求编号：`SF1`、`IR1`
- 对应契约：`D:\big_homework\docs\15_p0_auth_api_contract_v1.md` 第 4.2 节
- 测试文件：`D:\big_homework\backend\src\test\java\com\campusbuddy\auth\CampusEmailVerificationEndpointTest.java`
- 实现文件：
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\auth\CampusEmailVerificationController.java`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\auth\CampusEmailVerificationService.java`

## 测试设计

- 测试类型：Spring Boot Test + MockMvc 接口测试
- 成功路径：
  - 先调用 `POST /api/auth/campus-email/verification-codes` 为 `verify-success@campus.edu.cn` 生成验证码。
  - 测试专用 `CapturingCampusEmailVerificationCodeSender` 捕获验证码，仅用于测试断言，不进入生产响应或日志。
  - 再调用 `POST /api/auth/campus-email/verifications` 校验正确验证码。
  - 断言返回 `VERIFIED`、`verifiedAt`、`verificationTicket`。
- 异常路径：
  - 错误验证码返回 `EMAIL_VERIFICATION_CODE_INVALID`。
  - 过期验证码返回 `EMAIL_VERIFICATION_CODE_EXPIRED`。
  - 非白名单邮箱返回 `INVALID_CAMPUS_EMAIL_DOMAIN`。
  - 缺失或格式错误字段返回 `VALIDATION_FAILED`。

## 红灯记录

- 运行命令：`.\mvnw.cmd test -Dtest=CampusEmailVerificationEndpointTest`
- 失败测试：
  - `verifyCodeReturnsVerifiedForCorrectCode`
  - `verifyCodeRejectsInvalidCode`
  - `verifyCodeRejectsExpiredCode`
  - `verifyCodeRejectsInvalidCampusEmailDomain`
  - `verifyCodeRejectsMissingOrMalformedFields`
- 失败原因：
  - `POST /api/auth/campus-email/verifications` 尚未实现，5 个新增用例均命中既有 `404 RESOURCE_NOT_FOUND`。
  - 成功路径期望 `200`，实际为 `404`。
  - 异常路径期望 `400` 及对应业务错误码，实际为 `404`。
- 是否符合预期：符合。该结果证明校验校园邮箱验证码接口在实现前确实不存在。

## 实现摘要

- 扩展 `CampusEmailVerificationController`：
  - 将类级路径调整为 `/api/auth/campus-email`。
  - 保留 `POST /verification-codes` 发送验证码接口。
  - 新增 `POST /verifications` 校验验证码接口。
- 扩展 `CampusEmailVerificationService`：
  - 发送验证码时保存验证码哈希、过期时间和重发冷却时间。
  - 不在内存记录、响应或日志中保存/输出验证码明文。
  - 校验接口统一复用邮箱、用途和域名校验。
  - 正确验证码返回脱敏邮箱、`VERIFIED`、`verifiedAt` 和短期 `verificationTicket`。
  - 错误验证码返回 `EMAIL_VERIFICATION_CODE_INVALID`。
  - 过期验证码返回 `EMAIL_VERIFICATION_CODE_EXPIRED`。
  - 通过 `Clock` 注入支持测试中的可控时间，生产默认使用系统 UTC 时间。
- 本批没有实现：
  - 完整注册、登录、真实 JWT、refresh token、RBAC。
  - 认证资料提交、数据库业务表、JPA Entity、Repository。
  - 真实邮件生产发送或 Qt 页面。

## 绿灯记录

- 本批测试命令：`.\mvnw.cmd test -Dtest=CampusEmailVerificationEndpointTest`
- 本批测试结果：通过，`Tests run: 9, Failures: 0, Errors: 0, Skipped: 0`
- 必要回归命令：`.\mvnw.cmd test`
- 必要回归结果：通过，`Tests run: 16, Failures: 0, Errors: 0, Skipped: 0`
- 回归覆盖：
  - `CampusEmailVerificationEndpointTest`
  - `DatabaseMigrationTest`
  - `HealthEndpointTest`
  - `SecurityProbeEndpointTest`
  - `SystemInfoEndpointTest`

## 2026-05-18 复核补充

- 本批接口测试复跑命令：`.\mvnw.cmd test -Dtest=CampusEmailVerificationEndpointTest`
- 本批接口测试复跑结果：通过，`Tests run: 9, Failures: 0, Errors: 0, Skipped: 0`
- 后端完整回归复跑命令：`.\mvnw.cmd test`
- 后端完整回归复跑结果：未完成，`DatabaseMigrationTest` 在启动 Testcontainers 前失败。
- 阻塞原因：当前本机 Docker daemon 不可用，`docker info --format '{{.ServerVersion}}'` 返回 `open //./pipe/dockerDesktopLinuxEngine: The system cannot find the file specified`，Testcontainers 报告 `Could not find a valid Docker environment`。
- 本次复跑中已通过的非容器测试：
  - `CampusEmailVerificationEndpointTest`：9 个测试通过。
  - `HealthEndpointTest`：1 个测试通过。
  - `SecurityProbeEndpointTest`：3 个测试通过。
  - `SystemInfoEndpointTest`：2 个测试通过。
- 非容器回归复跑命令：`.\mvnw.cmd test "-Dtest=CampusEmailVerificationEndpointTest,HealthEndpointTest,SecurityProbeEndpointTest,SystemInfoEndpointTest"`
- 非容器回归复跑结果：通过，`Tests run: 15, Failures: 0, Errors: 0, Skipped: 0`
- 结论：校验校园邮箱验证码接口本批测试仍为绿灯；完整后端回归当前受 Docker 环境阻塞，需在 Docker Desktop / Docker daemon 可用后重新执行 `.\mvnw.cmd test`。

## 2026-05-18 09:19 本线程最终复核

- 本批接口测试复跑命令：`.\mvnw.cmd test -Dtest=CampusEmailVerificationEndpointTest`
- 本批接口测试复跑结果：通过，`Tests run: 9, Failures: 0, Errors: 0, Skipped: 0`
- 完整后端回归复跑命令：`.\mvnw.cmd test`
- 完整后端回归复跑结果：未完成，仍阻塞于 `DatabaseMigrationTest` 启动 Testcontainers，原因是当前本机没有可用 Docker 环境，错误为 `Could not find a valid Docker environment`。
- 非容器回归复跑命令：`.\mvnw.cmd test "-Dtest=CampusEmailVerificationEndpointTest,HealthEndpointTest,SecurityProbeEndpointTest,SystemInfoEndpointTest"`
- 非容器回归复跑结果：通过，`Tests run: 15, Failures: 0, Errors: 0, Skipped: 0`
- 结论：本接口闭环保持绿灯；完整数据库迁移回归需要在 Docker daemon 可用后补跑。

## 后续风险

- 当前验证码记录仍为进程内内存实现，服务重启后丢失；后续真实注册/登录链路需要迁移到数据库或缓存。
- 当前白名单域名仍为最小闭环测试值 `campus.edu.cn`，后续需要按真实学校域名配置化。
- 当前 `verificationTicket` 为短期占位凭据，只用于后续注册或提交链路的契约占位，不等同于登录 token。
- 当前邮件发送端仍为 no-op 替身，生产发送、模板、重试和投递失败处理均未实现。

## 下一步建议

- 下一项真实业务接口建议新开线程实现 `POST /api/auth/register`：注册或账号创建。
- 该线程仍需测试先行，重点覆盖已验证邮箱凭据、未验证邮箱拒绝、重复邮箱、字段校验和不自动签发真实登录 token 的边界。
## 2026-05-18 09:29 复核补充

- 复核命令：`.\mvnw.cmd test -Dtest=CampusEmailVerificationEndpointTest`
- 复核结果：通过，`Tests run: 9, Failures: 0, Errors: 0, Skipped: 0`
- 覆盖确认：
  - 正确验证码返回 `VERIFIED`、`verifiedAt`、`verificationTicket`。
  - 错误验证码返回 `EMAIL_VERIFICATION_CODE_INVALID`。
  - 过期验证码返回 `EMAIL_VERIFICATION_CODE_EXPIRED`；当前实现通过测试注入 `Clock` 完成可控时间验证。
  - 非白名单邮箱返回 `INVALID_CAMPUS_EMAIL_DOMAIN`。
  - 缺失或格式错误字段返回 `VALIDATION_FAILED`。
- 完整后端回归命令：`.\mvnw.cmd test`
- 完整后端回归结果：未完全通过，失败点为 `DatabaseMigrationTest` 启动 Testcontainers 前找不到可用 Docker 环境，错误为 `Could not find a valid Docker environment`。
- 非容器回归命令：`.\mvnw.cmd test "-Dtest=CampusEmailVerificationEndpointTest,HealthEndpointTest,SecurityProbeEndpointTest,SystemInfoEndpointTest"`
- 非容器回归结果：通过，`Tests run: 15, Failures: 0, Errors: 0, Skipped: 0`
