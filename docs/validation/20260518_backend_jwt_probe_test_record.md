# 测试记录：后端安全链路 / JWT 占位探路

## 基本信息

- 日期：2026-05-18
- 线程：技术探路批次 D：JWT 安全链路占位验证
- 模块：后端安全基础设施
- 功能：Spring Security + JWT 占位过滤链，验证公开接口、受保护探路接口与测试 token
- 对应需求编号：技术基线验证；对应 `docs/13_detailed_design_v1.md` 第 3 章中 Spring Security、JWT access token 与 REST JSON 安全链路约束
- 测试文件：`D:\big_homework\backend\src\test\java\com\campusbuddy\security\SecurityProbeEndpointTest.java`
- 实现文件：
  - `D:\big_homework\backend\pom.xml`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\security\SecurityConfiguration.java`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\security\JwtPlaceholderAuthenticationFilter.java`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\security\JwtPlaceholderProperties.java`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\probe\SecureProbeController.java`

## 测试设计

- 测试类型：Spring Boot Test + MockMvc 安全链路接口测试
- 公开路径：`GET /api/health`
  - 断言匿名访问返回 HTTP `200`
  - 断言响应 JSON 包含 `status = "UP"`
- 受保护路径无 token：`GET /api/probe/secure`
  - 断言返回 HTTP `401`
  - 断言统一错误结构包含 `code/message/details/traceId`
- 受保护路径携带测试 token：`GET /api/probe/secure`
  - 请求头：`Authorization: Bearer technical-spike-test-token`
  - 断言返回 HTTP `200`
  - 断言响应包含 `authenticated = true`
  - 断言响应包含 `principal = "technical-spike-user"`
  - 断言响应包含 `authenticationMode = "jwt-placeholder"`

## 红灯记录

- 运行命令：`.\mvnw.cmd test -Dtest=SecurityProbeEndpointTest`
- 失败测试：
  - `SecurityProbeEndpointTest.secureProbeEndpointRejectsRequestWithoutToken`
  - `SecurityProbeEndpointTest.secureProbeEndpointAllowsRequestWithTestToken`
- 失败原因：
  - `/api/probe/secure` 尚未实现，Spring Security/JWT 占位链路也尚未配置。
  - 无 token 请求实际返回 `404 RESOURCE_NOT_FOUND`，测试期望 `401`。
  - 携带测试 token 请求实际返回 `404 RESOURCE_NOT_FOUND`，测试期望 `200`。
- 是否符合预期：符合。该结果证明安全链路占位能力在实现前确实不存在。

## 实现摘要

- 在 `pom.xml` 新增 `spring-boot-starter-security`。
- 新增 `SecurityConfiguration`：
  - 禁用 CSRF；
  - 使用无状态 session；
  - `GET /api/probe/secure` 需要认证；
  - `/api/health` 与 `/api/system/info` 保持匿名可访问；
  - 其他路径保持放行，使既有统一 `404` 契约不被安全链路改写。
- 新增 `JwtPlaceholderAuthenticationFilter`：
  - 只识别固定测试 token：`technical-spike-test-token`；
  - 认证主体固定为 `technical-spike-user`；
  - 不解析真实 JWT，不连接用户表，不处理 refresh token。
- 新增 `JwtPlaceholderProperties`：
  - 为测试 token 与主体名称保留配置入口。
- 新增 `SecureProbeController`：
  - 只提供 `GET /api/probe/secure` 探路接口；
  - 返回当前认证占位结果。
- 新增 401 入口响应：
  - 使用统一错误结构 `code/message/details/traceId`；
  - 不输出 token 原文。

## 绿灯记录

- 本批测试命令：`.\mvnw.cmd test -Dtest=SecurityProbeEndpointTest`
- 本批测试结果：通过，`Tests run: 3, Failures: 0, Errors: 0, Skipped: 0`
- 必要回归命令：`.\mvnw.cmd test`
- 必要回归结果：通过，`Tests run: 7, Failures: 0, Errors: 0, Skipped: 0`
- 回归覆盖：
  - `DatabaseMigrationTest`
  - `HealthEndpointTest`
  - `SecurityProbeEndpointTest`
  - `SystemInfoEndpointTest`

## 本批明确未覆盖内容

- 未实现注册、登录、密码校验或 BCrypt 存储。
- 未实现真实 JWT 签发、签名校验、过期时间校验或 claims 解析。
- 未实现 refresh token、注销、设备管理或 token 撤销。
- 未实现真实用户表、用户状态、RBAC 角色权限矩阵或方法级权限。
- 未实现生产级认证业务、OAuth2 Resource Server 或完整 OpenAPI 安全文档。

## 后续风险

- 当前测试 token 是技术探路占位方案，只能用于验证过滤链位置和受保护接口行为，不具备生产安全性。
- 引入 Spring Security 后，测试日志会出现 Spring Boot 默认开发密码提示；当前安全链路未启用注册登录，后续真实认证实现时需要替换为正式用户认证配置。
- 当前 401 响应只覆盖认证缺失或无效 token 场景，尚未覆盖权限不足、账号禁用、业务状态限制等错误码。
- 后续进入真实认证模块前，需要重新设计 JWT 密钥、签名算法、过期策略、刷新策略、撤销策略和客户端安全存储协作。

## 下一步建议

- 若继续后端技术探路，可进入 Docker Compose 本地部署探路，在 Spring Security 占位链路存在的前提下验证后端服务与 PostgreSQL 的本地组合启动。
- 若切换客户端方向，可进入 Qt Widgets 最小启动与 Qt Test 批次，继续按测试先行方式建立桌面端骨架。
