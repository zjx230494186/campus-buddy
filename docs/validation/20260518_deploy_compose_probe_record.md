# 测试记录：本地部署 / Docker Compose 健康检查探路

## 基本信息

- 日期：2026-05-18
- 线程：技术探路批次 G：Docker Compose 本地部署探路
- 模块：本地部署基础设施
- 功能：使用 Docker Compose 启动 PostgreSQL 17.9 与后端服务，并验证 `GET /api/health`
- 对应需求/设计编号：技术基线验证；对应 `docs/13_detailed_design_v1.md` 第 3 章中的 PostgreSQL 17.9、Docker Compose、本地部署与后端 REST 服务约束
- 验证脚本：`D:\big_homework\deploy\verify_compose_health.ps1`
- 实现文件：
  - `D:\big_homework\deploy\docker-compose.yml`
  - `D:\big_homework\backend\Dockerfile`
  - `D:\big_homework\backend\.dockerignore`

## 本次任务边界

- 本批只验证本地 Compose 编排能启动 PostgreSQL 与当前最小后端。
- PostgreSQL 镜像固定为 `postgres:17.9`。
- 后端服务沿用当前 `D:\big_homework\backend` Spring Boot 4.0.6 + Java 21 最小工程。
- 不实现完整业务系统、完整认证、生产部署、Nginx HTTPS、对象存储或完整业务接口。
- Nginx 本批不接入，仅保留后续部署探路空间。

## 测试设计

- 新增 PowerShell 验证脚本 `verify_compose_health.ps1`。
- 脚本行为：
  - 检查 `deploy\docker-compose.yml` 是否存在；
  - 执行 `docker compose -p campus-buddy-local -f deploy\docker-compose.yml up -d --build`；
  - 轮询 `http://localhost:8080/api/health`；
  - 断言返回 JSON 中 `status = "UP"`；
  - 最后执行 `docker compose down --remove-orphans` 清理容器。

## 红灯记录

- 红灯运行命令：

```powershell
powershell -ExecutionPolicy Bypass -File D:\big_homework\deploy\verify_compose_health.ps1
```

- 红灯结果：失败，符合预期。
- 失败原因：`D:\big_homework\deploy\docker-compose.yml` 尚未实现，脚本入口检查报错：

```text
Compose file not found: D:\big_homework\deploy\docker-compose.yml
```

- 是否符合预期：符合。该失败证明 Compose 配置尚未实现时，本批验证不会误通过。

## 实现摘要

- 新增 `deploy\docker-compose.yml`：
  - `postgres` 服务使用 `postgres:17.9`；
  - 创建本地数据库 `campus_buddy`；
  - 暴露本机 `5432`；
  - 配置 `pg_isready` healthcheck；
  - `backend` 服务依赖 PostgreSQL healthy 后启动；
  - 通过环境变量注入 `SPRING_DATASOURCE_URL`、`SPRING_DATASOURCE_USERNAME`、`SPRING_DATASOURCE_PASSWORD` 与 `SPRING_FLYWAY_ENABLED`。
- 新增 `backend\Dockerfile`：
  - 使用 `eclipse-temurin:21-jdk` 构建；
  - 使用 `eclipse-temurin:21-jre` 运行；
  - 通过 Maven Wrapper 打包当前 Spring Boot 后端。
- 新增 `backend\.dockerignore`：
  - 排除 `target/` 和日志等不需要进入 Docker build context 的文件。
- 调整验证脚本：
  - 首轮绿灯定位中发现 `Invoke-RestMethod` 在当前环境访问 `localhost:8080` 会超时；
  - 经 `curl.exe` 验证宿主机与容器内均可访问 `GET /api/health`；
  - 脚本改用 `curl.exe --fail --silent --max-time 5` 获取健康检查响应。

## 绿灯记录

- Compose 验证命令：

```powershell
powershell -ExecutionPolicy Bypass -File D:\big_homework\deploy\verify_compose_health.ps1
```

- Compose 验证结果：通过。
- 关键结果：

```text
Health check passed: http://localhost:8080/api/health returned status=UP
```

- Compose 配置检查命令：

```powershell
docker compose -p campus-buddy-local -f D:\big_homework\deploy\docker-compose.yml config
```

- Compose 配置检查结果：通过，解析结果显示 `postgres` 使用 `postgres:17.9`，`backend` 映射 `8080:8080` 并依赖 `postgres` healthcheck。
- 必要回归命令：

```powershell
cd D:\big_homework\backend
.\mvnw.cmd test
```

- 必要回归结果：通过。

```text
Tests run: 7, Failures: 0, Errors: 0, Skipped: 0
BUILD SUCCESS
```

## 本批明确未覆盖内容

- 未实现完整业务系统。
- 未实现真实注册登录、refresh token、真实 JWT 签发与验签、RBAC 权限矩阵。
- 未新增完整业务接口。
- 未接入 Nginx、HTTPS、Certbot 或生产级反向代理配置。
- 未实现对象存储、附件上传、生产备份、监控告警或云服务器部署。
- 未建立生产级密钥管理，Compose 中数据库账号密码仅用于本地技术探路。

## 后续风险

- 当前 Compose 只验证本地开发部署，不代表生产部署安全性。
- `backend` 容器构建首次运行需要拉取基础镜像并下载 Maven 依赖，冷启动耗时较长。
- Spring Security 仍会输出开发用默认密码提示；这是批次 D 的占位安全链路遗留现象，后续真实认证实现时需要替换。
- 当前健康检查只验证后端 Web 服务可访问，不验证业务数据库读写接口。

## 下一步建议

- 第一轮技术探路 A-G 已形成最小工程闭环，建议暂时收束技术探路线程。
- 下一步可选择进入 P0 真实业务模块前的接口契约设计，或补充 Compose 小修，如后端容器 healthcheck、开发环境 `.env` 模板和更清晰的本地启动说明；若继续补充，仍需测试先行并另行留档。
