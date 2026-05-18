# CodeArts Round 02 Prompt：P0 账号认证模块正式实现

## 本轮背景与复核结论

CodeArts Round 01 已完成后端配置体系最小闭环：

- 新增 `CampusBuddyProperties` 配置属性类，覆盖 CampusEmail 与 ObjectStorage。
- 新增 Spring Profile：`local` / `test` / `deploy`。
- 将校园邮箱域名白名单、验证码过期时间、重发冷却从硬编码提取为配置项。
- 配置属性测试 7 个用例通过。
- 非容器后端回归 31 个用例通过。
- 配置矩阵文档与验证记录已落盘。
- `docs/03_current_plan.md` 与 `handoff/latest.md` 已更新。
- 敏感信息检查未发现真实凭据写入代码或文档；`deploy` profile 中数据库密码仅以 `${DB_PASSWORD}` 注释模板存在。

本线程已复核：

```powershell
cd D:\big_homework\backend
.\mvnw.cmd test '-Dspring.testcontainers.enabled=false' '-Dtest=!DatabaseMigrationTest'
```

复核结果：31 个测试全部通过。

当前 Git 状态：

- 本地仓库：`D:\big_homework`
- 分支：`main`
- 尚无首个提交。
- `.gitignore` 已存在，`target/`、密钥、环境变量、构建产物、缓存和 IDE 文件应被排除。

## 给 CodeArts 的正式 Prompt

```text
你将继续接手《校园搭子平台》的正式代码开发。请默认使用中文沟通。

项目根目录：
D:\big_homework

本轮不是继续做零散技术探路，而是进入 P0 账号认证模块的正式业务实现。你可以在一次对话中自主推进多个连续小闭环，但必须保持“测试先行 -> 红灯 -> 实现 -> 绿灯 -> 留档 -> Git 状态复核”的节奏。不要每完成一个很小的类就停下来问用户，除非遇到需要人类决策的需求冲突、安全风险或无法自行解决的环境阻塞。

请先阅读：
- D:\big_homework\AGENTS.md
- D:\big_homework\docs\03_current_plan.md
- D:\big_homework\handoff\latest.md
- D:\big_homework\docs\12_code_generation_constraints_v1.md
- D:\big_homework\docs\13_detailed_design_v1.md
- D:\big_homework\docs\14_technical_spike_plan_v1.md
- D:\big_homework\docs\15_p0_auth_api_contract_v1.md
- D:\big_homework\docs\21_backend_configuration_matrix_v1.md
- D:\big_homework\docs\validation\20260518_backend_configuration_properties_test_record.md

当前已完成：
- 后端配置体系第一轮闭环已完成。
- `CampusBuddyProperties`、local/test/deploy profile、配置属性测试已就位。
- 非容器后端回归 31 个测试已通过。
- 本地 Git 仓库已初始化，但尚无首个提交。

本轮第一步：先处理 Git 基线
1. 执行 `git status --short --branch`。
2. 复核 `.gitignore` 是否排除了 `backend/target/`、密钥、环境变量、构建产物和 IDE 文件。
3. 确认没有 AccessKey、SecretKey、SSH 私钥、数据库密码、JWT 密钥、对象存储临时凭证进入待提交内容。
4. 如果状态合理，请创建首个基线提交，提交内容应覆盖当前已完成的配置体系、文档和现有项目文件。
5. 提交信息建议：`chore: establish project baseline and backend configuration`
6. 提交前后都输出 `git status --short --branch` 摘要。

本轮正式开发目标：

进入 P0 账号认证模块的正式实现，不再只做“最小安全链路探路”。目标是在后端完成一个可运行、可测试、可交接的账号认证业务基础闭环。

推荐按以下内部阶段连续推进。每一阶段都必须先写测试，确认红灯，再实现绿灯；但你可以在同一轮对话中连续完成这些阶段，不必频繁停机询问。

阶段 A：数据库迁移与持久化基础
- 使用 Flyway 新增 P0 账号认证所需的最小数据库迁移。
- 至少包含用户账号、校园邮箱验证码/验证票据或等价持久化结构。
- 不要创建对象存储业务表。
- 不要实现认证材料附件上传。
- 补充或修复 `DatabaseMigrationTest`，优先解决 Testcontainers + PostgreSQL + Flyway 阻塞。
- 如果本地 Docker/Testcontainers 环境不可用，必须记录阻塞原因，并提供非容器替代验证不能替代真实迁移测试的说明。

阶段 B：注册/登录持久化
- 将当前内存状态的注册、登录相关逻辑迁移到 PostgreSQL/JPA 持久化路径。
- 保留现有 API 契约和错误响应结构，除非文档明确要求调整。
- 密码必须继续使用 BCrypt 哈希，不得明文保存。
- 邮箱唯一性、重复注册、无效 ticket、错误密码等行为需要测试覆盖。

阶段 C：真实 JWT 认证基础
- 用真实 JWT access token 替换当前占位 token。
- 使用可配置的 JWT 签名密钥入口，但不得把真实密钥写入仓库。
- test/local 可使用非敏感测试密钥；deploy 必须通过环境变量或外部私有配置注入。
- 至少覆盖：登录签发 JWT、合法 JWT 可访问受保护探针接口、无 token 返回 401、过期或签名错误 token 返回 401。
- 本轮可以不实现完整 refresh token / logout / 多设备撤销，除非已有文档明确要求且你能在测试先行下完成。

阶段 D：留档与交接
- 在 `docs/validation/` 新增本轮验证记录，建议文件名：
  `docs/validation/20260518_p0_auth_persistence_and_jwt_test_record.md`
- 更新 `docs/03_current_plan.md` 与 `handoff/latest.md`。
- 记录：
  - 本轮做了什么。
  - 哪些测试先红后绿。
  - 最终运行了哪些测试命令。
  - 哪些内容明确未做。
  - 下一步建议。
- 如创建 Git 提交，记录提交哈希。

本轮明确不做：
- 不实现业务附件上传接口。
- 不创建对象存储业务表。
- 不让 Qt 客户端直连 OBS。
- 不修改 Qt 客户端来适配尚未稳定的后端认证。
- 不申请域名、HTTPS 证书或备案。
- 不创建真实云产品、IAM 用户、访问密钥、委托或权限策略。
- 不把公网 IP 写死在深层业务逻辑中。
- 不把 AccessKey、SecretKey、SSH 私钥、数据库密码、JWT 真实密钥、对象存储临时凭证写入代码、文档、仓库或聊天。

自主推进要求：
- 你可以在本轮内连续完成多个阶段，但每个阶段都要有清晰测试边界。
- 不要因为“最小闭环”而只写一个很薄的探针；本轮目标是 P0 账号认证模块的正式业务基础。
- 不要一次性铺开 P0 全部模块；范围限定在账号认证。
- 如果发现现有设计与实现冲突，先指出冲突并给出推荐取舍；只有真正需要用户拍板时才停下来。

完成后请输出：
1. Git 提交情况，包括是否创建首个基线提交、是否创建本轮开发提交、提交哈希。
2. 文件变更摘要。
3. 测试命令与结果。
4. 验证记录路径。
5. 当前未完成事项和下一轮建议。
```

## 设计说明

这个 prompt 的意图是把 CodeArts 的工作粒度从“单点技术探路”提升为“模块级任务包”。它允许 CodeArts 在一次对话中连续完成多个小闭环，但仍要求每个内部阶段遵守测试先行和留档要求，避免一次性生成无测试的大系统。
