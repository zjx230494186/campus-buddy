# Latest Handoff

本文档服务于新线程快速接手。

## 当前状态

- 项目名称：校园搭子平台。
- 当前阶段：正式代码开发。
- 当前线程：Round 08 Qt 认证集成契约审计与修正（已完成）。
- Git 分支：main。
- 工作区：干净。
- 后端测试：56/56 通过。
- Qt 测试：3/3 通过（api_client_config_test、campus_api_client_test、auth_token_store_test）。

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

## 当前线程完成了什么

- SecureTokenStore 抽象 + InMemorySessionTokenStore 内存实现（token 不写 QSettings）
- AuthTokenStore 改为内存存储，不再使用 QSettings
- AuthResult 新增 authenticationStatus、verificationTicket 独立字段
- sendVerificationCode：请求体包含 purpose=REGISTER_OR_LOGIN
- verifyCampusEmail：请求体包含 purpose，读取 verificationTicket
- registerAccount：请求体使用 campusEmail/verificationTicket/password/displayName
- RegisterWidget：email → 发送验证码 → 校验验证码 → displayName+密码 → 注册
- HomePageWidget：读取 result.authenticationStatus
- Qt 3/3 测试通过，后端 56/56 不变

## 关键结论

- Qt 客户端认证流程与后端真实契约对齐
- Token 不再持久化到 QSettings（符合详细设计约束）
- 当前 token 为内存会话存储，进程退出即丢失

## 本线程没有做什么

- 没有实现 Windows Credential Manager 适配器
- 没有实现认证资料提交 UI
- 没有实现附件上传 UI
- 没有实现管理员审核 UI
- 没有适配真实 OBS SDK
- 没有替换 no-op 邮件发送
- 没有开始 P1 需求发布与审核模块

## 下一步候选事项

1. `提交 Round 09 纯文档留档` — 复用当前线程或交给 CodeArts 在下一轮开头处理，优先级最高
2. `Qt 认证资料提交 UI` — 复用或新开线程，优先级高；在 HomePageWidget 或独立 Widget 中添加认证资料表单和附件上传
3. `Windows Credential Manager 适配器` — 新开线程，优先级高；替换 InMemorySessionTokenStore
4. `P1 需求发布与审核模块` — 新开线程，优先级高；需要先完成详细设计
5. `真实 OBS SDK 适配器` — 新开线程，优先级中
6. `替换 no-op 邮件发送` — 新开线程，优先级中

## 建议归档与下一线程

- 建议归档当前线程：是。
- 下一线程名称：`Qt 认证资料提交 UI`
- CodeArts 提示词：`D:\big_homework\docs\prompts\codearts\20260519_round_09_qt_identity_submission_ui.md`
