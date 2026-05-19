# 21 CodeArts 提示词设计与提交复核工作流 V1

本文记录后续使用 CodeArts 智能体进行正式开发时，本项目如何设计提示词、复核 CodeArts 工作结果、形成下一轮提示词，并把过程留档供课程检查。

## 1. 当前线程定位

本线程用于：

- 接收用户对 CodeArts 已完成工作的描述。
- 检查 CodeArts 在本地 Git 仓库中的提交、变更、测试与留档情况。
- 与用户商量下一轮 CodeArts 工作目标。
- 生成可直接复制给 CodeArts 的提示词。
- 将重要提示词和复核结论落入项目文档。

本线程不直接承担大范围业务代码实现；如需我修复小问题，应先明确是否由本线程处理，避免和 CodeArts 的开发职责混杂。

## 2. 每轮标准输入

用户每次告知 CodeArts 已做工作时，建议至少提供：

```text
CodeArts 本轮做了什么：
-

它是否提交了 Git commit：
- 是 / 否 / 不确定

它声称运行了哪些测试：
-

它更新了哪些文档：
-

我希望下一轮让它做什么：
-
```

如果用户暂时不清楚上述信息，本线程会先通过本地 Git 状态和文件变更检查补齐事实。

## 3. 每轮复核清单

收到 CodeArts 工作结果后，优先执行以下检查：

1. Git 状态
   - `git status --short --branch`
   - `git log --oneline --decorate -n 5`
   - 如有提交，检查 `git show --stat --oneline HEAD`。

2. 变更范围
   - 识别新增、修改、删除文件。
   - 判断是否超出本轮提示词边界。
   - 判断是否修改了不应修改的文档、配置或云凭证相关文件。

3. 敏感信息
   - 检查是否出现 AccessKey、SecretKey、SSH 私钥、数据库密码、JWT 密钥、对象存储临时凭证、真实 token 等明文。
   - 检查 `.env`、私钥、构建产物是否被纳入 Git 跟踪。

4. 测试与留档
   - 复核 CodeArts 声称的测试命令是否实际可运行。
   - 检查是否符合“测试先行 -> 红灯 -> 实现 -> 绿灯 -> 留档”。
   - 检查 `docs/validation/` 是否有对应记录。

5. 项目边界
   - 不允许一次性生成大范围系统。
   - 不允许跳过用户确认直接进入新模块。
   - 不允许实现本轮明确不做的内容。

## 4. 下一轮提示词生成规则

每次给 CodeArts 的提示词应包含：

- 本轮背景。
- 必读文档。
- 当前 Git 状态摘要。
- 本轮只做什么。
- 本轮明确不做什么。
- 测试先行要求。
- 预期红灯测试。
- 实现后绿灯验证命令。
- 需要更新的验证记录路径。
- Git 提交边界。
- 敏感信息禁止项。

提示词应尽量让 CodeArts 独立完成一个最小闭环，但不得让它自行扩大范围。

## 5. 提示词归档方式

后续每轮提示词建议追加到本文档或新增专门文档：

```text
docs/prompts/codearts/YYYYMMDD_round_<序号>_<主题>.md
```

若本轮提示词很短，可直接追加到本文档“轮次记录”部分；若内容较长或很关键，应单独建档，并在本文档中索引。

## 6. 轮次记录模板

### Round N：待填写

- 日期：待填写
- CodeArts 本轮输入 prompt：待填写或链接到 `docs/prompts/codearts/...`
- CodeArts 本轮声称完成：待填写
- Git 提交：待填写
- 本线程复核结论：待填写
- 测试结果：待填写
- 留档结果：待填写
- 下一轮建议：待填写

## 9. Round 01 复核：后端配置体系与环境差异

- 日期：2026-05-18
- CodeArts 本轮声称完成：后端配置属性类、local/test/deploy profile、配置硬编码消除、配置测试、配置矩阵文档和验证记录。
- Git 提交：尚无提交。
- 本线程复核结论：关键文件存在；非容器后端回归已复跑通过。
- 复跑命令：

```powershell
cd D:\big_homework\backend
.\mvnw.cmd test '-Dspring.testcontainers.enabled=false' '-Dtest=!DatabaseMigrationTest'
```

- 复跑结果：31 个测试通过，0 失败，0 错误。
- 敏感信息：未发现真实 AK/SK、私钥或数据库密码；扫描出现的 `password` 多为代码字段、测试字段或 Maven Wrapper 脚本中的普通文本，`target/` 构建产物已被 `.gitignore` 排除，不应进入提交。
- 下一轮建议：让 CodeArts 进入 P0 账号认证模块的正式业务实现，但要求其内部继续按测试先行的小闭环推进。
- 下一轮 prompt：`D:\big_homework\docs\prompts\codearts\20260518_round_02_p0_auth_module_formal_implementation.md`

