# 31 AI 帖子自动预审设计 V1

## 1. 目标与边界

本设计用于降低管理员人工审核 `PartnerPost` 的工作量。V1 只处理帖子文本预审，不处理身份认证材料、聊天内容、评价内容、附件图片或 PDF 内容。

推荐结论：采用“AI 自动预审 + 人工兜底”，但 V1 不修改数据库。AI 结果只在本次提交审核流程内使用：高置信通过、驳回或保留人工审核。

### V1 目标

1. 学生帖子进入 `PENDING_REVIEW` 后，系统可调用 AI 生成预审结论。
2. 明显安全的帖子可自动发布为 `PUBLISHED`。
3. 明显违规的帖子可自动驳回为 `REJECTED`，并写入用户可读的 `rejectReason`。
4. 低置信度、模型异常、解析异常、策略不确定的帖子保持 `PENDING_REVIEW`，继续进入管理员人工队列。
5. 不新增表、不新增字段、不新增 Flyway 迁移。

### V1 明确不做

1. 不把身份认证材料、学号、真实姓名、邮箱、对象存储地址发送给大模型。
2. 不让客户端直接调用大模型 API。
3. 不把大模型 API key 写入仓库、文档、客户端或日志。
4. 不审核附件内容，只发送 `hasAttachments=true/false`。
5. 不改变现有管理员手动审核接口的语义。
6. 不引入新的帖子主状态，例如 `AI_REVIEWING`。
7. 不持久化完整 prompt、模型原始响应、风险类别或置信度。
8. 不在管理员页面展示 AI 建议；如果需要展示，放到 V2 另行设计。

## 2. 现有后端基线

现有帖子审核主链路如下：

1. `PartnerPostService.submitReview(...)` 将帖子从 `DRAFT` 改为 `PENDING_REVIEW`。
2. `PartnerPostAdminService.reviewQueue(...)` 查询 `PENDING_REVIEW` 队列。
3. `PartnerPostAdminService.reviewPost(...)` 只允许管理员审核 `PENDING_REVIEW` 帖子。
4. 管理员通过时改为 `PUBLISHED`，驳回时改为 `REJECTED` 并写入 `rejectReason`。
5. 通过时仍需保留每个发布者最多 10 个 `PUBLISHED` 帖子的限制。

V1 自动预审应复用现有发布/驳回规则，避免绕过 `PUBLISHED_POST_LIMIT_EXCEEDED`、`POST_STATUS_CONFLICT` 等既有约束。

## 3. 审核策略

### 3.1 输入字段

发送给大模型的字段只允许来自 `partner_post` 的非敏感业务字段：

| 字段 | 是否发送 | 说明 |
|---|---:|---|
| `postId` | 是 | 只作为引用 ID，不含个人隐私 |
| `sceneType` | 是 | 场景类型 |
| `title` | 是 | 标题 |
| `description` | 是 | 描述 |
| `timeMode` | 是 | 时间模式 |
| `timeText` | 是 | 文本时间偏好 |
| `startAt/endAt` | 是 | 时间范围 |
| `locationText` | 是 | 公开地点描述 |
| `participantCount` | 是 | 人数 |
| `targetRequirement` | 是 | 对搭子的要求 |
| `contactPreference` | 是 | 联系偏好，可能包含违规联系方式 |
| `tags` | 是 | 标签 |
| `scenePayload` | 是 | 场景补充字段 |
| `attachmentIds` | 否 | V1 只发送 `hasAttachments=true/false` |
| `publisherId` | 否 | 不发送 |
| `publisherDisplayName` | 否 | 不发送 |
| `campusEmail/studentNumber/realName` | 否 | 严禁发送 |

### 3.2 风险类别

模型必须按固定类别输出，可多选：

1. `DIRECT_CONTACT`: 公开手机号、微信号、QQ、邮箱、群号、二维码引导等直接联系方式。
2. `ILLEGAL_OR_DANGEROUS`: 违法、危险、作弊、代写、买卖证件、暴力、自伤等内容。
3. `HARASSMENT_OR_HATE`: 侮辱、骚扰、歧视、仇恨表达。
4. `SEXUAL_OR_ADULT`: 性暗示、成人交易、露骨内容。
5. `SCAM_OR_SPAM`: 广告、引流、诈骗、刷单、兼职骗局。
6. `OFF_PLATFORM_TRANSACTION`: 绕开平台交易、付费拉人、返现、押金等高风险线下交易。
7. `PRIVATE_INFO`: 公开他人隐私或要求提交敏感信息。
8. `LOW_QUALITY_OR_AMBIGUOUS`: 内容过短、语义不清、明显不完整。
9. `POLICY_UNCERTAIN`: 模型无法可靠判断。

