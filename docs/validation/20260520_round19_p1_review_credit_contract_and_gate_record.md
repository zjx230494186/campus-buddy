# Round 19 Validation Record — P1-1 信用摘要契约收束与门禁复核

**日期**: 2026-05-20
**轮次**: Round 19
**目标**: 收束公开/本人信用摘要 DTO 差异 + 服务器 smoke test 门禁复核

## 问题摘要

GET /api/users/{userId}/credit-summary 返回与本人接口相同的 `CreditSummaryResponse`，包含 `disputedReviewCount` 字段。公开信用卡片不应暴露争议/治理相关扩展字段。

## 红灯测试命令与失败摘要

```bash
mvn test -Dtest="CreditSummaryControllerTest#publicCreditSummaryDoesNotExposeDisputedReviewCount,CreditSummaryControllerTest#myCreditSummaryIncludesDisputedReviewCount"
```

- `publicCreditSummaryDoesNotExposeDisputedReviewCount`: 失败，响应中包含 `disputedReviewCount` **(红灯)**
- `myCreditSummaryIncludesDisputedReviewCount`: 通过（确认本人接口保留该字段）

## 修复摘要

### CreditSummaryService.java
- 新增 `PublicCreditSummaryResponse` record（不含 `disputedReviewCount`）
- 新增 `toPublicResponse(CreditSummaryResponse)` 转换方法

### CreditSummaryController.java
- 公开接口返回类型改为 `PublicCreditSummaryResponse`
- 调用 `creditSummaryService.toPublicResponse()` 做转换
- 本人接口保持返回 `CreditSummaryResponse`（含 `disputedReviewCount`）

### CreditSummaryControllerTest.java
- 新增 `publicCreditSummaryDoesNotExposeDisputedReviewCount` 测试
- 新增 `myCreditSummaryIncludesDisputedReviewCount` 测试

## 绿灯测试命令与结果

```bash
mvn test -Dtest="CreditSummaryControllerTest,CreditSummaryServiceTest,ReviewEndpointTest"
```

- Tests run: 31, Failures: 0, Errors: 0

## Contact 依赖回归结果

Contact 测试包含在 120/120 全量回归中，全部通过。

## 后端回归测试结果

| 指标 | 值 |
|------|-----|
| 总测试数 | 120 |
| 通过 | 120 |
| 失败 | 0 |
| 错误 | 0 |
| BUILD | SUCCESS |

## V6/V7 服务器 smoke test 状态

- **未执行**。
- 未执行原因：当前不具备 Ubuntu 24 服务器访问权限，也未获用户授权部署。
- V6/V7 迁移已在本地自动化测试（含 Testcontainers PostgreSQL）中通过。
- P1-1 当前仍只达到**本地开发闭环**，未达到完整服务器门禁。
- 剩余风险：生产部署前需按 `docs/23_server_smoke_test_readiness_checklist_v1.md` 执行 Ubuntu 24 服务器 smoke test，确认迁移版本、健康检查和关键接口最小调用。

## 敏感信息检查结果

- 无 JWT secret、数据库密码、OBS AK/SK、SSH 私钥、真实 token、联系方式明文泄露。

## Git 变更范围

| 文件 | 变更类型 |
|------|---------|
| `CreditSummaryService.java` | 修改（+PublicCreditSummaryResponse record + toPublicResponse） |
| `CreditSummaryController.java` | 修改（公开接口返回 PublicCreditSummaryResponse） |
| `CreditSummaryControllerTest.java` | 修改（+2 测试） |

## 未完成事项

1. Ubuntu 24 服务器 smoke test — 需待服务器就绪和用户授权后执行。
2. 评价列表查询 API — 后续新功能批次。
3. Qt 评价 UI — 后续新功能批次。