## 10. Round 02 后评估：P0 认证持久化与 JWT 大批次

- 日期：2026-05-18
- 评估来源：用户提供的 CodeArts 完整日志与 GPT-5.5 网页版总结；本线程补充核对本地 Git 状态。
- 当前提交：`447b0af chore:-establish-project-baseline-and-backend-configuration`。
- 主要进展：P0 账号认证持久化、JPA/Flyway/Testcontainers、真实 JWT、认证测试改造均有大幅推进。
- 主要风险：基线提交边界过大，包含构建产物、生成物、备份目录；P0 认证开发改动尚未形成稳定绿灯闭环和正式开发提交。
- 当前工作区状态：存在多项未提交后端认证/JWT 改动，以及 `backend/test_output*.txt`、`backend/test_error*.txt` 等临时测试输出。
- 提示词工程结论：后续可让 CodeArts 做长时间无人监督开发，但必须采用短批次、强 Git 护栏、明确停止条件和每轮审计。
- 优化文档：`D:\big_homework\docs\22_codearts_unattended_prompt_engineering_v1.md`
- 下一轮 prompt：`D:\big_homework\docs\prompts\codearts\20260518_round_03_repo_audit_and_p0_auth_containment.md`

## 11. Round 03 复核：仓库审计与 P0 认证收束

- 日期：2026-05-18
- CodeArts 本轮声称完成：Git 污染清理、`.gitignore` 补充、P0 认证/JWT 测试修复、敏感信息审计、非容器快速测试 32/32 通过、验证记录与交接文档更新。
- Git 提交：尚未创建新提交；当前仍只有基线提交 `447b0af chore:-establish-project-baseline-and-backend-configuration`。
- 本线程复核结论：工作区确实存在仓库清理、P0 认证/JWT 和文档留档改动；下一步不宜继续开发新业务，应先做提交门禁和剩余污染复核。
- 额外风险：`git ls-files` 仍能检出部分历史跟踪的 `desktop/build-f-red/` 与 `latex_work/build_*` 文件。提交前需要分类判断：明确构建产物应清理；可能属于课程历史交付物的 PDF/DOCX 不应擅自删除。
- 下一轮建议：执行 Git 提交门禁与剩余污染复核，在满足测试、敏感信息、提交边界条件后再创建提交。
- 下一轮 prompt：`D:\big_homework\docs\prompts\codearts\20260518_round_04_git_commit_gate_and_remaining_pollution_audit.md`

## 12. Round 04 复核：Git 提交门禁与剩余污染复核

- 日期：2026-05-18
- CodeArts 本轮声称完成：清理 14 个上一轮遗漏的跟踪构建产物，复跑非容器快速测试 32/32 通过，敏感信息复查通过，创建 2 个正式 Git 提交。
- Git 提交：
  - `18e2a01 chore(repo): clean generated artifacts and record git hygiene`
  - `157abc0 feat(auth): persist authentication flow and introduce jwt`
- 本线程复核结论：提交历史存在且顺序正确；`git ls-files` 对构建产物、缓存、临时输出等污染模式匹配为空。
- 注意事项：本地当前仍有 Round 04 后续交接文档和验证记录未提交，包括 `docs/03_current_plan.md`、`handoff/latest.md`、`docs/validation/20260518_round04_git_commit_gate_and_pollution_audit_record.md`。下一轮 CodeArts 开始前应先复核并提交这些纯文档改动，或确认由本线程另行提交。
- 下一轮建议：进入 P0 认证资料提交接口，但继续采用测试先行、短批次、显式 Git 暂存和提交门禁。
- 下一轮 prompt：`D:\big_homework\docs\prompts\codearts\20260518_round_05_p0_identity_profile_submission.md`

## 13. Round 05 复核：P0 认证资料提交接口

- 日期：2026-05-19
- CodeArts 本轮声称完成：校内身份认证资料提交与状态查询后端最小闭环，新增 6 个源码文件、1 个迁移、1 个测试文件，修改 `UserAccount` 和 `SecurityConfiguration`。
- Git 提交：`2ee5b68 feat(auth): add identity verification submission`。
- 测试结果：测试先行红灯 7/7 因 Controller 不存在返回 404 失败；实现后 7/7 通过；非容器快速回归 39/39 通过。
- 本线程复核结论：提交存在且范围符合 Round 05 边界；`git show --stat` 显示 8 个文件、655 行新增，与报告一致。
- 注意事项：CodeArts 提交后仍留下 `docs/03_current_plan.md`、`handoff/latest.md` 和 `docs/validation/20260519_round05_identity_verification_submission_record.md` 未提交；本线程又新增 Round 06 prompt，下一轮开头应先处理纯文档提交。
- 下一轮建议：优先做管理员审核接口，先闭合 `PENDING_REVIEW -> VERIFIED / REJECTED` 状态流转；暂缓 OBS 附件上传，避免过早引入云凭证和对象存储适配复杂度。
- 下一轮 prompt：`D:\big_homework\docs\prompts\codearts\20260519_round_06_identity_verification_admin_review.md`

