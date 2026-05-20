# 25 Server Deploy And OBS Runbook V1

本文记录当前 Ubuntu 24 服务器部署、私有配置、真实 OBS 连通性验证和 smoke test 的稳定操作方法。本文只记录可交接的非敏感信息和命令模板，不记录任何真实密钥、密码、JWT secret、SSH 私钥或云账号凭据。

## 1. 当前服务器基线

| 项目 | 当前值 |
|---|---|
| 云服务器 | 华为云 ECS |
| 操作系统 | Ubuntu 24.04.4 LTS |
| 主机名 | `ecs-ead3` |
| 公网 IP | `114.116.203.78` |
| 后端部署目录 | `/srv/campus-buddy` |
| 后端运行方式 | systemd 服务 `campus-buddy-backend`，开机自启 |
| 反向代理 | Nginx，公网 HTTP 转发到本机后端 |
| 数据库 | PostgreSQL 17.9，绑定 `127.0.0.1:5432` |
| 对象存储 | 华为云 OBS 私有桶 |

当前仍是开发 / 演示部署形态。域名、HTTPS、备案、IAM 委托和生产级密钥托管尚未完成。systemd 服务化已于 Round 24 完成。

## 2. 私有配置入口

服务器私有配置文件：

```text
/etc/campus-buddy/backend.env
```

权限要求：

```bash
chmod 600 /etc/campus-buddy/backend.env
chown root:root /etc/campus-buddy/backend.env
```

该文件只能位于服务器私有环境中，不得提交到仓库，不得复制到 Markdown 文档，不得发到聊天，不得截图外传。

当前需要的变量名如下。表中只记录变量名和用途，不记录真实值。

| 变量名 | 用途 |
|---|---|
| `OBJECT_STORAGE_ACCESS_KEY_ID` | OBS Access Key Id，敏感 |
| `OBJECT_STORAGE_SECRET_ACCESS_KEY` | OBS Secret Access Key，敏感 |
| `OBJECT_STORAGE_ENDPOINT` | OBS endpoint，当前为 `obs.cn-north-4.myhuaweicloud.com` |
| `OBJECT_STORAGE_REGION` | OBS region，当前为 `cn-north-4` |
| `OBJECT_STORAGE_BUCKET` | OBS bucket，当前为 `20260518-bighomework` |
| `DB_HOST` | 数据库主机，当前为 `127.0.0.1` |
| `DB_PORT` | 数据库端口，当前为 `5432` |
| `DB_NAME` | 数据库名，当前为 `campus_buddy` |
| `DB_USERNAME` | 数据库用户名，当前为 `campus_buddy` |
| `DB_PASSWORD` | 数据库密码，敏感 |
| `JWT_SECRET` | JWT 签名密钥，敏感 |

当前后端 deploy profile 使用 `OBS_ACCESS_KEY_ID` 和 `OBS_SECRET_ACCESS_KEY`。启动脚本会把历史变量名映射过去：

```bash
export OBS_ACCESS_KEY_ID="${OBJECT_STORAGE_ACCESS_KEY_ID}"
export OBS_SECRET_ACCESS_KEY="${OBJECT_STORAGE_SECRET_ACCESS_KEY}"
```

## 3. 后端启动（systemd 服务化）

后端已托管于 systemd 服务 `campus-buddy-backend`。

### 3.1 服务管理

```bash
# 查看状态
systemctl status campus-buddy-backend --no-pager

# 启动 / 停止 / 重启
systemctl start campus-buddy-backend
systemctl stop campus-buddy-backend
systemctl restart campus-buddy-backend

# 查看日志
journalctl -u campus-buddy-backend -n 80 --no-pager
```

### 3.2 关键文件

| 文件 | 服务器路径 | 仓库路径 |
|------|-----------|---------|
| systemd unit | `/etc/systemd/system/campus-buddy-backend.service` | `deploy/campus-buddy-backend.service` |
| wrapper 脚本 | `/srv/campus-buddy/start_backend_service.sh` | `deploy/start_backend_service.sh` |
| 安装脚本 | — | `deploy/install_systemd_service.sh` |

wrapper 脚本完成以下动作：

