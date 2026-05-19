# 03 Current Plan

本文只维护当前阶段最重要事项，不承载全部历史。

## 文档编码规则

- 项目 Markdown 文档统一使用 UTF-8 编码保存。
- 不使用 GBK/ANSI 保存长期维护文档。
- 使用 PowerShell 写入中文文档时必须显式指定 `-Encoding UTF8`。

## 当前阶段主题

- 阶段：详细设计深化 + 后续实现准备。
- 当前线程：详细设计深化：P1 支撑能力模块。
- 当前目标：P0 主链路详细设计已具备阶段性基线，P0-2 需求发布与审核已完成字段、状态、接口、DTO、异常、测试入口和追踪矩阵细化；下一步按软件工程过程模型进入 P1 文档工作，优先从“评价与信用摘要”开始，不写代码、不调用 CodeArts、不生成实现 prompt。

## Git 提交历史

- `447b0af` chore:-establish-project-baseline-and-backend-configuration
- `18e2a01` chore(repo): clean generated artifacts and record git hygiene
- `157abc0` feat(auth): persist authentication flow and introduce jwt
- `3be73bf` docs(codearts): record round04 handoff and round05 prompt
- `2ee5b68` feat(auth): add identity verification submission
- `d5c78fa` docs(codearts): record round05 handoff and round06 prompt
- `4955bee` feat(auth): add identity verification admin review
- `8edd057` feat(auth): add identity material attachment upload
- `b850d37` docs(codearts): record round06/07 validation and update handoff
- `ca058c3` feat(desktop): add login/register UI and auth API integration
- `957c283` docs(handoff): update latest.md after Qt client auth integration
- `8bb54bd` docs(codearts): record round07 handoff and round08 prompt
- `e53f11a` fix(desktop): align auth flow with backend contract
- `b44f519` docs(codearts): record round08 validation and update handoff
- `bc0ae9e` docs(codearts): record round08 handoff and round09 prompt
- `c59a138` feat(desktop): add identity verification submission UI
- `bb8b4a7` docs(codearts): record round09 validation and update handoff

## 已完成事项

### Qt 认证资料提交 UI（提交 `c59a138`）
- `CampusApiClient` 新增 multipart 上传能力。
- `AuthApiService` 新增认证材料上传、完整认证资料提交、扩展认证状态读取。
- `IdentityVerificationWidget` 支持真实姓名、学号、学院、专业、年级、材料选择、上传和提交。
- `HomePageWidget` 嵌入认证资料组件并传递认证状态。
- Qt 测试 3/3 通过。

### Qt 登录/注册最小 UI（提交 `ca058c3`）
- 新增 LoginWidget、RegisterWidget、HomePageWidget。
- 新增 AuthApiService、AuthTokenStore。
- CampusApiClient 支持 Bearer 认证头。
- Qt 测试 3/3 通过。

### Qt 认证契约修正（提交 `e53f11a`）
- 新增 `SecureTokenStore` 抽象和 `InMemorySessionTokenStore` 内存实现。
- token 不再写入 QSettings。
- `AuthResult` 拆分 `accessToken`、`authenticationStatus`、`verificationTicket`。
- 验证码接口统一传 `purpose=REGISTER_OR_LOGIN`。
- 校验验证码后读取 `verificationTicket`。
- 注册请求体改为 `campusEmail/verificationTicket/password/displayName`。
- 注册 UI 改为 email → 发送验证码 → 校验验证码 → displayName + 密码 → 注册。
- Qt 测试 3/3 通过。

### 认证材料附件上传闭环（提交 `8edd057`）
- Flyway V5 迁移：`identity_material_attachment` 表 + ALTER `identity_verification_submission` ADD `material_attachment_id`
- JPA 实体：`IdentityMaterialAttachment` + Repository
- `ObjectStorageService` 接口 + `InMemoryObjectStorageService` 测试替身
- `IdentityMaterialAttachmentService` + `IdentityVerificationMaterialController`（multipart 上传）
- 修改 `IdentityVerificationService` 支持 `materialAttachmentId`（含归属校验）
- 修改 AdminService/AdminController 返回附件摘要 + 管理员受控读取 GET /{submissionId}/material
- Security 配置新增 /materials 需认证
- 9 个集成测试通过，55+1=56/56 回归

### 管理员审核接口（提交 `4955bee`）
- Flyway V4 迁移：`user_account.account_role`，默认 `STUDENT`。
- 最小 ADMIN 授权：`accountRole`、JWT claim、`ROLE_ADMIN`。
- 管理员审核 Service + Controller。
- Security 配置：`/api/admin/**` 需要 `ROLE_ADMIN`，并新增 403 handler。
- 管理员审核闭环：`PENDING_REVIEW -> APPROVED/VERIFIED` 或 `REJECTED/REJECTED`。
- 7 个管理员审核集成测试通过。