## 14. Round 06 复核：认证资料管理员审核接口

- 日期：2026-05-19
- CodeArts 本轮声称完成：管理员审核闭环、最小 ADMIN 授权、`accountRole` + JWT claim + `ROLE_ADMIN`、403 handler。
- Git 提交：`4955bee feat(auth): add identity verification admin review`。
- 测试结果：本轮 7 个管理员审核测试通过；非容器快速回归 46/46 通过。
- 本线程复核结论：提交存在，`git show --stat` 显示 11 个文件、`+452/-8`，与报告一致；验证记录存在。
- 注意事项：CodeArts 提交后仍留下 `handoff/latest.md` 和 `docs/validation/20260519_round06_identity_verification_admin_review_record.md` 未提交；本线程又新增 Round 07 prompt，下一轮开头应先处理纯文档提交。
- 口径纠偏：下一轮附件上传不应采用“OBS 预签名/客户端直传”，应遵循既有详细设计和对象存储底座，采用后端中转上传、后端权限校验、ObjectStorageService 抽象和测试替身。
- 下一轮 prompt：`D:\big_homework\docs\prompts\codearts\20260519_round_07_identity_material_attachment_upload.md`

## 15. Round 07 与 Qt 方向复核：认证材料附件上传 + Qt 认证集成

- 日期：2026-05-19
- CodeArts 本轮实际提交：
  - `8edd057 feat(auth): add identity material attachment upload`
  - `b850d37 docs(codearts): record round06/07 validation and update handoff`
  - `ca058c3 feat(desktop): add login/register UI and auth API integration`
  - `957c283 docs(handoff): update latest.md after Qt client auth integration`
- 本线程复核结论：
  - Round 07 后端附件上传符合后端中转口径：不做预签名 URL、不连接真实 OBS、不写 AK/SK。
  - CodeArts 在用户选择后继续做了 Qt 登录/注册 UI 与 AuthApiService 集成，提交边界清晰，工作区干净。
  - Qt 认证集成存在契约风险：注册流程未按后端 `verificationTicket` 契约，验证码接口缺少 `purpose`，`authenticationStatus` 被塞进 `accessToken` 字段，且 access token 当前用 QSettings 持久化，违反 `docs/13_detailed_design_v1.md` 中 token 不得写入 QSettings 的安全约束。
- 下一轮建议：不要继续扩 UI，先做 Qt 认证集成契约审计与修正。
- 下一轮 prompt：`D:\big_homework\docs\prompts\codearts\20260519_round_08_qt_auth_integration_contract_fix.md`

## 16. Round 08 复核：Qt 认证集成契约审计与修正

- 日期：2026-05-19
- CodeArts 本轮声称完成：修正 Qt token 存储、验证码 purpose、verificationTicket、注册请求体、认证状态字段语义。
- Git 提交：
  - `e53f11a fix(desktop): align auth flow with backend contract`
  - `b44f519 docs(codearts): record round08 validation and update handoff`
- 测试结果：Qt 3/3 通过；`campus_api_client_test` 9 个测试通过；`auth_token_store_test` 6 个测试通过。后端未修改，56/56 状态不变。
- 本线程复核结论：提交存在且范围符合 Round 08 目标；token 已不再写入 QSettings，Qt 注册流程与后端 `verificationTicket` 契约对齐。
- 剩余风险：Windows Credential Manager 适配器未实现，当前 token 为内存会话存储，进程退出会丢失。
- 下一轮建议：为优先形成可演示学生端闭环，先做 Qt 认证资料提交 UI；Windows Credential Manager 可作为随后安全增强单独实现。
- 下一轮 prompt：`D:\big_homework\docs\prompts\codearts\20260519_round_09_qt_identity_submission_ui.md`

## 7. 当前初始状态

- 本地 Git 仓库：`D:\big_homework`
- 当前分支：`main`
- 当前状态：已初始化，尚无首个提交。
- 服务器 Git 仓库：`/srv/big_homework`
- 服务器状态：空仓库，尚无首个提交。
- CodeArts 初始启动 prompt 文档：`D:\big_homework\docs\20_codearts_formal_development_start_prompt_v1.md`

## 8. 严格禁止

- 不把 AccessKey、SecretKey、SSH 私钥、数据库密码、JWT 密钥、对象存储临时凭证写入提示词、文档、代码或 Git。
- 不要求 CodeArts 申请域名、HTTPS 证书、备案、IAM 用户、访问密钥或云产品。
- 不要求 CodeArts 直接操作真实云控制台。
- 不允许 Qt 客户端直连 OBS 或持有 OBS 凭证。
- 不允许跳过测试记录直接实现大范围业务。
