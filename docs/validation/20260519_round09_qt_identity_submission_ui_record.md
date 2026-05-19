# Round 09 验证记录：Qt 认证资料提交 UI

## 日期

2026-05-19

## 目标

实现 Qt 学生侧认证资料提交 UI 最小闭环：上传材料 → 填写资料 → 提交认证 → 查看状态。

## 提交

- `c59a138` feat(desktop): add identity verification submission UI

## 新增/修改项

### CampusApiClient
- 新增 `uploadMultipart`：multipart/form-data 上传，带 Bearer 认证头

### AuthApiService
- `uploadIdentityMaterial(fileData, fileName, contentType)`：上传文件到 `/auth/identity-verifications/materials`，读取 `attachmentId`
- `submitIdentityVerification(realName, studentNumber, college, major, grade, materialAttachmentId)`：提交完整认证资料到 `/auth/identity-verifications`
- `getIdentityVerificationStatus` 扩展：读取 `reviewStatus`、`rejectReason`、`allowedActions`

### AuthResult
- 新增字段：`attachmentId`、`reviewStatus`、`rejectReason`、`allowedActions`

### IdentityVerificationWidget
- 表单字段：真实姓名、学号、学院、专业、年级
- 文件选择：jpg/jpeg/png/pdf 过滤
- 流程：选择文件 → 上传（获取 attachmentId）→ 填写表单 → 提交认证
- 上传成功前禁用提交按钮
- PENDING_REVIEW/VERIFIED 状态禁用表单
- REJECTED 状态显示驳回原因并允许重新提交

### HomePageWidget
- 嵌入 IdentityVerificationWidget
- 查询认证状态后传递 authenticationStatus 和 rejectReason

## 测试结果

### 新增测试
- `submitIdentityVerificationRequestBodyContainsAllFields`：验证请求体包含 realName/studentNumber/college/major/grade/materialAttachmentId
- `identityVerificationStatusResponseParsesAllFields`：验证响应读取 authenticationStatus/reviewStatus/rejectReason/allowedActions
- IdentityVerificationWidget 加入 widget 层 QNetworkAccessManager 检查

### 全部测试
- `api_client_config_test`：Passed
- `campus_api_client_test`（11 个测试）：Passed
- `auth_token_store_test`（6 个测试）：Passed
- **3/3 通过**

### 后端回归
- 未修改后端代码，56/56 不变

## 已知遗留

- Windows Credential Manager 未实现
- 管理员审核 UI 未实现
- 真实 OBS SDK 未适配
- no-op 邮件发送未替换
- P1 需求发布与审核模块未开始
