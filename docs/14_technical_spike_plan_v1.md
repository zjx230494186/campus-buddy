# 14 Technical Spike Plan V1

本文用于收束《校园搭子平台》技术探路边界，并规划下一阶段关键工程环境验证。当前结论是：技术探路不再继续按业务接口逐个实现，而只验证关键组件、运行环境、跨端集成和部署链路的高风险点。

## 0. 文档状态

- 日期：2026-05-18
- 当前阶段：技术探路收束与关键工程环境验证规划
- 文档性质：阶段收束 + 后续验证清单
- 上游约束：
  - `D:\big_homework\AGENTS.md`
  - `D:\big_homework\docs\12_code_generation_constraints_v1.md`
  - `D:\big_homework\docs\13_detailed_design_v1.md`
  - `D:\big_homework\docs\15_p0_auth_api_contract_v1.md`
  - `D:\big_homework\docs\validation\20260518_backend_auth_register_test_record.md`
  - `D:\big_homework\docs\validation\20260518_backend_auth_login_test_record.md`

## 1. 收束结论

### 1.1 技术探路新边界

后续技术探路只验证以下类型问题：

- 本地、测试和部署环境是否可重复搭建。
- 关键组件版本组合是否可构建、可运行、可测试。
- 数据库迁移、对象存储、安全链路、跨端 HTTP/JSON、部署反代等基础设施是否可形成最小闭环。
- Qt 客户端与后端之间的错误响应、认证头、token 存储方案是否具备可落地路径。

后续技术探路不再覆盖以下类型工作：

- 不逐个业务接口做“探路式最小实现”。
- 不继续扩展 `refresh token`、`logout`、认证资料提交或 Qt 页面。
- 不以业务功能数量作为技术探路进度。
- 不在探路阶段生成完整业务表、完整 RBAC、完整附件业务或完整客户端页面。

### 1.2 业务实现回归正式节奏

当前 4 个认证接口暂不继续横向扩展。后续业务功能应回到正式模块迭代节奏：

1. 先确认模块边界、数据模型和接口契约。
2. 再按 `docs/12_code_generation_constraints_v1.md` 执行测试先行。
3. 每次只实现一个模块内的一个明确子闭环。
4. 完成红灯、最小实现、绿灯、回归和留档后，再进入下一项。

## 2. 当前 4 个认证接口复核

### 2.1 可保留骨架

以下内容可作为后续正式实现的可运行骨架样例保留：

| 接口 | 可保留价值 | 当前测试覆盖 |
|---|---|---|
| `POST /api/auth/campus-email/verification-codes` | Controller/Service/DTO 分层、统一错误响应、发送冷却、域名校验入口 | 合法邮箱、非白名单域名、字段校验、发送冷却 |
| `POST /api/auth/campus-email/verifications` | 验证码校验状态流、ticket 签发与消费入口、可控时间测试方式 | 正确验证码、错误验证码、过期验证码、字段校验 |
| `POST /api/auth/register` | 注册接口形状、BCrypt 密码哈希、重复邮箱错误码、注册初始状态 | 成功注册、无效 ticket、重复邮箱、字段校验、BCrypt 哈希 |
| `POST /api/auth/login` | 登录接口形状、BCrypt 密码校验、统一错误凭证响应、token 响应字段样例 | 成功登录、错误凭证、字段校验 |

### 2.2 占位或测试替身

以下内容不能当作生产实现或正式架构完成项：

| 项目 | 当前性质 | 后续处理 |
|---|---|---|
| 邮件发送 | `NoopCampusEmailVerificationCodeSender` 测试替身 | 后续在配置体系和通知能力明确后替换为真实邮件发送适配器 |
| 校园邮箱域名白名单 | 固定测试值 `campus.edu.cn` | 后续改为配置化，并区分本地、测试、部署环境 |
| 验证码、ticket、账号、refresh token 会话 | 进程内内存状态 | 后续正式业务实现时迁移到 PostgreSQL 持久化和唯一约束 |
| `accessToken` | 占位格式 | 后续在 JWT 最小技术验证通过后替换为真实 JWT |
| `refreshToken` | 只保存 SHA-256 哈希的内存会话占位 | 当前不继续实现刷新轮换、撤销或多设备策略 |
| 数据库业务表 | 未实现 | 后续按正式模块建模，不在技术探路中顺手扩展 |
| RBAC 与认证资料流 | 未实现 | 暂不进入技术探路扩展范围 |