### 3.3 决策枚举

模型输出的 `decision` 只能是：

1. `APPROVE`: 明显安全。
2. `REJECT`: 明显违规。
3. `NEEDS_HUMAN`: 不确定或需要人工判断。

后端最终执行策略：

| 模型决策 | 置信度 | 后端动作 |
|---|---:|---|
| `APPROVE` | `>= autoApproveThreshold` | 自动发布 |
| `REJECT` | `>= autoRejectThreshold` | 自动驳回 |
| `NEEDS_HUMAN` | 任意 | 保持待审 |
| 任意决策 | 低于阈值 | 保持待审 |
| 模型异常 | 不适用 | 保持待审 |
| JSON 解析失败 | 不适用 | 保持待审 |

推荐阈值：

1. `autoApproveThreshold=0.92`
2. `autoRejectThreshold=0.85`

原因：误通过的风险高于误驳回，自动通过阈值应更高；明显违规可以用较低阈值自动驳回，但仍需保留人工队列兜底。

### 3.4 后端硬规则优先级

后端硬规则优先于模型：

1. `PartnerPostService.validateForSubmission(...)` 仍负责必填、长度、场景字段和 11 位手机号检测。
2. `PUBLISHED_POST_LIMIT_EXCEEDED` 仍由后端检查，模型不能覆盖。
3. 只有当前仍为 `PENDING_REVIEW` 的帖子才能自动改状态。
4. 如果用户在 AI 调用期间撤回审核，AI 结果只能被丢弃，不能改帖子状态。

## 4. 状态流转

### 4.1 帖子主状态

V1 保持现有状态集合：

```text
DRAFT
PENDING_REVIEW
PUBLISHED
REJECTED
```

### 4.2 推荐同步流转

第一版建议采用同步触发、失败降级：

```text
学生提交审核
  -> 后端硬校验通过
  -> partner_post.status = PENDING_REVIEW
  -> 调用 PostModerationService.moderateAfterSubmit(postId)
      -> 调用 PostModerationClient
      -> 解析模型 JSON
      -> 若高置信 APPROVE: 尝试自动发布
      -> 若高置信 REJECT: 尝试自动驳回
      -> 否则保持 PENDING_REVIEW
  -> 返回当前帖子状态
```

同步方案优点是最小闭环清晰、测试容易；代价是提交审核接口会增加外部 API 延迟。若延迟不可接受，后续再改为后台任务。

### 4.3 人工兜底流转

管理员仍可使用现有接口审核：

```text
PENDING_REVIEW -> POST /api/admin/partner-posts/{postId}/review
```

如果 AI 已经自动发布或驳回，该帖子不再出现在 `review-queue`。如果 AI 保持待审，管理员仍按原流程处理。

## 5. 数据库设计

V1 不修改数据库。

### 5.1 复用现有字段

AI 自动动作只复用 `partner_post` 现有字段：

| 字段 | 用途 |
|---|---|
| `status` | 自动通过时改为 `PUBLISHED`，自动驳回时改为 `REJECTED`，不确定时保持 `PENDING_REVIEW` |
| `reviewedBy` | 自动通过/驳回时写入系统审核 UUID |
| `reviewedAt` | 自动通过/驳回时写入审核时间 |
| `publishedAt` | 自动通过时写入发布时间 |
| `rejectReason` | 自动驳回时写入用户可读原因 |
| `updatedAt` | 自动动作发生时更新 |

### 5.2 V1 取舍

优点：

1. 不需要 Flyway 迁移，部署风险低。
2. 不改变现有数据结构和本地 PostgreSQL 初始化脚本。
3. 不影响现有管理员审核接口和移动端接口。
4. 更适合课程演示和快速闭环。

代价：

1. 无法长期追溯 AI 的完整判断、风险类别和置信度。
2. `PENDING_REVIEW` 队列里无法展示 AI 建议。
3. 模型异常只能通过运行日志和请求 trace 排查，不能从数据库恢复。
4. 后续如果要做管理员可见的 AI 建议、重跑审核或模型版本对比，需要 V2 新增审计表。

### 5.3 V2 可选审计表

如果后续确实需要持久化 AI 审核轨迹，再单独设计 `partner_post_moderation_result` 表和 Flyway 迁移。该表不属于 V1 范围。

## 6. 环境变量与配置

新增配置前缀：`campus-buddy.post-moderation.*`

### 6.1 非敏感配置

可写入 `application-local.properties`、`application-test.properties`、`application-deploy.properties`：

