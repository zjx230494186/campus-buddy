# Round 23 Validation Record — OBS 生命周期关闭与进程命令行敏感信息治理

**日期**: 2026-05-20
**轮次**: Round 23
**目标**: ObsClient 接入 Spring Bean 生命周期关闭 + 启动脚本移除命令行敏感参数

## 问题摘要

1. `ObsObjectStorageService` 有 `close()` 方法但未接入 Spring `@PreDestroy` / `destroyMethod`，应用关闭时 ObsClient 可能未释放连接资源。
2. `start_backend_deploy.sh` 通过 `--spring.datasource.password=...` 命令行参数传递数据库密码，`ps -ef` 可见明文。

## 红灯测试

### 测试命令

```bash
mvn test -Dtest=DeployScriptSecurityTest,ObsObjectStorageServiceLifecycleTest
```

### 红灯来源

1. `DeployScriptSecurityTest.scriptDoesNotContainDatasourcePasswordArg`: 失败，脚本包含 `--spring.datasource.password`
2. `ObsObjectStorageServiceLifecycleTest`: 修改前 `isClosed()` 方法不存在，编译失败

### 红灯确认

- 启动脚本静态检查 1 个失败，符合预期
- 生命周期测试编译失败，符合预期

## 修复摘要

### ObsObjectStorageService.java

- 新增 `volatile boolean closed` 字段和 `isClosed()` 方法
- `close()` 方法中设置 `closed = true`

### ObjectStorageConfiguration.java

- deploy profile 的 `@Bean` 添加 `destroyMethod = "close"`，Spring 容器销毁时自动调用 `ObsObjectStorageService.close()`

### start_backend_deploy.sh

- 移除 `--spring.datasource.url`、`--spring.datasource.username`、`--spring.datasource.password` 命令行参数
- 敏感配置完全通过环境变量解析：
  - `backend.env` 注入 `DB_HOST`、`DB_PORT`、`DB_NAME`、`DB_USERNAME`、`DB_PASSWORD`、`JWT_SECRET`、`OBJECT_STORAGE_ACCESS_KEY_ID`、`OBJECT_STORAGE_SECRET_ACCESS_KEY`
  - `application-deploy.properties` 通过 `${DB_PASSWORD}`、`${JWT_SECRET}`、`${OBS_ACCESS_KEY_ID}`、`${OBS_SECRET_ACCESS_KEY}` 引用
- 最终 Java 命令行仅包含：`java -jar campus-buddy-backend-0.0.1-SNAPSHOT.jar --spring.profiles.active=deploy`

## 新增测试

| 测试类 | 测试数 | 说明 |
|--------|--------|------|
| `ObsObjectStorageServiceLifecycleTest` | 2 | close() 直调 + Spring destroyMethod 触发 |
| `DeployScriptSecurityTest` | 4 | 启动脚本不含 datasource.password、jwt.secret、OBS AK/SK 参数 |

## 绿灯测试

```bash
mvn test -Dtest=DeployScriptSecurityTest,ObsObjectStorageServiceLifecycleTest
```

- Tests run: 6, Failures: 0, Errors: 0
- 结果：GREEN

## 后端全量回归测试

| 指标 | 值 |
|------|-----|
| 总测试数 | 139 |
| 通过 | 139 |
| 失败 | 0 |
| BUILD | SUCCESS |

## 服务器重启结果

- 启动脚本：`start_backend_deploy.sh`（已上传新版本）
- 后端启动成功，耗时 8.7s
- 结果：SUCCESS

## Health Check

| 入口 | HTTP 状态 | 响应 | 结果 |
|------|-----------|------|------|
| http://127.0.0.1:8080/api/health | 200 | `{"status":"UP"}` | PASS |
| http://114.116.203.78/api/health | 200 | `{"status":"UP"}` | PASS |

## 进程命令行敏感信息检查

- `ps -ef` 输出：`java -jar campus-buddy-backend-0.0.1-SNAPSHOT.jar --spring.profiles.active=deploy`
- 检查 `password`、`secret`、`access-key`、`secret-access` 关键字：**未发现**
- 结论：**命令行不含 DB/JWT/OBS secret**

## OBS Smoke Test 复跑

- 复跑原因：修改了 ObsObjectStorageService 和启动脚本
- 结果：PUT 200, GET 200 SHA-256 match, DELETE 204, 再 GET 404 confirmed
- 测试对象已删除，无残留
- 结论：**PASS**

## 私有配置检查

- 所有敏感配置仅通过 `/etc/campus-buddy/backend.env`（权限 600，owner root:root）注入
- `ps -ef` 不暴露任何敏感值
- `application-deploy.properties` 只引用环境变量占位符
- 检查结论：通过

## 结论

- ObsClient 通过 `@Bean(destroyMethod="close")` 接入 Spring 生命周期，应用关闭时释放连接
- 启动脚本不再通过命令行参数传递敏感配置，进程命令行安全
- 全量回归 139/139 通过
- OBS PUT/GET/DELETE smoke 复跑通过

## 未覆盖风险

1. ObsClient 连接池/超时/重试配置未调优（使用 SDK 默认值）
2. IAM 委托模式未实现（仍使用长期 AK/SK，但 AK/SK 不在命令行可见）
3. systemd 服务化未实现（当前 nohup 启动）
4. HTTPS / 域名 / 备案未配置
5. 邮件发送仍为 Noop
6. getObject 返回的 InputStream 依赖调用方正确关闭

## 下一步建议

1. **后端 systemd 服务化** — 注册为 systemd service，开机自启 + 日志管理，新开线程
2. **OBS IAM 委托模式** — ECS 绑定委托获取临时凭证，新开线程
3. **Qt 客户端服务器联调** — 验证完整闭环，新开线程
4. **邮件发送替换** — 接入 SMTP，新开线程
