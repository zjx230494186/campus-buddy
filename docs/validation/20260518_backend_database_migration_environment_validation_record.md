# 测试记录：关键环境验证 / Docker Testcontainers PostgreSQL Flyway

## 基本信息

- 日期：2026-05-18
- 线程：关键环境验证：Docker/Testcontainers + PostgreSQL/Flyway
- 模块：后端数据库迁移测试环境
- 功能：验证 Docker Desktop / Testcontainers 能拉起 PostgreSQL 17.9 并执行 Flyway 迁移
- 对应技术约束：`D:\big_homework\docs\12_code_generation_constraints_v1.md` 第 4 节；`D:\big_homework\docs\14_technical_spike_plan_v1.md` 第 3 节 P0-1
- 测试文件：`D:\big_homework\backend\src\test\java\com\campusbuddy\database\DatabaseMigrationTest.java`
- 迁移脚本：`D:\big_homework\backend\src\main\resources\db\migration\V1__technical_spike_baseline.sql`
- 本批代码变更：无

## 验证范围

- 检查 Docker daemon / Docker Desktop 是否可用。
- 运行 `.\mvnw.cmd test -Dtest=DatabaseMigrationTest`。
- 在 Docker 环境可用时，确认 Testcontainers 能启动 PostgreSQL 容器。
- 确认 Flyway 能执行 `classpath:db/migration` 下的迁移脚本。
- 补跑完整后端回归，确认不再因 `DatabaseMigrationTest` 找不到 Docker 环境失败。

## 严格未做事项

- 未新增业务接口。
- 未新增业务表。
- 未迁移当前内存账号、验证码、ticket 或 token 状态到数据库。
- 未修改认证业务接口实现。
- 未把 Docker 不可用时的失败伪装成测试通过。

## Docker 环境检查记录

### 初始检查

- 运行目录：`D:\big_homework\backend`
- 命令：`docker version`
- 初始结果：失败。
- 关键输出：
  - Docker Client 存在，版本为 `28.1.1`。
  - 当前 context 为 `desktop-linux`。
  - Server 连接失败：`open //./pipe/dockerDesktopLinuxEngine: The system cannot find the file specified.`

- 命令：`docker info`
- 初始结果：失败。
- 关键输出：
  - Client 存在，版本为 `28.1.1`。
  - Server 连接失败：`open //./pipe/dockerDesktopLinuxEngine: The system cannot find the file specified.`

- 命令：`Get-Service -Name 'com.docker.service' -ErrorAction SilentlyContinue | Select-Object Name,Status,StartType`
- 初始结果：
  - `com.docker.service`
  - `Status = Stopped`
  - `StartType = Manual`

- 命令：`docker context ls`
- 初始结果：
  - `default` 指向 `npipe:////./pipe/docker_engine`
  - `desktop-linux *` 指向 `npipe:////./pipe/dockerDesktopLinuxEngine`

- 命令：`docker ps`
- 初始结果：失败。
- 关键输出：`open //./pipe/dockerDesktopLinuxEngine: The system cannot find the file specified.`

### 启动 Docker Desktop 后复查

- 补救命令：`Start-Process -FilePath 'C:\Program Files\Docker\Docker\Docker Desktop.exe' -WindowStyle Hidden`
- 等待：20 秒

- 复查命令：`docker version`
- 复查结果：通过。
- 关键输出：
  - Client：Docker `28.1.1`
  - Server：Docker Desktop `4.41.2 (191736)`
  - Engine：Docker `28.1.1`
  - OS/Arch：`linux/amd64`

- 复查命令：`docker info`
- 复查结果：通过。
- 关键输出：
  - Server Version：`28.1.1`
  - Operating System：`Docker Desktop`
  - Kernel Version：`6.6.87.2-microsoft-standard-WSL2`
  - Containers：`0`
  - Images：`3`
  - Docker Root Dir：`/var/lib/docker`
  - HTTP Proxy：`http://127.0.0.1:7897`
  - HTTPS Proxy：`http://127.0.0.1:7897`

