# Latest Handoff

这份文档服务于“新线程快速接手”。历史乱码版本已备份到 `D:\big_homework\docs\encoding_repair_backup_20260518_1036\`。

## 文档编码规则

- 本项目 Markdown 文档统一使用 UTF-8 编码保存。
- 不使用 GBK/ANSI 保存长期维护文档。
- 使用 PowerShell 写入中文文档时必须显式指定 `-Encoding UTF8`。
- 若发现乱码，先备份原文件，再修复为 UTF-8，不直接覆盖唯一副本。

## 当前状态

- 项目名称：校园搭子平台
- 当前阶段：关键环境验证执行
- 当前阶段补充：Docker/Testcontainers + PostgreSQL/Flyway 环境验证已完成
- 当前新增方向：云服务器租用与部署底座确认
- 当前服务器采购状态：已创建华为云 ECS 按需计费短时实例，并完成 SSH 连通性确认
- 当前服务器采购记录：`D:\big_homework\docs\16_server_purchase_and_deployment_base_v1.md`
- 当前实际服务器：华为云 ECS，华北-北京四 / 可用区1，t6.large.1，2 vCPU / 2 GiB，Ubuntu 24.04.3 LTS，公网 IP `114.116.203.78`，私有 IP `192.168.0.165`，按需计费
- 技术探路收束文档：`D:\big_homework\docs\14_technical_spike_plan_v1.md`
- 当前推荐下一步：`关键环境验证：后端配置体系与环境差异`

## 当前线程完成了什么

- 已阅读并遵守：
  - `D:\big_homework\AGENTS.md`
  - `D:\big_homework\docs\03_current_plan.md`
  - `D:\big_homework\handoff\latest.md`
  - `D:\big_homework\docs\13_detailed_design_v1.md`
  - `D:\big_homework\docs\14_technical_spike_plan_v1.md`
- 已查证阿里云、腾讯云、华为云官方页面或官方文档中的轻量云服务器计费、备案、实名或产品定位信息。
- 已新增服务器采购与部署底座记录：`D:\big_homework\docs\16_server_purchase_and_deployment_base_v1.md`。
- 已形成购买前确认清单、安全组最小开放端口建议和购买后记录模板。
- 用户最终选择华为云 ECS 按需计费短时实例，而不是长期包年包月轻量服务器。
- 已确认本机 PEM 私钥文件存在：`C:\Users\zjx230494186\.ssh\campus-huawei-ecs-20260518-BigHomework.pem`。
- 已修复该 PEM 文件 Windows ACL 权限过宽问题，当前 OpenSSH 可使用该私钥。
- 已通过 SSH 登录服务器，远端主机名 `ecs-ead3`，用户 `root`，系统 `Ubuntu 24.04.3 LTS`。
- 用户已确认服务器安全组无误。
- 已在服务器采购文档中补充连接流程 Runbook、Windows PEM 权限坑、SSH 命令、常见错误排查和按需资源清理提醒。
- 本线程没有记录任何密码、私钥、AccessKey 或长期可用凭据。
- 本线程没有部署业务代码、没有创建业务数据库表、没有修改后端或 Qt 代码。

## 上一环境验证线程完成了什么

- 已执行关键环境验证：Docker/Testcontainers + PostgreSQL/Flyway。
- 已新增验证记录：`D:\big_homework\docs\validation\20260518_backend_database_migration_environment_validation_record.md`。
- 已确认初始 Docker Desktop 未运行时，`docker version` / `docker info` 无法连接 `dockerDesktopLinuxEngine` 管道。
- 已启动 Docker Desktop 并复查 Docker daemon 可用：Docker Desktop `4.41.2`，Engine `28.1.1`。
- 已运行 `.\mvnw.cmd test -Dtest=DatabaseMigrationTest`，结果通过，`Tests run: 1, Failures: 0, Errors: 0, Skipped: 0`。
- 已确认 Testcontainers 能拉起 `postgres:17.9`，Flyway 成功应用 1 个迁移到 schema `public`，版本为 `v1`。
- 已运行完整后端回归 `.\mvnw.cmd test`，结果通过，`Tests run: 25, Failures: 0, Errors: 0, Skipped: 0`。
- 当前完整后端回归不再因 `DatabaseMigrationTest` 找不到 Docker 环境失败。
- 本线程没有修改业务代码、业务接口、业务表、认证实现或 Qt 页面。

## 上一规划线程完成了什么

- 已阅读并复核：
  - `D:\big_homework\AGENTS.md`
  - `D:\big_homework\docs\03_current_plan.md`
  - `D:\big_homework\handoff\latest.md`
  - `D:\big_homework\docs\12_code_generation_constraints_v1.md`
  - `D:\big_homework\docs\13_detailed_design_v1.md`
  - `D:\big_homework\docs\15_p0_auth_api_contract_v1.md`
  - `D:\big_homework\docs\validation\20260518_backend_auth_register_test_record.md`
  - `D:\big_homework\docs\validation\20260518_backend_auth_login_test_record.md`
- 已将技术探路边界收束为“关键组件、环境和跨端集成风险验证”，不再按业务接口逐个探路实现。
- 已复核当前 4 个认证接口，明确可保留骨架与占位/测试替身。
- 已更新 `D:\big_homework\docs\14_technical_spike_plan_v1.md`，写入后续验证清单、优先级、成功标准和线程拆分。
- 已更新 `D:\big_homework\docs\03_current_plan.md`，将当前阶段明确为“关键环境验证规划”。
- 本线程没有修改业务代码。

## 当前 4 个认证接口冻结口径

当前已完成接口：

- `POST /api/auth/campus-email/verification-codes`
- `POST /api/auth/campus-email/verifications`
- `POST /api/auth/register`
- `POST /api/auth/login`

可保留为骨架：

- Controller/Service/DTO 分层样例。
- 统一错误响应样例。
- BCrypt 密码哈希与登录校验样例。
- 验证码、注册、登录的接口测试和回归样例。

必须继续标记为占位或测试替身：

- 邮件发送为 no-op 测试替身。
- 校园邮箱域名为测试值 `campus.edu.cn`。
- 验证码、ticket、账号和 refresh token 会话为进程内内存状态。
- `accessToken` 不是真实 JWT。
- `refreshToken` 只保存 SHA-256 哈希作为内存会话占位。
- 未实现 refresh token 轮换、logout、RBAC、认证资料提交、认证状态查询、数据库业务表或 Qt 页面。

冻结结论：

- 当前 4 个认证接口暂不继续横向扩展。
- 不继续实现 `POST /api/auth/token/refresh`、`POST /api/auth/logout`、认证资料提交或 Qt 页面。
- 后续业务实现回到正式模块迭代节奏。

## 最近验证结果

- Docker/Testcontainers + PostgreSQL/Flyway 环境验证：
  - 初始检查：Docker CLI 存在，但 Docker Desktop 未运行，`docker version` / `docker info` 无 Server 段，错误为 `open //./pipe/dockerDesktopLinuxEngine: The system cannot find the file specified.`；`com.docker.service` 为 `Stopped / Manual`。
  - 补救动作：启动 Docker Desktop。
  - 复查结果：`docker version` / `docker info` 通过，Docker Desktop `4.41.2`，Engine `28.1.1`。
  - 指定迁移测试：`.\mvnw.cmd test -Dtest=DatabaseMigrationTest`
    - 结果：通过，`Tests run: 1, Failures: 0, Errors: 0, Skipped: 0`
  - 完整后端回归：`.\mvnw.cmd test`
    - 结果：通过，`Tests run: 25, Failures: 0, Errors: 0, Skipped: 0`