1. `set -a` + `. /etc/campus-buddy/backend.env` + `set +a` — source 并 export 所有环境变量
2. 映射 OBS 变量名：`OBJECT_STORAGE_ACCESS_KEY_ID` → `OBS_ACCESS_KEY_ID`
3. `exec /usr/bin/java -jar ... --spring.profiles.active=deploy`

### 3.3 备用：手动 nohup 启动

仓库仍保留旧脚本 `deploy/start_backend_deploy.sh` 作为备用回退：

```bash
/srv/campus-buddy/start_backend_deploy.sh
```

Java 命令行只应保留非敏感参数：

```bash
java -jar campus-buddy-backend-0.0.1-SNAPSHOT.jar --spring.profiles.active=deploy
```

不得把以下内容放进 Java 命令行参数：

- `--spring.datasource.password=...`
- `--campus-buddy.security.jwt.secret=...`
- OBS Access Key / Secret Key
- 任何数据库密码、JWT secret、云密钥或 token

## 4. 健康检查

本机入口：

```bash
curl -s -i http://127.0.0.1:8080/api/health
```

期望：

```text
HTTP/1.1 200
{"status":"UP"}
```

公网 Nginx 入口：

```bash
curl -s -i http://114.116.203.78/api/health
```

期望同样为 `200` 和 `{"status":"UP"}`。

如果 localhost 成功但公网失败，优先检查 Nginx 配置、Nginx 服务状态、防火墙和安全组。不要先修改后端业务代码。

## 5. OBS Smoke Test

仓库脚本：

```text
deploy/run_obs_smoke.sh
deploy/obs_put_get_delete_smoke.py
```

服务器侧应从 `/etc/campus-buddy/backend.env` 读取凭据，不得把凭据写入脚本。

执行：

```bash
/srv/campus-buddy/run_obs_smoke.sh
```

或在服务器私有 shell 中：

```bash
set -a
. /etc/campus-buddy/backend.env
set +a
python3 /srv/campus-buddy/obs_put_get_delete_smoke.py
```

最小闭环必须覆盖：

1. PUT 上传测试对象。
2. GET 读取测试对象。
3. 校验 SHA-256。
4. DELETE 删除测试对象。
5. 再次 GET 得到 404，确认已删除。

测试对象前缀应使用技术验证前缀，例如：

```text
technical-spike/round22/
```

不得使用包含真实姓名、学号、邮箱、手机号或业务敏感信息的对象 key。

## 6. 常用检查命令

检查私有配置文件权限：

```bash
ls -l /etc/campus-buddy/backend.env
stat -c 'mode=%a owner=%U group=%G size=%s' /etc/campus-buddy/backend.env
```

期望：

```text
mode=600 owner=root group=root
```

检查变量是否存在但不打印值：

```bash
set -a
. /etc/campus-buddy/backend.env
set +a
for name in OBJECT_STORAGE_ACCESS_KEY_ID OBJECT_STORAGE_SECRET_ACCESS_KEY DB_PASSWORD JWT_SECRET; do
  value="${!name:-}"
  if [ -n "$value" ]; then
    printf '%s present length=%s\n' "$name" "${#value}"
  else
    printf '%s missing\n' "$name"
  fi
done
```

检查后端进程命令行是否暴露敏感参数：

```bash
pgrep -af 'campus-buddy-backend-0.0.1-SNAPSHOT.jar|java -jar'
```

期望只看到：

```text
java -jar campus-buddy-backend-0.0.1-SNAPSHOT.jar --spring.profiles.active=deploy
```

检查敏感关键词是否出现在进程命令行：

```bash
pgrep -af 'campus-buddy-backend-0.0.1-SNAPSHOT.jar|java -jar' | grep -Ei 'password|secret|access-key|secret-access|AKIA|token' || echo 'process args clean'
```

期望输出：

```text
process args clean
```

查看最近启动日志：

```bash
tail -80 /srv/campus-buddy/backend.log
```

日志中不得出现真实 AK/SK、JWT secret、数据库密码或 Authorization header。

## 7. 故障排查

### 7.1 后端启动失败

先看日志：

```bash
tail -120 /srv/campus-buddy/backend.log
```

常见原因：

