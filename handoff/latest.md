# Latest Handoff

本文档服务于新线程快速接手。

## 当前状态

- 项目名称：校园搭子平台。
- 当前阶段：正式代码开发。
- 当前线程：Qt 客户端对接后端 P0 认证 API（已完成最小闭环）。
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

## 当前线程完成了什么

- CampusApiClient：新增 postJson、getJson/postJson 带 Bearer 认证头
- AuthTokenStore：基于 QSettings 的 accessToken 持久化
- AuthApiService：封装 login、register、sendVerificationCode、verifyCampusEmail、submitIdentityVerification、getIdentityVerificationStatus
- LoginWidget：邮箱+密码登录表单
- RegisterWidget：注册表单（含验证码发送）
- HomePageWidget：简单主页（查询认证状态+退出登录）
- main.cpp：QStackedWidget 导航（登录/注册/主页切换）
- Qt 测试：AuthTokenStoreTest (5) + CampusApiClientTest (6) + ApiClientConfigTest (原有)

## 关键结论

- Qt 客户端从技术探路骨架升级为可交互的登录/注册/主页应用
- 所有 P0 认证 API 均已通过 AuthApiService 对接
- UI 层不直接使用 QNetworkAccessManager（测试验证）

## 本线程没有做什么

- 没有实现认证资料提交 UI（身份验证表单+附件上传）
- 没有实现管理员审核 UI
- 没有适配真实 OBS SDK
- 没有替换 no-op 邮件发送
- 没有运行 Testcontainers/Docker 测试
- 没有实现完整 RBAC
- 没有开始 P1 需求发布与审核模块

## 下一步候选事项

1. `认证资料提交 UI` — 复用当前线程，优先级高；在 HomePageWidget 中添加身份验证提交表单
2. `P1 需求发布与审核模块` — 新开线程，优先级高；需要先完成详细设计
3. `真实 OBS SDK 适配器` — 新开线程，优先级中
4. `替换 no-op 邮件发送` — 新开线程，优先级中
5. `Qt 客户端附件上传 UI` — 复用线程，优先级中

## 建议归档与下一线程

- 建议归档当前线程：否（可复用，继续补齐认证资料提交 UI）。
- 下一线程名称：`Qt 客户端认证资料提交 UI`
- 或：归档后新开 `P1 需求发布与审核模块详细设计`
