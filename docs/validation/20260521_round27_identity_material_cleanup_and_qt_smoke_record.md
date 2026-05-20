# Round 27 Validation — 认证材料删除能力与 Qt Smoke 清理

**日期**: 2026-05-21
**目标**: 实现最小"本人未引用认证材料删除"后端能力，Qt smoke 上传后主动清理测试产物。

---

## 1. 变更清单

| 变更项 | 路径 | 说明 |
|--------|------|------|
| 删除端点 | `backend/.../IdentityVerificationMaterialController.java` | `DELETE /api/auth/identity-verifications/materials/{attachmentId}` |
| 删除业务逻辑 | `backend/.../IdentityMaterialAttachmentService.java` | `delete(userId, attachmentId)` — 归属校验 + 引用检查 + 对象存储删除 + status=DELETED |
| Submission 引用查询 | `backend/.../IdentityVerificationSubmissionRepository.java` | `existsByMaterialAttachmentId(UUID)` |
| Security 路径 | `backend/.../SecurityConfiguration.java` | 添加 `/api/auth/identity-verifications/materials/*` 到 authenticated |
| 删除测试 (6个) | `backend/.../IdentityVerificationMaterialAttachmentEndpointTest.java` | 未登录401、本人删除204+status=DELETED、他人删除404、引用删除409、不存在404、重复删除404 |
| Qt deleteResource | `desktop/.../CampusApiClient.h/cpp` | 新增 `deleteResource(path, accessToken, callback)` |
| Qt 204 支持 | `desktop/.../CampusApiClient.cpp` | `parseReply` 中 2xx+空body 视为成功 |
| Qt smoke 清理 | `desktop/tests/QtServerIntegrationSmoke.cpp` | 上传后保存 attachmentId，立即 DELETE 清理 |

## 2. 红灯测试

6 个新删除测试在删除端点/逻辑实现前红灯（404 或 405 Method Not Allowed）。

## 3. 绿灯测试

### 3.1 后端全量回归

**153/153 通过**（含原有 147 + 新增 6 删除端点测试）。

### 3.2 Qt 本地测试

| 测试套件 | 结果 |
|----------|------|
| `api_client_config_test` | 7/7 通过 |
| `campus_api_client_test` | 11/11 通过 |
| `auth_token_store_test` | 6/6 通过 |
| `server_smoke_security_test` | 3/3 通过 |

**合计 27/27 通过**。

## 4. 服务器部署/重启

```
systemctl restart campus-buddy-backend → active(running)
```

## 5. Health Check

```
curl http://114.116.203.78/api/health → 200 {"status":"UP"}
```

## 6. Qt Server Smoke 上传与删除清理

| # | 端点 | 结果 |
|---|------|------|
| 1 | `GET /health` | PASS: status=UP |
| 2 | `POST /auth/login` | PASS: token length=414 |
| 3 | `GET /me/credit-summary` | PASS: averageRating=3.6, ratingSampleCount=7 |
| 4 | `POST /auth/identity-verifications/materials` | PASS: attachmentId length=36, contentType=application/pdf, sizeBytes=512, sha256 length=64 |
| 5 | `DELETE /auth/identity-verifications/materials/{id}` | PASS: 204 No Content |

全部 5 项通过，退出码 0。

## 7. OBS 测试对象清理

- API 层面：删除返回 204，后端调用了 `ObjectStorageService.deleteObject(objectKey)`
- 独立 OBS 验证：未独立复核 OBS 物理对象删除（需要读取服务器日志或单独 OBS smoke 确认）
- 结论：**API 清理成功，OBS 物理对象未独立复核**

## 8. 敏感信息检查

- [x] 不打印 token 内容（仅长度）
- [x] 不打印密码、Authorization header
- [x] 不打印对象存储 key
- [x] 不打印 OBS 凭据
- [x] 删除成功仅记录 "delete succeeded (204 No Content)"
- [x] `AuthTokenStoreTest.tokenIsNotPersistedToQSettings` 仍通过
- [x] 删除接口不返回 objectKey，只返回 204 或标准错误

## 9. 删除接口业务规则

| 条件 | HTTP 状态 | 错误码 |
|------|-----------|--------|
| 成功删除 | 204 No Content | — |
| 未登录 | 401 | UNAUTHORIZED |
| 非本人附件 | 404 | ATTACHMENT_NOT_FOUND |
| 已被 Submission 引用 | 409 Conflict | ATTACHMENT_REFERENCED |
| 不存在 | 404 | ATTACHMENT_NOT_FOUND |
| 已删除（重复） | 404 | ATTACHMENT_NOT_FOUND |

- 删除后 DB 记录保留，`status` 标记为 "DELETED"（软删除）
- 调用 `ObjectStorageService.deleteObject(objectKey)` 删除 OBS/内存对象

## 10. 未覆盖风险

| 风险 | 说明 | 建议 |
|------|------|------|
| HTTPS | 公网仍为 HTTP | 真实试用前必须加 |
| OBS 物理删除复核 | 未独立确认对象存储物理删除 | 后续按需 |
| 并发删除 | 同一附件并发 DELETE 未测试 | 后续按需 |
| 管理员附件治理 | 管理员无法删除附件 | 后续按需 |

## 11. 下一轮建议

1. **HTTPS 配置** — 保护公网凭据传输
2. **Qt 评价链路联调** — 扩展 smoke 覆盖 POST /me/reviews
3. **Qt UI 完整服务器模式验证** — 在真实 API 下完整验证 UI 交互