- 本批注册测试：`.\mvnw.cmd test -Dtest=AuthRegistrationEndpointTest`
  - 结果：通过，`Tests run: 6, Failures: 0, Errors: 0, Skipped: 0`
- 本批登录测试：`.\mvnw.cmd test -Dtest=AuthLoginEndpointTest`
  - 结果：通过，`Tests run: 3, Failures: 0, Errors: 0, Skipped: 0`
- 认证相关回归：`.\mvnw.cmd test "-Dtest=AuthLoginEndpointTest,AuthRegistrationEndpointTest,CampusEmailVerificationEndpointTest"`
  - 结果：通过，`Tests run: 18, Failures: 0, Errors: 0, Skipped: 0`
- 非容器后端回归：`.\mvnw.cmd test "-Dtest=AuthLoginEndpointTest,AuthRegistrationEndpointTest,CampusEmailVerificationEndpointTest,HealthEndpointTest,SecurityProbeEndpointTest,SystemInfoEndpointTest"`
  - 结果：通过，`Tests run: 24, Failures: 0, Errors: 0, Skipped: 0`
- 历史完整后端回归阻塞已处理：
  - 旧问题：`DatabaseMigrationTest` 启动 Testcontainers 前找不到可用 Docker 环境，错误为 `Could not find a valid Docker environment`。
  - 当前结论：启动 Docker Desktop 后已补跑通过。

