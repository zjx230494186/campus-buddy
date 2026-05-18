# 测试记录：后端工程骨架 / 健康检查

## 基本信息

- 日期：2026-05-17
- 线程：技术探路批次 A：后端工程骨架与健康检查
- 模块：后端基础工程
- 功能：`GET /api/health`
- 对应需求编号：技术基线验证；对应 `docs/13_detailed_design_v1.md` 第 3 章技术与部署约束
- 测试文件：`D:\big_homework\backend\src\test\java\com\campusbuddy\health\HealthEndpointTest.java`
- 实现文件：
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\CampusBuddyBackendApplication.java`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\health\HealthController.java`
  - `D:\big_homework\backend\pom.xml`

## 测试设计

- 测试类型：Spring Boot Test + MockMvc 接口测试
- 覆盖路径：正常访问 `GET /api/health`
- 断言内容：
  - HTTP 状态码为 `200`
  - 响应 JSON 包含 `status = "UP"`
- 本批不覆盖：
  - 数据库连接
  - 认证授权
  - 统一错误响应
  - OpenAPI 文档

## 红灯记录

- 首次运行命令：`mvn test -Dtest=HealthEndpointTest`
- 首次结果：失败，原因是 Spring Boot 4 中 MockMvc 测试自动配置已拆分，旧包名 `org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc` 不适用。
- 修正动作：
  - 在 `pom.xml` 中补充 `spring-boot-starter-webmvc-test`
  - 将测试导入改为 `org.springframework.boot.webmvc.test.autoconfigure.AutoConfigureMockMvc`
- 重新运行命令：`mvn test -Dtest=HealthEndpointTest`
- 预期红灯结果：失败
- 失败测试：`HealthEndpointTest.getHealthReturnsUp`
- 失败原因：`GET /api/health` 尚未实现，返回 `404`，断言期望 `200`
- 是否符合预期：符合。该结果证明健康检查接口在实现前确实不存在。

## 实现摘要

- 新增 Spring Boot 4.0.6 + Java 21 Maven 后端工程骨架。
- 新增 Maven Wrapper：
  - `D:\big_homework\backend\mvnw`
  - `D:\big_homework\backend\mvnw.cmd`
  - `D:\big_homework\backend\.mvn\wrapper\maven-wrapper.properties`
- 新增应用入口 `CampusBuddyBackendApplication`。
- 新增 `HealthController`，仅提供 `GET /api/health`。
- 响应体为最小 JSON：`{"status":"UP"}`。
- 未实现任何完整业务系统、认证、数据库、消息、发布、审核或用户相关能力。

## 绿灯记录

- 本批测试命令：`mvn test -Dtest=HealthEndpointTest`
- 本批测试结果：通过，`Tests run: 1, Failures: 0, Errors: 0, Skipped: 0`
- 必要回归命令：`.\mvnw.cmd test`
- 必要回归结果：通过，`Tests run: 1, Failures: 0, Errors: 0, Skipped: 0`
- Java 环境：Java 21.0.7
- Maven 环境：Apache Maven 3.9.10；Wrapper 已配置 Maven 3.9.10

## 后续风险

- 当前仅验证健康检查，不代表数据库、Flyway、Testcontainers 或 Docker Compose 可用。
- 测试运行时出现 Mockito/Byte Buddy 动态 agent 警告，不影响本批结果，但后续可在测试构建配置中显式处理。
- Spring Boot 4 的测试依赖已拆分，后续新增 WebMVC 测试时应继续使用 `spring-boot-starter-webmvc-test` 与新的测试包名。

## 下一步建议

- 复用当前技术探路上下文，进入批次 B：Testcontainers + PostgreSQL + Flyway 迁移验证。
- 批次 B 仍需先写测试、确认迁移脚本缺失时失败，再补最小迁移脚本并通过测试。
