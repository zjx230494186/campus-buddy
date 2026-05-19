# Latest Handoff

本文档服务于新线程快速接手。

## 当前状态

- 项目名称：校园搭子平台。
- 当前阶段：正式代码开发。
- 当前线程：Round 09 Qt 认证资料提交 UI（已完成）。
- Git 分支：main。
- 工作区：有文档待提交。
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
- `b44f519` docs(codearts): record round08 validation and update handoff
- `bc0ae9e` docs(codearts): record round08 handoff and round09 prompt
- `c59a138` feat(desktop): add identity verification submission UI

## 当前线程完成了什么

- CampusApiClient：新增 `uploadMultipart` 方法，支持 multipart/form-data + Bearer 认证头
- AuthApiService：新增 `uploadIdentityMaterial`（文件上传）、`submitIdentityVerification`（含 college/major/grade/materialAttachmentId）、扩展 `getIdentityVerificationStatus`（读取 reviewStatus/rejectReason/allowedActions）
- AuthResult：新增 attachmentId/reviewStatus/rejectReason/allowedActions 字段
- IdentityVerificationWidget：完整认证资料提交表单 + 文件选择/上传 + 状态感知
- HomePageWidget：嵌入 IdentityVerificationWidget，查询状态后传递给认证组件
- Qt 3/3 测试通过，后端 56/56 不变

## 关键结论

- Qt 客户端 P0 认证全链路闭环：注册→登录→查询状态→上传材料→提交认证→查看审核结果
- 认证资料提交包含完整字段：realName/studentNumber/college/major/grade/materialAttachmentId
- 附件上传采用后端中转，不直传 OBS

## 本线程没有做什么

- 没有实现 Windows Credential Manager 适配器
- 没有实现管理员审核 UI
- 没有适配真实 OBS SDK
- 没有替换 no-op 邮件发送
- 没有开始 P1 需求发布与审核模块

## 下一步候选事项

1. `Windows Credential Manager 适配器` — 新开线程，优先级高；替换 InMemorySessionTokenStore
2. `P1 需求发布与审核模块详细设计` — 新开线程，优先级高
3. `真实 OBS SDK 适配器` — 新开线程，优先级中
4. `替换 no-op 邮件发送` — 新开线程，优先级中
5. `管理员审核 UI` — 新开线程，优先级中

## 建议归档与下一线程

- 建议归档当前线程：是。
- 下一线程名称：`P1 需求发布与审核模块详细设计` 或 `Windows Credential Manager 适配器`