- 缺少 `DB_PASSWORD` 或 `JWT_SECRET`。
- 缺少 `OBJECT_STORAGE_ACCESS_KEY_ID` / `OBJECT_STORAGE_SECRET_ACCESS_KEY`。
- PostgreSQL 容器未启动。
- Flyway 迁移失败。
- 8080 端口被旧进程占用。

处理顺序：

1. 检查 `/etc/campus-buddy/backend.env` 权限和变量存在性。
2. 检查 PostgreSQL。
3. 检查旧 Java 进程。
4. 检查后端日志。
5. 再考虑代码或迁移问题。

### 7.2 localhost health 成功但公网失败

检查 Nginx：

```bash
systemctl status nginx --no-pager
nginx -t
```

再检查公网入口：

```bash
curl -s -i http://114.116.203.78/api/health
```

可能原因：

- Nginx 未运行。
- Nginx 反向代理路径错误。
- UFW 或云安全组未开放 80。
- 后端只在 localhost 成功，Nginx 未转发到正确端口。

### 7.3 OBS smoke 返回 403

优先检查：

- AK/SK 是否已轮换但服务器仍使用旧值。
- IAM 权限是否覆盖当前桶和对象前缀。
- 桶名、endpoint、region 是否正确。
- 系统时间是否明显漂移。

不得通过打印 AK/SK 排查。只允许打印变量名、长度、endpoint、region、bucket 和对象 key。

### 7.4 OBS smoke 返回 404

如果是 DELETE 后的再次 GET 返回 404，这是期望结果。

如果首次 GET 返回 404，通常表示 PUT 没有成功写入、对象 key 不一致或签名请求路径不一致。检查脚本输出的对象 key 和 PUT 状态。

### 7.5 进程命令行出现敏感值

如果 `pgrep -af` 或 `ps -ef` 中出现数据库密码、JWT secret 或 OBS 凭据，应立即停止当前进程，并改为通过 `/etc/campus-buddy/backend.env` 和环境变量启动。

不要把这类进程命令行复制到文档、聊天或提交信息中。

## 8. AI 与人工操作边界

AI / CodeArts 可以执行：

- 检查文件是否存在。
- 检查权限、owner、变量名是否存在。
- 启动后端和执行 health check。
- 执行不打印 secret 的 OBS smoke。
- 读取非敏感日志摘要。

必须由用户在私有 shell 中执行或确认：

- 创建、轮换、删除云访问密钥。
- 首次写入或替换 `/etc/campus-buddy/backend.env` 中的真实 secret。
- 截图或导出任何可能包含 secret 的云控制台信息。
- 修改云账号、IAM、计费、域名、备案或安全组的长期设置。

## 9. 当前未完成事项

| 事项 | 当前状态 | 建议 |
|---|---|---|
| systemd 服务化 | **已完成（Round 24）** | 服务 active + enabled |
| IAM 委托 | 未完成，当前为长期 AK/SK | 后续单独设计 |
| HTTPS / 域名 / 备案 | 未完成 | 真实试用或展示前处理 |
| 邮件发送 | 仍为 Noop | 后续接入 SMTP 或等价服务 |
| OBS 连接池 / 超时 / 重试 | 使用 SDK 默认值 | 进入高并发或真实试用前调优 |
| 服务器备份与恢复 | 未形成流程 | 后续补充 |

## 10. 关联验证记录

- `docs/validation/20260518_server_base_environment_configuration_record.md`
- `docs/validation/20260518_ecs_obs_min_connectivity_probe_record.md`
- `docs/validation/20260520_round21_server_smoke_p1_review_credit.md`
- `docs/validation/20260520_round22_obs_real_connectivity_and_deploy_storage_record.md`
- `docs/validation/20260520_round23_obs_lifecycle_and_secret_process_hygiene_record.md`
- `docs/validation/20260521_round24_systemd_service_deploy_record.md`

当前结论：截至 Round 24，后端 deploy profile 已在 Ubuntu 24 服务器上以 systemd 服务运行，开机自启，健康检查通过，真实 OBS PUT/GET/DELETE smoke 通过，进程命令行不暴露 DB/JWT/OBS secret，全量回归 146/146 通过。

