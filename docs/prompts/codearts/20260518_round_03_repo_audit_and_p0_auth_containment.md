# CodeArts Round 03 Prompt：仓库审计与 P0 认证开发收束

## 背景

上一轮 CodeArts 推进了 P0 账号认证持久化与 JWT 基础实现，但过程暴露出两个风险：

1. Git 基线提交 `447b0af` 使用了过大的提交边界，包含 Qt 构建目录、`.ninja_*`、生成物、备份目录等不应进入长期源码基线的内容。
2. 当前工作区存在大量 P0 认证持久化/JWT 未提交改动，以及若干临时测试输出文件。

因此本轮不是继续新增业务功能，而是做仓库审计、Git 卫生清理和当前开发状态收束。

## 给 CodeArts 的正式 Prompt

```text
你将继续接手《校园搭子平台》。本轮是“仓库审计与 P0 认证开发收束批次”，不是新增业务功能批次。

项目根目录：
D:\big_homework

请先阅读：
- D:\big_homework\AGENTS.md
- D:\big_homework\docs\03_current_plan.md
- D:\big_homework\handoff\latest.md
- D:\big_homework\docs\12_code_generation_constraints_v1.md
- D:\big_homework\docs\21_codearts_prompt_review_workflow_v1.md
- D:\big_homework\docs\22_codearts_unattended_prompt_engineering_v1.md

当前事实：
- 本地 Git 仓库位于 `D:\big_homework`。
- 当前分支是 `main`。
- 已有基线提交：`447b0af chore:-establish-project-baseline-and-backend-configuration`。
- 该基线提交疑似包含构建产物、生成物和备份目录。
- 当前工作区还有 P0 认证持久化/JWT 未提交改动。

本轮目标：
1. 审计当前 Git 状态和 `HEAD` 提交污染情况。
2. 清理 Git 跟踪边界，使构建产物、缓存、临时测试输出、备份目录不再作为后续提交内容。
3. 不新增业务功能。
4. 不扩大 P0 认证实现范围。
5. 判断当前 P0 认证持久化/JWT 改动是否达到可提交条件。
6. 若未达到可提交条件，输出最小收束计划。

本轮允许做：
- 查看 `git status --short --branch`、`git log --oneline --decorate -n 5`、`git show --stat --oneline HEAD`。
- 查看 `git diff --name-only`、`git diff --stat`。
- 更新 `.gitignore`，补充构建产物、缓存、测试临时输出和备份目录规则。
- 使用 `git rm --cached <明确路径>` 从 Git 跟踪中移除不应跟踪的构建产物或备份文件，但不得删除本地文件。
- 删除临时测试输出文件前，必须先列出文件路径并说明理由；如果无法确认，先不要删除，只更新 `.gitignore`。
- 运行必要测试以判断当前 P0 认证改动状态。
- 新增或更新一份审计记录：
  `docs/validation/20260518_codearts_repo_audit_and_p0_auth_containment_record.md`
- 更新 `docs/03_current_plan.md` 与 `handoff/latest.md`。

本轮明确不做：
- 不继续新增 P0 认证功能。
- 不实现附件上传。
- 不修改 Qt 客户端。
- 不申请域名、HTTPS 证书或备案。
- 不创建云产品、IAM 用户、访问密钥、委托或权限策略。
- 不把任何 AccessKey、SecretKey、SSH 私钥、数据库密码、JWT 真实密钥、对象存储临时凭证写入代码、文档、仓库或聊天。
- 不使用 `git add .`。
- 不使用 `git add -A`。
- 不使用 `git reset --hard`。
- 不删除用户文件，除非用户明确确认。

Git 纪律：
- 禁止 `git add .` 和 `git add -A`。
- 只能 `git add <明确文件路径>`。
- 如果需要取消跟踪已提交的构建产物，只能使用 `git rm --cached <明确路径>`，不得删除本地文件。
- 如果需要修正已污染的基线提交，先给出方案，不要擅自 rewrite history。

测试要求：
- 先运行当前可行的快速测试，至少包括非容器回归：
  `cd D:\big_homework\backend`
  `.\mvnw.cmd test '-Dspring.testcontainers.enabled=false' '-Dtest=!DatabaseMigrationTest'`
- 如果要运行 Testcontainers / Docker 相关测试，先确认 Docker 可用；不可用则记录环境阻塞，不要伪造绿灯。
- 不允许删除或弱化测试来换取通过。

完成后输出：
1. 当前 `git status --short --branch` 摘要。
2. `HEAD` 提交污染审计结论。
3. `.gitignore` 修改摘要。
4. 从 Git 跟踪中移除或建议移除的文件/目录。
5. 当前 P0 认证持久化/JWT 改动是否可提交。
6. 测试命令与结果。
7. 审计记录路径。
8. 是否创建提交；如果创建，提交哈希是什么。
9. 下一轮建议。
```

## 设计说明

这轮提示词专门压制 CodeArts 的弱点：Git 边界控制和大范围铺开。它要求先收束仓库状态，再决定是否继续开发，避免在污染基线上继续叠功能。
