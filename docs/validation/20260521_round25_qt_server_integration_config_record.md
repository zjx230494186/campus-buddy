# Round 25 Validation — Qt 客户端服务器联调配置

**日期**: 2026-05-21
**目标**: 让 Qt 客户端 API base URL 可运行时配置（命令行参数 > 环境变量 > 默认值），并用 Ubuntu 24 systemd 后端完成最小服务器联调验证。

---

## 1. 变更清单

| 变更项 | 路径 | 说明 |
|--------|------|------|
| ApiClientConfig 新增 fromRuntime | `desktop/src/domain/ApiClientConfig.h` | 声明静态工厂方法 + 私有辅助方法 |
| ApiClientConfig 实现 fromRuntime | `desktop/src/domain/ApiClientConfig.cpp` | 命令行 `--api-base-url=` > 环境变量 `CAMPUS_BUDDY_API_BASE_URL` > 默认值；末尾 `/` 去除 |
| main.cpp 改用 fromRuntime | `desktop/src/main.cpp` | 使用 `QCoreApplication::arguments()` + `QProcessEnvironment::systemEnvironment()` 创建配置 |
| 测试扩展 | `desktop/tests/ApiClientConfigTest.cpp` | 7 个测试：默认值、fromRuntime 默认、env 读取、arg 覆盖 env、末尾 slash 去除、空 env 回退、空 arg 回退 |
| 集成 smoke | `desktop/tests/QtServerIntegrationSmoke.cpp` | 独立可执行，打真实服务器：health + login + credit-summary |
| CMakeLists.txt | `desktop/CMakeLists.txt` | 新增 qt_server_integration_smoke 构建目标（不纳入 CTest 自动运行） |

## 2. 红灯测试

测试先写后实现。新增的 6 个 `fromRuntime` 测试在实现前会因链接失败（`fromRuntime` 未定义）而红灯。

## 3. 绿灯测试

### 3.1 Qt 本地单元测试

| 测试套件 | 结果 |
|----------|------|
| `api_client_config_test` | 7/7 通过 |
| `campus_api_client_test` | 11/11 通过 |
| `auth_token_store_test` | 6/6 通过（含 token 不持久化到 QSettings 检查） |

### 3.2 Qt 桌面应用构建

`campus_buddy_desktop` 构建成功，`main.cpp` 使用 `fromRuntime` 编译通过。

## 4. 服务器 Health Check

```
curl -s -i http://114.116.203.78/api/health
→ HTTP/1.1 200, {"status":"UP"}
```

## 5. Qt API Client 服务器联调 Smoke

运行命令：

```bash
CAMPUS_BUDDY_API_BASE_URL=http://114.116.203.78/api \
  qt_server_integration_smoke.exe
```

| # | 端点 | 结果 |
|---|------|------|
| 1 | `GET /health` | PASS: status=UP |
| 2 | `POST /auth/login` | PASS: token length=414 |
| 3 | `GET /me/credit-summary` (Bearer) | PASS: averageRating=3.6, ratingSampleCount=7 |

全部 3 项通过，退出码 0。

## 6. 敏感信息检查

- [x] `ApiClientConfig.cpp` 不含硬编码 IP/密码/密钥，仅含默认 localhost URL
- [x] `QtServerIntegrationSmoke.cpp` 不打印 access token 内容，仅记录长度
- [x] `AuthTokenStoreTest.tokenIsNotPersistedToQSettings` 仍通过
- [x] 集成 smoke 使用测试账号邮箱（已脱敏），不记录真实用户隐私
- [x] API base URL 通过环境变量/命令行参数传入，不在深层业务逻辑或 UI 组件中散落

## 7. fromRuntime 优先级规则

| 优先级 | 来源 | 示例 |
|--------|------|------|
| 1（最高） | 命令行 `--api-base-url=URL` | `--api-base-url=http://192.168.1.100/api` |
| 2 | 环境变量 `CAMPUS_BUDDY_API_BASE_URL` | `export CAMPUS_BUDDY_API_BASE_URL=http://114.116.203.78/api` |
| 3（兜底） | 默认值 | `http://localhost:8080/api` |

规范化规则：去除末尾 `/`。空值回退到默认值。

## 8. 未覆盖风险

| 风险 | 说明 | 建议 |
|------|------|------|
| HTTPS / TLS | 当前公网仍为 HTTP，token 明文传输 | 真实试用前必须加 HTTPS |
| 集成 smoke 不在 CI 中 | 需要真实服务器，未纳入 CTest | 后续按需手动运行或配置集成 CI |
| 多实例 / 负载均衡 | 当前单实例 systemd | 后续按需 |
| Qt 客户端更多链路 | 仅验证 health + login + credit-summary | 后续逐步扩展评价、材料上传等 |

## 9. 下一轮建议

1. **Qt 客户端评价/材料上传联调** — 扩展集成 smoke 覆盖 POST /me/reviews 和身份认证材料上传链路
2. **HTTPS 配置** — 为公网配置 Let's Encrypt / 华为云 SSL，保护 token 传输
3. **Qt 客户端 UI 服务器模式验证** — 在服务器 API base URL 下完整验证 UI 交互链路
