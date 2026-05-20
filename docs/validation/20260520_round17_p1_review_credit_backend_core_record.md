# Round 17 Validation Record — P1-1 评价与信用摘要 Batch 1 后端核心

**日期**: 2026-05-20
**轮次**: Round 17
**目标**: 实现 P1-1 评价与信用摘要 Batch 1 后端核心 (POST/PUT /api/me/reviews, GET /api/me/credit-summary, GET /api/users/{userId}/credit-summary)

## 完成事项

### 新增文件
| 文件 | 说明 |
|------|------|
| `V7__create_review_table.sql` | Flyway 迁移：review 表 + 约束 + 索引 |
| `Review.java` | JPA 实体 |
| `ReviewRepository.java` | JPA Repository |
| `ReviewService.java` | 评价提交/修改业务逻辑 |
| `CreditSummaryService.java` | 信用摘要计算（虚拟基线、默认4星、标签聚合） |
| `ReviewController.java` | POST/PUT /api/me/reviews |
| `CreditSummaryController.java` | GET /api/me/credit-summary + /api/users/{userId}/credit-summary |
| `ReviewEndpointTest.java` | 13 个评价接口端点测试 |
| `CreditSummaryServiceTest.java` | 7 个信用摘要服务层测试 |

### 修改文件
| 文件 | 变更 |
|------|------|
| `SecurityConfiguration.java` | 添加 /api/me/reviews/**, /api/me/credit-summary, /api/users/*/credit-summary 认证路径 |

### 关键修复
1. **ReviewEndpointTest JWT 认证修复**: 原测试使用假 token `"test-jwt-for-" + userId`，无法通过 JwtAuthenticationFilter 验证。改为使用 `registerAndLogin` + `CapturingCampusEmailVerificationCodeSender` 模式获取真实 JWT access token。
2. **CreditSummaryServiceTest 精度修复**: Service 使用 `Math.round(averageRating * 10) / 10.0` 做一位小数四舍五入，测试期望值从精确浮点改为四舍五入后值（3.6, 3.7, 3.3）。

### 补充测试用例（3个）
- `unverifiedUserSubmitReviewReturnsAuthStatusRequired` — UNVERIFIED 用户提交评价返回 403 AUTHENTICATION_STATUS_REQUIRED
- `unauthenticatedSubmitReviewReturns401` — 未登录提交返回 401
- `otherUserModifyReviewReturnsNotFound` — 非评价者本人修改返回 404 REVIEW_NOT_FOUND

## 测试记录

### 本轮新增测试
| 测试类 | 通过 | 失败 | 错误 |
|--------|------|------|------|
| ReviewEndpointTest | 13 | 0 | 0 |
| CreditSummaryServiceTest | 7 | 0 | 0 |
| **合计** | **20** | **0** | **0** |

### 全量回归
| 指标 | 值 |
|------|-----|
| 总测试数 | 109 |
| 通过 | 109 |
| 失败 | 0 |
| 错误 | 0 |
| BUILD | SUCCESS |

## 未实现项（本轮范围外）
- 评价列表查询 (GET /api/users/{userId}/reviews)
- 争议流程
- 投诉申诉
- 管理端治理
- 通知推送
- Qt UI
- 超 24 小时修改测试（需 Clock 注入，待后续）
- 正常对话修改为 6 星校验测试（待后续）

## 下轮候选
1. **评价列表查询 API** (GET /api/users/{userId}/reviews) — 新开线程
2. **CreditSummaryController 端点测试** (GET /api/me/credit-summary, GET /api/users/{userId}/credit-summary) — 复用或新开
3. **Qt 评价提交 UI** — 新开线程
