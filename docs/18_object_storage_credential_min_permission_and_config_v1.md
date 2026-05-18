# 18 对象存储凭证最小权限与配置体系 V1

本文记录《校园搭子平台》V1 后续正式后端服务访问华为云 OBS 的凭证、权限与配置边界。本文只记录非敏感决策，不记录 AccessKey、SecretKey、临时凭证、SSH 私钥、数据库密码或云账号密码。

## 1. 当前结论

- OBS 桶已创建：`20260518-bighomework`。
- 区域：`cn-north-4`，华北-北京四。
- Endpoint：`obs.cn-north-4.myhuaweicloud.com`。
- 桶访问策略：私有桶，不开启公共读或公共读写。
- 本机与 ECS 服务器端最小连通性验证均已通过：PUT 上传、GET 读取、SHA-256 校验、DELETE 删除。
- Qt 客户端不持有 OBS 凭证，不直连 OBS；附件访问继续采用 `Qt 客户端 -> 后端 API -> ObjectStorageService -> OBS`。

## 2. 官方依据

本轮判断参考华为云官方文档：

- OBS IAM 权限与系统权限说明：`https://support.huaweicloud.com/intl/en-us/perms-cfg-obs/obs_40_0003.html`
- OBS 通过 IAM 委托临时访问密钥访问：`https://support.huaweicloud.com/intl/en-us/perms-cfg-obs/obs_40_0010.html`
- 第三方临时上传权限说明：`https://support.huaweicloud.com/intl/en-us/usermanual-obs/obs_03_1007.html`
- ECS 通过委托获取临时凭证访问云服务：`https://support.huaweicloud.com/intl/en-us/bestpractice-iam/iam_0511.html`
- IAM 获取委托临时 AK/SK 与 security token API：`https://support.huaweicloud.com/eu/api-iam/iam_04_0101.html`

## 3. 凭证方案取舍

| 方案 | 当前判断 | 使用场景 | 主要优点 | 主要代价 |
|---|---|---|---|---|
| ECS 绑定 IAM 委托，后端获取临时凭证 | 正式部署首选 | 后端运行在华为云 ECS，且控制台/SDK 接入可完成 | 不在服务器长期落 AK/SK；临时凭证自动轮换；权限可绑定到委托 | 需要创建 IAM 委托并绑定 ECS；后端实现需支持从元数据或 SDK 获取临时凭证 |
| 专用 IAM 用户 + 最小权限 AK/SK | 短期兜底 | 委托接入暂时不可用，或先做最小后端配置体系验证 | 实现简单，兼容现有 S3/OBS 签名方式 | 存在长期密钥管理风险；必须轮换、禁用旧密钥、严格限制存放位置 |
| 向 Qt 客户端发放临时凭证 | 当前不采用 | 未来若改为客户端直传才重新评估 | 可降低后端传输压力 | 当前 V1 违反“Qt 不持有对象存储凭证”的边界，且会扩大客户端安全面 |
| 临时 URL | 当前不采用 | 未来若特定只读/上传场景明确成立再评估 | 可短期授权单次对象访问 | 当前 V1 后端中转已经满足权限控制；临时 URL 容易被误当作长期公开入口 |

结论：正式后端服务优先采用 ECS IAM 委托。若短期必须使用 AK/SK，只允许使用专用 IAM 用户的新密钥，不能复用前序验证中曾暴露过 Access Key Id 的密钥。

## 4. 最小权限边界

正式后端服务只应获得当前桶与必要对象前缀上的对象级读写删除能力。

| 权限范围 | 建议 | 说明 |
|---|---|---|
| 桶级管理 | 不授予 | 不允许后端创建/删除桶、修改桶 ACL、修改桶策略、开启公共访问、配置 CORS、生命周期、静态网站、日志或跨区域复制 |
| 对象上传 | 授予 | 后端业务校验通过后写入对象 |
| 对象读取/下载 | 授予 | 后端校验登录态、业务归属、附件状态后中转读取 |
| 对象删除 | 授予 | 用于审核驳回、业务清理、测试清理或用户/管理员触发的受控删除 |
| 对象列表 | 默认不授予；如需要，仅限前缀 | 业务运行应依赖数据库中的对象 key；列表权限只用于受控运维清理或健康检查 |
| 对象 ACL 修改 | 不授予 | 不允许后端把对象改成公开读或公开读写 |
| 公共读/写 | 不授予 | 桶保持私有，客户端不使用 OBS 裸 URL |

建议将业务对象 key 限制在明确前缀下：

```text
auth-materials/
post-attachments/
case-evidence/
technical-spike/
```

