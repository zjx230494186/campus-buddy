# 测试记录：后端 P0-1 / 发送校园邮箱验证码接口

## 基本信息

- 日期：2026-05-18
- 线程：P0-1 发送校园邮箱验证码接口测试先行实现
- 模块：账号准入与身份认证
- 功能：`POST /api/auth/campus-email/verification-codes`
- 对应需求编号：`SF1`、`IR1`
- 对应契约：`D:\big_homework\docs\15_p0_auth_api_contract_v1.md` 第 4.1 节
- 测试文件：`D:\big_homework\backend\src\test\java\com\campusbuddy\auth\CampusEmailVerificationEndpointTest.java`
- 实现文件：
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\auth\CampusEmailVerificationController.java`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\auth\CampusEmailVerificationService.java`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\auth\CampusEmailVerificationCodeSender.java`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\auth\NoopCampusEmailVerificationCodeSender.java`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\common\ApiException.java`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\common\GlobalExceptionHandler.java`

## 测试设计

- 测试类型：Spring Boot Test + MockMvc 接口测试
- 成功路径：合法校园邮箱 `student@campus.edu.cn` 发送验证码。
- 成功响应断言：
  - HTTP 状态码为 `200`
  - `verificationStatus = "CODE_SENT"`
  - `expiresInSeconds = 600`
  - `resendAfterSeconds = 60`
  - `campusEmail` 返回脱敏邮箱，不返回验证码明文
- 异常路径：
  - 非白名单邮箱域名返回 `INVALID_CAMPUS_EMAIL_DOMAIN`
  - 缺失或格式错误字段返回 `VALIDATION_FAILED`
  - 冷却期内重复发送返回 `EMAIL_VERIFICATION_TOO_FREQUENT`

## 红灯记录

- 运行命令：`.\mvnw.cmd test -Dtest=CampusEmailVerificationEndpointTest`
- 失败测试：
  - `sendCodeReturnsCodeSentForAllowedCampusEmail`
  - `sendCodeRejectsInvalidCampusEmailDomain`
  - `sendCodeRejectsMissingOrMalformedFields`
  - `sendCodeRejectsTooFrequentRequests`
- 失败原因：
  - `POST /api/auth/campus-email/verification-codes` 尚未实现，四个用例均命中既有 `404 RESOURCE_NOT_FOUND`。
  - 成功路径期望 `200`，实际为 `404`。
  - 非白名单、字段校验和冷却路径分别期望 `400/400/429`，实际均为 `404`。
- 是否符合预期：符合。该结果证明发送校园邮箱验证码接口在实现前确实不存在。

## 实现摘要

- 新增 `CampusEmailVerificationController`，只暴露 `POST /api/auth/campus-email/verification-codes`。
- 新增 `CampusEmailVerificationService`：
  - 校验 `campusEmail` 与 `purpose`。
  - 当前白名单域名为 `campus.edu.cn`，用于最小闭环测试。
  - 当前仅支持 `REGISTER_OR_LOGIN`。
  - 成功时返回脱敏邮箱、`CODE_SENT`、600 秒有效期和 60 秒重发冷却。
  - 使用内存记录实现最小冷却校验，重复发送返回 `EMAIL_VERIFICATION_TOO_FREQUENT`。
- 新增 `CampusEmailVerificationCodeSender` 与 `NoopCampusEmailVerificationCodeSender`，作为最小邮件发送端口与测试替身；本批不接入真实生产邮件发送。
- 新增 `ApiException` 并扩展 `GlobalExceptionHandler`，让业务错误沿用统一 `code/message/details/traceId` 结构。
- 本批没有实现完整注册、登录、真实 JWT、refresh token、RBAC、认证资料提交、数据库业务表、真实邮件生产发送或 Qt 页面。
- 验证码明文没有写入响应、文档或日志。

## 绿灯记录

- 本批测试命令：`.\mvnw.cmd test -Dtest=CampusEmailVerificationEndpointTest`
- 本批测试结果：通过，`Tests run: 4, Failures: 0, Errors: 0, Skipped: 0`
- 必要回归命令：`.\mvnw.cmd test`
- 必要回归结果：通过，`Tests run: 11, Failures: 0, Errors: 0, Skipped: 0`
- 回归覆盖：
  - `CampusEmailVerificationEndpointTest`
  - `DatabaseMigrationTest`
  - `HealthEndpointTest`
  - `SecurityProbeEndpointTest`
  - `SystemInfoEndpointTest`

## 后续风险

- 当前验证码记录与冷却状态为进程内内存实现，服务重启后丢失；后续真实注册/登录链路需要迁移到数据库或缓存。
- 当前白名单域名为最小闭环测试值 `campus.edu.cn`，后续需要根据真实学校域名配置化。
- 当前邮件发送端口为 no-op 替身，不发送真实邮件；生产发送、模板、重试和投递失败处理均未实现。
- 当前只实现发送验证码，不实现验证码校验、注册、登录或认证资料提交。

## 下一步建议

- 下一项真实业务接口建议新开线程实现 `POST /api/auth/campus-email/verifications`：校验校园邮箱验证码。
- 该线程仍需测试先行，覆盖正确验证码、错误验证码、过期验证码、域名校验和统一错误结构。
