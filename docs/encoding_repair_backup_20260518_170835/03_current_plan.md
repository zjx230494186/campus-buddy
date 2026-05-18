# 03 Current Plan

本文档只维护当前阶段最重要事项，不承载全部历史。

## 文档编码规则

- 项目 Markdown 文档统一使用 UTF-8 编码保存。
- 不使用 GBK/ANSI 保存长期维护文档。
- 使用 PowerShell 写入中文文档时必须显式指定 `-Encoding UTF8`。
- 若发现乱码，先备份原文件，再修复为 UTF-8。
- 本次修复前已备份旧文件到：`D:\big_homework\docs\encoding_repair_backup_20260518_141039\`。

## 当前阶段主题

- 阶段：关键环境验证执行。
- 当前线程：关键环境验证：ECS 服务器端 OBS 最小连通性。
- 当前服务器背景：华为云 ECS，华北-北京四 / 可用区 a，`t6.large.1`，2 vCPU / 2 GiB，40 GiB 系统盘，公网 IP `114.116.203.78`，私有 IP `192.168.0.165`，按需计费。
- 当前目标：基于已创建的华为云 OBS 桶 `20260518-bighomework`，完成 ECS 服务器端到 OBS 的上传、读取校验和删除最小闭环；不实现业务附件上传接口，不创建业务数据库表，不修改后端或 Qt 代码。

## 本线程完成情况

- 已读取本线程指定文档，确认对象存储验证边界：只允许服务端测试或探路脚本访问 OBS，Qt 客户端不得持有对象存储凭证。
- 已新增独立探路脚本：`D:\big_homework\scripts\obs_min_connectivity_probe.ps1`。
- 探路脚本通过当前进程环境变量读取服务端凭证，兼容 `OBJECT_STORAGE_*`、`OBS_*`、`HUAWEICLOUD_OBS_*` 和 `AWS_*` 命名。
- 已执行一次无凭证环境检查，结果为安全退出，未向 OBS 发起上传 / 读取 / 删除请求，且未输出任何敏感值。
- 用户在本机私有 PowerShell 中注入服务端凭证后复跑探路脚本，已完成 PUT 上传、GET 读取、SHA-256 校验和 DELETE 删除闭环。
- 已新增验证记录：`D:\big_homework\docs\validation\20260518_obs_min_connectivity_probe_record.md`。
- 当前本机 OBS 最小连通性验证已通过。
- 已新增 ECS 服务器端验证脚本：`D:\big_homework\scripts\obs_min_connectivity_probe_ecs.py`。
- 已在 ECS 上执行无凭证网络检查，确认 `obs.cn-north-4.myhuaweicloud.com:443` 可连接，TLS version 为 `TLSv1.2`；缺少凭证时脚本安全退出，未执行对象操作。
- 用户在 ECS 私有 shell 中临时注入服务端凭证后，已完成 PUT 上传、GET 读取、SHA-256 校验和 DELETE 删除闭环。
- 已删除 ECS `/tmp/obs_min_connectivity_probe_ecs.py` 临时脚本，用户已执行 `unset` 清理本轮 shell 中的对象存储凭证环境变量。
- 已新增 ECS 服务器端验证记录：`D:\big_homework\docs\validation\20260518_ecs_obs_min_connectivity_probe_record.md`。
- 已复核详细设计中对象存储约束：后端中转访问对象存储、客户端不持有对象存储凭证、通过 S3 兼容对象存储接口抽象隔离厂商差异。
- 已比较华为云 OBS、阿里云 OSS、腾讯云 COS 和本地 / 自建 MinIO。
- 已按用户偏好确认推荐方向：优先使用华为云 OBS。
- 已形成对象存储租用与配置底座记录：`D:\big_homework\docs\17_object_storage_purchase_and_configuration_base_v1.md`。
- 用户已在华为云控制台创建 OBS 开发验证桶，非敏感配置已记录到 `D:\big_homework\docs\17_object_storage_purchase_and_configuration_base_v1.md`。

## 当前对象存储结论摘要

- 推荐云厂商：华为云。
- 推荐产品：对象存储服务 OBS。
- 已创建桶：`20260518-bighomework`。
- 地域：`华北-北京四`，与当前华为云 ECS 保持一致，便于后续同区域内网访问验证。
- Endpoint：`obs.cn-north-4.myhuaweicloud.com`。
- 访问域名：`20260518-bighomework.obs.cn-north-4.myhuaweicloud.com`。
- 推荐计费方式：初期按需计费，不先购买资源包；后续用量稳定后再评估标准存储包或流量包。
- 桶权限：私有；未开启公共读或公共读写。
- 存储类别：标准存储；数据冗余存储策略为单 AZ 存储。
- 公网读策略：默认不允许；不把 OBS 裸 URL 暴露给 Qt 客户端或未授权用户。
- AccessKey 管理：不使用主账号长期密钥；后续只允许后端服务使用最小权限 IAM 用户、IAM 委托或等价服务端授权；不得写入项目文档明文。
- CORS：当前 V1 Qt 桌面端 + 后端中转模式默认不需要配置 CORS；当前桶 CORS 未配置。
- MinIO：仅作为本地开发、测试或私有化部署备选，不作为当前云生产底座首选。
- ECS 服务器端验证：已通过。成功对象 key 为 `technical-spike/min-connectivity-ecs/20457e0012d045c996c6ed1e2454a493.txt`，PUT `200`，GET `200`，SHA-256 校验通过，DELETE `204`。

## 重要偏差与风险

- 对象存储当前已创建开发验证桶；验证过程中使用过一次服务端访问密钥，但未在项目文档中记录 SecretKey 或临时凭证明文。
- 用户曾在聊天中暴露 Access Key Id；虽然未暴露 SecretKey，但仍建议验证结束后删除或禁用本次访问密钥，后续重新创建未泄露的新密钥用于正式服务端配置。
- 第二次探路执行在 DELETE 前中断，桶内可能残留测试对象：`technical-spike/min-connectivity/2653b580948d435bb978c2220427fc8e.txt`，建议在 OBS 控制台手动检查并删除。
- 对象存储默认按需计费；即使不买资源包，只要桶和对象存在并被访问，仍可能产生存储、请求和公网流出等费用。
- 华为云 OBS 已通过本机和 ECS 服务器端最小连通性验证；当前对象存储底座验证最小闭环已完成。
- 后续正式业务实现仍需补充权限校验、MIME / 大小限制、附件元数据表、后端业务接口测试和生产级凭证管理。
- 当前 ECS 是按需计费资源，验证结束后必须删除 ECS、云硬盘、公网 IP 等关联资源，避免持续扣费。
- 前一线程记录：Docker 官方 apt 仓库在该 ECS 上 TLS 握手失败，因此服务器基础环境改用 Ubuntu 24.04 / 华为云镜像源中的 `docker.io`、`docker-compose-v2`、`docker-buildx` 完成闭环。

## 当前明确不做

- 不部署业务代码。
- 不创建业务数据库表。
- 不配置真实业务数据库密码。
- 不修改后端或 Qt 代码。
- 不实现业务附件上传接口。
- 不让 Qt 客户端或用户侧直接持有对象存储访问凭证。
- 不把密码、SSH 私钥、AccessKey、SecretKey、临时凭证、数据库密码等敏感信息写入项目文档明文。
- 不配置对象存储公共读作为附件访问方案。
- 不创建长期付费附加云产品。

## 当前线程收束判断

- ECS 服务器端 OBS 最小连通性验证已完成最小闭环，可收束。
- 本线程只验证了 ECS 服务器端探路脚本访问 OBS 的上传、读取校验和删除，不代表业务附件上传接口已实现。
- 收束前建议用户清理可能残留的本机验证测试对象，并删除或禁用本次使用的访问密钥。

## 推荐下一步

首选下一步：清理 OBS 验证残留对象与访问密钥，然后新开 `对象存储凭证最小权限与配置体系确认` 或回到更高优先级的环境验证队列。

推荐理由：

- 本机 OBS 最小连通性已经通过，继续保留已暴露 Access Key Id 对应的访问密钥没有必要。
- 第二次执行可能留下一个测试对象，应手动检查并删除。
- ECS 服务器端到 OBS 的真实网络、Linux 环境和凭证注入方式已验证通过；后续重点转向最小权限和正式配置体系。
- 若短时 ECS 暂不继续使用，应优先清理按需资源，避免持续计费。
