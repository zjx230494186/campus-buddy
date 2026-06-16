# AI 帖子自动审核云端部署验证记录

日期：2026-06-16

## 范围

- 将已实现的 AI 帖子自动审核后端部署到云端服务器。
- 验证 Qwen OpenAI-compatible provider 在云端真实生效。
- 不记录真实 `POST_MODERATION_API_KEY`。

## 服务器信息

- 公网 API：`http://114.116.203.78/api`
- SSH 用户：`campus_hcj`
- 主机名：`ecs-ead3`
- systemd 服务：`campus-buddy-backend`
- 后端目录：`/srv/campus-buddy`
- 后端 jar：`/srv/campus-buddy/campus-buddy-backend-0.0.1-SNAPSHOT.jar`
- 私有配置：`/etc/campus-buddy/backend.env`

## 本地冲突检查

命令：

```powershell
git diff --name-only --diff-filter=U
rg -n "<<<<<<<|=======|>>>>>>>" backend/src/main docs/31_ai_post_moderation_design_v1.md
```

结果：

- 未发现未解决合并冲突文件。
- 未发现冲突标记。

## 本地构建

命令：

```powershell
cd D:\Github\campus-buddy\backend
mvn -DskipTests package
```

结果：

```text
BUILD SUCCESS
```

生成文件：

```text
backend/target/campus-buddy-backend-0.0.1-SNAPSHOT.jar
```

## 云端配置

`/etc/campus-buddy/backend.env` 需要包含以下变量名：

```bash
POST_MODERATION_ENABLED=true
POST_MODERATION_PROVIDER=openai-compatible
POST_MODERATION_API_KEY=<server-private-secret>
POST_MODERATION_MODEL=qwen-plus
POST_MODERATION_BASE_URL=https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions
```

安全要求：

- 真实 key 只写入服务器私有 env 文件。
- 不写入 Git、Markdown、日志或聊天记录。
- 修改 env 后执行 `sudo systemctl restart campus-buddy-backend`。

## 云端服务检查

服务器命令：

```bash
sudo systemctl status campus-buddy-backend --no-pager
```

结果：

```text
active
```

公网 health：

```powershell
GET http://114.116.203.78/api/health
```

结果：

```text
UP
```

## 真实账号 smoke

测试学生账号：`24301121@bjtu.edu.cn`

测试管理员账号：`smokeadmin@campus.edu.cn`

创建的测试帖子：

```text
postId=f5277005-919c-44ee-9441-038839523a4c
sceneType=STUDY
title=AI Smoke Study Test
```

提交审核结果：

```text
submit_status=PUBLISHED
submit_allowedActions=UNPUBLISH
```

管理员审核队列检查：

```text
queue_contains_post=False
```

结论：

- 云端 AI 自动审核已生效。
- 安全学习帖提交审核后直接发布。
- 自动发布后的帖子不会出现在管理员人工审核队列。

## 若后续帖子未自动通过

优先检查：

1. 帖子内容是否使模型返回低置信或 `NEEDS_HUMAN`。
2. 用户是否已达到已发布帖子数量上限。
3. `POST_MODERATION_ENABLED` 是否仍为 `true`。
4. `POST_MODERATION_PROVIDER` 是否为 `openai-compatible`。
5. `POST_MODERATION_BASE_URL` 是否为完整 `/chat/completions` 地址。
6. `sudo journalctl -u campus-buddy-backend -n 100 --no-pager` 是否有 provider 调用异常。