### 2.3 冻结口径

- 当前 4 个认证接口可继续作为后端骨架样例和回归样例。
- 当前 4 个认证接口暂不继续横向扩展。
- 不继续实现 `POST /api/auth/token/refresh`、`POST /api/auth/logout`、`POST /api/me/authentication/submissions`、`GET /api/me/authentication/status`。
- 登录接口中的 token 字段只代表响应契约样例，不代表真实安全链路已经完成。

## 3. 下一阶段验证清单

### P0-1 Docker/Testcontainers + PostgreSQL/Flyway

- 优先级：最高。
- 建议线程：新开 `关键环境验证：Docker Testcontainers PostgreSQL Flyway`。
- 验证目标：
  - Docker daemon / Docker Desktop 可用。
  - `DatabaseMigrationTest` 能通过 Testcontainers 拉起真实 PostgreSQL。
  - Flyway 迁移脚本能重复执行并被测试断言。
- 成功标准：
  - `.\mvnw.cmd test -Dtest=DatabaseMigrationTest` 通过。
  - `.\mvnw.cmd test` 不再因 `Could not find a valid Docker environment` 失败。
  - 形成验证记录，写入 `docs\validation\`。
- 不做事项：
  - 不新增业务表。
  - 不把内存账号系统迁移到数据库。
  - 不实现新业务接口。

### P0-2 后端配置体系与环境差异

- 优先级：高。
- 建议线程：新开 `关键环境验证：后端配置体系`。
- 验证目标：
  - 明确 `local`、`test`、`deploy` 配置边界。
  - 校园邮箱域名、JWT 密钥、数据库连接、对象存储配置、日志级别均有合理配置入口。
  - 敏感配置不进入长期文档明文或仓库明文。
- 成功标准：
  - 至少形成配置矩阵文档。
  - 本地测试 profile 与部署 profile 的差异可说明、可复现。
  - 后端测试能在测试 profile 下稳定运行。
- 不做事项：
  - 不接入真实云密钥。
  - 不实现完整生产运维平台。

### P0-3 真实 JWT / 安全链路最小验证

- 优先级：高。
- 建议线程：新开 `关键环境验证：真实 JWT 安全链路`。
- 验证目标：
  - 使用 Spring Security 验证真实 JWT 签发、解析、过期和无效 token 拒绝。
  - 保持范围在最小安全链路，不扩展完整 RBAC。
  - 明确当前占位 token 与真实 JWT 的替换边界。
- 成功标准：
  - 公开接口匿名可访问。
  - 受保护探路接口无 token 返回 401。
  - 受保护探路接口携带合法 JWT 可访问。
  - 过期或签名错误 JWT 被拒绝。
- 不做事项：
  - 不实现 refresh token 轮换。
  - 不实现 logout。
  - 不实现角色权限矩阵。

### P1-1 对象存储最小连通性

- 优先级：中高。
- 建议线程：新开 `关键环境验证：对象存储最小连通性`。
- 验证目标：
  - 在 MinIO 或一个 S3 兼容候选上验证后端上传、下载、删除和对象 key 命名。
  - 明确本地 MinIO 与云厂商 OSS/COS/OBS 的配置差异。
  - 验证后端中转访问路径，不让客户端持有对象存储凭据。
- 成功标准：
  - 后端测试或探路脚本可完成一次上传、读取校验和删除。
  - 配置项和敏感项边界被记录。
  - 明确正式附件业务实现前仍需补充的权限、MIME、大小限制和元数据表。
- 不做事项：
  - 不实现附件上传业务接口。
  - 不接入认证资料提交。
  - 不暴露对象存储长期公开 URL 给客户端。

### P1-2 Qt 到后端 HTTP/JSON 连通

- 优先级：中高。
- 建议线程：新开 `关键环境验证：Qt HTTP JSON 与 token 存储方案`。
- 验证目标：
  - Qt API Client 层可访问后端健康检查或系统信息接口。
  - 能解析统一错误响应 `code/message/details/traceId`。
  - 明确 token 存储抽象 `SecureTokenStore` 与 Windows Credential Manager 的验证路径。
- 成功标准：
  - Qt Test 覆盖 JSON 成功响应解析。
  - Qt Test 覆盖错误响应转换。
  - 文档明确 `QSettings` 只保存非敏感配置，token 不落明文。
- 不做事项：
  - 不做登录页面。
  - 不做完整客户端导航或业务页面。
  - 不保存真实 token 到明文文件。

### P1-3 部署骨架：Compose + Nginx + HTTPS 方案

- 优先级：中。
- 建议线程：新开 `关键环境验证：部署骨架 Compose Nginx HTTPS`。
- 验证目标：
  - 明确后端、PostgreSQL、反向代理的 Docker Compose 关系。
  - 验证 Nginx 反向代理到后端健康检查。
  - 形成 HTTPS 证书方案对比：Certbot 或云厂商免费证书。
- 成功标准：
  - 本地 Compose 可启动基础服务。
  - 经 Nginx 访问后端健康检查成功。
  - 文档记录证书申请、续期、reload 和回滚的最小方案。
- 不做事项：
  - 不部署真实生产域名。
  - 不承诺生产可用 SLA。
  - 不扩展监控告警平台。

## 4. 推荐执行顺序

1. `Docker/Testcontainers + PostgreSQL/Flyway`
2. `后端配置体系与环境差异`
3. `真实 JWT / 安全链路最小验证`
4. `对象存储最小连通性`
5. `Qt 到后端 HTTP/JSON 连通`
6. `部署骨架：Compose + Nginx + HTTPS 方案`

推荐理由：

- 当前完整后端回归的唯一已知阻塞是 Docker/Testcontainers 不可用，因此数据库迁移测试环境应最先解决。
- 配置体系会影响 JWT、对象存储和部署验证，应前置。
- JWT 是后续客户端认证头、受保护接口和 token 存储验证的基础。
- 对象存储与部署骨架可以相对独立推进，但都不应混入业务接口实现。

## 5. 进入正式业务实现前门槛

建议同时满足以下条件后，再进入下一项 P0 业务模块实现：

1. Docker/Testcontainers + PostgreSQL/Flyway 可重复运行。
2. 后端配置体系已区分本地、测试、部署环境。
3. 真实 JWT 最小验签链路已有测试记录。
4. Qt API Client 至少完成 HTTP/JSON 成功与错误响应解析验证。
5. 对象存储候选已有最小连通性结论，或明确暂缓并说明替代方案。
6. `docs\03_current_plan.md` 与 `handoff\latest.md` 已同步最新阶段判断。

## 6. 当前线程结论

本轮收束后，技术探路的颗粒度从“业务接口逐项实现”调整为“关键环境和集成风险验证”。当前 4 个认证接口保留为可运行骨架和回归样例，但其中内存状态、no-op 邮件、测试域名和占位 token 都必须继续标记为非生产实现。后续业务实现回到正式模块迭代节奏，不在技术探路中继续横向铺开。
## 7. 2026-05-18 公网 IP 开发演示访问决策补充

当前项目首版交付形态是 Win11 PC 端 Qt 软件。用户已确认当前开发与课程演示阶段直接使用 ECS 公网 IP 作为后端访问入口，不申请域名，不申请 HTTPS 证书。

因此，本文前文中 `部署骨架：Compose + Nginx + HTTPS 方案` 的后续执行边界调整为：

- 当前阶段优先验证 `Compose + Nginx + 公网 IP HTTP`。
- HTTPS、域名、备案、DNS 解析和证书续期不作为当前开发或演示阻塞项。
- HTTPS 仍保留为未来生产化、真实用户试用或正式对外服务前的安全增强项。
- Qt 客户端应通过可配置 API base URL 访问后端，当前可配置为公网 IP。

详细决策记录见：`D:\big_homework\docs\19_public_ip_development_demo_access_decision_v1.md`。
