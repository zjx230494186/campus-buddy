# 27 Course Demo & Delivery Checklist V1

本文面向课程答辩和项目交付。所有内容可执行，不写泛泛介绍。

## 1. 当前可演示能力清单

| # | 能力 | 后端 | Qt 桌面端 | Server Smoke |
|---|------|------|-----------|-------------|
| 1 | 注册 / 登录 / JWT | V1 | 登录页 | Step 2 |
| 2 | 身份认证资料提交 | V2 | 认证提交页 | Step 4-5 |
| 3 | 管理员审核认证 | V2 | 管理员审核 Tab | Step 27 |
| 4 | 学生发布需求草稿 | V8 | 草稿编辑器 | Step 11 |
| 5 | 提交审核 / 撤回审核 | V8 | 我的发布列表 | Step 15-16 |
| 6 | 管理员审核发布（通过/驳回） | V8 | 管理员审核 Tab | Step 26 |
| 7 | 广场浏览需求 | V8 | 广场 Tab | Step 6 |
| 8 | 发起联系 → 创建会话 | V9 | 广场详情 → 会话 | Step 7 |
| 9 | 会话发送消息 | V9 | 会话 Tab | Step 9 |
| 10 | 未读数 / 标记已读 | V10 | 会话列表未读标记 | Step 29-30 |
| 11 | 关闭会话 / 重新发起 | V10 | 关闭/重开按钮 | Step 31-33 |
| 12 | 编辑联系方式卡片 | V11 | 联系方式编辑区 | Step 35 |
| 13 | 双方确认交换联系方式 | V11 | 确认交换按钮 | Step 37-38 |
| 14 | 查看对方联系方式 | V11 | 查看对方卡片按钮 | Step 38 |
| 15 | 提交评价 | V7 | 评价 Tab | Step 20 |
| 16 | 信用摘要 | V7 | 信用摘要 Tab | Step 19 |
| 17 | 6 星评价解锁联动 | V11 | 解锁后可提交 6 星 | 后端测试覆盖 |
| 18 | 对象存储上传 | V3 | 认证材料上传 | Step 4 |

后端 249/249 测试通过；Qt ctest 10/10；Server smoke 38/38。

## 2. 推荐演示角色与账号

| 角色 | 用途 | 账号来源 |
|------|------|----------|
| 学生 A | 发布需求、发起联系、提交评价 | smoke 测试学生账号（环境变量 `CAMPUS_BUDDY_SMOKE_EMAIL`） |
| 学生 B / 对方用户 | 接收联系、确认交换、被评价 | smoke 测试管理员账号（环境变量 `CAMPUS_BUDDY_SMOKE_ADMIN_EMAIL`） |
| 管理员 | 审核认证、审核发布 | 同学生 B 账号（兼具 ADMIN 角色） |

**不记录真实邮箱、密码或 token。** 演示前通过环境变量配置。

## 3. 推荐演示主线

### Step 1：登录
- 页面：Qt 登录页
- 验证点：输入凭据后获得 JWT，跳转到主页

### Step 2：身份认证资料提交
- 页面：Qt 认证提交页
- 验证点：上传材料后状态变为 PENDING_REVIEW

### Step 3：管理员审核认证
- 页面：Qt 管理员审核 Tab（身份认证区域）
- 验证点：管理员批准后学生状态变为 VERIFIED

### Step 4：学生发布需求草稿
- 页面：Qt 草稿编辑器
- 验证点：保存草稿成功，状态 DRAFT

### Step 5：提交审核
- 页面：Qt 我的发布列表 → 提交审核按钮
- 验证点：状态变为 PENDING_REVIEW

### Step 6：管理员审核发布
- 页面：Qt 管理员审核 Tab（发布审核区域）
- 验证点：管理员通过后状态变为 PUBLISHED

### Step 7：广场浏览需求
- 页面：Qt 广场 Tab
- 验证点：已发布需求出现在列表中

### Step 8：发起联系
- 页面：Qt 广场详情 → 发起联系
- 验证点：会话创建成功，跳转到会话 Tab

### Step 9：会话发送消息
- 页面：Qt 会话 Tab
- 验证点：双方各发送消息，消息实时显示在列表中

### Step 10：编辑联系方式卡片
- 页面：Qt 会话 Tab → 联系方式编辑区
- 验证点：保存微信/手机/QQ 后状态显示"已保存"

### Step 11：双方确认交换联系方式
- 页面：Qt 会话 Tab → 确认交换按钮
- 验证点：A 确认后状态变为"等待对方确认"；B 确认后状态变为"已解锁"

