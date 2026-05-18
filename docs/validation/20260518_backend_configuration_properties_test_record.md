# 测试记录：后端配置体系与环境差异

## 基本信息

- 日期：2026-05-18
- 线程：正式开发第一轮 — 后端配置体系与环境差异
- 模块：后端配置
- 功能：CampusBuddyProperties 配置属性类 + Spring Profile + 消除硬编码
- 对应需求编号：跨模块基础设施
- 测试文件：`D:\big_homework\backend\src\test\java\com\campusbuddy\config\ConfigurationPropertiesTest.java`
- 实现文件：
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\config\CampusBuddyProperties.java`
  - `D:\big_homework\backend\src\main\resources\application-local.properties`
  - `D:\big_homework\backend\src\main\resources\application-test.properties`
  - `D:\big_homework\backend\src\main\resources\application-deploy.properties`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\auth\CampusEmailVerificationService.java`（修改）
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\auth\AuthRegistrationService.java`（修改）
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\CampusBuddyBackendApplication.java`（修改）

## 测试设计

- 测试类型：Spring Boot Test 配置属性加载测试。
- 正常路径：
  - `campusEmailAllowedDomainsAreLoadedFromConfiguration` — 白名单域名从配置加载，非 null，非空，包含 `campus.edu.cn`
  - `campusEmailAllowedDomainsAreNotHardcodedConstant` — 域名列表来自配置注入
  - `campusEmailCodeExpiresInSecondsHasPositiveValue` — 验证码过期秒数为正值
  - `campusEmailResendAfterSecondsHasPositiveValue` — 重发冷却秒数为正值
  - `objectStorageAccessModeIsBackendProxy` — 对象存储访问模式默认为 `backend-proxy`
  - `objectStoragePublicReadIsFalse` — 对象存储公共读默认为 false
  - `objectStorageCorsEnabledIsFalse` — 对象存储 CORS 默认为 false

## 红灯记录

- 运行命令：`.\mvnw.cmd test -Dtest=ConfigurationPropertiesTest`
- 失败测试：全部 7 个用例编译失败
- 失败原因：`CampusBuddyProperties` 类尚不存在，编译器无法解析 import
- 是否符合预期：符合。该结果证明配置属性类在实现前确实不存在。

## 实现摘要

- 新增 `CampusBuddyProperties` 配置属性类，前缀 `campus-buddy`：
  - `CampusEmail` 内部类：`allowedDomains`（Set<String>）、`codeExpiresInSeconds`、`resendAfterSeconds`
  - `ObjectStorage` 内部类：`provider`、`endpoint`、`region`、`bucket`、`accessMode`、`publicRead`、`corsEnabled`
- 新增三个 Spring Profile 配置文件：
  - `application-local.properties`：开发环境，日志 DEBUG
  - `application-test.properties`：测试环境，日志 DEBUG
  - `application-deploy.properties`：部署环境，日志 INFO，数据库连接预留环境变量模板
- 修改 `application.properties`：设置 `spring.profiles.active=local`
- 修改 `CampusBuddyBackendApplication`：添加 `@EnableConfigurationProperties(CampusBuddyProperties.class)`
- 修改 `CampusEmailVerificationService`：
  - 移除 `private static final Set<String> ALLOWED_DOMAINS = Set.of("campus.edu.cn")`
  - 从 `CampusBuddyProperties` 注入 `allowedDomains`、`codeExpiresInSeconds`、`resendAfterSeconds`
  - `sendCode` 和 `validate` 方法使用实例字段替代静态常量
- 修改 `AuthRegistrationService`：
  - 移除 `private static final Set<String> ALLOWED_DOMAINS = Set.of("campus.edu.cn")`
  - 从 `CampusBuddyProperties` 注入 `allowedDomains`
  - `validate` 方法使用实例字段替代静态常量
- 本批没有实现：真实数据库连接、真实 JWT、真实邮件发送、业务附件上传、新业务接口、Qt 客户端修改
- 本批没有写入任何敏感凭据

## 绿灯与回归记录

- 本批测试命令：`.\mvnw.cmd test -Dtest=ConfigurationPropertiesTest`
- 本批测试结果：通过，`Tests run: 7, Failures: 0, Errors: 0, Skipped: 0`
- 非容器后端全量回归命令：`.\mvnw.cmd test '-Dtest=ConfigurationPropertiesTest,CampusEmailVerificationEndpointTest,AuthRegistrationEndpointTest,AuthLoginEndpointTest,HealthEndpointTest,SecurityProbeEndpointTest,SystemInfoEndpointTest'`
- 非容器后端全量回归结果：通过，`Tests run: 31, Failures: 0, Errors: 0, Skipped: 0`
- 完整后端回归命令：`.\mvnw.cmd test`
- 完整后端回归结果：未完全通过，`DatabaseMigrationTest` 仍因 Docker 不可用失败（已知阻塞，本轮不处理）

## 后续风险

- 当前 `deploy` profile 的数据库连接、JWT 密钥和邮件发送仍为占位或注释模板，需在真实 JWT 和数据库迁移线程中补全。
- 校园邮箱真实域名尚未替换 `campus.edu.cn`，部署前必须修改。
- `DatabaseMigrationTest` 的 Docker 环境问题仍需专项线程解决。
- 对象存储敏感凭据（AK/SK）未在本轮涉及，部署时需通过环境变量或 IAM 委托提供。

## 下一步建议

- 建议下一项：Docker/Testcontainers + PostgreSQL/Flyway 环境验证（解决 `DatabaseMigrationTest` 阻塞）。
- 或：真实 JWT 最小安全链路验证。
- 或：P0 认证资料提交接口（需数据库迁移先行）。
