# 03 Current Plan

这份文档只维护当前阶段最重要事项，不承载全部历史。历史乱码版本已备份到 `D:\big_homework\docs\encoding_repair_backup_20260518_1036\`。

## 文档编码规则

- 本项目 Markdown 文档统一使用 UTF-8 编码保存。
- 不使用 GBK/ANSI 保存长期维护文档。
- 使用 PowerShell 写入中文文档时必须显式指定 `-Encoding UTF8`。
- 若发现乱码，先备份原文件，再修复为 UTF-8，不直接覆盖唯一副本。

## 当前阶段主题

- 阶段：关键环境验证执行。
- 当前判断：
  - 当前新增方向：云服务器租用与部署底座确认，记录见 `D:\big_homework\docs\16_server_purchase_and_deployment_base_v1.md`。
  - 当前服务器采购状态：已创建华为云 ECS 按需计费短时实例，并完成 SSH 连通性确认。
  - 当前实际服务器：华为云 ECS，华北-北京四 / 可用区1，t6.large.1，2 vCPU / 2 GiB，Ubuntu 24.04.3 LTS，公网 IP `114.116.203.78`，私有 IP `192.168.0.165`，按需计费。
  - 当前服务器连接状态：已通过本机 PEM 文件完成 SSH 连通性确认，安全组已由用户确认无误；连接流程和踩坑记录已写入 `D:\big_homework\docs\16_server_purchase_and_deployment_base_v1.md` 第 7.2 节。
  - 当前成本控制要求：该实例为按需资源，验证结束后必须删除实例、云硬盘、公网 IP 等关联资源，避免持续扣费。
  - 技术探路边界已收束，详见 `D:\big_homework\docs\14_technical_spike_plan_v1.md`。
  - `Docker/Testcontainers + PostgreSQL/Flyway` 环境验证已完成，记录见 `D:\big_homework\docs\validation\20260518_backend_database_migration_environment_validation_record.md`。
  - 当前 4 个认证接口暂不继续横向扩展。
  - 后续不再按“每个业务接口都先探路实现一遍”的方式推进。
  - 技术探路只验证关键组件、环境和跨端集成风险：Docker/Testcontainers、PostgreSQL/Flyway、配置体系、对象存储、真实 JWT 最小链路、Qt HTTP/JSON、部署骨架等。
  - 后续业务实现回到正式模块迭代节奏：先确认模块边界、数据模型和接口契约，再按测试先行逐个子闭环推进。

## 当前已完成的 4 个认证接口

### 1. 发送校园邮箱验证码

- 接口：`POST /api/auth/campus-email/verification-codes`
- 测试记录：`D:\big_homework\docs\validation\20260518_backend_campus_email_verification_code_test_record.md`
- 契约回链：`D:\big_homework\docs\15_p0_auth_api_contract_v1.md` 第 4.1 节
- 可保留骨架：Controller/Service/DTO 分层、统一错误响应、发送冷却、域名校验入口。
- 占位性质：邮件发送为 no-op 测试替身；白名单域名仍为测试值 `campus.edu.cn`。

### 2. 校验校园邮箱验证码

- 接口：`POST /api/auth/campus-email/verifications`
- 测试记录：`D:\big_homework\docs\validation\20260518_backend_campus_email_verification_check_test_record.md`
- 契约回链：`D:\big_homework\docs\15_p0_auth_api_contract_v1.md` 第 4.2 节
- 可保留骨架：验证码校验状态流、ticket 签发与消费入口、可控时间测试方式。
- 占位性质：验证码和 ticket 为进程内内存状态，不是持久化实现。

### 3. 注册或账号创建

- 接口：`POST /api/auth/register`
- 测试记录：`D:\big_homework\docs\validation\20260518_backend_auth_register_test_record.md`
- 契约回链：`D:\big_homework\docs\15_p0_auth_api_contract_v1.md` 第 4.3 节
- 可保留骨架：注册接口形状、BCrypt 密码哈希、重复邮箱错误码、注册初始状态。
- 占位性质：账号为进程内内存状态；重复邮箱尚未落到数据库唯一约束；注册后不签发 token。

### 4. 登录

- 接口：`POST /api/auth/login`
- 测试记录：`D:\big_homework\docs\validation\20260518_backend_auth_login_test_record.md`
- 契约回链：`D:\big_homework\docs\15_p0_auth_api_contract_v1.md` 第 4.4 节
- 可保留骨架：登录接口形状、BCrypt 密码校验、统一错误凭证响应、token 响应字段样例。
- 占位性质：`accessToken` 不是真实 JWT；`refreshToken` 仅为内存会话占位；未实现刷新轮换、撤销、logout 或 RBAC。

## 最近验证结果

- Docker/Testcontainers + PostgreSQL/Flyway 环境验证：
  - 初始检查：Docker CLI 存在，但 Docker Desktop 未运行，`docker version` / `docker info` 无 Server 段，错误为 `open //./pipe/dockerDesktopLinuxEngine: The system cannot find the file specified.`；`com.docker.service` 为 `Stopped / Manual`。
  - 处理：启动 Docker Desktop。
  - 复查：`docker version` / `docker info` 通过，Docker Desktop `4.41.2`，Engine `28.1.1`，Docker Desktop Linux engine 可用。
  - 指定迁移测试：`.\mvnw.cmd test -Dtest=DatabaseMigrationTest`
    - 结果：通过，`Tests run: 1, Failures: 0, Errors: 0, Skipped: 0`
    - 结论：Testcontainers 能拉起 `postgres:17.9`，Flyway 成功应用 1 个迁移到 schema `public`，版本为 `v1`。
  - 完整后端回归：`.\mvnw.cmd test`
    - 结果：通过，`Tests run: 25, Failures: 0, Errors: 0, Skipped: 0`
    - 结论：完整后端回归不再因 `DatabaseMigrationTest` 找不到 Docker 环境失败。
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