### Step 12：查看对方联系方式
- 页面：Qt 会话 Tab → 查看对方联系方式按钮
- 验证点：解锁后可看到对方的微信/手机/QQ 字段

### Step 13：满足有效会话后提交评价
- 页面：Qt 评价 Tab
- 验证点：对对方提交 5 星评价成功（解锁后可提交 6 星）

### Step 14：查看信用摘要变化
- 页面：Qt 信用摘要 Tab
- 验证点：评价后信用摘要更新（平均分、评价数、标签）

## 4. 演示前检查清单

- [ ] 后端 systemd 服务 active：`ssh root@114.116.203.78 systemctl status campus-buddy-backend`
- [ ] Health 200：`curl http://114.116.203.78/api/health` 返回 `{"status":"UP"}`
- [ ] Qt 构建可运行：`campus_buddy_desktop.exe` 启动无崩溃
- [ ] Server smoke 关键路径通过：运行 `qt_server_integration_smoke.exe`
- [ ] Smoke 账号环境变量已配置：`CAMPUS_BUDDY_SMOKE_EMAIL`、`CAMPUS_BUDDY_SMOKE_PASSWORD`、`CAMPUS_BUDDY_SMOKE_ADMIN_EMAIL`、`CAMPUS_BUDDY_SMOKE_ADMIN_PASSWORD`
- [ ] OBS 不需要在演示中展示密钥，只需说明"对象存储已接入华为云 OBS"

## 5. 答辩时应主动说明的边界

| 边界项 | 说明 |
|--------|------|
| 公网 HTTP | 当前公网仍为 HTTP，生产环境需要 HTTPS；可说明 Nginx 反代已就绪，加 TLS 证书即可升级 |
| 邮件发送 | 当前邮件发送为 Noop/日志策略，不真实投递；生产需要接入 SMTP 服务 |
| WebSocket 实时聊天 | 未实现；当前会话为刷新/轮询式，可手动刷新查看新消息 |
| 投诉申诉 / 治理 / 通知 | 属于后续扩展，后端 REST API 架构可复用 |
| 移动端 | 未实现，但后端 REST API 已就绪可直接复用 |
| OBS 凭据 | 使用服务器端私有 env 配置，仓库和文档不记录 AK/SK |
| 评价去重 | 每个会话每对用户只能提交一次评价，重复提交返回 REVIEW_ALREADY_EXISTS |

## 6. 故障应急说明

| 故障场景 | 诊断步骤 |
|----------|----------|
| 后端不通 | `curl http://114.116.203.78/api/health` → 检查 systemd 状态 → 检查 `/var/log/campus-buddy/` 日志 |
| Qt 连不上服务器 | 检查 Qt 启动时 API base URL 配置是否为 `http://114.116.203.78/api` |
| 登录失败 | 检查 smoke 环境变量是否正确设置；检查服务器数据库中账号是否存在且 VERIFIED |
| OBS 上传失败 | 不打印凭据；检查服务器 `/etc/campus-buddy/backend.env` 中 OBS 配置；检查 `journalctl -u campus-buddy-backend` 日志 |
| 评价提交失败 | 检查会话是否有足够 USER_TEXT 消息；检查是否已提交过评价（REVIEW_ALREADY_EXISTS） |

## 7. 交付物清单

| 交付物 | 位置 |
|--------|------|
| 后端代码 | `backend/` |
| Qt 桌面端代码 | `desktop/` |
| Flyway 迁移 | `backend/src/main/resources/db/migration/` (V1-V11) |
| Validation 记录 | `docs/validation/` |
| Prompts 记录 | `docs/prompts/codearts/` |
| Server deploy / OBS runbook | `deploy/` + `docs/` 中相关文档 |
| 演示脚本 | 本文档（`docs/27_course_demo_and_delivery_checklist_v1.md`） |
| 项目状态 | `docs/03_current_plan.md` + `handoff/latest.md` |
| 功能路线图 | `docs/26_remaining_function_completion_roadmap_v1.md` |

## 8. 残余风险清单

| 风险 | 影响 | 缓解 |
|------|------|------|
| 公网 HTTP | 传输明文 | 生产加 TLS |
| OBS 物理删除未独立复核 | 孤储可能泄漏 | 后续补删除 smoke |
| 无 WebSocket | 会话需手动刷新 | 说明为轮询策略 |
| 无邮件真实发送 | 注册/认证邮件不投递 | 说明为 Noop 策略 |
| 投诉/治理/通知未实现 | 无争议处理 | 后续扩展 |
| 移动端未实现 | 仅桌面端 | 后端 API 可复用 |
