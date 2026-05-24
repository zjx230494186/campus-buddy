# Round 51 Validation Record — 用户行为负向测试矩阵与错误处理修复

日期：2026-05-25

## 1. 本轮目标

- 系统性补齐负向用户行为测试矩阵文档。
- 修复 Qt 发帖提交审核返回 VALIDATION_FAILED 时 UI 只显示模糊错误的问题。
- 添加字段级错误详情展示。
- 添加场景字段动态 UI（MEAL→食堂, STUDY→学习目标, SPORT→运动类型, COURSE_TEAM→课程名称, INNOVATION_PROJECT→项目方向）。
- 添加提交审核按钮防重复提交。
- 新增 Qt 错误解析和错误 details 传递的自动化测试。

## 2. 代码变更清单

### 2.1 Qt 桌面端

| 文件 | 变更类型 | 说明 |
|------|----------|------|
| `desktop/src/domain/MyPartnerPostModels.h` | 修改 | MyPostResult/MyPostListResult/PostActionResult 新增 errorDetails(QJsonObject) + httpStatus(int) |
| `desktop/src/api/MyPartnerPostApiService.cpp` | 修改 | 所有错误分支传递 response.error.details 和 response.error.httpStatus |
| `desktop/src/ui/PostEditorWidget.h` | 修改 | 新增 sceneFieldEdit_/sceneFieldLabel_/submitting_/onSceneTypeChanged/formatErrorDetails/sceneFieldKey/sceneFieldLabel |
| `desktop/src/ui/PostEditorWidget.cpp` | 修改 | 场景字段动态 UI + formatErrorDetails + 防重复提交 + 分类错误提示 |
| `desktop/tests/CampusApiClientTest.cpp` | 修改 | 新增 5 个错误解析测试 |
| `desktop/tests/MyPartnerPostApiServiceTest.cpp` | 修改 | 新增 3 个错误 details 传递测试 |

### 2.2 文档

| 文件 | 变更类型 | 说明 |
|------|----------|------|
| `docs/29_negative_user_behavior_test_matrix_v1.md` | 新增 | 200+ 负向测试案例，覆盖 13 个模块 |

## 3. 测试结果

### 3.1 Qt ctest

```
ctest 10/10 PASS
```

新增测试用例清单：

**CampusApiClientTest.cpp（5 个）：**
1. `validationFailedWithDetailsParsesFieldErrors` — VALIDATION_FAILED + details 解析
2. `unauthorizedResponseParsesCodeAndMessage` — 401 错误解析
3. `networkConnectionRefusedSetsNetworkError` — 网络连接失败
4. `emptyBody4xxResponseParsesHttpError` — 空 body + 4xx → InvalidJson
5. `malformedJsonResponseSetsInvalidJsonError` — 非法 JSON + 500 → InvalidJson

**MyPartnerPostApiServiceTest.cpp（3 个）：**
1. `submitReviewValidationFailedPreservesDetails` — submitReview 错误保留 details + httpStatus
2. `submitReviewPostStatusConflictPreservesDetails` — 409 POST_STATUS_CONFLICT
3. `createDraftErrorPreservesHttpStatusAndDetails` — 403 IDENTITY_NOT_VERIFIED + details

### 3.2 Qt server integration smoke

```
qt_server_integration_smoke 38/38 PASS
```

### 3.3 后端回归

后端无变更，249/249 保持通过。

## 4. 负向测试矩阵摘要

`docs/29_negative_user_behavior_test_matrix_v1.md` 覆盖：

| 模块 | 案例数 |
|------|--------|
| 登录/注册 | 20+ |
| 身份认证 | 15+ |
| 发帖/提交审核 | 25+ |
| 会话消息 | 20+ |
| 联系方式/解锁 | 15+ |
| 评价信用 | 15+ |
| 管理员审核 | 15+ |
| 网络错误 | 15+ |
| UI 体验 | 15+ |
| 权限边界 | 15+ |
| 数据完整性 | 10+ |
| 并发/竞态 | 10+ |
| 其他 | 10+ |

## 5. 修复的核心问题链

1. **VALIDATION_FAILED 模糊提示** → PostEditorWidget 现在解析 details JSON，格式化字段级错误信息展示给用户。
2. **场景字段缺失** → PostEditorWidget 根据 sceneType 动态生成对应字段输入框（MEAL→食堂, STUDY→学习目标, SPORT→运动类型, COURSE_TEAM→课程名称, INNOVATION_PROJECT→项目方向）。
3. **Result struct 丢失 errorDetails** → MyPartnerPostModels.h 新增 errorDetails + httpStatus，MyPartnerPostApiService.cpp 所有错误分支传递。
4. **防重复提交** → onSubmitReview 增加 submitting_ 状态 + 按钮禁用。
5. **分类错误提示** → VALIDATION_FAILED → 字段级详情，POST_STATUS_CONFLICT → 状态冲突，401 → 登录过期，403 → 权限不足，httpStatus==0 → 网络连接失败。

## 6. 边界确认

- 未修改后端代码、Flyway 迁移或 deploy 配置。
- 未修改服务器配置。
- 未引入新的第三方依赖。
- Qt 构建、ctest、server smoke 全部通过。
- 无敏感信息泄露。

## 7. 残余风险

- 负向测试矩阵为文档级测试案例，尚未全部转化为自动化测试。
- Qt 默认 API base URL 仍为 localhost:8080，双击启动未设置环境变量时连接失败。
- 公网仍为 HTTP。
