# 测试记录：后端数据库迁移 / Testcontainers + PostgreSQL + Flyway

## 基本信息

- 日期：2026-05-17
- 线程：技术探路批次 B：Testcontainers + PostgreSQL + Flyway 迁移验证
- 模块：后端数据库迁移基础设施
- 功能：使用 Testcontainers 拉起 PostgreSQL 17.9，并通过 Flyway 执行最小迁移脚本
- 对应需求编号：技术基线验证；对应 `docs/13_detailed_design_v1.md` 第 3 章技术与部署约束
- 测试文件：`D:\big_homework\backend\src\test\java\com\campusbuddy\database\DatabaseMigrationTest.java`
- 实现文件：
  - `D:\big_homework\backend\pom.xml`
  - `D:\big_homework\backend\src\main\resources\db\migration\V1__technical_spike_baseline.sql`
  - `D:\big_homework\backend\src\test\java\com\campusbuddy\health\HealthEndpointTest.java`

## 测试设计

- 测试类型：JUnit 5 + Testcontainers + PostgreSQL + Flyway 集成测试
- 容器镜像：`postgres:17.9`
- 测试目标：
  - Testcontainers 能启动真实 PostgreSQL 容器
  - Flyway 能连接测试数据库并加载 `classpath:db/migration`
  - 至少执行一个版本号为 `1` 的迁移
  - 迁移后存在 `technical_spike_marker` 表
  - 表中存在 `marker_key = 'database_migration'` 且 `marker_value = 'flyway_testcontainers_postgresql'` 的记录
- 本批不覆盖：
  - 完整业务表结构
  - Spring Data JPA 实体映射
  - 应用运行时数据库连接配置
  - Docker Compose 部署
  - 生产级数据库初始化策略

## 红灯记录

- 首次运行命令：`.\mvnw.cmd test -Dtest=DatabaseMigrationTest`
- 首次结果：失败，原因是 `org.testcontainers:junit-jupiter` 与 `org.testcontainers:postgresql` 缺少版本管理。
- 修正动作：在 `pom.xml` 中引入 `org.testcontainers:testcontainers-bom:1.21.3` 作为测试容器依赖版本基线。
- 第二次运行结果：失败，原因是 Docker daemon 尚未启动，Testcontainers 无法找到可用 Docker 环境。
- 修正动作：启动 Docker Desktop。
- 第三次运行结果：命令超时，随后发现 Docker daemon 继承了 Windows 用户代理 `127.0.0.1:7897`，该地址在 Docker VM 中不可达，导致镜像拉取失败。
- 修正动作：
  - 临时保存并关闭 Windows 当前用户代理配置；
  - 重启 Docker Desktop；
  - 成功拉取 `postgres:17.9` 与 `testcontainers/ryuk:0.12.0`；
  - 测试完成后恢复 Windows 用户代理配置为 `ProxyEnable = 1`、`ProxyServer = 127.0.0.1:7897`。
- 预期红灯运行命令：`.\mvnw.cmd test -Dtest=DatabaseMigrationTest`
- 预期红灯结果：失败。
- 失败测试：`DatabaseMigrationTest.flywayMigrationCreatesTechnicalSpikeMarkerTable`
- 失败原因：Flyway 日志显示 `Successfully validated 0 migrations` 与 `No migrations found`，测试断言已应用迁移版本包含 `1`，实际为空列表。
- 是否符合预期：符合。该结果证明迁移脚本尚未实现时，数据库迁移验证测试会失败。

## 实现摘要

- 在 `pom.xml` 中新增：
  - `spring-boot-starter-jdbc`
  - `flyway-core`
  - `flyway-database-postgresql`
  - `postgresql`
  - `testcontainers-bom:1.21.3`
  - `testcontainers-junit-jupiter`
  - `testcontainers-postgresql`
- 新增最小迁移脚本 `V1__technical_spike_baseline.sql`：
  - 创建 `technical_spike_marker` 表；
  - 插入数据库迁移探路标记记录。
- 调整 `HealthEndpointTest`：
  - 在 health 接口回归测试中排除 `DataSourceAutoConfiguration`，避免数据库探路依赖影响不需要数据库的健康检查测试。
- 本批没有实现任何完整业务系统、业务表、注册登录、JWT、发布审核、消息或联系机制。

## 绿灯记录

- 本批测试命令：`.\mvnw.cmd test -Dtest=DatabaseMigrationTest`
- 本批测试结果：通过，`Tests run: 1, Failures: 0, Errors: 0, Skipped: 0`
- 关键日志：
  - PostgreSQL 容器启动成功，数据库版本为 PostgreSQL 17.9
  - Flyway `Successfully validated 1 migration`
  - Flyway `Successfully applied 1 migration to schema "public", now at version v1`
- 必要回归命令：`.\mvnw.cmd test`
- 必要回归结果：通过，`Tests run: 2, Failures: 0, Errors: 0, Skipped: 0`

## 后续风险

- 当前仅验证 Flyway 最小迁移链路，不代表业务表结构已经确定。
- 当前还没有应用运行时数据库连接配置；数据库连接只在集成测试中由 Testcontainers 提供。
- 当前 Docker Desktop 运行时可用，但镜像拉取曾受 Windows 代理配置影响；后续首次拉取新镜像时仍需关注 Docker Desktop 代理状态。
- 测试日志仍存在 Mockito / Byte Buddy 动态 agent 警告，暂不影响本批测试结果，后续可在测试构建配置中显式处理。

## 下一步建议

- 进入技术探路批次 C：基础 REST JSON 契约与统一错误响应占位。
- 批次 C 仍应遵守测试先行：先写 `/api/system/info` 与统一错误结构测试，确认未实现时失败，再做最小实现。
