# Round 28 — PartnerPost 学生草稿后端 API 验证记录

**日期**: 2026-05-23
**轮次**: Round 28
**目标**: 实现 PartnerPost 学生草稿 CRUD 后端 API（POST/PUT/GET/GET 分页）

---

## 1. 实现内容

### 1.1 Flyway 迁移
- `V8__create_partner_post_table.sql`
  - `partner_post` 表：UUID id, publisher_id, scene_type, status, title, description, time_mode, time_text, start_at, end_at, location_text, participant_count, target_requirement, contact_preference, tags (JSONB), attachment_ids (JSONB), scene_payload (JSONB), reject_reason, reviewed_by, reviewed_at, published_at, created_at, updated_at
  - 索引：idx_partner_post_publisher(publisher_id), idx_partner_post_status(status)

### 1.2 实体 & Repository
- `PartnerPost.java` — JPA 实体，JSONB 字段用 `@Column(columnDefinition = "jsonb")` + `@Type(JsonBinaryType)` 存储
- `PartnerPostRepository.java` — findByPublisherId, findByPublisherIdAndStatus, findByIdAndPublisherId

### 1.3 Service
- `PartnerPostService.java`
  - createDraft: 创建 status=DRAFT 的草稿，设置 allowedActions
  - updateDraft: 校验 status 必须为 DRAFT（否则 409），部分更新
  - listMyPosts: 分页查询当前用户的帖子，默认按 createdAt DESC
  - getMyPost: 查询单个帖子并校验归属
  - DraftFields 接口：定义可写入草稿的字段

### 1.4 Controller
- `PartnerPostController.java`
  - POST /api/me/partner-posts — 创建草稿
  - PUT /api/me/partner-posts/{postId} — 更新草稿
  - GET /api/me/partner-posts — 列表（分页，默认 page=0, size=20）
  - GET /api/me/partner-posts/{postId} — 详情

### 1.5 Security
- `SecurityConfiguration.java` 添加 `/api/me/partner-posts/**` → authenticated

---

## 2. 本地测试结果

```
Tests run: 163, Failures: 0, Errors: 0, Skipped: 0
BUILD SUCCESS (01:19 min)
```

### PartnerPostStudentDraftEndpointTest (10 个测试)

| # | 测试 | 结果 |
|---|------|------|
| 1 | 未认证 POST → 401 | PASS |
| 2 | 未认证 GET → 401 | PASS |
| 3 | 创建草稿 → 200 + status=DRAFT | PASS |
| 4 | 创建草稿验证字段 | PASS |
| 5 | 更新草稿 → 200 | PASS |
| 6 | 更新他人草稿 → 404 | PASS |
| 7 | 更新非DRAFT状态 → 409 | PASS |
| 8 | 列表只返回当前用户 | PASS |
| 9 | 详情包含 allowedActions | PASS |
| 10 | 详情排除敏感字段 | PASS |

### 回归测试
- 全部 163 个测试通过，无回归问题

---

## 3. 服务器部署

### 3.1 部署步骤
1. `mvn package -DskipTests` 构建 jar（71MB）
2. scp 上传至 `/srv/campus-buddy/campus-buddy-backend-0.0.1-SNAPSHOT.jar`
3. `systemctl restart campus-buddy-backend`

### 3.2 Flyway V8 迁移
```
Migrating schema "public" to version "8 - create partner post table"
Successfully applied 1 migration to schema "public", now at version v8
```

### 3.3 服务状态
```
Active: active (running)
Flyway: validated 8 migrations, schema at v8
```

---

## 4. 服务器 Smoke Test

使用 smoketest@campus.edu.cn 认证。

| # | 操作 | 方法 | 路径 | 状态码 | 验证点 |
|---|------|------|------|--------|--------|
| 1 | 创建草稿 | POST | /api/me/partner-posts | 200 | postId 存在, status=DRAFT, allowedActions=[EDIT] |
| 2 | 列表 | GET | /api/me/partner-posts | 200 | items=1, totalElements=1 |
| 3 | 详情 | GET | /api/me/partner-posts/{id} | 200 | 全部字段正确 |
| 4 | 更新草稿 | PUT | /api/me/partner-posts/{id} | 200 | title/description/tags 已更新, updatedAt 变化 |
| 5 | 未认证 | GET | /api/me/partner-posts (无效token) | 401 | 拒绝访问 |

---

## 5. 结论

- PartnerPost 学生草稿 CRUD 后端 API 实现完成
- 163/163 本地测试通过
- 服务器 Flyway V8 迁移成功
- 5/5 服务器 smoke test 通过
- 无回归问题
