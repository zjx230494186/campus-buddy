# 2026-05-18 Round 04 CodeArts Prompt：Git 提交门禁与剩余污染复核

以下提示词用于交给 CodeArts。目标是把上一轮已经完成的仓库清理和 P0 认证收束工作，安全地固定为可追溯 Git 提交；本轮不继续开发新业务功能。

```text
你将继续接手《校园搭子平台》开发。请默认使用中文沟通。

本轮名称：
Round 04：Git 提交门禁与剩余污染复核

本轮背景：
上一轮你已经完成：
1. Git 污染清理：移除部分 Qt 构建产物、LaTeX 产物、.arts/、ppt_extracted.txt。
2. .gitignore 补充：新增 Qt 构建目录、ninja 文件、autogen、LaTeX 构建目录、.arts/、调试输出文件等忽略规则。
3. P0 认证收束：修复 AuthRegistrationService 逻辑顺序、SecurityProbeEndpointTest 改用真实 JWT、SecureProbeController authenticationMode 更新。
4. 敏感信息审计通过：deploy profile JWT secret 通过 ${JWT_SECRET} 注入，无真实密钥泄露。
5. 非容器快速测试：32/32 通过。

当前关键风险：
1. 当前只有基线提交 `447b0af chore:-establish-project-baseline-and-backend-configuration`。
2. 当前清理改动、P0 认证/JWT 改动、文档改动尚未形成正式开发提交。
3. 基线提交曾包含生成物和构建产物；上一轮已清理一部分，但提交前必须再次审计是否仍有历史跟踪污染。

请先阅读：
- AGENTS.md
- docs/03_current_plan.md
- handoff/latest.md
- docs/21_codearts_prompt_review_workflow_v1.md
- docs/22_codearts_unattended_prompt_engineering_v1.md
- docs/validation/20260518_repo_audit_and_p0_auth_containment_record.md

本轮总目标：
把当前工作区收束为“可审查、可提交、可继续开发”的状态。

本轮只允许做：
1. Git 状态审计。
2. 剩余已跟踪生成物/构建产物审计。
3. 必要的 .gitignore 小修补。
4. 复跑非容器快速测试。
5. 敏感信息复查。
6. 在满足门禁条件后，用显式文件路径创建 Git 提交。
7. 更新必要的验证记录、docs/03_current_plan.md、handoff/latest.md。

本轮明确不做：
1. 不实现新的业务功能。
2. 不继续扩展 P0 认证资料提交接口。
3. 不替换 no-op 邮件发送。
4. 不实现 OBS 附件上传。
5. 不修改 Qt 客户端。
6. 不引入新云服务、新密钥、新 IAM、新数据库表。
7. 不重写历史提交，不执行 git reset --hard，不执行 git checkout -- <path>。
8. 不把任何真实密钥、密码、token、AccessKey、SecretKey、SSH 私钥写入代码、文档、仓库或聊天。

阶段 A：开始审计
请先运行并记录：

```powershell
git status --short --branch
git log --oneline -5
git diff --name-only
git diff --stat
```

然后检查仍被 Git 跟踪的可疑路径：

```powershell
git ls-files | Select-String -Pattern 'desktop/build|\.ninja_|backend/target|\.arts|test_output|test_error|ppt_extracted|latex_work/build'
```

请把输出分类为：
1. 明确应移出 Git 跟踪的构建/缓存/临时文件。
2. 可能是课程交付物或历史文档产物，不能擅自删除，需要记录并等待用户确认。
3. 无风险或无需处理的文件。

重要规则：
- 对 `desktop/build-*`、`.ninja_*`、`.arts/`、`backend/target/`、`backend/test_output*.txt`、`backend/test_error*.txt`、`ppt_extracted.txt` 这类明确构建/缓存/临时文件，可以纳入清理。
- 对 `latex_work/build_*/*.pdf`、`latex_work/build_*/*.docx` 这类可能属于课程历史交付产物的文件，不要擅自删除；如需从 Git 跟踪中移除，先在本轮报告中列出建议，等待用户确认。
- 如果无法判断某类文件是否应移除，停止，不要继续扩大清理范围。

阶段 B：测试与敏感信息门禁
在提交前必须复跑非容器快速测试：

```powershell
cd D:\big_homework\backend
.\mvnw.cmd test '-Dspring.testcontainers.enabled=false' '-Dtest=!DatabaseMigrationTest'
```

必须做敏感信息复查，至少覆盖：
- AccessKey
- SecretKey
- AK/SK
- JWT_SECRET 的真实值
- DB_PASSWORD 的真实值
- SSH 私钥
- 对象存储临时凭证
- `.env`
- `*.pem`
- `*.key`
- `*.p12`
- `*.pfx`

允许出现：
- `${JWT_SECRET}`
- `${DB_PASSWORD}`
- 测试 profile 中明确标注为测试用途的非真实 secret。

阶段 C：提交策略
如果阶段 A、B 均通过，本轮最多创建两个提交。必须使用显式文件路径暂存，禁止 `git add .` 和 `git add -A`。

优先拆成两个提交：

提交 1：仓库卫生与文档留档
- 内容：.gitignore、明确清理的构建/缓存/临时文件删除、仓库审计验证记录、current_plan/handoff 更新。
- 推荐提交信息：
  `chore(repo): clean generated artifacts and record git hygiene`

提交 2：P0 认证持久化与 JWT 收束
- 内容：P0 认证/JWT 相关源码、测试、迁移、配置模板、验证记录。
- 推荐提交信息：
  `feat(auth): persist authentication flow and introduce jwt`

如果文件边界无法清晰拆分，或者拆分会导致某个提交无法独立解释，则停止并输出建议，不要强行提交。

提交前必须输出将要暂存的文件清单。
提交后必须运行：

```powershell
git status --short --branch
git log --oneline -5
git show --stat --oneline HEAD
```

Git 禁止项：
- 禁止 `git add .`
- 禁止 `git add -A`
- 禁止提交 `backend/target/`
- 禁止提交 `desktop/build*/`
- 禁止提交 `.ninja_*`
- 禁止提交 `.arts/`
- 禁止提交测试输出日志
- 禁止提交真实密钥、密码、token、AccessKey、SecretKey、SSH 私钥

停止条件：
出现以下任一情况，立即停止，不要提交：
1. 非容器快速测试失败。
2. 敏感信息疑似进入仓库。
3. Git 状态中出现新的构建产物、缓存、临时日志或密钥文件。
4. 仍有大量无法分类的历史生成物需要用户判断。
5. 为了提交需要重写历史、reset hard、checkout 覆盖文件。
6. 提交边界无法拆清。

完成后请输出：
1. 本轮实际完成了什么。
2. 剩余 Git 污染审计结果。
3. 非容器快速测试命令与结果。
4. 敏感信息复查结论。
5. 创建了几个提交，提交哈希分别是什么。
6. 每个提交包含哪些类型文件。
7. 当前 `git status --short --branch` 摘要。
8. 更新了哪些文档和验证记录。
9. 明确未完成事项。
10. 下一轮是否可以进入“P0 认证资料提交接口”开发。
```

