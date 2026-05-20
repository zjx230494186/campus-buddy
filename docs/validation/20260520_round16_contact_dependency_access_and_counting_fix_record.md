# 测试记录：P0 联系最小依赖跨模块可用性小修

## 基本信息

- 日期：2026-05-20
- 线程：CodeArts Round 16
- 模块：P0 低压力联系最小后端依赖
- 功能：ContactContextService 跨包可用性 + 用户消息计数白名单修正
- 对应文档：docs/24_p0_contact_min_dependency_spec_v1.md
- 测试文件：
  - backend/src/test/java/com/campusbuddy/review/ContactContextServiceAccessTest.java（8 个新增测试）
  - backend/src/test/java/com/campusbuddy/contact/ContactContextServiceTest.java（19 个既有测试）
  - backend/src/test/java/com/campusbuddy/contact/ContactPersistenceIntegrationTest.java（6 个既有测试，1 个修改）
- 实现文件：
  - backend/src/main/java/com/campusbuddy/contact/ContactContextService.java（修改：public 类和方法）
  - backend/src/main/java/com/campusbuddy/contact/ConversationMessageRepository.java（修改：白名单查询方法）

## 红灯记录

- 运行命令：`.\mvnw.cmd test -Dtest=ContactContextServiceAccessTest`
- 失败测试：编译失败
- 失败原因：`ContactContextService` 在 `com.campusbuddy.contact` 中不是公共的，无法从 `com.campusbuddy.review` 外部程序包访问
- 是否符合预期：是

## 实现摘要

- 修改文件：2 个源码 + 1 个测试
- 修复 1：ContactContextService 从包内可见改为 public，所有 6 个 P1-1 所需方法改为 public
- 修复 2：countUserMessages 从 `messageTypeNot("SYSTEM")` 改为 `messageType("USER_TEXT")` 白名单口径，UNKNOWN 等未知类型不再被计入用户消息
- ConversationMessageRepository 查询方法从 `countBy...Not` 改为 `countBy...`（白名单）

## 绿灯记录

- 运行命令：`.\mvnw.cmd test -Dtest=ContactContextServiceTest,ContactPersistenceIntegrationTest,ContactContextServiceAccessTest`
- 通过测试：33/33（19 + 6 + 8）
- 回归测试命令：`.\mvnw.cmd test -Dspring.testcontainers.enabled=false -Dtest=!DatabaseMigrationTest`
- 回归测试结果：88/88 通过
- 结果：GREEN

## 服务器 smoke test 记录

- 是否适用：否（本轮不新增迁移、不改部署配置、不改数据库结构）

## 敏感信息检查

- 检查结论：通过（无新增敏感信息）
