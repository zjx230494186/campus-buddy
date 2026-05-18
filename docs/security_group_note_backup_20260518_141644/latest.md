# Latest Handoff

本文档服务于新线程快速接手。当前文件已按 UTF-8 重写；旧文件备份在：`D:\big_homework\docs\encoding_repair_backup_20260518_141039\`。

## 当前状态

- 项目名称：校园搭子平台
- 当前阶段：关键环境验证执行
- 当前线程：短时云服务器基础环境配置
- 云服务器：华为云 ECS，按需计费短时实例
- 公网 IP：`114.116.203.78`
- 私有 IP：`192.168.0.165`
- 主机名：`ecs-ead3`
- 系统：Ubuntu 24.04.4 LTS
- 服务器基础环境配置记录：`D:\big_homework\docs\validation\20260518_server_base_environment_configuration_record.md`

## 当前线程完成了什么

- 使用项目文档记录的公网 IP 和本机 PEM 路径完成 SSH 登录；未记录私钥内容。
- 检查系统版本、磁盘、内存、当前用户和基础网络。
- 执行系统更新并重启；重启后确认无需继续 reboot。
- 创建普通运维用户 `ops`，配置 SSH 公钥登录、sudo 权限和 docker 组权限。
- 禁用 SSH 密码交互登录，保留密钥登录。
- 启用 UFW 主机防火墙：
  - 默认拒绝入站。
  - 允许 `22/tcp`、`80/tcp`、`443/tcp`。
  - 显式拒绝 `5432/tcp`。
- 安装并验证 Docker、Docker Compose v2、Nginx。
- 验证 Nginx 默认页公网 HTTP 可访问：`http://114.116.203.78/` 返回 200，标题为 `Welcome to nginx!`。
- 更新 `docs/03_current_plan.md`。
- 更新 `handoff/latest.md`。
- 新增验证记录：`docs/validation/20260518_server_base_environment_configuration_record.md`。

## 关键验证结果

- Docker：`29.1.3`
- Docker Compose：`2.40.3+ds1-0ubuntu1~24.04.1`
- Nginx：`1.24.0-2ubuntu7.8`
- `docker version`：通过
- `docker compose version`：通过
- `sudo nginx -t`：通过
- Docker 服务：enabled / active
- Nginx 服务：enabled / active
- 服务器侧 `ss -tulpen`：公网监听 `22/tcp`、`80/tcp`；未监听 `5432/tcp`。
- UFW：active；`5432/tcp` 显式 DENY。

## 注意事项

- Docker 官方 apt 仓库在 ECS 上出现 TLS 握手失败，因此本线程使用 Ubuntu 24.04 / 华为云镜像源中的 `docker.io`、`docker-compose-v2`、`docker-buildx` 完成基础环境闭环。
- Codex 当前网络接口的 `Test-NetConnection` 对未监听端口结果不可靠：`5432` 和未监听的 `443` 都曾返回 True。服务器侧 `ss` 与 `ufw` 更可信。
- 本线程没有云账号 AccessKey，也未读取华为云控制台安全组规则。用户仍应在控制台最终确认入方向只包含 `22`、`80`、`443`，不得开放 `5432` 或全部端口。
- 当前 ECS 是按需计费资源。若不继续验证，必须删除 ECS 实例、云硬盘、弹性公网 IP 等关联资源，避免持续扣费。

## 本线程没有做什么

- 没有部署业务代码。
- 没有创建业务数据库表。
- 没有配置真实业务数据库密码。
- 没有修改后端代码。
- 没有修改 Qt 代码。
- 没有创建长期付费附加云产品。
- 没有把密码、SSH 私钥、AccessKey、数据库密码等敏感信息写入文档明文。

## 下一步候选事项

1. `华为云控制台安全组最终复核`
   - 建议：复用当前线程或由用户在控制台完成后回复结果。
   - 目标：确认入方向只保留 `22`、`80`、`443`，没有 `5432`、`0.0.0.0/0 全端口` 或其他宽泛规则。

2. `云服务器部署骨架验证：Compose Nginx HTTPS 方案`
   - 建议：新开线程。
   - 目标：只验证 Compose 目录结构、Nginx 反向代理健康检查、HTTPS 证书方案；不部署完整业务，不创建业务数据库表。

3. `清理按需云资源`
   - 建议：复用当前线程或新开短线程。
   - 目标：如果暂不继续验证，删除 ECS 实例、云硬盘、公网 IP 等关联资源，停止按需计费。

## 建议归档与下一线程

- 建议归档当前线程：是。
- 原因：本线程目标的服务器基础环境配置已完成，并已形成可交接记录；剩余工作要么是云控制台人工确认，要么是新的部署骨架验证方向。
- 下一线程名称：`云服务器部署骨架验证：Compose Nginx HTTPS 方案`

## 下一线程可直接复制的启动 prompt

```text
请先阅读：
- D:\big_homework\AGENTS.md
- D:\big_homework\docs\03_current_plan.md
- D:\big_homework\handoff\latest.md
- D:\big_homework\docs\validation\20260518_server_base_environment_configuration_record.md
- D:\big_homework\docs\16_server_purchase_and_deployment_base_v1.md

当前任务：云服务器部署骨架验证：Compose Nginx HTTPS 方案。

本线程目标：
1. 基于已完成基础环境配置的华为云 ECS，验证最小部署骨架。
2. 只做 Compose 目录结构、Nginx 反向代理健康检查、HTTPS 证书方案文档化。
3. 不部署完整业务代码，不创建业务数据库表，不配置真实业务数据库密码。
4. 不把任何真实密钥、密码、AccessKey、数据库密码写入聊天或项目文档明文。
5. 明确当前 ECS 为按需计费资源，验证结束后必须清理或转入长期低成本方案。

严格限制：
1. 不开放 PostgreSQL 5432 到公网。
2. 不创建长期付费附加云产品。
3. 不修改后端或 Qt 业务代码，除非本线程后续明确转入代码实现并遵守测试先行规则。
```

