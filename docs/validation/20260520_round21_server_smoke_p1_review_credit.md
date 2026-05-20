# 服务器 Smoke Test：P0 联系依赖 + P1 评价信用

## 基本信息

- 日期：2026-05-20
- 线程：CodeArts Round 21
- 模块：P0 低压力联系最小后端依赖 + P1-1 评价与信用摘要
- 批次：服务器 smoke test（V6/V7 迁移 + 关键接口验证）
- 服务器系统：Ubuntu 24.04.4 LTS (Noble Numbat)
- 后端版本 / commit：01e2270 feat(review): add my review list APIs
- 是否涉及数据库：是（Flyway V1-V7，含 V6 联系依赖表 + V7 评价表）
- 是否涉及对象存储：本批非关键路径（deploy profile 使用 InMemoryObjectStorageService 兜底，未验证真实 OBS 连通性）
- 是否涉及 Nginx / 公网 IP：是

## 服务器环境基线

| 项目 | 值 |
|---|---|
| 操作系统 | Ubuntu 24.04.4 LTS |
| Java 运行环境 | OpenJDK 21.0.10 (headless) |
| PostgreSQL | 17.9 (Docker 容器，绑定 127.0.0.1:5432) |
| Nginx | 1.24.0 (Ubuntu) |
| Docker | 29.1.3 |
| 磁盘 | 40G 总量，5.1G 已用 |
| 内存 | 1.7G 总量，~1.3G 可用 |
| 后端运行方式 | java -jar 直接运行，deploy profile |

## 部署配置修正（本轮最小修正）

本轮发现并修正了以下部署适配问题：

1. **application-deploy.properties**：数据库配置被注释掉，取消注释并使用环境变量占位符 `${DB_HOST}`、`${DB_USERNAME}`、`${DB_PASSWORD}`；补充 JPA 和 Flyway 配置。
2. **ObjectStorageConfiguration.java**：`InMemoryObjectStorageService` 的 `@Profile` 不包含 `deploy`，导致 deploy 环境启动失败。添加 `"deploy"` 到 `@Profile` 列表。
3. **deploy/docker-compose.yml**：添加 `SPRING_PROFILES_ACTIVE: deploy` 和 `JWT_SECRET: ${JWT_SECRET}` 环境变量；数据库密码改为 `${DB_PASSWORD:-campus_buddy_dev_password}` 形式。
4. **Nginx 反向代理**：新建 `/etc/nginx/sites-available/campus-buddy`，配置 `location /api/` 反向代理到 `127.0.0.1:8080`。

以上均为部署配置适配必须的最小修正，未实现新业务功能。

## 部署 / 重启结果

- 部署方式：本地构建 jar → scp 上传 → java -jar 直接运行
- 后端启动命令：`java -jar campus-buddy-backend-0.0.1-SNAPSHOT.jar --spring.profiles.active=deploy --spring.datasource.url=... --campus-buddy.security.jwt.secret=...`
- PostgreSQL 容器：`docker run --name campus-buddy-postgres postgres:17.9`，绑定 127.0.0.1:5432
- 服务状态：运行中，PID 212531
- 启动日志摘要：Flyway V1-V7 迁移成功，Tomcat port 8080，启动耗时 8.7s
- 结果：SUCCESS

## 健康检查

| 入口 | HTTP 状态 | 响应摘要 | 结果 |
|---|---|---|---|
| http://127.0.0.1:8080/api/health (localhost) | 200 | `{"status":"UP"}` | PASS |
| http://\<server-ip\>/api/health (Nginx 公网) | 200 | `{"status":"UP"}` | PASS |

## 数据库验证

- Flyway 迁移结果：V1-V7 全部成功应用
- 数据库连通性：PostgreSQL 17.9 连接正常
- 关键表确认：

| 表名 | 存在 | 关键约束 |
|---|---|---|
| conversation | 是 | chk_different_participants (CHECK), 主键, 2 外键 |
| conversation_message | 是 | 主键, conversation_id 外键 |
| contact_unlock_record | 是 | conversation_id UNIQUE, conversation_id 外键 |
| review | 是 | chk_rating_range (CHECK), chk_reviewer_not_reviewee (CHECK), uq_review_unique (UNIQUE), 3 外键 |

- 结果：PASS

## 关键接口 Smoke Test

### 基线测试（新用户，无评价数据）