## 下一步候选线程

1. `短时云服务器基础环境配置`
   - 建议：新开线程。
   - 优先级：高。
   - 目标：在华为云 ECS 上完成最小安全加固、系统更新、普通运维用户、Docker Engine / Compose Plugin、Nginx 基础安装验证；不部署业务代码，不创建业务数据库表。
2. `关键环境验证：后端配置体系`
   - 建议：新开线程。
   - 优先级：高。
   - 目标：梳理 local/test/deploy 配置矩阵，明确数据库、JWT、对象存储、校园邮箱域名和日志配置边界。
3. `关键环境验证：真实 JWT 安全链路`
   - 建议：新开线程。
   - 优先级：高。
   - 目标：验证真实 JWT 签发、验签、过期和错误 token 拒绝，不扩展完整 RBAC。
4. `关键环境验证：对象存储最小连通性`
   - 建议：独立线程。
   - 优先级：中高。
   - 目标：验证 MinIO 或 S3 兼容对象存储上传、读取校验、删除和配置边界。
5. `关键环境验证：Qt HTTP JSON 与 token 存储方案`
   - 建议：独立线程。
   - 优先级：中高。
   - 目标：验证 Qt API Client 到后端的 HTTP/JSON 连通、错误响应解析和 `SecureTokenStore` 路径。
6. `关键环境验证：部署骨架 Compose Nginx HTTPS`
   - 建议：独立线程。
   - 优先级：中。
   - 目标：验证 Compose 基础服务、Nginx 反代健康检查和 HTTPS 证书方案。

## 建议归档与下一线程

- 建议归档当前线程：是。
- 原因：本线程已完成云服务器候选方案比较、用户选定华为云 ECS 按需短时实例、服务器创建、密钥文件确认、PEM 权限修复、SSH 连通性验证、安全组确认、连接流程文档化、计划文档更新和交接文档更新。
- 下一线程名称：`短时云服务器基础环境配置`

## 下一线程可直接复制的启动 prompt

```text
请先阅读：
- D:\big_homework\AGENTS.md
- D:\big_homework\docs\03_current_plan.md
- D:\big_homework\handoff\latest.md
- D:\big_homework\docs\12_code_generation_constraints_v1.md
- D:\big_homework\docs\13_detailed_design_v1.md
- D:\big_homework\docs\14_technical_spike_plan_v1.md
- D:\big_homework\docs\16_server_purchase_and_deployment_base_v1.md

当前任务：短时云服务器基础环境配置。

本线程目标：
1. 基于已创建的华为云 ECS 按需短时实例，完成服务器基础环境配置。
2. 只做系统级基础设施准备：最小安全加固、系统更新、普通运维用户、Docker Engine / Docker Compose Plugin、Nginx 基础安装验证。
3. 不部署业务代码，不创建业务数据库表，不配置真实业务数据库密码。
4. 全程不把密码、SSH 私钥、AccessKey、数据库密码等敏感信息写入聊天或项目文档明文。
5. 形成可交接的服务器基础环境配置记录，并明确按需资源清理提醒。

必须完成：
1. 使用 `D:\big_homework\docs\16_server_purchase_and_deployment_base_v1.md` 中记录的公网 IP 和本机 PEM 路径登录服务器。
2. 复核安全组只开放必要端口：22、80、443；不得开放 5432 或全部端口。
3. 在服务器上检查系统版本、磁盘、内存、当前用户和基础网络。
4. 执行系统更新，并安装 Docker Engine、Docker Compose Plugin、Nginx。
5. 验证 `docker version`、`docker compose version`、`nginx -t` 和 Nginx 默认页或本机监听状态。
6. 新增或更新服务器基础环境配置记录。
7. 更新 `D:\big_homework\docs\03_current_plan.md` 和 `D:\big_homework\handoff\latest.md`。

严格限制：
1. 不部署业务代码。
2. 不创建业务数据库表。
3. 不修改后端或 Qt 代码。
4. 不把 PostgreSQL 5432 暴露到公网。
5. 不把任何真实密钥或密码写入项目文档明文。
6. 不创建长期付费附加云产品。
```
