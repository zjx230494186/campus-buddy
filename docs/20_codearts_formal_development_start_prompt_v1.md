# 20 CodeArts 正式开发启动 Prompt V1

本文记录交给 CodeArts 智能体的正式开发开场 prompt。当前项目已经完成服务器、对象存储、公网 IP 开发演示访问方式和 Git 初始化确认。

## 当前 Git 状态

- 本地项目目录：`D:\big_homework`
- 本地 Git 状态：已初始化 Git 仓库，当前分支 `main`，尚无首个提交。
- 本地已新增 `.gitignore`，用于排除密钥、环境变量、构建产物、缓存和 IDE 文件。
- 服务器项目目录：`/srv/big_homework`
- 服务器 Git 状态：已初始化空 Git 仓库，当前分支 `main`，尚无首个提交。
- 后续正式代码开发必须使用 Git 进行版本管理。
- 不得把 AccessKey、SecretKey、SSH 私钥、数据库密码、JWT 密钥、对象存储临时凭证提交到 Git。
- 第一次提交前必须复核 `.gitignore` 与 `git status --short`。

## 可直接复制给 CodeArts 的启动 Prompt

```text
你将接手《校园搭子平台》项目的正式代码开发工作。请默认使用中文沟通。

项目根目录：
D:\big_homework

请先阅读以下文档，不要跳过：

1. 项目协作与开发规则
- D:\big_homework\AGENTS.md
- D:\big_homework\docs\00_project_map.md
- D:\big_homework\docs\03_current_plan.md
- D:\big_homework\handoff\latest.md

2. 需求、产品、原型与执行准备
- D:\big_homework\docs\04_product_design_v1.md
- D:\big_homework\docs\05_prototype_ia_v1.md
- D:\big_homework\docs\06_execution_preparation_v1.md

3. 代码生成与正式开发约束
- D:\big_homework\docs\12_code_generation_constraints_v1.md
- D:\big_homework\docs\13_detailed_design_v1.md
- D:\big_homework\docs\14_technical_spike_plan_v1.md

4. P0 相关接口与验证记录
- D:\big_homework\docs\15_p0_auth_api_contract_v1.md
- D:\big_homework\docs\validation\20260518_backend_auth_register_test_record.md
- D:\big_homework\docs\validation\20260518_backend_auth_login_test_record.md

5. 当前基础设施与配置结论
- D:\big_homework\docs\16_server_purchase_and_deployment_base_v1.md
- D:\big_homework\docs\17_object_storage_purchase_and_configuration_base_v1.md
- D:\big_homework\docs\18_object_storage_credential_min_permission_and_config_v1.md
- D:\big_homework\docs\19_public_ip_development_demo_access_decision_v1.md
- D:\big_homework\docs\20_codearts_formal_development_start_prompt_v1.md
- D:\big_homework\docs\validation\20260518_obs_min_connectivity_probe_record.md
- D:\big_homework\docs\validation\20260518_ecs_obs_min_connectivity_probe_record.md

当前项目概况：

本项目是一个 Win11 PC 端“校园搭子平台”，首版客户端采用 C++ + Qt Widgets，后端采用 Java 21 + Spring Boot，数据库采用 PostgreSQL，数据库迁移工具采用 Flyway，后端测试采用 JUnit 5 + Spring Boot Test + Testcontainers。客户端通过 HTTP/JSON 调用后端 REST API。

首版 P0 主链路包括：
1. 账号认证
2. 需求发布与审核
3. 广场发现
4. 低压力联系机制

当前已确认的关键技术约束：

- 客户端形态：Win11 桌面 PC 软件。
- Qt 技术路线：C++ + Qt Widgets。
- Qt 网络通信：QNetworkAccessManager。
- Qt token 存储：SecureTokenStore + Windows Credential Manager；禁止明文保存 token。
- Qt 普通配置：QSettings 只保存非敏感配置，例如 API base URL、超时、轮询间隔。
- 后端：Java 21 LTS + Spring Boot。
- 数据库：PostgreSQL。
- 数据库迁移：Flyway。
- 部署：Docker Compose。
- 当前开发/演示访问方式：直接使用 ECS 公网 IP，不申请域名，不申请 HTTPS 证书。
- 对象存储：华为云 OBS 已验证可用，但当前只视为“配件就位”，不要先实现业务附件上传。
- 对象存储访问方式：只能由后端服务访问 OBS；Qt 客户端不得持有 OBS AK/SK、临时凭证或直连 OBS。
- OBS 凭证：不得写入项目文档、仓库、聊天、代码注释或 Qt 客户端。
- 当前不做域名、HTTPS、备案、真实 IAM 委托创建、真实密钥创建。

当前基础设施状态：

- ECS 服务器已可用。
- OBS 桶 `20260518-bighomework` 已创建。
- 本机与 ECS 服务器端 OBS 最小连通性验证均已通过。
- 当前开发/演示阶段使用 ECS 公网 IP 即可。
- 域名和 HTTPS 是未来生产化增强项，不是当前开发阻塞项。

当前 Git 状态与版本管理要求：

- 本地项目目录 `D:\big_homework` 已初始化 Git 仓库，当前分支 `main`，尚无首个提交。
- 本地已新增 `.gitignore`，用于排除密钥、环境变量、构建产物、缓存和 IDE 文件。
- 服务器目录 `/srv/big_homework` 已初始化空 Git 仓库，当前分支 `main`，尚无首个提交。
- 后续正式代码开发必须使用 Git 进行版本管理。
- 每一轮正式开发前后都要查看 `git status --short --branch`。
- 首次提交前必须复核 `.gitignore` 与 `git status --short`。
- 提交前不得包含 AccessKey、SecretKey、SSH 私钥、数据库密码、JWT 密钥、对象存储临时凭证或其他敏感明文。
- 不要把服务器当作主开发源；本地仓库作为主要开发仓库，服务器仓库后续作为部署或同步目标。

非常重要的代码生成规则：

请严格遵守 D:\big_homework\docs\12_code_generation_constraints_v1.md：

1. 正式开发必须逐个模块、逐个功能推进。
2. 不得一次性生成大范围系统。
3. 每个模块或功能开始编码前，必须先写对应测试用例。
4. 编码前需要确认：在该功能尚未实现时，这批测试用例应当失败。
5. 完成功能实现后，必须运行并通过本批测试以及必要的既有回归测试。
6. 所有测试设计、失败记录、修复记录和通过结果都需要留档。
7. 不得绕过测试先行。
8. 不得在技术探路阶段继续横向扩展大量业务接口。

当前建议开发起点：

请不要从对象存储附件业务开始。对象存储已经就位，但附件上传属于后续业务功能。

建议你先做“正式开发起步：后端配置体系与 P0 最小开发闭环”。

优先候选方向：

A. 后端配置体系与环境差异
目标：
- 明确 local / test / deploy 配置边界。
- 建立后端配置读取结构。
- 区分非敏感配置和敏感配置。
- 不接入真实云密钥。
- 不把数据库密码、OBS 密钥、JWT 密钥写入仓库或文档明文。
- 形成配置矩阵和测试记录。

B. P0 账号认证最小闭环
目标：
- 在已有 P0 auth API contract 基础上，选择一个很小的正式实现闭环。
- 按测试先行推进。
- 不一次性实现完整账号系统。
- 不引入业务附件、审核、广场、消息等其他模块。

请你完成以下工作：

1. 先阅读上述文档，并总结你理解到的项目边界、技术栈、当前状态和严格禁止事项。
2. 检查当前代码仓库结构，说明已有后端、Qt、脚本、测试、文档分别在哪里。
3. 检查 Git 状态，确认当前分支、未跟踪文件和是否已有提交。
4. 根据文档，提出正式开发第一轮最小闭环计划。
5. 计划必须包含：
   - 本轮只做什么。
   - 本轮明确不做什么。
   - 需要新增或修改哪些测试。
   - 预期红灯测试是什么。
   - 实现后如何验证绿灯。
   - 结果记录写入哪个 docs/validation 文件。
   - 本轮 Git 提交边界是什么。
6. 在开始写代码前，先给出计划并等待确认。
7. 用户确认后，再按“测试先行 -> 红灯 -> 实现 -> 绿灯 -> 留档 -> 复核 git diff/status -> 提交建议”的流程独立编写代码。

严格禁止：

- 不要一次性生成完整系统。
- 不要实现业务附件上传接口。
- 不要创建对象存储业务表。
- 不要修改 Qt 客户端来直连 OBS。
- 不要把任何 AccessKey、SecretKey、SSH 私钥、数据库密码、JWT 密钥、临时凭证写入代码、文档、仓库或聊天。
- 不要申请域名、HTTPS 证书或备案。
- 不要创建新的云产品、IAM 用户、访问密钥、委托或权限策略。
- 不要把公网 IP 写死在深层业务逻辑中，应作为可配置 API base URL。
- 不要跳过测试记录。
- 不要在未复核 `git status` 和敏感信息风险前建议提交。
```