### 认证资料提交接口（提交 `2ee5b68`）
- Flyway V3 迁移：`identity_verification_submission` 表
- JPA 实体：`IdentityVerificationSubmission`
- Repository：`IdentityVerificationSubmissionRepository`
- Service：`IdentityVerificationService`（submit + getStatus）
- Controller：`IdentityVerificationController`（POST + GET /me）
- Security 配置：`/api/auth/identity-verifications` 和 `/me` 需要 JWT
- UserAccount 新增 `setAuthenticationStatus` 方法
- 7 个集成测试全部通过

### 业务规则实现
- UNVERIFIED 用户提交资料 → PENDING_REVIEW
- PENDING_REVIEW 用户重复提交 → 409 IDENTITY_VERIFICATION_PENDING
- REJECTED 用户重新提交 → PENDING_REVIEW
- VERIFIED 用户重新提交 → 409 IDENTITY_ALREADY_VERIFIED
- 缺少必填字段 → 400 VALIDATION_FAILED
- 未登录提交 → 401 UNAUTHORIZED

### 当前测试状态
- 非容器快速测试：56/56 通过
- Testcontainers 测试：需要 Docker 可用才能运行

## 本轮边界说明

- 不实现 refresh token 轮换、logout、RBAC
- 真实 OBS SDK 未适配（InMemoryObjectStorageService 是测试替身）
- 真实邮件发送未替换（仍为 no-op）
- 不实现预签名 URL，不让 Qt 客户端直传

## 配置边界摘要

可进入项目文档或模板的非敏感配置：
- `campus-buddy.campus-email.*`、`campus-buddy.object-storage.*`、`campus-buddy.security.jwt.*`（不含 secret）

不得进入项目文档、仓库、聊天或 Qt 客户端的敏感配置：
- `JWT_SECRET`（deploy profile）、`OBJECT_STORAGE_ACCESS_KEY_ID`、`OBJECT_STORAGE_SECRET_ACCESS_KEY`、`DB_PASSWORD`

## 推荐下一步

1. `提交当前纯文档留档`
   - 优先级：最高。
   - 目标：P1-1 评价与信用摘要、P1-2 投诉申诉与案件、P1-3 管理端治理、P1-4 通知与留痕均已阶段性收束并补充追踪矩阵；后续实现环境门禁也已更新为本地测试 + Ubuntu 24 服务器 smoke test。建议先提交当前纯文档改动，避免后续实现准备混入设计尾巴。

2. `服务器实战部署准备清单`
   - 优先级：高。
   - 目标：围绕 Ubuntu 24 服务器、后端服务、数据库、对象存储、Nginx/公网 IP、私有配置和 smoke test 留档，形成后续实现前的部署准备清单。

3. `P1 投诉申诉与案件模块小修`
   - 优先级：高。
   - 目标：仅限发现字段、编号或表述小问题时处理。

4. `P1 评价与信用摘要模块小修`
   - 优先级：高。
   - 目标：仅限发现字段、编号或表述小问题时处理。

5. `P1 管理端治理模块小修`
   - 优先级：中。
   - 目标：仅限发现字段、编号或表述小问题时处理；P1-3 主体已阶段性收束。

6. `Windows Credential Manager 适配器`
   - 优先级：高。
   - 目标：替换当前内存 token store，使 token 可跨进程安全保存。

7. `真实 OBS SDK 适配器`
   - 优先级：中。
   - 目标：实现 HuaweiOBSObjectStorageService 替换 InMemoryObjectStorageService。
   - 需要华为云 OBS 凭据。

8. `替换 no-op 邮件发送`
   - 优先级：中。
   - 目标：接入 SMTP 或华为云邮件服务发送校园邮箱验证码。

9. `Testcontainers 集成测试`
   - 优先级：低。
   - 目标：在 Docker 可用环境运行完整集成测试。

10. `完整 RBAC 权限矩阵`
   - 优先级：低。
   - 目标：STUDENT / ADMIN / SUPER_ADMIN 角色权限细化。

## 后续实现环境原则

- 后续实现不再只以 Win11 本地环境通过为闭环，应尽早引入 Ubuntu 24 服务器实战部署验证。
- 后端、数据库、对象存储适配、Nginx/公网 IP 访问和运行配置应在服务器环境中验证；Win11 本地主要承担 Qt 客户端开发、基础单元测试和快速调试。
- 对象存储后续应从 `InMemoryObjectStorageService` 测试替身推进到真实 OBS/对象存储适配器；凭据只放服务器私有环境变量或私有配置，不写入仓库、文档或 Qt 客户端。
- 后续每个实现批次除本地测试外，应尽量增加服务器 smoke test 或部署验证记录，避免 Win11 与 Ubuntu 24 环境差异在最后集中暴露。
- 涉及后端、数据库迁移、对象存储、部署配置或公网访问的批次，完成门禁调整为“本地测试通过 + Ubuntu 24 服务器 smoke test 通过”；只在 Win11 本地通过不视为完整闭环。
- 服务器 smoke test 至少记录部署/重启结果、健康检查、关键接口最小调用、数据库迁移或连通性、对象存储连通性和私有配置就绪情况。