## 当前明确不做

- 不在未人工确认前下单、付款或开通长期付费云服务。
- 不把云账号密码、SSH 私钥、AccessKey、数据库密码等敏感信息写入聊天或项目文档明文。
- 不部署业务代码，不创建业务数据库表，不修改后端或 Qt 代码。
- 不新增业务接口。
- 不继续实现 `POST /api/auth/token/refresh`。
- 不继续实现 `POST /api/auth/logout`。
- 不实现认证资料提交、认证状态查询或 Qt 页面。
- 不实现完整 RBAC、完整 refresh token 轮换、完整附件上传或完整业务数据库表。
- 不把内存账号、验证码、ticket 和 token 占位状态误标成生产实现。

## 下一阶段关键验证清单

1. `关键环境验证：后端配置体系与环境差异`
   - 优先级：高。
   - 成功标准：形成 local/test/deploy 配置矩阵；校园邮箱域名、JWT、数据库、对象存储、日志级别有配置入口；敏感项不入库明文。
   - 建议：新开规划或执行线程。
2. `关键环境验证：真实 JWT 安全链路`
   - 优先级：高。
   - 成功标准：公开接口匿名访问；受保护探路接口无 token 返回 401；合法 JWT 可访问；过期或签名错误 JWT 被拒绝。
   - 建议：新开执行线程，不扩展完整 RBAC。
3. `关键环境验证：对象存储最小连通性`
   - 优先级：中高。
   - 成功标准：MinIO 或 S3 兼容候选完成上传、读取校验、删除；配置和凭据边界清楚。
   - 建议：独立技术验证线程。
4. `关键环境验证：Qt HTTP/JSON 与 token 存储方案`
   - 优先级：中高。
   - 成功标准：Qt API Client 可解析后端成功响应和统一错误响应；明确 `SecureTokenStore` 与 Windows Credential Manager 路径；不明文保存 token。
   - 建议：独立技术验证线程。
5. `关键环境验证：部署骨架 Compose/Nginx/HTTPS`
   - 优先级：中。
   - 成功标准：Compose 可启动基础服务；Nginx 可反代健康检查；HTTPS 证书申请、续期、reload 和回滚方案成文。
   - 建议：独立技术验证线程。

## 推荐下一步

首选下一线程：`短时云服务器基础环境配置`

推荐理由：

- 云服务器已创建、SSH 已打通、安全组已确认，适合先完成服务器基础环境配置。
- 下一线程只做系统更新、普通运维用户、Docker Engine / Docker Compose Plugin、Nginx 基础安装验证。
- 该线程仍属于工程环境验证，不部署业务代码、不创建业务数据库表、不修改后端或 Qt 代码。

## 当前残余风险

- 当前 4 个认证接口已经可运行，但仍包含内存状态、no-op 邮件、测试域名和占位 token。
- Docker Desktop 需要在运行 `DatabaseMigrationTest` 或完整后端回归前保持启动；若重启电脑后 daemon 不可用，需要先启动 Docker Desktop 再补跑测试。
- 如果不先收束配置体系，JWT、对象存储和部署验证容易把本地配置、测试配置和部署配置混在一起。
- 技术探路阶段若继续实现业务接口，会提前生成半套不可持久化、不可安全上线的业务系统。