| # | 接口 | 输入摘要 | 期望 | 实际 | 结果 |
|---|---|---|---|---|---|
| 1 | POST /api/auth/login | campusEmail+password | 登录成功获取 JWT | 成功 | PASS |
| 2 | GET /api/me/credit-summary | Bearer token | 新账号基线摘要 | avg=3.5, realConv=0, sample=6, disputed=0 | PASS |
| 3 | GET /api/users/{userId}/credit-summary | 公开摘要 | 不含 disputedReviewCount | 无该字段 | PASS |
| 4 | GET /api/me/reviews/given | Bearer token | 空分页 | items=[], total=0 | PASS |
| 5 | GET /api/me/reviews/received | Bearer token | 空分页 | items=[], total=0 | PASS |
| 6 | GET /api/me/credit-summary (无 token) | 无 Authorization | 401 | HTTP 401 | PASS |

### 评价提交后测试（插入有效会话 + 评价数据）

| # | 接口 | 输入摘要 | 期望 | 实际 | 结果 |
|---|---|---|---|---|---|
| 7 | POST /api/me/reviews | conversationId=1, revieweeId=用户2, rating=5 | 评价创建成功 | id=1, rating=5, status=ACTIVE | PASS |
| 8 | GET /api/me/credit-summary | 提交后 | realConvCount=1 | avg=3.6, realConv=1, sample=7 | PASS |
| 9 | GET /api/me/reviews/given | 提交后 | 1条评价 | totalElements=1 | PASS |
| 10 | GET /api/users/{userId}/credit-summary | 被评者公开摘要 | 不含 disputedReviewCount | 无该字段, avg=3.7 | PASS |

## 对象存储验证

- 是否适用：本批非关键路径
- deploy profile 使用 InMemoryObjectStorageService 兜底
- 真实 OBS 连通性未验证（无安全验证入口，不泄露凭据）
- 结果：非本批关键路径，未验证

## 私有配置检查

- JWT_SECRET 是否仅在服务器私有配置：是（通过 java -jar 命令行参数传入，不写入文件）
- 数据库密码是否仅在服务器私有配置：是（PostgreSQL 容器环境变量，绑定 127.0.0.1 不对公网开放）
- 对象存储凭据是否仅在服务器私有配置或 IAM 委托：deploy profile 未使用真实 OBS 凭据
- Qt 客户端是否不持有对象存储凭据：是
- 仓库 / 文档 / 聊天中是否未出现敏感明文：
  - application-test.properties 中有测试用 secret key（仅本地测试用，非生产）
  - docker-compose.yml 中有 `campus_buddy_dev_password` 开发默认值（已改为环境变量优先形式）
  - 无真实 JWT secret、数据库生产密码、OBS AK/SK 泄露
- 检查结论：通过

## 结论

- 是否通过服务器 smoke test：**是**
- Flyway V6/V7 迁移在 Ubuntu 24 + PostgreSQL 17.9 环境下成功执行
- P0 联系依赖（conversation, conversation_message, contact_unlock_record）表和约束正确创建
- P1 评价信用（review）表和约束正确创建
- 关键接口（健康检查、信用摘要、评价列表、评价提交）在服务器环境下全部正常
- 公开/本人信用摘要 DTO 差异（disputedReviewCount）在服务器环境下正确生效

## 未覆盖风险

1. 真实 OBS 对象存储连通性未验证（当前使用内存实现兜底）
2. HTTPS / 域名 / 备案未配置（当前为 HTTP 开发演示环境）
3. PostgreSQL 未做公网隔离验证（已绑定 127.0.0.1，但安全组规则未独立验证）
4. 后端进程管理未使用 systemd（当前为 nohup 直接运行，重启不自动）
5. 评价争议流程未实现
6. 超时修改评价（24 小时）未做服务器侧测试
7. 邮件发送为 Noop 实现，注册流程完整闭环未在服务器验证

## 下一步建议

1. **对象存储 OBS 真实连通性验证** — 实现 ObsObjectStorageService 后，在服务器上验证上传/读取。新开线程。
2. **后端 systemd 服务化** — 将后端注册为 systemd service，实现开机自启和日志管理。复用或新开。
3. **HTTPS / 域名配置** — 生产化增强项，非当前阻塞。新开线程。
4. **Qt 客户端服务器联调** — 配置 API base URL 为公网 IP，验证关键页面。新开线程。
5. **评价争议流程实现** — 后续功能批次。新开线程。
