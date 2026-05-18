# Latest Handoff

本文档服务于新线程快速接手。当前文件按 UTF-8 保存；旧文件备份在：`D:\big_homework\docs\encoding_repair_backup_20260518_141039\`。

## 当前状态

- 项目名称：校园搭子平台
- 当前阶段：关键环境验证执行
- 当前线程：关键环境验证：ECS 服务器端 OBS 最小连通性
- 推荐对象存储：华为云 OBS
- 已创建桶：`20260518-bighomework`
- 地域：`华北-北京四`
- Endpoint：`obs.cn-north-4.myhuaweicloud.com`
- 访问域名：`20260518-bighomework.obs.cn-north-4.myhuaweicloud.com`
- 桶策略：私有桶，不开放公共读或公共读写
- 对象存储底座记录：`D:\big_homework\docs\17_object_storage_purchase_and_configuration_base_v1.md`
- 对象存储本机连通性验证记录：`D:\big_homework\docs\validation\20260518_obs_min_connectivity_probe_record.md`
- 对象存储 ECS 服务器端连通性验证记录：`D:\big_homework\docs\validation\20260518_ecs_obs_min_connectivity_probe_record.md`
- 对象存储连通性探路脚本：`D:\big_homework\scripts\obs_min_connectivity_probe.ps1`
- 对象存储 ECS 服务器端探路脚本：`D:\big_homework\scripts\obs_min_connectivity_probe_ecs.py`
- 服务器背景：华为云 ECS，按需计费短时实例，公网 IP `114.116.203.78`，私有 IP `192.168.0.165`

## 当前线程完成了什么

- 读取并复核本线程指定文档，确认本线程只验证对象存储最小连通性，不实现业务附件上传接口，不创建业务数据库表，不修改后端或 Qt 代码。
- 新增独立探路脚本：`D:\big_homework\scripts\obs_min_connectivity_probe.ps1`。
- 探路脚本通过 S3 SigV4 请求访问 OBS，设计闭环为 PUT 上传、GET 读取、SHA-256 校验、DELETE 删除。
- 探路脚本只从当前进程环境变量读取服务端凭证，不输出 AccessKey、SecretKey 或临时凭证明文。
- 已执行一次无凭证环境检查，脚本安全退出，未向 OBS 发起真实对象操作。
- 新增验证记录：`D:\big_homework\docs\validation\20260518_obs_min_connectivity_probe_record.md`。
- 用户在本机私有 PowerShell 中注入服务端凭证后复跑探路脚本，最终完成 PUT 上传、GET 读取、SHA-256 校验和 DELETE 删除闭环。
- 已明确本机结果只代表本机服务端探路脚本连通性，因此本轮继续补充 ECS 服务器端真实验证。
- 已新增 ECS 服务器端探路脚本：`D:\big_homework\scripts\obs_min_connectivity_probe_ecs.py`。
- 已通过 SSH 将脚本临时放置到 ECS `/tmp/obs_min_connectivity_probe_ecs.py`。
- 已在 ECS 上执行无凭证网络检查，确认 `obs.cn-north-4.myhuaweicloud.com:443` 可连接，TLS version 为 `TLSv1.2`；缺少凭证时脚本安全退出，未执行对象操作。
- 用户在 ECS 私有 shell 中临时注入服务端凭证后，已完成 PUT 上传、GET 读取、SHA-256 校验和 DELETE 删除闭环。
- 已删除 ECS `/tmp/obs_min_connectivity_probe_ecs.py` 临时脚本，用户已执行 `unset` 清理本轮 shell 中的对象存储凭证环境变量。
- 已新增 ECS 服务器端验证记录：`D:\big_homework\docs\validation\20260518_ecs_obs_min_connectivity_probe_record.md`。
- 已更新验证记录、当前计划和交接文档。
- 读取并复核项目协作规则、当前计划、交接记录、服务器基础环境记录、服务器租用部署底座、详细设计和技术探路计划。
- 基于详细设计约束确认对象存储必须采用后端中转访问，Qt 客户端不得持有对象存储凭证。
- 比较华为云 OBS、阿里云 OSS、腾讯云 COS 和本地 / 自建 MinIO。
- 按用户偏好形成推荐：优先使用华为云 OBS，地域优先 `华北-北京四`，与当前 ECS 同区域。
- 确认权限策略：私有桶，不公共读，不公共读写，不返回对象存储裸 URL；附件访问由后端业务权限校验后中转。
- 确认计费策略：初期按需计费，不先购买资源包；后续用量稳定后再评估标准存储包或流量包。
- 确认 CORS 策略：当前 Qt 桌面端 + 后端中转模式默认不配置 CORS。
- 用户已在华为云控制台创建 OBS 开发验证桶：`20260518-bighomework`。
- 已记录非敏感配置：地域、Endpoint、访问域名、标准存储、单 AZ、私有桶、CORS 未配置、WORM 未启用。
- 新增对象存储底座记录：`docs/17_object_storage_purchase_and_configuration_base_v1.md`。
- 更新 `docs/03_current_plan.md`。
- 更新 `handoff/latest.md`。

## 关键结论

- OBS 本机最小连通性验证已通过。
- 成功对象 key：`technical-spike/min-connectivity/bcd78c4da4b9421eb69a351631aae8c3.txt`。
- 成功结果：PUT `200`，GET `200`，SHA-256 校验通过，DELETE `204`。
- OBS ECS 服务器端最小连通性验证已通过。
- ECS 成功对象 key：`technical-spike/min-connectivity-ecs/20457e0012d045c996c6ed1e2454a493.txt`。
- ECS 成功结果：PUT `200`，GET `200`，SHA-256 校验通过，DELETE `204`。
- ECS 到 `obs.cn-north-4.myhuaweicloud.com:443` 的网络访问可用，TLS version 为 `TLSv1.2`。
- 因 Access Key Id 曾经出现在聊天中，建议验证结束后删除或禁用本次访问密钥，后续重新创建未泄露的新密钥用于正式服务端配置。
- 第二次执行在 DELETE 前中断，桶内可能残留对象：`technical-spike/min-connectivity/2653b580948d435bb978c2220427fc8e.txt`，建议在 OBS 控制台手动检查并删除。
- 已创建方案：华为云 OBS，标准存储，单 AZ，私有桶，后端中转访问。
- 桶名称：`20260518-bighomework`。
- 地域：`华北-北京四`。
- Endpoint：`obs.cn-north-4.myhuaweicloud.com`。
- 访问域名：`20260518-bighomework.obs.cn-north-4.myhuaweicloud.com`。
- 权限策略：默认私有；不启用公共读或公共读写；后端服务使用最小权限服务端凭证；Qt 客户端无对象存储凭证。
- 访问方式：`Qt 客户端 -> 后端 REST API -> ObjectStorageService 抽象 -> 华为云 OBS`。
- CORS：当前不配置；未来只有 Web 管理端或浏览器直连对象存储需求明确后再评估。
- MinIO：仅作为本地开发测试候选，不作为当前云生产底座首选。
- 阿里云 OSS / 腾讯云 COS：作为迁移备选，后续通过对象 key、元数据和对象存储适配器保留迁移路径。

## 注意事项

- 对象存储开发验证桶已创建；项目文档没有记录 SecretKey 或临时凭证明文。
- 用户曾在聊天中暴露 Access Key Id，建议删除或禁用本次访问密钥。
- 不得把 AccessKey、SecretKey、临时凭证、云账号密码、SSH 私钥或数据库密码写入聊天或项目文档明文。
- 对象存储按需计费也会因存储、请求、公网流出等产生费用；验证桶不用后应删除对象和桶。
- 当前 ECS 是按需计费资源。若不继续验证，必须删除 ECS 实例、云硬盘、弹性公网 IP 等关联资源，避免持续扣费。
- 前一服务器线程记录：Docker 官方 apt 仓库在 ECS 上出现 TLS 握手失败，因此使用 Ubuntu 24.04 / 华为云镜像源中的 `docker.io`、`docker-compose-v2`、`docker-buildx` 完成基础环境闭环。

## 本线程没有做什么

- 已完成本机和 ECS 服务器端真实 OBS 上传、读取校验和删除。
- 没有部署业务代码。
- 没有创建业务数据库表。
- 没有配置真实业务数据库密码。
- 没有修改后端代码。
- 没有修改 Qt 代码。
- 没有实现业务附件上传接口。
- 没有生成或记录对象存储 AccessKey / SecretKey / 临时凭证。
- 没有让 Qt 客户端持有对象存储凭证。
- 没有创建长期付费附加云产品。

## 下一步候选事项

1. `清理 OBS 验证残留对象与访问密钥`
   - 建议：复用当前线程。
   - 优先级：高。
   - 目标：删除可能残留的测试对象 `technical-spike/min-connectivity/2653b580948d435bb978c2220427fc8e.txt`，并在 IAM 控制台删除或禁用本次使用的访问密钥。
   - 严格限制：不记录 AccessKey、SecretKey、临时凭证或密码明文；不让 Qt 客户端持有对象存储凭证。

2. `关键环境验证：ECS 服务器端 OBS 最小连通性`
   - 建议：已完成，无需再新开同名线程。
   - 优先级：已关闭。
   - 目标：在华为云 ECS 上执行等价 OBS 最小连通性验证，确认服务器端上传、读取校验和删除闭环。
   - 严格限制：不记录 AccessKey、SecretKey、临时凭证或密码明文；不部署业务后端；不实现业务附件上传接口；不让 Qt 客户端持有对象存储凭证。

3. `对象存储凭证最小权限与配置体系确认`
   - 建议：新开线程。
   - 优先级：中高。
   - 目标：确认 IAM 用户或委托、最小权限策略、服务器本地私有配置方式；不把任何密钥写入项目文档。

4. `清理按需云资源`
   - 建议：复用当前线程或新开短线程。
   - 目标：如果暂不继续使用服务器，删除 ECS 实例、云硬盘、公网 IP 等关联资源，停止按需计费。

5. `华为云控制台安全组最终复核`
   - 建议：复用当前线程或由用户在控制台完成后回复结果。
   - 目标：确认入方向只保留 `22`、`80`、`443`，没有 `5432`、`0.0.0.0/0 全端口` 或其他宽泛规则。

6. `云服务器部署骨架验证：Compose Nginx HTTPS 方案`
   - 建议：新开线程。
   - 目标：只验证 Compose 目录结构、Nginx 反向代理健康检查、HTTPS 证书方案；不部署完整业务，不创建业务数据库表。

## 建议归档与下一线程

- 建议归档当前线程：是，在完成残留对象和访问密钥清理后归档。
- 原因：本线程目标的 ECS 服务器端 OBS 最小连通性已完成；继续推进应转向凭证最小权限、配置体系或其他关键环境验证。
- 下一线程名称：`对象存储凭证最小权限与配置体系确认`

## 下一线程可直接复制的启动 prompt

```text
请先阅读：
- D:\big_homework\AGENTS.md
- D:\big_homework\docs\03_current_plan.md
- D:\big_homework\handoff\latest.md
- D:\big_homework\docs\17_object_storage_purchase_and_configuration_base_v1.md
- D:\big_homework\docs\13_detailed_design_v1.md
- D:\big_homework\docs\14_technical_spike_plan_v1.md
- D:\big_homework\docs\validation\20260518_obs_min_connectivity_probe_record.md
- D:\big_homework\docs\validation\20260518_ecs_obs_min_connectivity_probe_record.md

