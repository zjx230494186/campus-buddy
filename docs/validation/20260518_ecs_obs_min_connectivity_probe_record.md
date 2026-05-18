# 验证记录：ECS 服务器端华为云 OBS 最小连通性

## 基本信息

- 日期：2026-05-18
- 线程：关键环境验证：ECS 服务器端 OBS 最小连通性
- 验证对象：华为云 OBS 私有桶 `20260518-bighomework`
- ECS：华为云 ECS，主机名 `ecs-ead3`，Ubuntu 24.04.4 LTS
- Endpoint：`obs.cn-north-4.myhuaweicloud.com`
- Region：`cn-north-4`
- 验证方式：在 ECS `/tmp` 目录临时放置 Python 探路脚本，通过 S3 SigV4 请求执行 PUT / GET / SHA-256 / DELETE
- 本机留存脚本：`D:\big_homework\scripts\obs_min_connectivity_probe_ecs.py`
- ECS 临时脚本：`/tmp/obs_min_connectivity_probe_ecs.py`，验证后已删除

## 安全边界

- 本记录不保存 AccessKey、SecretKey、临时凭证、SSH 私钥、数据库密码或云账号密码明文。
- OBS 凭证只允许在 ECS 当前 shell 会话中通过环境变量临时注入。
- 本次输出只记录环境变量名称、Endpoint、Region、Bucket、网络检查结果、对象 key、HTTP 状态和 SHA-256 校验值。
- Qt 客户端不得持有 OBS 凭证，也不得直连 OBS。
- 本次不实现业务附件上传接口，不创建业务数据库表，不修改后端或 Qt 代码。

## 配置项边界

可记录的非敏感配置：

| 配置项 | 当前值 |
|---|---|
| `OBJECT_STORAGE_ENDPOINT` | `obs.cn-north-4.myhuaweicloud.com` |
| `OBJECT_STORAGE_REGION` | `cn-north-4` |
| `OBJECT_STORAGE_BUCKET` | `20260518-bighomework` |

不得记录明文的敏感配置：

| 配置项 | 处理方式 |
|---|---|
| `OBJECT_STORAGE_ACCESS_KEY_ID` | 仅允许存在于服务器本地私有环境变量、密钥管理服务或等价私有配置中 |
| `OBJECT_STORAGE_SECRET_ACCESS_KEY` | 仅允许存在于服务器本地私有环境变量、密钥管理服务或等价私有配置中 |
| `OBJECT_STORAGE_SESSION_TOKEN` | 如使用临时凭证，仅允许存在于服务器本地私有环境变量、密钥管理服务或等价私有配置中 |

## 执行记录

### 第一次执行：ECS 无凭证网络检查

- 执行位置：华为云 ECS `ecs-ead3`
- 执行命令：`python3 /tmp/obs_min_connectivity_probe_ecs.py`
- 执行结果：
  - Endpoint：`obs.cn-north-4.myhuaweicloud.com`
  - Region：`cn-north-4`
  - Bucket：`20260518-bighomework`
  - TCP/TLS 网络检查：`obs.cn-north-4.myhuaweicloud.com:443` 可连接
  - TLS version：`TLSv1.2`
  - 缺少服务端凭证时脚本安全退出，未执行 PUT / GET / DELETE。

### 第二次执行：ECS 私有 shell 临时注入凭证后运行

- 执行位置：华为云 ECS `ecs-ead3`
- 凭证注入方式：用户在 ECS 私有 shell 中通过临时环境变量注入，验证后执行 `unset`。
- 凭证环境变量名称：`OBJECT_STORAGE_ACCESS_KEY_ID`、`OBJECT_STORAGE_SECRET_ACCESS_KEY`
- 测试对象 key：`technical-spike/min-connectivity-ecs/20457e0012d045c996c6ed1e2454a493.txt`
- 执行结果：
  - TCP/TLS 网络检查：`obs.cn-north-4.myhuaweicloud.com:443` 可连接
  - TLS version：`TLSv1.2`
  - PUT 上传返回 HTTP `200`
  - GET 读取返回 HTTP `200`
  - SHA-256 校验通过：`9401dd11adbec915231f744120aa22bf95ad269cd864ad971278ec830b3e5429`
  - DELETE 删除返回 HTTP `204`
  - 脚本输出：`ECS OBS minimal connectivity probe passed.`

## 清理记录

- 第二次执行创建的测试对象已由脚本 DELETE 删除。
- ECS 临时脚本 `/tmp/obs_min_connectivity_probe_ecs.py` 已在验证完成后删除。
- 用户截图显示验证后已执行 `unset OBJECT_STORAGE_ACCESS_KEY_ID OBJECT_STORAGE_SECRET_ACCESS_KEY OBJECT_STORAGE_SESSION_TOKEN`。
- 本机前序验证中可能残留对象仍需在 OBS 控制台检查并删除：`technical-spike/min-connectivity/2653b580948d435bb978c2220427fc8e.txt`。

## 当前结论

- ECS 服务器端到 `obs.cn-north-4.myhuaweicloud.com:443` 的网络访问可用。
- ECS 服务器端可通过服务端凭证访问华为云 OBS 私有桶 `20260518-bighomework`。
- ECS 服务器端已完成 PUT 上传、GET 读取、SHA-256 校验和 DELETE 删除最小闭环。
- 凭证未写入项目文档、仓库、聊天或 Qt 客户端；本记录只保存非敏感执行结果。
- 该结论只代表对象存储最小连通性与凭证注入方式验证通过，不代表业务附件上传接口已实现。

## 后续提醒

- 因前序聊天中曾暴露 Access Key Id，建议完成本轮验证后在华为云 IAM 控制台禁用或删除本次使用的访问密钥，后续重新创建未泄露的新密钥用于正式服务端配置。
- 若短期不再继续使用 OBS 验证桶，应删除桶内对象并删除桶，避免继续产生存储、请求或公网流出等费用。
- 若短期不再继续使用 ECS，应删除 ECS 实例、云硬盘、弹性公网 IP 等关联资源，避免按需计费持续扣费。

## 本次明确未做

- 未实现业务附件上传接口。
- 未创建业务数据库表。
- 未修改后端代码。
- 未修改 Qt 代码。
- 未向 Qt 客户端提供对象存储凭证。
- 未记录 AccessKey、SecretKey、SSH 私钥、数据库密码或对象存储临时凭证明文。
- 未创建新的长期付费云产品。
