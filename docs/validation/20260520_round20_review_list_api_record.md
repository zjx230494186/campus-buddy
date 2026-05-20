# Round 20 Validation Record — 评价列表查询 API 后端小批次

**日期**: 2026-05-20
**轮次**: Round 20
**目标**: 实现 GET /api/me/reviews/given 和 GET /api/me/reviews/received 分页评价列表查询

## 红灯测试命令与失败摘要

```bash
mvn test -Dtest="ReviewListEndpointTest"
```

- 8 个测试中 6 个失败（接口未实现，返回 404）
- 2 个通过（401 认证测试，Security 已配置 `/api/me/reviews/**` 路径）

## 实现摘要

### ReviewRepository.java
- 新增 `Page<Review> findByReviewerId(UUID reviewerId, Pageable pageable)`
- 新增 `Page<Review> findByRevieweeId(UUID revieweeId, Pageable pageable)`

### ReviewService.java
- 新增 `listGivenReviews(UUID reviewerId, int page, int size)` — 按 createdAt 降序分页查询发出评价
- 新增 `listReceivedReviews(UUID revieweeId, int page, int size)` — 按 createdAt 降序分页查询收到评价
- 新增 `ReviewListResponse` record（items, page, size, totalElements, totalPages）

### ReviewController.java
- 新增 `GET /api/me/reviews/given` — 分页返回当前用户发出的评价
- 新增 `GET /api/me/reviews/received` — 分页返回当前用户收到的评价
- 参数：page（默认 0）、size（默认 20）

### ReviewListEndpointTest.java（8 个测试）
- givenListRequiresAuth / receivedListRequiresAuth — 401
- givenListReturnsOnlyOwnReviews — 只返回自己发出的
- receivedListReturnsOnlyOwnReceivedReviews — 只返回自己收到的，别人收不到
- givenListOrderByCreatedAtDesc — 降序排列
- givenListSupportsPagination — 分页参数
- unverifiedUserCanQueryEmptyList — UNVERIFIED 也可查空列表
- responseDoesNotContainSensitiveFields — 不含邮箱/密码/认证材料

## 绿灯测试命令与结果

```bash
mvn test -Dtest="ReviewListEndpointTest,ReviewEndpointTest,CreditSummaryControllerTest,CreditSummaryServiceTest"
```

- Tests run: 39, Failures: 0, Errors: 0

## Review / CreditSummary 回归结果

全部包含在 128/128 全量回归中。

## Contact 依赖回归结果

全部包含在 128/128 全量回归中。

## 后端回归测试结果

| 指标 | 值 |
|------|-----|
| 总测试数 | 128 |
| 通过 | 128 |
| 失败 | 0 |
| BUILD | SUCCESS |

## 服务器 smoke test

不适用。未新增数据库迁移，未改部署配置。

## 敏感信息检查结果

- 无泄露。响应不含邮箱、密码哈希、认证材料、联系方式明文。

## Git 变更范围

| 文件 | 变更类型 |
|------|---------|
| `ReviewRepository.java` | 修改（+2 分页查询方法） |
| `ReviewService.java` | 修改（+2 列表方法 + ReviewListResponse DTO） |
| `ReviewController.java` | 修改（+2 GET 端点） |
| `ReviewListEndpointTest.java` | 新增（8 个测试） |

## 未完成事项

无。本轮所有计划项均已完成。
