# 验证记录：华为云 OBS 最小连通性

## 基本信息

- 日期：2026-05-18
- 线程：关键环境验证：对象存储最小连通性
- 验证对象：华为云 OBS 私有桶 `20260518-bighomework`
- 地域：`华北-北京四`
- Endpoint：`obs.cn-north-4.myhuaweicloud.com`
- 访问方式：本机 Windows PowerShell 中的服务端探路脚本通过 S3 SigV4 请求访问 OBS
- 探路脚本：`D:\big_homework\scripts\obs_min_connectivity_probe.ps1`

## 安全边界

- AccessKey、SecretKey、临时凭证不得写入本文档、聊天记录、代码注释或长期项目配置明文。
- 探路脚本只从当前进程环境变量读取服务端凭证。
- 探路脚本输出只允许显示环境变量名称、Endpoint、地域、桶名、对象 key、HTTP 状态和校验结果。
- Qt 客户端不得持有 OBS 凭证，也不得直接访问 OBS。
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

## 验证设计

脚本计划执行以下最小闭环：

1. 生成一次性测试对象 key，前缀为 `technical-spike/min-connectivity/`。
2. PUT 上传一段 UTF-8 文本到私有桶。
3. GET 读取同一对象。
4. 对读取内容计算 SHA-256，并与上传前哈希比对。
5. DELETE 删除测试对象。
6. 输出每一步 HTTP 状态和最终通过 / 失败结论。

## 本次执行记录

### 第一次执行：无凭证环境检查

- 执行命令：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File 'D:\big_homework\scripts\obs_min_connectivity_probe.ps1'
```

- 执行结果：未进入真实 OBS 请求，脚本安全退出。
- 退出原因：当前进程未检测到服务端对象存储凭证环境变量。
- 已验证事项：
  - 探路脚本存在并可启动。
  - 缺少服务端凭证时，脚本不会构造真实上传 / 读取 / 删除请求。
  - 脚本不会输出 AccessKey、SecretKey 或临时凭证明文。
  - 当前本机环境未发现 `OBS`、`OBJECT_STORAGE`、`AWS`、`HUAWEI`、`HWC`、`S3` 相关凭证环境变量名称。

### 第二次执行：用户私有 PowerShell 注入凭证后运行

- 执行位置：用户本机 PowerShell，工作目录 `D:\big_homework`
- 执行命令：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\obs_min_connectivity_probe.ps1
```

- 已确认结果：
  - 脚本成功启动并识别非敏感配置：Endpoint、Region、Bucket。
  - 凭证来自环境变量名称：`OBJECT_STORAGE_ACCESS_KEY_ID`、`OBJECT_STORAGE_SECRET_ACCESS_KEY`。
  - 测试对象 key：`technical-spike/min-connectivity/2653b580948d435bb978c2220427fc8e.txt`。
  - PUT 上传返回 HTTP `200`。
- 中断原因：
  - Windows PowerShell 不允许 GET 请求携带内容正文。
  - 原脚本在 GET / DELETE 阶段传入了空 byte body，导致错误：`无法发送具有此谓词类型的内容正文。`
- 修复记录：
  - 已修复 `D:\big_homework\scripts\obs_min_connectivity_probe.ps1`。
  - 修复后只有 PUT 请求携带 body；GET / DELETE 只发送签名头，不再发送空 body。
- 当前状态：
  - 写入链路已验证成功。
  - 读取校验和删除仍需复跑脚本确认。
  - 因第二次执行在 DELETE 前中断，桶内可能残留一个测试对象，路径为 `technical-spike/min-connectivity/2653b580948d435bb978c2220427fc8e.txt`；后续复跑成功后，新对象会自动删除，旧残留对象可在 OBS 控制台手动删除。

### 第三次执行：脚本修复后复跑通过