后续如果进入正式业务实现，`technical-spike/` 应只保留给验证脚本，不混入业务附件。

## 5. 配置矩阵

### 5.1 可进入项目文档或模板的非敏感配置

| 配置项 | 当前建议值 | 是否可写入文档 | 说明 |
|---|---|---|---|
| `OBJECT_STORAGE_PROVIDER` | `huaweicloud-obs` | 是 | 对象存储供应商标识 |
| `OBJECT_STORAGE_ENDPOINT` | `obs.cn-north-4.myhuaweicloud.com` | 是 | OBS Endpoint |
| `OBJECT_STORAGE_REGION` | `cn-north-4` | 是 | OBS 区域 |
| `OBJECT_STORAGE_BUCKET` | `20260518-bighomework` | 是 | 当前开发验证桶 |
| `OBJECT_STORAGE_ACCESS_MODE` | `backend-proxy` | 是 | 客户端只访问后端 API |
| `OBJECT_STORAGE_PUBLIC_READ` | `false` | 是 | 禁止公共读 |
| `OBJECT_STORAGE_CORS_ENABLED` | `false` | 是 | 当前 Qt + 后端中转场景不需要 OBS CORS |
| `OBJECT_STORAGE_ALLOWED_PREFIXES` | 见第 4 节 | 是 | 可记录前缀规则，不记录凭证 |

### 5.2 不得进入项目文档、仓库、聊天或 Qt 客户端的敏感配置

| 配置项 | 处理方式 |
|---|---|
| `OBJECT_STORAGE_ACCESS_KEY_ID` | 仅在服务器私有密钥入口或临时 shell 中出现；正式首选 IAM 委托避免长期保存 |
| `OBJECT_STORAGE_SECRET_ACCESS_KEY` | 仅在服务器私有密钥入口或临时 shell 中出现；不得写入 `.bashrc`、`.env`、systemd、Compose 文件、项目文档或仓库 |
| `OBJECT_STORAGE_SESSION_TOKEN` | 使用临时凭证时随临时 AK/SK 一起在运行期获取；不得落入长期文件 |
| IAM 用户登录密码 | 不记录 |
| SSH 私钥 | 不记录 |
| 数据库密码 | 不记录 |

## 6. 服务器端私有配置入口

推荐顺序：

1. 正式部署：ECS 绑定 IAM 委托，后端运行期从云环境获取临时凭证。项目文档只记录“使用委托模式”和非敏感桶配置，不记录任何 AK/SK。
2. 短期兜底：使用专用 IAM 用户的新 AK/SK，由用户在服务器私有管理入口注入。该入口必须位于仓库之外，不写入 `.bashrc`、`.env`、systemd unit、Compose 文件、Markdown 文档或聊天。
3. 临时验证：只允许在用户私有 shell 当前会话中临时设置环境变量，验证后立即 `unset`。该方式不得升级为正式部署配置。

如果后续采用 Docker Compose 部署，Compose 文件只允许引用抽象的外部 secret 或运行环境能力，不得直接写入密钥明文。

## 7. 密钥生命周期规则

- 前序验证中曾暴露 Access Key Id，建议用户在华为云 IAM 控制台删除或禁用本次验证用访问密钥。
- 正式服务端配置应重新创建未泄露的新密钥，或优先改用 IAM 委托。
- 若使用专用 IAM 用户 AK/SK，应建立轮换记录：创建时间、用途、权限范围、轮换时间、禁用/删除时间。记录中不得写明文密钥值。
- 任一凭证疑似进入聊天、截图、日志、仓库、客户端或文档后，应立即禁用或删除，并重新生成。

## 8. 待用户在控制台确认的事项

本线程不执行真实开通、创建密钥、创建委托或付费动作。若进入控制台操作，建议用户按以下顺序确认：

1. 删除或禁用前序验证用访问密钥。
2. 检查并删除可能残留对象：`technical-spike/min-connectivity/2653b580948d435bb978c2220427fc8e.txt`。
3. 若采用首选方案，创建 ECS 可用的 IAM 委托，并只授予当前桶必要对象权限。
4. 若采用兜底方案，创建专用 IAM 用户和新访问密钥，只授予当前桶必要对象权限，并保存到服务器私有密钥入口。

## 9. 本轮明确不做

- 不实现业务附件上传接口。
- 不创建业务数据库表。
- 不修改后端或 Qt 代码。
- 不创建真实 IAM 用户、访问密钥、委托或权限策略。
- 不把任何敏感凭证写入项目文档、仓库、聊天或 Qt 客户端。
- 不创建新的长期付费附加云产品。
