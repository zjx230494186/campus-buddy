# Round 26 Validation — Qt Server Smoke 凭据卫生与材料上传链路

**日期**: 2026-05-21
**目标**: 收束 smoke 凭据硬编码问题，扩展 Qt 服务器联调覆盖身份认证材料上传链路。

---

## 1. 变更清单

| 变更项 | 路径 | 说明 |
|--------|------|------|
| Smoke 凭据从 env 读取 | `desktop/tests/QtServerIntegrationSmoke.cpp` | 邮箱/密码通过 `CAMPUS_BUDDY_SMOKE_EMAIL` / `CAMPUS_BUDDY_SMOKE_PASSWORD` 传入 |
| 缺少凭据安全退出 | `desktop/tests/QtServerIntegrationSmoke.cpp` | 缺少 env 变量时 exit(2)，只打印变量名不打印值 |
| 材料上传链路 | `desktop/tests/QtServerIntegrationSmoke.cpp` | 构造 512B 测试文件，multipart 上传到 `/auth/identity-verifications/materials` |
| 安全静态检查 | `desktop/tests/ServerSmokeSecurityTest.cpp` | 3 项检查：无硬编码密码、无硬编码邮箱、必须引用 env 变量名 |
| CMakeLists.txt | `desktop/CMakeLists.txt` | 新增 `server_smoke_security_test` 目标；更新 `qt_server_integration_smoke` 链接 |

## 2. 红灯测试

重构前 `QtServerIntegrationSmoke.cpp` 第99-100行包含硬编码 `"smoketest@campus.edu.cn"` 和 `"SmokeTest123!"`。`ServerSmokeSecurityTest` 对旧代码会红灯（包含硬编码邮箱和密码模式）。

## 3. 绿灯测试

### 3.1 Qt 本地单元测试

| 测试套件 | 结果 |
|----------|------|
| `api_client_config_test` | 7/7 通过 |
| `campus_api_client_test` | 11/11 通过 |
| `auth_token_store_test` | 6/6 通过（含 token 不持久化到 QSettings） |
| `server_smoke_security_test` | 3/3 通过（无硬编码密码、无硬编码邮箱、引用 env 变量） |

**合计 27/27 通过**。

### 3.2 缺少凭据安全退出

```
$ qt_server_integration_smoke.exe  (无 CAMPUS_BUDDY_SMOKE_EMAIL/PASSWORD)
→ ERROR: CAMPUS_BUDDY_SMOKE_EMAIL and/or CAMPUS_BUDDY_SMOKE_PASSWORD not set
→ Required environment variables:
    CAMPUS_BUDDY_SMOKE_EMAIL
    CAMPUS_BUDDY_SMOKE_PASSWORD
→ EXIT: 2
```

## 4. 服务器 Health Check

```
curl -s -i http://114.116.203.78/api/health → 200 {"status":"UP"}
```

## 5. Qt Server Smoke 联调结果

| # | 端点 | 结果 |
|---|------|------|
| 1 | `GET /health` | PASS: status=UP |
| 2 | `POST /auth/login` | PASS: token length=414 |
| 3 | `GET /me/credit-summary` (Bearer) | PASS: averageRating=3.6, ratingSampleCount=7 |
| 4 | `POST /auth/identity-verifications/materials` (multipart) | PASS: attachmentId length=36, contentType=application/pdf, sizeBytes=512, sha256 length=64 |

全部 4 项通过，退出码 0。

## 6. 敏感信息检查

- [x] `QtServerIntegrationSmoke.cpp` 不包含硬编码邮箱或密码
- [x] `ServerSmokeSecurityTest` 静态检查通过
- [x] 缺少 env 变量时安全退出，只打印变量名
- [x] 不打印 token 内容（仅长度）、不打印密码
- [x] 不打印 Authorization header
- [x] 不打印测试文件原文
- [x] `AuthTokenStoreTest.tokenIsNotPersistedToQSettings` 仍通过
- [x] 上传结果仅记录 attachmentId 长度、contentType、sizeBytes、sha256 长度

## 7. 环境变量配置清单

| 变量名 | 用途 | 是否敏感 |
|--------|------|----------|
| `CAMPUS_BUDDY_API_BASE_URL` | 后端 API base URL | 否（公开地址） |
| `CAMPUS_BUDDY_SMOKE_EMAIL` | Smoke 测试账号邮箱 | 否（测试账号标识） |
| `CAMPUS_BUDDY_SMOKE_PASSWORD` | Smoke 测试账号密码 | **是** |

`CAMPUS_BUDDY_SMOKE_PASSWORD` 必须只在私有 shell 中设置，不得写入脚本、文档、CI 配置或聊天。

## 8. 未覆盖风险

| 风险 | 说明 | 建议 |
|------|------|------|
| HTTPS / TLS | 公网仍为 HTTP，token 和密码明文传输 | 真实试用前必须加 HTTPS |
| 上传后身份认证状态变化 | smoke 账号已 VERIFIED，上传材料后状态变化未检查 | 后续按需 |
| 集成 smoke 不在 CI 中 | 需要真实服务器和凭据 | 手动运行或配置集成 CI |
| 测试文件清理 | 上传的 512B 文件留在 OBS 桶中 | 后续可添加清理逻辑或使用隔离前缀 |

## 9. 下一轮建议

1. **HTTPS 配置** — 为公网配置 SSL，保护凭据传输安全
2. **Qt 评价链路联调** — 扩展 smoke 覆盖 POST /me/reviews
3. **Qt 客户端 UI 完整服务器模式验证** — 在真实 API 下完整验证 UI 交互