当前任务：对象存储凭证最小权限与配置体系确认。

当前状态：
1. 华为云 OBS 桶 `20260518-bighomework` 已创建。
2. 本机 OBS 最小连通性验证已通过：PUT 上传、GET 读取、SHA-256 校验、DELETE 删除。
3. 验证记录位于 `D:\big_homework\docs\validation\20260518_obs_min_connectivity_probe_record.md`。
4. ECS 服务器端 OBS 最小连通性验证已通过：PUT 上传、GET 读取、SHA-256 校验、DELETE 删除。
5. ECS 验证记录位于 `D:\big_homework\docs\validation\20260518_ecs_obs_min_connectivity_probe_record.md`。
6. 本机前序验证可能残留一个测试对象：`technical-spike/min-connectivity/2653b580948d435bb978c2220427fc8e.txt`。
7. 访问密钥不得写入项目文档、仓库、聊天或 Qt 客户端。

本轮目标：
1. 确认后续正式后端服务访问 OBS 的最小权限方案。
2. 确认 AccessKey / SecretKey、临时凭证或 IAM 委托的取舍。
3. 确认服务器端私有配置入口，不把凭证写入项目文档、仓库、聊天或 Qt 客户端。
4. 形成配置矩阵或凭证管理记录，并更新 `docs\03_current_plan.md` 与 `handoff\latest.md`。

严格限制：
1. 不实现业务附件上传接口。
2. 不创建业务数据库表。
3. 不修改后端或 Qt 代码。
4. 不把 AccessKey、SecretKey、SSH 私钥、数据库密码、对象存储临时凭证写入聊天或项目文档明文。
5. 不让 Qt 客户端或用户侧直接持有对象存储访问凭证。
6. 不创建超出对象存储确认所需的长期付费附加云产品。
7. 如涉及真实开通或付费动作，必须先给出完整决策包并等待用户确认。

建议操作边界：
1. 优先先形成决策包，不直接创建或粘贴新密钥。
2. 若需要真实创建 IAM 用户、访问密钥、委托或权限策略，先让用户在控制台确认并执行。
3. 不把凭证写入 `.bashrc`、`.env`、systemd、Compose 文件、项目文档或仓库。
4. 因前序聊天中曾暴露 Access Key Id，建议先在华为云 IAM 控制台删除或禁用本次验证用密钥，再为正式服务端配置重新创建未泄露的新密钥或改用更安全授权方式。
```