```properties
campus-buddy.post-moderation.enabled=${POST_MODERATION_ENABLED:false}
campus-buddy.post-moderation.provider=${POST_MODERATION_PROVIDER:noop}
campus-buddy.post-moderation.model=${POST_MODERATION_MODEL:}
campus-buddy.post-moderation.base-url=${POST_MODERATION_BASE_URL:}
campus-buddy.post-moderation.timeout-millis=${POST_MODERATION_TIMEOUT_MILLIS:8000}
campus-buddy.post-moderation.auto-approve-threshold=${POST_MODERATION_AUTO_APPROVE_THRESHOLD:0.92}
campus-buddy.post-moderation.auto-reject-threshold=${POST_MODERATION_AUTO_REJECT_THRESHOLD:0.85}
campus-buddy.post-moderation.max-input-chars=${POST_MODERATION_MAX_INPUT_CHARS:2000}
```

### 6.2 敏感配置

只能放在服务器私有环境变量或私有配置文件，例如 `/etc/campus-buddy/backend.env`：

```text
POST_MODERATION_API_KEY=...
```

严禁写入：

1. Git 仓库。
2. 项目 Markdown 文档的真实值。
3. Qt 或 Android 客户端。
4. 日志。
5. 测试输出。

### 6.3 默认模式

默认 `enabled=false` 且 `provider=noop`。本地测试和 CI 不依赖外部网络，不需要真实 API key。

## 7. 后端模块规划

### 7.1 包结构

建议新增包：

```text
backend/src/main/java/com/campusbuddy/moderation
```

建议类：

1. `PostModerationService`: 预审编排和自动动作。
2. `PostModerationClient`: 大模型调用接口。
3. `NoopPostModerationClient`: 默认替身，返回 `NEEDS_HUMAN`。
4. `LlmPostModerationClient`: 真实 HTTP 调用实现。
5. `PostModerationProperties`: 配置绑定。

V1 不新增 `PartnerPostModerationResult` 实体或 Repository。

### 7.2 与现有审核服务的关系

推荐给 `PartnerPostAdminService` 抽取内部方法：

1. `approvePendingPost(UUID reviewerId, UUID postId, Instant now)`
2. `rejectPendingPost(UUID reviewerId, UUID postId, String reason, Instant now)`

人工审核和 AI 自动动作都复用同一批状态检查，避免逻辑分叉。

AI 自动动作的 `reviewedBy` 推荐使用固定系统 UUID：

```text
00000000-0000-0000-0000-000000000001
```

并在文档中约定该 UUID 表示系统自动审核，不对应真实管理员账号。后续如需要更严格审计，可新增专门系统账号或审计表。

## 8. 模型输出契约

后端只接受 JSON 对象。V1 解析后立即用于本次决策，不写入数据库：

```json
{
  "decision": "APPROVE",
  "confidence": 0.96,
  "riskCategories": [],
  "reason": "内容为正常学习搭子招募，未发现直接联系方式或高风险交易。",
  "userVisibleReason": null
}
```

字段约束：

| 字段 | 必填 | 约束 |
|---|---:|---|
| `decision` | 是 | `APPROVE/REJECT/NEEDS_HUMAN` |
| `confidence` | 是 | 0 到 1 |
| `riskCategories` | 是 | 固定风险类别数组，可为空 |
| `reason` | 是 | 1 到 500 字 |
| `userVisibleReason` | 否 | 自动驳回时必填，最多 200 字 |

解析策略：

1. 缺少必填字段：视为失败，保持待审。
2. 出现未知 `decision`：视为失败，保持待审。
3. 出现未知风险类别：整体降级为 `NEEDS_HUMAN`。
4. `REJECT` 但无 `userVisibleReason`：不得自动驳回，保持待审。

## 9. 测试用例规划

实现阶段必须测试先行，并先确认红灯。

### 9.1 Service 单元测试

测试文件建议：`backend/src/test/java/com/campusbuddy/moderation/PostModerationServiceTest.java`

用例：

1. `enabled=false` 时跳过预审，帖子保持 `PENDING_REVIEW`。
2. `APPROVE + 高置信度` 自动发布。
3. `REJECT + 高置信度 + userVisibleReason` 自动驳回，并写入 `rejectReason`。
4. `NEEDS_HUMAN` 保持 `PENDING_REVIEW`。
5. `APPROVE` 但低于阈值，保持 `PENDING_REVIEW`。
6. `REJECT` 但缺少 `userVisibleReason`，保持 `PENDING_REVIEW`。
7. 模型调用抛异常时不影响提交审核主流程，帖子保持 `PENDING_REVIEW`。
8. 模型返回非法 JSON 时不影响提交审核主流程，帖子保持 `PENDING_REVIEW`。
9. 用户在模型返回前撤回审核时，不得改为发布或驳回。
10. 发布数已达 10 个时，即使模型建议通过，也不绕过既有限制。

