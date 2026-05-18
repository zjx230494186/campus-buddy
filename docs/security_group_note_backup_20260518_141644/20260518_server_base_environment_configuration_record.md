# 服务器基础环境配置记录

## 基本信息

- 日期：2026-05-18
- 线程：短时云服务器基础环境配置
- 云厂商：华为云
- 产品：弹性云服务器 ECS，按需计费短时实例
- 区域 / 可用区：华北-北京四 / 可用区 a
- 公网 IP：`114.116.203.78`
- 私有 IP：`192.168.0.165`
- 主机名：`ecs-ead3`
- 登录方式：SSH 密钥文件登录；本机 PEM 路径记录在 `docs/16_server_purchase_and_deployment_base_v1.md`，不记录私钥内容。

## 本线程边界

- 已完成：系统级基础设施准备、最小主机防火墙加固、系统更新、普通运维用户、Docker、Docker Compose v2、Nginx 安装与验证。
- 未执行：业务代码部署、业务数据库表创建、真实业务数据库密码配置、后端代码修改、Qt 代码修改、长期付费附加云产品创建。
- 敏感信息处理：未在聊天或项目文档中写入密码、SSH 私钥、AccessKey、数据库密码等敏感明文。

## 初始检查

- 操作系统：Ubuntu 24.04.3 LTS，系统更新后为 Ubuntu 24.04.4 LTS。
- Kernel：`6.8.0-87-generic`
- 磁盘：`/dev/vda1`，ext4，40G；配置完成后约 5.1G 已用，33G 可用。
- 内存：约 1.7GiB；配置完成后可用约 1.4GiB。
- Swap：未启用。
- 初始登录用户：`root`
- 运维用户：`ops`
- 基础网络：`eth0` 私有地址 `192.168.0.165/24`，默认网关 `192.168.0.1`。

## 安全与访问控制

### 云安全组复核状态

- 前序文档记录用户已在华为云控制台确认安全组无误。
- 本线程没有云账号 AccessKey，也没有读取云控制台规则的权限，因此未通过 API 导出安全组规则。
- 本机端口探测存在异常：Codex 当前网络接口显示为 `Meta`，对 `5432` 在无服务监听、主机防火墙拒绝的情况下仍返回 `TcpTestSucceeded=True`；同样对服务器未监听的 `443` 也返回 True。该结果不能作为安全组真实放行结论。
- 服务器侧权威复核：
  - `ss -tulpen` 显示公网监听只有 `22/tcp` 与 `80/tcp`。
  - 未发现 `5432/tcp` 监听。
  - `ufw` 已启用，默认拒绝入站，只允许 `22/tcp`、`80/tcp`、`443/tcp`，并显式拒绝 `5432/tcp`。

### 主机防火墙

已执行 `ufw` 最小规则：

```text
Default: deny (incoming), allow (outgoing)
22/tcp   ALLOW IN  Anywhere  # SSH
80/tcp   ALLOW IN  Anywhere  # HTTP
443/tcp  ALLOW IN  Anywhere  # HTTPS
5432/tcp DENY IN   Anywhere  # Do not expose PostgreSQL
```

### SSH 加固

- 新增普通运维用户：`ops`
- `ops` 加入组：`sudo`、`docker`
- `ops` 使用 SSH 公钥登录；未创建或记录明文密码。
- SSH 配置：
  - `PasswordAuthentication no`
  - `KbdInteractiveAuthentication no`
  - `PubkeyAuthentication yes`
  - `PermitRootLogin prohibit-password`
- 已验证 `ops` 可通过同一 PEM 登录并执行 `sudo -n true`。

## 系统更新与安装

### 系统更新

- 已执行 `apt-get update`
- 已执行 `apt-get -y upgrade`
- 升级后系统提示需要重启，已执行重启。
- 重启后复核：`/var/run/reboot-required` 不存在，输出 `no-reboot-required`。

### Docker

安装过程说明：

- 先按 Docker 官方 Ubuntu apt 仓库方式尝试安装 Docker Engine 与 Compose Plugin。
- 远端访问 `https://download.docker.com/linux/ubuntu` 时出现 TLS 握手失败，Docker CE 仓库索引不可用。
- 为完成短时服务器基础环境闭环，改用 Ubuntu 24.04 / 华为云镜像源中的 `docker.io`、`docker-compose-v2`、`docker-buildx`。
- 该安装仍提供 Docker daemon、Docker CLI 与 `docker compose` v2 命令；偏差已在本文档记录。

验证结果：

```text
docker version
Client Version: 29.1.3
Server Version: 29.1.3

docker compose version
Docker Compose version 2.40.3+ds1-0ubuntu1~24.04.1

systemctl is-enabled docker
enabled

systemctl is-active docker
active
```

## Nginx

安装来源：Ubuntu 24.04 / 华为云镜像源。

验证结果：

```text
nginx -t
nginx: the configuration file /etc/nginx/nginx.conf syntax is ok
nginx: configuration file /etc/nginx/nginx.conf test is successful

systemctl is-enabled nginx
enabled

systemctl is-active nginx
active

curl -I http://127.0.0.1/
HTTP/1.1 200 OK
Server: nginx/1.24.0 (Ubuntu)
```

公网 HTTP 默认页验证：

```text
GET http://114.116.203.78/
StatusCode: 200
Title: Welcome to nginx!
```

## 重启后监听状态

重启后 `ss -tulpen` 关键公网监听：

```text
0.0.0.0:22  sshd
0.0.0.0:80  nginx
[::]:22     sshd
[::]:80     nginx
```

未监听：

- `443/tcp`：当前未配置 HTTPS 证书，因此 Nginx 尚未监听 443；防火墙已预留允许规则，供后续 HTTPS 验证使用。
- `5432/tcp`：未监听，且 UFW 显式拒绝入站。

## 当前结论

- 服务器基础环境配置已完成，可作为后续部署骨架验证的短时基础环境。
- 当前实例是按需计费资源，验证结束后必须删除 ECS 实例、云硬盘、弹性公网 IP 等关联资源，避免持续扣费。
- 后续若需要长期保留环境，应重新评估包年包月轻量服务器或更低成本套餐，不建议长期运行当前按需 ECS。

## 后续建议

1. 在华为云控制台由用户最终确认安全组入方向只包含必要端口：`22`、`80`、`443`；不得包含 `5432` 或全部端口。
2. 后续部署业务前再配置 Docker 日志轮转、Compose 目录结构、`.env` 敏感配置管理和 Nginx 反向代理。
3. 配置 HTTPS 前先确认域名、备案和 DNS 解析状态。