- 执行位置：用户本机 PowerShell，工作目录 `D:\big_homework`
- 执行命令：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\obs_min_connectivity_probe.ps1
```

- 测试对象 key：`technical-spike/min-connectivity/bcd78c4da4b9421eb69a351631aae8c3.txt`
- 执行结果：
  - PUT 上传返回 HTTP `200`
  - GET 读取返回 HTTP `200`
  - SHA-256 校验通过：`b7ded3d9bd66b00b7cf6ae6ae41ab2dcd5743497c0534b8bef9c8eae9eefe777`
  - DELETE 删除返回 HTTP `204`
  - 脚本输出：`OBS minimal connectivity probe passed.`
- 敏感信息处理：
  - 本记录不保存 AccessKey、SecretKey 或临时凭证明文。
  - 本次输出只记录环境变量名称、对象 key、HTTP 状态和内容哈希。

## 清理提醒

- 第三次执行创建的对象已由脚本删除。
- 第二次执行在 DELETE 前中断，桶内可能残留对象：`technical-spike/min-connectivity/2653b580948d435bb978c2220427fc8e.txt`。
- 建议用户在 OBS 控制台进入桶 `20260518-bighomework`，检查 `technical-spike/min-connectivity/` 前缀并删除残留测试对象。
- 因 Access Key Id 曾经出现在聊天中，建议完成验证后在华为云 IAM 控制台禁用或删除本次使用的访问密钥，后续重新创建未泄露的新密钥用于正式服务端配置。

## 复跑方式

需要由用户在本机或服务器的私有终端中设置服务端凭证环境变量后复跑，不要把具体值粘贴到聊天或写入项目文档。

推荐使用以下变量名：

```powershell
$env:OBJECT_STORAGE_ACCESS_KEY_ID = '<仅在私有终端设置，不写入文档>'
$env:OBJECT_STORAGE_SECRET_ACCESS_KEY = '<仅在私有终端设置，不写入文档>'
# 如使用临时凭证，再设置：
# $env:OBJECT_STORAGE_SESSION_TOKEN = '<仅在私有终端设置，不写入文档>'

powershell -NoProfile -ExecutionPolicy Bypass -File 'D:\big_homework\scripts\obs_min_connectivity_probe.ps1'
```

脚本也兼容以下环境变量名称：

- AccessKey：`OBJECT_STORAGE_ACCESS_KEY_ID`、`OBS_ACCESS_KEY_ID`、`HUAWEICLOUD_OBS_ACCESS_KEY_ID`、`AWS_ACCESS_KEY_ID`
- SecretKey：`OBJECT_STORAGE_SECRET_ACCESS_KEY`、`OBS_SECRET_ACCESS_KEY`、`HUAWEICLOUD_OBS_SECRET_ACCESS_KEY`、`AWS_SECRET_ACCESS_KEY`
- 临时凭证：`OBJECT_STORAGE_SESSION_TOKEN`、`OBS_SESSION_TOKEN`、`HUAWEICLOUD_OBS_SESSION_TOKEN`、`AWS_SESSION_TOKEN`

## 当前结论

- 本机对象存储最小连通性验证已通过。
- 基于华为云 OBS 桶 `20260518-bighomework`，本机 Windows PowerShell 中的服务端探路脚本已完成上传、读取、SHA-256 校验和删除闭环。
- 已确认非敏感配置 `Endpoint`、`Region`、`Bucket` 可用于后续服务端对象存储配置边界。
- 已确认 AccessKey、SecretKey、临时凭证仍不得写入项目文档、仓库、聊天或 Qt 客户端。
- 本记录尚未验证 ECS 服务器端到 OBS 的真实连通性；正式部署前仍需在 ECS 上执行等价最小连通性验证。
- 正式附件业务实现前仍需单独补充权限校验、MIME / 文件大小限制、对象 key 规则、附件元数据表和后端业务接口测试。

## 本次明确未做

- 未实现业务附件上传接口。
- 未创建业务数据库表。
- 未修改后端代码。
- 未修改 Qt 代码。
- 未向 Qt 客户端提供对象存储凭证。
- 未记录 AccessKey、SecretKey、SSH 私钥、数据库密码或对象存储临时凭证明文。
- 未创建新的长期付费云产品。
