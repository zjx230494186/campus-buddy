# Round 24 Validation — systemd 服务化部署

**日期**: 2026-05-21
**目标**: 将后端从 nohup 手动启动升级为 systemd 托管服务，确保开机自启、进程命令行无敏感信息、OBS 环境变量正确映射、全量回归通过。

---

## 1. 变更清单

| 变更项 | 路径 | 说明 |
|--------|------|------|
| systemd unit 文件 | `deploy/campus-buddy-backend.service` | ExecStart 调用 wrapper 脚本，After=network.target |
| wrapper 启动脚本 | `deploy/start_backend_service.sh` | source env + 映射 OBS 变量名 + exec java -jar |
| 安装脚本 | `deploy/install_systemd_service.sh` | cp unit + cp wrapper + systemctl daemon-reload + enable + start |
| 安全测试 | `backend/src/test/java/.../SystemdServiceSecurityTest.java` | 7 项静态检查 |

## 2. 本地回归测试

- **全量测试**: 146/146 通过
- **关键测试类**:
  - `SystemdServiceSecurityTest` — 7/7 通过（unit 不含敏感参数、wrapper 不含敏感参数、引用 env 文件、映射 OBS 变量名）
  - `DeployScriptSecurityTest` — 通过（旧脚本仍不含敏感参数）
  - `ObsObjectStorageServiceLifecycleTest` — 通过（close 生命周期）
  - `ObsObjectStorageServiceTest` — 通过（凭据校验 + InMemory deleteObject）

## 3. 服务器部署验证

### 3.1 服务状态

| 检查项 | 结果 |
|--------|------|
| `systemctl is-active campus-buddy-backend` | `active` |
| `systemctl is-enabled campus-buddy-backend` | `enabled` |
| 进程命令行 | `/usr/bin/java -jar /srv/campus-buddy/campus-buddy-backend-0.0.1-SNAPSHOT.jar --spring.profiles.active=deploy` |
| 命令行敏感信息 | 无（仅 --spring.profiles.active=deploy） |
| Flyway 迁移 | Schema "public" is up to date. No migration necessary. |

### 3.2 API Smoke Test（8 项全部通过）

| # | 端点 | 结果 |
|---|------|------|
| 1 | `GET /api/health` | `{"status":"UP"}` |
| 2 | `GET /api/system/info` | serviceName=campus-buddy-backend |
| 3 | `POST /api/auth/login` | Token acquired: 414 chars |
| 4 | `GET /api/me/credit-summary` | averageRating=3.6, ratingSampleCount=7 |
| 5 | `GET /api/me/reviews/given` | totalElements=1 |
| 6 | `GET /api/probe/secure` (auth) | authenticated=True |
| 7 | `GET /api/probe/secure` (no auth) | 401 |
| 8 | `GET /api/users/{userId}/credit-summary` (auth) | creditScore 返回 |

### 3.3 OBS Smoke Test（PUT/GET/DELETE 闭环）

| 步骤 | 结果 |
|------|------|
| PUT | 200 |
| GET + SHA-256 match | 200, match=True |
| DELETE | 204 |
| GET after DELETE (expect 404) | 404 confirmed |

## 4. 敏感信息红线检查

- [x] systemd unit 文件不含硬编码密码/AK/SK
- [x] wrapper 脚本不含硬编码密码/AK/SK
- [x] 进程命令行不含 `--spring.datasource.password` 等敏感参数
- [x] OBS 凭据通过 `/etc/campus-buddy/backend.env` → wrapper 脚本 source + export 映射
- [x] env 文件权限 600, owner root:root

## 5. 架构决策记录

| 决策 | 原因 |
|------|------|
| systemd unit 的 ExecStart 调用 wrapper 脚本而非直接 java -jar | systemd `Environment=` 不支持变量二次展开（`${OBJECT_STORAGE_ACCESS_KEY_ID}`），需 wrapper 做 source + 映射 |
| wrapper 中 `set -a` + `. env` + `set +a` | 自动 export 所有 env 变量，避免逐个 export |
| OBS 变量名映射：`OBJECT_STORAGE_ACCESS_KEY_ID` → `OBS_ACCESS_KEY_ID` | env 文件用 `OBJECT_STORAGE_*` 前缀，deploy profile 期望 `OBS_*` 前缀 |
| 保留 `deploy/start_backend_deploy.sh`（旧 nohup 脚本） | 作为备用回退手段 |

## 6. 结论

**Round 24 systemd 服务化部署验证通过。** 后端服务已完全托管于 systemd，开机自启、进程命令行干净、OBS 凭据映射正确、API 和 OBS smoke 全部通过。
