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
