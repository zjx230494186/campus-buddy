# Round 31 — PartnerPost 广场发现后端验证记录

**日期**: 2026-05-23
**轮次**: Round 31
**目标**: 实现已发布 PartnerPost 的学生侧发现入口（列表+详情）

---

## 1. 实现内容

### 1.1 新增端点

- `GET /api/partner-posts` — PUBLISHED 分页列表（publishedAt DESC）
  - sceneType 精确筛选
  - keyword 大小写不敏感模糊搜索（title/description/locationText/tags）
  - size 上限 50
- `GET /api/partner-posts/{postId}` — PUBLISHED 详情
  - 发布者摘要：publisherId, publisherDisplayName, publisherAuthenticationStatus
  - 公开信用摘要（无 disputedReviewCount）
  - ownPost 标识

### 1.2 不返回的敏感字段
campusEmail, studentNumber, realName, objectKey, contactPreference, rejectReason, reviewedBy

### 1.3 SecurityConfiguration
新增 `/api/partner-posts/**` → authenticated

### 1.4 无新增 Flyway 迁移

---

## 2. 红灯测试

```bash
mvn test -Dtest=PartnerPostPlazaEndpointTest -pl .
```
```
Tests run: 12, Failures: 10, Errors: 0, Skipped: 0
BUILD FAILURE
```
10 失败（端点 404 + 认证不符合预期），2 通过（permitAll 阶段暂时通过，后续 SecurityConfig 修复后变为红灯）。

---

## 3. 绿灯测试

### PartnerPostPlazaEndpointTest (12 个测试)

| # | 测试 | 结果 |
|---|------|------|
| 1 | plazaListRequiresAuthentication → 401 | PASS |
| 2 | plazaDetailRequiresAuthentication → 401 | PASS |
| 3 | unverifiedUserCanViewPlazaList | PASS |
| 4 | plazaListOnlyReturnsPublished | PASS |
| 5 | plazaListSortedByPublishedAtDesc | PASS |
| 6 | plazaListFilterBySceneType | PASS |
| 7 | plazaListKeywordSearch | PASS |
| 8 | plazaDetailOnlyAllowsPublished → 404 | PASS |
| 9 | plazaDetailNonPublishedReturnsPostNotFound → 404 | PASS |
| 10 | plazaListAndDetailExcludeSensitiveFields | PASS |
| 11 | publicCreditSummaryExcludesDisputedCount | PASS |
| 12 | ownPostTrueForOwnPublishedPost | PASS |

---

## 4. 全量回归

```
Tests run: 202, Failures: 0, Errors: 0, Skipped: 0
BUILD SUCCESS (01:57 min)
```

---

## 5. 服务器部署

- 构建上传 jar，systemctl restart → active(running)
- Health: `{"status":"UP"}`
- Flyway: validated 8 migrations, schema at v8

---

## 6. 服务器 Smoke Test

| # | 操作 | 状态码 | 验证点 |
|---|------|--------|--------|
| 1 | GET /api/partner-posts (已认证) | 200 | items含PUBLISHED, ownPost=true, 无contactPreference |
| 2 | GET /api/partner-posts/{id} (已认证) | 200 | 详情含publisherCreditSummary(无disputedReviewCount), 无contactPreference |
| 3 | GET /api/partner-posts (未认证) | 401 | 拒绝访问 |

---

## 7. 敏感字段检查
列表和详情均不包含: campusEmail, studentNumber, realName, objectKey, contactPreference, rejectReason, reviewedBy, disputedReviewCount
结论：通过

---

## 8. 实际修改文件

- `backend/src/main/java/com/campusbuddy/post/PartnerPostRepository.java` — 添加广场查询方法
- `backend/src/main/java/com/campusbuddy/post/PartnerPostPlazaService.java` — 新增
- `backend/src/main/java/com/campusbuddy/post/PartnerPostPlazaController.java` — 新增
- `backend/src/main/java/com/campusbuddy/security/SecurityConfiguration.java` — 添加 /api/partner-posts/** authenticated
- `backend/src/test/java/com/campusbuddy/post/PartnerPostPlazaEndpointTest.java` — 新增 12 个测试

---

## 9. 未覆盖风险

- keyword 搜索 tags 使用 CAST(tags AS string) 做 LIKE，JSONB 数组转字符串匹配可能漏命中（如标签含特殊字符）
- 推荐算法、热度排序、曝光统计未实现（本轮明确排除）
- 联系方式解锁逻辑未实现
- 场景筛选+关键词搜索的 JPQL 可能对大数据集性能不佳，生产环境可能需要全文索引

---

## 10. 下一轮建议

P0-4 低压力联系后端：基于已发布需求发起联系、站内短文本消息、轮询、未读数、联系方式卡片和解锁。
