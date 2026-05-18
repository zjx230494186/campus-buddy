# 测试记录：后端 REST JSON 契约 / 统一错误响应占位

## 基本信息

- 日期：2026-05-18
- 线程：技术探路批次 C：基础 REST JSON 契约与统一错误响应占位
- 模块：后端基础接口契约
- 功能：`GET /api/system/info` 与统一错误响应结构
- 对应需求编号：技术基线验证；对应 `docs/13_detailed_design_v1.md` 第 3 章“技术与部署约束”中的 REST JSON、统一错误响应和 traceId 约束
- 测试文件：`D:\big_homework\backend\src\test\java\com\campusbuddy\system\SystemInfoEndpointTest.java`
- 实现文件：
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\system\SystemInfoController.java`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\common\ApiErrorResponse.java`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\common\GlobalExceptionHandler.java`
  - `D:\big_homework\backend\src\main\java\com\campusbuddy\common\TraceIdFilter.java`

## 测试设计

- 测试类型：Spring Boot Test + MockMvc 接口测试
- 正常路径：`GET /api/system/info`
- 正常响应断言：
  - HTTP 状态码为 `200`
  - JSON 包含 `serviceName = "campus-buddy-backend"`
  - JSON 包含 `version = "0.0.1-SNAPSHOT"`
  - JSON 包含 `technicalSpike = true`
- 异常路径：`GET /api/unknown-resource`
- 错误响应断言：
  - HTTP 状态码为 `404`
  - JSON 包含 `code = "RESOURCE_NOT_FOUND"`
  - JSON 包含 `message = "Resource not found"`
  - JSON 包含 `details = "/api/unknown-resource"`
  - JSON 包含非空 `traceId`

## 红灯记录

- 运行命令：`.\mvnw.cmd test -Dtest=SystemInfoEndpointTest`
- 失败测试：
  - `SystemInfoEndpointTest.getSystemInfoReturnsBasicRestJsonContract`
  - `SystemInfoEndpointTest.unknownApiPathReturnsUnifiedErrorResponse`
- 失败原因：
  - `/api/system/info` 尚未实现，返回 `404`，测试期望 `200`
  - 不存在的 API 路径虽然返回 `404`，但响应体为空，没有统一错误结构字段 `code/message/details/traceId`
- 是否符合预期：符合。该结果证明接口和统一错误处理在实现前确实不存在。

## 实现摘要

- 新增 `SystemInfoController`，仅提供 `GET /api/system/info` 的技术探路信息。
- 新增 `ApiErrorResponse`，占位统一错误响应结构：`code/message/details/traceId`。
- 新增 `GlobalExceptionHandler`，先覆盖不存在资源的 `404 RESOURCE_NOT_FOUND` 场景。
- 新增 `TraceIdFilter`，为每个请求生成或传播 `X-Trace-Id`，并将 traceId 放入请求上下文与响应头。
- 本批没有实现完整认证、业务模块、JWT、RBAC、生产级 OpenAPI 全量文档或业务错误码体系。

## 绿灯记录

- 本批测试命令：`.\mvnw.cmd test -Dtest=SystemInfoEndpointTest`
- 本批测试结果：通过，`Tests run: 2, Failures: 0, Errors: 0, Skipped: 0`
- 必要回归命令：`.\mvnw.cmd test`
- 必要回归结果：通过，`Tests run: 4, Failures: 0, Errors: 0, Skipped: 0`
- 回归覆盖：
  - `DatabaseMigrationTest`
  - `HealthEndpointTest`
  - `SystemInfoEndpointTest`

## 后续风险

- 当前统一错误结构只覆盖不存在资源的 404 场景，尚未覆盖参数校验、认证失败、权限不足、业务规则冲突和服务器内部错误。
- 当前 `traceId` 已在请求级生成和响应头返回，但尚未接入结构化日志 MDC。
- 当前 `/api/system/info` 只作为技术探路接口，不代表业务系统版本、部署元数据或生产运维信息已经完整设计。
- OpenAPI 仍未做生产级全量文档；后续进入真实业务接口时，需要按接口契约约束补充文档入口。

## 下一步建议

- 进入技术探路批次 D：JWT 安全链路占位验证。
- 批次 D 仍需遵守测试先行：先验证公开接口、受保护探路接口、无 token 401 与测试 token 通过，再做最小安全链路实现。
