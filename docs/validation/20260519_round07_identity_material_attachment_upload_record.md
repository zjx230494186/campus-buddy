# Round 07 验证记录：认证材料附件上传闭环

## 日期

2026-05-19

## 目标

实现认证材料附件后端中转上传、附件元数据持久化、ObjectStorageService 抽象、认证申请关联和管理员受控读取。

## 提交

- `8edd057` feat(auth): add identity material attachment upload

## Flyway 迁移

- V5：`identity_material_attachment` 表 + ALTER `identity_verification_submission` ADD `material_attachment_id`

## 新增代码

- `IdentityMaterialAttachment` JPA 实体 + `IdentityMaterialAttachmentRepository`
- `ObjectStorageService` 接口 + `InMemoryObjectStorageService` 测试替身
- `IdentityMaterialAttachmentService` + `IdentityVerificationMaterialController`（multipart 上传）
- 修改 `IdentityVerificationService` 支持 `materialAttachmentId`（含归属校验）
- 修改 `IdentityVerificationAdminService` / `IdentityVerificationAdminController` 返回附件摘要 + 管理员受控读取
- Security 配置新增 /materials 需认证

## 测试结果

### 新增测试（9 个）

- `IdentityVerificationMaterialAttachmentEndpointTest`
  - 上传附件成功 → 201 + attachmentId
  - 未登录上传 → 401
  - 上传到不属于自己的 submission → 403
  - 上传到不存在的 submission → 404
  - 上传空文件 → 400
  - 管理员读取附件 → 200 + 二进制内容
  - 非管理员读取附件 → 403
  - 管理员读取不存在的附件 → 404
  - 学生读取自己附件 → 200

### 回归测试

- 非容器快速测试：56/56 通过（含原有 46 + 新增 9 + 1 额外）
- Testcontainers：未运行（需要 Docker）

## 边界确认

- 不做预签名 URL
- 不让 Qt 客户端直传
- 不连接真实 OBS（InMemoryObjectStorageService 是测试替身）
- 不写入 AK/SK

## 已知遗留

- 真实 OBS SDK 适配器未实现
- 管理员审核通过后通知邮件未实现
- 完整 RBAC 权限矩阵未实现
