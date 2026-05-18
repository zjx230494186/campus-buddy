# 测试记录：后端 P0-1 / 注册或账号创建接口

## 基本信息

- 日期：2026-05-18
- 线程：P0-1 注册或账号创建接口测试先行实现
- 模块：账号准入与身份认证
- 功能：`POST /api/auth/register`
- 对应需求编号：`SF1`、`IR1`、`IR2`
- 对应契约：`D:\big_homework\docs\15_p0_auth_api_contract_v1.md` 第 4.3 节
- 测试文件：`D:\big_homework\backend\src\test\java\com\campusbuddy\auth\AuthRegistrationEndpointTest.java`
- 实现文件：
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\auth\AuthRegistrationController.java`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\auth\AuthRegistrationService.java`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\auth\CampusEmailVerificationService.java`

## 文档编码规则

- 本记录使用 UTF-8 编码保存。
- 历史乱码版本已备份到 `D:\big_homework\docs\encoding_repair_backup_20260518_1036\20260518_backend_auth_register_test_record.md.bak`。

## 测试设计

- 测试类型：Spring Boot Test + MockMvc 接口测试。
- 成功路径：
  - 先通过 `POST /api/auth/campus-email/verification-codes` 发送验证码。
  - 再通过 `POST /api/auth/campus-email/verifications` 校验验证码并取得 `verificationTicket`。
  - 最后调用 `POST /api/auth/register` 创建账号。
  - 断言返回 `userId`、脱敏 `campusEmail`、`displayName`、`authenticationStatus=UNVERIFIED`、`campusEmailVerificationStatus=VERIFIED`、`createdAt`。
- 异常路径：
  - 未验证或无效 `verificationTicket` 返回 `CAMPUS_EMAIL_NOT_VERIFIED`。
  - 重复邮箱返回 `EMAIL_ALREADY_REGISTERED`。
  - 非白名单邮箱返回 `INVALID_CAMPUS_EMAIL_DOMAIN`。
  - 缺失或格式错误字段返回 `VALIDATION_FAILED`。
  - 注册后密码只保存 BCrypt 哈希，不保存明文。

## 红灯记录

- 运行命令：`.\mvnw.cmd test -Dtest=AuthRegistrationEndpointTest`
- 失败测试：
  - `registerCreatesUnverifiedAccountForVerifiedCampusEmail`
  - `registerRejectsInvalidVerificationTicket`
  - `registerRejectsDuplicateCampusEmail`
  - `registerRejectsInvalidCampusEmailDomain`
  - `registerRejectsMissingOrMalformedFields`
- 失败原因：`POST /api/auth/register` 尚未实现，注册相关用例命中既有 `404 RESOURCE_NOT_FOUND`。
- 是否符合预期：符合。该结果证明注册或账号创建接口在实现前确实不存在。

## 实现摘要

- 新增 `AuthRegistrationController`，只暴露 `POST /api/auth/register`。
- 新增 `AuthRegistrationService`：
  - 校验 `campusEmail`、`verificationTicket`、`password`、`displayName`。
  - 当前白名单域名为最小闭环测试域名 `campus.edu.cn`。
  - 消费邮箱验证码校验成功后签发的短期 `verificationTicket`。
  - 使用 `BCryptPasswordEncoder` 保存密码哈希，不保存明文密码。
  - 使用进程内内存表保存账号摘要，支持重复邮箱检测。
  - 成功返回脱敏邮箱、展示名、初始 `UNVERIFIED` 认证状态和 `VERIFIED` 邮箱状态。
- 扩展 `CampusEmailVerificationService`：
  - 校验验证码成功后保存短期 ticket 的哈希，不保存 ticket 原文。
  - 注册接口消费 ticket 后立即移除，避免重复使用。
- 本批没有实现登录、真实 JWT、refresh token、RBAC、认证资料提交、数据库业务表、JPA Entity/Repository 或 Qt 页面。
- 本批没有把验证码、`verificationTicket`、密码或 token 原文写入日志。

## 绿灯与回归记录

- 本批测试命令：`.\mvnw.cmd test -Dtest=AuthRegistrationEndpointTest`
- 本批测试结果：通过，`Tests run: 6, Failures: 0, Errors: 0, Skipped: 0`
- 认证相关回归命令：`.\mvnw.cmd test "-Dtest=AuthRegistrationEndpointTest,CampusEmailVerificationEndpointTest"`
- 认证相关回归结果：通过，`Tests run: 15, Failures: 0, Errors: 0, Skipped: 0`
- 非容器后端回归命令：`.\mvnw.cmd test "-Dtest=AuthRegistrationEndpointTest,CampusEmailVerificationEndpointTest,HealthEndpointTest,SecurityProbeEndpointTest,SystemInfoEndpointTest"`
- 非容器后端回归结果：通过，`Tests run: 21, Failures: 0, Errors: 0, Skipped: 0`
- 完整后端回归命令：`.\mvnw.cmd test`
- 完整后端回归结果：未完全通过，`Tests run: 22, Failures: 0, Errors: 1, Skipped: 0`。
- 完整回归阻塞原因：`DatabaseMigrationTest` 启动 Testcontainers 前找不到可用 Docker 环境，错误为 `Could not find a valid Docker environment`。

## 后续风险

- 当前账号、验证码和 ticket 仍为进程内内存实现，服务重启后丢失。
- 当前白名单域名仍为最小闭环测试值 `campus.edu.cn`，后续需要配置化。
- 当前注册接口不签发 access token 或 refresh token，注册后应继续通过后续登录接口获取令牌。
- 当前没有数据库唯一约束，重复邮箱检测仅在进程内账号表内生效。
- 完整数据库迁移回归需要在 Docker daemon 可用后补跑 `.\mvnw.cmd test`。

## 下一步建议

- 下一项真实业务接口建议新开线程实现 `POST /api/auth/login`。
- 登录线程仍应测试先行，重点覆盖 BCrypt 密码校验、错误凭证统一响应、字段校验，以及不把密码或 token 原文写入日志。

