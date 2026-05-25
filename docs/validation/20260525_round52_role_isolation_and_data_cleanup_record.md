# Round 52 Validation Record — 角色隔离 UI 与切换账号数据清理

日期：2026-05-25

## 1. 本轮目标

- 后端登录响应增加 `accountRole` 字段，使客户端可感知用户角色。
- Qt 客户端根据角色动态构建 tab：STUDENT 只显示学生功能，ADMIN 只显示审核功能。
- 退出登录时清空所有子 widget 缓存数据，防止切换账号时数据泄漏。

## 2. 代码变更清单

### 2.1 后端

| 文件 | 变更类型 | 说明 |
|------|----------|------|
| `backend/src/main/java/com/campusbuddy/auth/AuthLoginService.java` | 修改 | `AuthenticatedUserResponse` 新增 `accountRole` 字段，构造时传入 `account.getAccountRole()` |

### 2.2 Qt 桌面端

| 文件 | 变更类型 | 说明 |
|------|----------|------|
| `desktop/src/auth/AuthApiService.h` | 修改 | `AuthResult` 新增 `accountRole`、`displayName` 字段 |
| `desktop/src/auth/AuthApiService.cpp` | 修改 | `login()` 解析 `user.accountRole`、`user.displayName`、`user.authenticationStatus` |
| `desktop/src/ui/LoginWidget.h` | 修改 | `loginSuccess` 信号增加 `accountRole` 参数 |
| `desktop/src/ui/LoginWidget.cpp` | 修改 | `onLoginClicked` 传递 `result.accountRole` |
| `desktop/src/ui/HomePageWidget.h` | 修改 | 新增 `setupTabsForRole()`、`clearAllData()`、`welcomeLabel_`、`currentRole_` |
| `desktop/src/ui/HomePageWidget.cpp` | 重写 | 根据 `accountRole` 动态构建 tab（STUDENT: 认证/发布/我的发布/广场/会话/评价信用；ADMIN: 发布审核/认证审核）；退出时清空所有子 widget |
| `desktop/src/ui/ConversationsWidget.h` | 修改 | `onRefreshConversations()` 改为 public slots |
| `desktop/src/ui/AdminReviewWidget.h` | 修改 | `onRefreshPostQueue()` 改为 public slots |
| `desktop/src/ui/MyPostsWidget.h` | 修改 | `onRefresh()` 改为 public slots |
| `desktop/src/ui/PlazaWidget.h` | 修改 | `onRefresh()` 改为 public slots |
| `desktop/src/main.cpp` | 修改 | `loginSuccess` 连接传递 `accountRole` 并调用 `setupTabsForRole` |

## 3. 测试结果

### 3.1 后端

```
249/249 PASS
BUILD SUCCESS
```

### 3.2 登录响应验证

- 学生登录：`user.accountRole = "STUDENT"` ✓
- 管理员登录：`user.accountRole = "ADMIN"` ✓

### 3.3 Qt ctest

```
10/10 PASS
```

### 3.4 Qt server integration smoke

```
38/38 PASS
```

## 4. 角色隔离方案

### STUDENT 角色可见 tab

1. 认证（身份认证资料提交 + 查询认证状态）
2. 发布草稿
3. 我的发布
4. 广场
5. 会话
6. 评价信用

### ADMIN 角色可见 tab

1. 发布审核（PartnerPost 审核队列 + 通过/驳回）
2. 认证审核（IdentityVerification 审核入口）

### 退出登录数据清理

调用 `clearAllData()` 清空：
- `PostEditorWidget::clearForm()`
- `MyPostsWidget::onRefresh()`
- `PlazaWidget::onRefresh()`
- `ConversationsWidget::onRefreshConversations()`
- `AdminReviewWidget::onRefreshPostQueue()`
- 各状态标签清空

## 5. 边界确认

- 后端仅新增 `accountRole` 字段到登录响应，无 Flyway 迁移、无数据库变更、无 API 路径变更。
- 后端 249/249 回归通过。
- Qt 构建通过，ctest 10/10 通过，server smoke 38/38 通过。
- 无敏感信息泄露。

## 6. 残余风险

- `clearAllData()` 中的 `onRefresh` 系列调用会发起 API 请求（此时 token 已清），可能产生网络错误日志；后续可改为本地内存清空方法。
- 认证审核 tab 中未单独实现身份认证审核队列 UI（AdminReviewWidget 的 innerTab_ 有 identity queue，但 ADMIN 角色下 tab 名称改为"认证审核"即可复用）。
- 公网仍为 HTTP。
