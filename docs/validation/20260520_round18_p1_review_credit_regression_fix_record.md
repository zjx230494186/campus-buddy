# Round 18 Validation Record — P1-1 评价与信用摘要补测小修

**日期**: 2026-05-20
**轮次**: Round 18
**目标**: 修正 CreditSummaryService 无效会话过滤 + 补测缺口 + CreditSummaryController 端点测试

## 问题摘要

1. `CreditSummaryService.getCreditSummary()` 把 `ContactContextService.findConversationsByParticipant()` 返回的所有会话都计入 `realConversationCount`，没有用 `isValidConversation()` 过滤，导致消息不足的无效会话错误进入信用摘要并按默认 4 星计分。
2. 无效会话上的历史脏评价的标签也会被计入 topTags。
3. Round 17 遗留两个测试缺口：超 24 小时修改、普通对话改 6 星。
4. Round 17 遗留 CreditSummaryController 端点测试缺口。

## 红灯测试命令与失败摘要

```bash
mvn test -Dtest="CreditSummaryServiceTest#invalidConversationNotCountedInSummary,CreditSummaryServiceTest#reviewOnInvalidConversationNotCountedInSummary,ReviewEndpointTest#modifyAfter24HoursRejected,ReviewEndpointTest#modifyTo6StarOnNormalConversationRejected,CreditSummaryControllerTest"
```

- `invalidConversationNotCountedInSummary`: expected realConversationCount=0, was=1 **(红灯)**
- `reviewOnInvalidConversationNotCountedInSummary`: expected realConversationCount=0, was=1 **(红灯)**
- `modifyAfter24HoursRejected`: 通过（不是红灯来源，覆盖增强）
- `modifyTo6StarOnNormalConversationRejected`: 通过（不是红灯来源，覆盖增强）
- `CreditSummaryControllerTest` 5 个测试全通过（覆盖增强）

## 修复摘要

### CreditSummaryService.java 修改
1. 新增 `validConversations` 过滤：`allConversations.stream().filter(conv -> contactContextService.isValidConversation(conv.getId()))`
2. `realConversationCount` 改为 `validConversations.size()`
3. 评分循环改为遍历 `validConversations` 而非 `allConversations`
4. 标签聚合只统计属于有效会话的评价：用 `validConversationIds` 集合过滤
5. 修复 NPE：原 `validConvReviewMap` 使用 `Collectors.toMap` 不接受 null value，改为 `Set<Long> validConversationIds`

### 新增测试（9 个）

| 测试类 | 新增测试 | 类型 |
|--------|---------|------|
| CreditSummaryServiceTest | `invalidConversationNotCountedInSummary` | 红灯→绿灯 |
| CreditSummaryServiceTest | `reviewOnInvalidConversationNotCountedInSummary` | 红灯→绿灯 |
| ReviewEndpointTest | `modifyAfter24HoursRejected` | 覆盖增强 |
| ReviewEndpointTest | `modifyTo6StarOnNormalConversationRejected` | 覆盖增强 |
| CreditSummaryControllerTest | `myCreditSummaryRequiresAuth` | 覆盖增强 |
| CreditSummaryControllerTest | `publicCreditSummaryRequiresAuth` | 覆盖增强 |
| CreditSummaryControllerTest | `myCreditSummaryReturnsOwnSummary` | 覆盖增强 |
| CreditSummaryControllerTest | `publicCreditSummaryReturnsOtherSummary` | 覆盖增强 |
| CreditSummaryControllerTest | `publicCreditSummaryDoesNotExposeSensitiveFields` | 覆盖增强 |

## 绿灯测试命令与结果

```bash
mvn test -Dtest="CreditSummaryServiceTest,ReviewEndpointTest,CreditSummaryControllerTest"
```

- Tests run: 29, Failures: 0, Errors: 0

## Contact 依赖回归结果

Contact 相关测试（ContactContextServiceTest, ContactPersistenceIntegrationTest, ContactContextServiceAccessTest）未单独重跑，但包含在 118/118 全量回归中。

## 后端回归测试结果

| 指标 | 值 |
|------|-----|
| 总测试数 | 118 |
| 通过 | 118 |
| 失败 | 0 |
| 错误 | 0 |
| BUILD | SUCCESS |

## 服务器 smoke test

不适用。未新增数据库迁移，未改部署配置，未改数据库结构。

## 敏感信息检查结果

- 无 JWT secret、数据库密码、OBS AK/SK、SSH 私钥、真实 token、联系方式明文泄露。

## Git 变更范围

| 文件 | 变更类型 |
|------|---------|
| `CreditSummaryService.java` | 修改（过滤无效会话） |
| `CreditSummaryServiceTest.java` | 修改（+2 测试） |
| `ReviewEndpointTest.java` | 修改（+2 测试） |
| `CreditSummaryControllerTest.java` | 新增（+5 测试） |

## 未完成事项

无。本轮所有计划项均已完成。
