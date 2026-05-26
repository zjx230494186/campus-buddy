# Plaza Requires Verified Server Fix Record

日期：2026-05-26

## 1. 问题

用户测试发现：新注册但尚未完成身份认证的账号，仍可访问广场帖子。

项目文档与既有发布/提交逻辑要求：广场帖子浏览也应要求用户身份认证状态为 `VERIFIED`。

## 2. 根因

`PartnerPostService` 的学生发布链路已调用 `requireVerified(userId)`，所以未认证用户不能创建、提交、撤回或下架帖子。

但 `PartnerPostPlazaService` 的广场列表和详情只使用登录用户 id 计算 `ownPost`，没有先校验当前访问者的 `authenticationStatus`，导致 `UNVERIFIED` 用户登录后可读取广场列表和详情。

同时旧测试中存在错误合同：

```text
unverifiedUserCanViewPlazaList
```

该测试与产品文档冲突，已改为禁止访问。

## 3. 修复内容

修改：

- `backend/src/main/java/com/campusbuddy/post/PartnerPostPlazaService.java`
  - `listPosts(...)` 入口增加 `requireVerified(currentUserId)`。
  - `getPostDetail(...)` 入口增加 `requireVerified(currentUserId)`。
  - 未认证时返回：
    - HTTP 403
    - `AUTHENTICATION_STATUS_REQUIRED`

测试：

- `backend/src/test/java/com/campusbuddy/post/PartnerPostPlazaServiceTest.java`
  - 新增 service 单测，不依赖 Docker/Testcontainers。
  - 覆盖未认证用户不能访问广场列表。
  - 覆盖未认证用户不能访问广场详情。
- `backend/src/test/java/com/campusbuddy/post/PartnerPostPlazaEndpointTest.java`
  - 将错误合同 `unverifiedUserCanViewPlazaList` 改为 `unverifiedUserCannotViewPlazaList`。
  - 新增 `unverifiedUserCannotViewPlazaDetail`。

## 4. 本地验证

红灯：

```text
.\mvnw.cmd "-Dtest=PartnerPostPlazaServiceTest" test

Tests run: 2, Failures: 2
Expecting code to raise a throwable.
```

绿灯：

```text
.\mvnw.cmd "-Dtest=PartnerPostPlazaServiceTest" test

Tests run: 2, Failures: 0, Errors: 0, Skipped: 0
```

相关轻量回归：

```text
.\mvnw.cmd "-Dtest=PartnerPostPlazaServiceTest,SmtpCampusEmailVerificationCodeSenderTest" test

Tests run: 4, Failures: 0, Errors: 0, Skipped: 0
```

构建：

```text
.\mvnw.cmd -DskipTests package

BUILD SUCCESS
```

说明：`PartnerPostPlazaEndpointTest` 仍依赖 Docker/Testcontainers；本机当前 Docker 不可用，因此未在本机跑完整 endpoint 集成测试。本轮用 service 单测和服务器公网 smoke 覆盖关键行为。

## 5. 服务器上线

已重新构建后端 jar 并部署到华为云服务器。

旧 jar 备份：

```text
/srv/campus-buddy/campus-buddy-backend-0.0.1-SNAPSHOT.jar.backup.20260526_234539
```

服务状态：

```text
systemd service=active
GET http://114.116.203.78/api/health
{"status":"UP"}
```

## 6. 服务器公网验证

为避免使用真实用户状态做破坏性验证，服务器数据库中创建/更新专用 smoke 账号：

```text
codex-unverified-smoke@bjtu.edu.cn
authenticationStatus=UNVERIFIED
campusEmailVerificationStatus=VERIFIED
accountRole=STUDENT
```

未认证账号访问广场列表：

```text
GET /api/partner-posts
HTTP 403
code=AUTHENTICATION_STATUS_REQUIRED
details=current: UNVERIFIED
```

已认证初始学生账号访问广场列表：

```text
GET /api/partner-posts?size=1
HTTP 200
```

未认证账号访问广场详情：

```text
GET /api/partner-posts/{postId}
HTTP 403
code=AUTHENTICATION_STATUS_REQUIRED
details=current: UNVERIFIED
```

已认证初始学生账号访问同一详情：

```text
GET /api/partner-posts/{postId}
HTTP 200
```

## 7. 边界与风险

- 本轮不修改 Flyway migration。
- 本轮不修改 Qt 客户端。
- 本轮不修改 deploy 脚本。
- 本轮没有写入真实用户密码、验证码、token、SMTP 授权码、数据库密码、OBS AK/SK 或服务器私钥。
- 服务器保留了专用未认证 smoke 账号，用于后续权限回归；该账号不是正式演示账号。
