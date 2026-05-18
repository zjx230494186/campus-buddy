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
- 当前线程：短时云服务器基础环境配置。
- 当前服务器：华为云 ECS，华北-北京四 / 可用区 a，`t6.large.1`，2 vCPU / 2 GiB，40 GiB 系统盘，公网 IP `114.116.203.78`，私有 IP `192.168.0.165`，按需计费。
- 当前目标：只完成系统级基础设施准备，不部署业务代码，不创建业务数据库表，不配置真实业务数据库密码。

## 本线程完成情况

- 已使用本机 PEM 登录华为云 ECS，未记录私钥内容。
- 已检查系统版本、磁盘、内存、当前用户和基础网络。
- 已执行系统更新并重启，重启后确认不再要求 reboot。
- 已创建普通运维用户 `ops`，配置 SSH 公钥登录、sudo 和 docker 组权限。
- 已关闭 SSH 密码交互登录，保留密钥登录。
- 已启用 UFW：默认拒绝入站，只允许 `22/80/443`，并显式拒绝 `5432`。
- 已安装并验证 Docker、Docker Compose v2、Nginx。
- 已验证 Nginx 默认页公网 HTTP 返回 200，标题为 `Welcome to nginx!`。
- 已新增服务器基础环境配置记录：`D:\big_homework\docs\validation\20260518_server_base_environment_configuration_record.md`。

## 当前验证结果摘要

- 操作系统：Ubuntu 24.04.4 LTS
- Kernel：`6.8.0-87-generic`
- Docker：`29.1.3`
- Docker Compose：`2.40.3+ds1-0ubuntu1~24.04.1`
- Nginx：`1.24.0-2ubuntu7.8`
- Docker 服务：enabled / active
- Nginx 服务：enabled / active
- `sudo nginx -t`：通过
- `http://114.116.203.78/`：HTTP 200，Nginx 默认页
- 服务器侧公网监听：`22/tcp`、`80/tcp`
- `5432/tcp`：服务器侧未监听，UFW 显式拒绝入站

## 重要偏差与风险

- Docker 官方 apt 仓库在该 ECS 上 TLS 握手失败，因此本线程改用 Ubuntu 24.04 / 华为云镜像源中的 `docker.io`、`docker-compose-v2`、`docker-buildx` 完成基础环境闭环。
- Codex 当前网络接口对未监听端口的 `Test-NetConnection` 结果不可靠：`5432` 和未监听的 `443` 都返回过 True。因此以服务器侧 `ss`、`ufw` 和 Nginx 实际响应作为主要验证依据。
- 本线程没有云账号 AccessKey，也没有读取华为云控制台安全组规则的权限。云控制台安全组入方向仍建议由用户最终确认：只保留 `22`、`80`、`443`，不得开放 `5432` 或全部端口。
- 当前 ECS 是按需计费资源，验证结束后必须删除 ECS、云硬盘、公网 IP 等关联资源，避免持续扣费。

## 当前明确不做

- 不部署业务代码。
- 不创建业务数据库表。
- 不配置真实业务数据库密码。
- 不修改后端或 Qt 代码。
- 不把 PostgreSQL `5432` 暴露到公网。
- 不把密码、SSH 私钥、AccessKey、数据库密码等敏感信息写入项目文档明文。
- 不创建长期付费附加云产品。

## 推荐下一步

首选下一线程：`云服务器部署骨架验证：Compose Nginx HTTPS 方案`

推荐理由：

- 服务器基础环境已经具备 Docker、Compose v2、Nginx 和主机防火墙。
- 下一步可以只验证部署骨架：Compose 目录结构、Nginx 反向代理健康检查、HTTPS 证书方案，不进入完整业务部署。
- 若暂时不继续部署验证，应优先清理当前按需资源，避免持续计费。