- 复查命令：`docker ps`
- 复查结果：通过，当前无运行中容器。

## DatabaseMigrationTest 记录

- 运行目录：`D:\big_homework\backend`
- 运行命令：`.\mvnw.cmd test -Dtest=DatabaseMigrationTest`
- 运行时间：2026-05-18 12:44 左右
- 结果：通过。
- Maven 汇总：`Tests run: 1, Failures: 0, Errors: 0, Skipped: 0`
- 构建结果：`BUILD SUCCESS`

关键日志：

- Testcontainers 版本：`1.21.3`
- Docker 环境：`Found Docker environment with local Npipe socket (npipe:////./pipe/docker_engine)`
- Docker Server：`28.1.1`
- PostgreSQL 镜像：`postgres:17.9`
- PostgreSQL 容器启动成功。
- Flyway 连接数据库：`PostgreSQL 17.9`
- Flyway 校验结果：`Successfully validated 1 migration`
- Flyway 迁移结果：`Successfully applied 1 migration to schema "public", now at version v1`

## 完整后端回归记录

- 运行目录：`D:\big_homework\backend`
- 运行命令：`.\mvnw.cmd test`
- 运行时间：2026-05-18 12:44 左右
- 结果：通过。
- Maven 汇总：`Tests run: 25, Failures: 0, Errors: 0, Skipped: 0`
- 构建结果：`BUILD SUCCESS`

本次完整回归已覆盖：

- `AuthLoginEndpointTest`
- `AuthRegistrationEndpointTest`
- `CampusEmailVerificationEndpointTest`
- `DatabaseMigrationTest`
- `HealthEndpointTest`
- `SecurityProbeEndpointTest`
- `SystemInfoEndpointTest`

## 结论

- 当前机器上 Docker CLI 已安装。
- 初始状态下 Docker Desktop 未运行，导致 Docker daemon 不可用。
- 启动 Docker Desktop 后，Docker daemon 可用。
- Testcontainers 能连接 Docker Desktop，并能拉起 `postgres:17.9` 容器。
- Flyway 能在 Testcontainers PostgreSQL 中执行 V1 迁移。
- 当前完整后端回归不再因 `DatabaseMigrationTest` 找不到 Docker 环境失败。

## 可重复运行命令

```powershell
cd D:\big_homework\backend
docker version
docker info
.\mvnw.cmd test -Dtest=DatabaseMigrationTest
.\mvnw.cmd test
```

如果后续再次出现 `Could not find a valid Docker environment` 或 Docker pipe 不存在：

1. 手动启动 Docker Desktop。
2. 等待 Docker Desktop 显示 engine running。
3. 运行 `docker version` 或 `docker info` 确认 Server 段存在。
4. 补跑：

```powershell
cd D:\big_homework\backend
.\mvnw.cmd test -Dtest=DatabaseMigrationTest
.\mvnw.cmd test
```

## 后续风险

- Docker Desktop 当前服务 `com.docker.service` 仍显示 `Stopped / Manual`，但 Docker Desktop Linux engine 已可通过用户进程访问；后续若重启电脑，需要先启动 Docker Desktop 再跑容器测试。
- Docker Desktop 当前记录了代理 `127.0.0.1:7897`。本次未阻塞既有镜像容器启动；若后续首次拉取新镜像失败，需要检查 Docker Desktop 代理设置或本机代理服务。
- 当前迁移脚本仍只是技术探路 marker 表，不代表业务数据模型已开始实现。
- 测试日志存在 Mockito / Byte Buddy 动态 agent 警告与 Testcontainers CloseableResource 警告，暂不影响本批测试通过。

## 下一步建议

- 本线程建议结束并归档。
- 下一项建议新开线程：`关键环境验证：后端配置体系与环境差异`。
- 不建议在本线程继续实现 refresh token、logout、认证资料提交、Qt 页面或业务持久化。