### 9.2 接口集成测试

测试文件建议：`backend/src/test/java/com/campusbuddy/post/PartnerPostAiModerationEndpointTest.java`

用例：

1. 提交正常学习帖子后，替身模型返回高置信 `APPROVE`，接口响应状态为 `PUBLISHED`。
2. 提交含明显联系方式的帖子后，替身模型返回高置信 `REJECT`，接口响应状态为 `REJECTED`，`rejectReason` 存在。
3. 替身模型返回 `NEEDS_HUMAN` 时，帖子仍出现在管理员 `review-queue`。
4. 模型异常时，`submit-review` 仍返回成功，帖子仍为 `PENDING_REVIEW`。
5. 非认证用户、未认证用户、非本人帖子仍按现有规则失败，不触发外部模型调用。

### 9.3 配置与安全测试

测试文件建议：

1. `backend/src/test/java/com/campusbuddy/config/PostModerationConfigurationTest.java`
2. `backend/src/test/java/com/campusbuddy/deploy/DeployScriptSecurityTest.java` 增补断言

用例：

1. 默认 `provider=noop`，不需要 API key。
2. `enabled=true` 且真实 provider 缺少 API key 时调用失败方式明确，不泄露 secret。
3. systemd 和 deploy 脚本不得在命令行直接出现 `POST_MODERATION_API_KEY` 的值。
4. 日志和错误响应不得包含 API key。

### 9.4 管理员页面兼容测试

用例：

1. 自动通过后的帖子不出现在待审队列。
2. 自动驳回后的帖子不出现在待审队列。
3. 需要人工的帖子仍出现在待审队列。
4. 管理员仍可手动审核 `PENDING_REVIEW` 帖子。

## 10. 实现批次建议

### Batch 1: 配置与本地替身

范围：

1. 新增 `PostModerationClient` 和 `NoopPostModerationClient`。
2. 新增 `PostModerationProperties`。
3. 新增配置绑定与默认 `enabled=false/provider=noop`。
4. 不新增 Flyway、不新增表、不新增 JPA 实体。

验证：

1. 配置测试红灯到绿灯。
2. Noop client 测试通过。

### Batch 2: 预审编排与自动动作

范围：

1. 新增 `PostModerationService`。
2. `submitReview` 后调用预审服务。
3. 高置信通过/驳回自动改状态。
4. 异常保持待审。

验证：

1. Service 单元测试。
2. 接口集成测试。
3. 现有 `PartnerPostAdminReviewEndpointTest` 回归。

### Batch 3: 真实大模型 HTTP 客户端

范围：

1. 新增真实 HTTP 客户端。
2. 增加超时、解析、脱敏日志和错误降级。
3. 只通过环境变量启用。

验证：

1. 不使用真实 API key 的测试仍可通过。
2. 私有环境变量可用时进行人工 smoke。

### Batch 4: V2 审计与管理员展示评估

范围：

1. 评估是否需要新增数据库审计表。
2. 若需要，再单独设计 Flyway、实体、接口与管理员展示。

验证：

1. 本批不在 V1 中实施。

## 11. 验证与留档

实现线程每个 batch 都必须新增验证记录，建议路径：

```text
docs/validation/YYYYMMDD_ai_post_moderation_batchN_record.md
```

记录至少包含：

1. 红灯测试命令与失败原因。
2. 实现摘要。
3. 绿灯测试命令与结果。
4. 回归测试范围。
5. 是否调用真实大模型 API。
6. 敏感信息检查结果。
7. 未覆盖风险。

## 12. 当前推荐决策

推荐先按 Batch 1 和 Batch 2 做无数据库变更的最小闭环。真实大模型 API 客户端放到 Batch 3，等本地替身和状态流转全部稳定后再接入外部网络。若后续需要持久化 AI 预审轨迹，再新增 V2 数据库审计表，不并入 V1。

本设计仍保留一个需要用户最终拍板的问题：

推荐答案：V1 允许 AI 高置信自动通过和自动驳回，低置信进入人工队列，但不保存 AI 审核明细到数据库。

理由：这个方案能明显减少管理员工作量，同时避免数据库迁移和审计表设计带来的实现成本。代价是不能长期追溯 AI 的完整判断；如果后续需要复盘与监管，再做 V2 审计表。
