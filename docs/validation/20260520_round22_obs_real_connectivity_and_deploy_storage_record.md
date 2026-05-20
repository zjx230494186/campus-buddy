# Round 22 Validation Record — OBS 真实连通性与 deploy 对象存储适配

**日期**: 2026-05-20
**轮次**: Round 22
**目标**: 让 deploy 环境使用真实华为云 OBS 对象存储适配，完成服务器端最小 PUT/GET/DELETE smoke test

## 完成事项

### 新增文件

| 文件 | 说明 |
|------|------|
| `ObsObjectStorageService.java` | 华为云 OBS 真实适配实现 |
| `ObsObjectStorageServiceTest.java` | OBS 适配测试（6 个测试） |
| `deploy/start_backend_deploy.sh` | 服务器启动脚本（引用私有 env 文件） |
| `deploy/obs_put_get_delete_smoke.py` | OBS PUT/GET/DELETE 一次性探针脚本 |
| `deploy/run_obs_smoke.sh` | 执行探针脚本的包装脚本 |

### 修改文件

| 文件 | 变更 |
|------|------|
| `pom.xml` | 添加 `esdk-obs-java:3.24.3` 依赖 |
| `ObjectStorageService.java` | 新增 `deleteObject(String key)` 方法 |
| `InMemoryObjectStorageService.java` | 实现 `deleteObject` |
| `ObjectStorageConfiguration.java` | deploy profile 使用 `ObsObjectStorageService`，其余保持 `InMemoryObjectStorageService` |
| `CampusBuddyProperties.java` | ObjectStorage 新增 `accessKeyId` 和 `secretAccessKey` 属性 |
| `application-deploy.properties` | 添加 `campus-buddy.object-storage.access-key-id=${OBS_ACCESS_KEY_ID}` 和 `secret-access-key=${OBS_SECRET_ACCESS_KEY}` |

### 新增依赖及理由

| 依赖 | 版本 | 理由 |
|------|------|------|
| `com.huaweicloud:esdk-obs-java` | 3.24.3 | 华为云 OBS 官方 Java SDK，项目已选定华为云 OBS 作为对象存储，使用官方 SDK 是最直接稳定的方案。替代方案为 AWS S3 SDK + S3 兼容 endpoint，但增加兼容性风险。 |

## 关键设计决策

1. **deploy profile 使用真实 OBS**：`ObjectStorageConfiguration` 中 deploy profile 创建 `ObsObjectStorageService`，test/local/local-h2 继续使用 `InMemoryObjectStorageService`
2. **缺少凭据时启动失败**：`ObsObjectStorageService` 构造器在 accessKeyId 或 secretAccessKey 为空/blank 时抛出 `IllegalArgumentException`，不会静默退回内存存储
3. **接口扩展**：`ObjectStorageService` 新增 `deleteObject(String key)` 方法，`InMemoryObjectStorageService` 和 `ObsObjectStorageService` 均实现
4. **凭据不落盘**：deploy profile 通过 `${OBS_ACCESS_KEY_ID}` 和 `${OBS_SECRET_ACCESS_KEY}` 环境变量引用，服务器私有 env 文件权限 600，owner root:root

## 红灯测试

- 测试命令：`mvn test -Dtest=ObsObjectStorageServiceTest`
- 红灯来源：构造器拒绝空凭据的 4 个测试 + InMemory deleteObject 的 2 个测试
- ObsObjectStorageService 类在实现前不存在，编译失败即红灯
- 红灯确认：实现前编译失败符合预期

## 绿灯测试

- 测试命令：`mvn test -Dtest=ObsObjectStorageServiceTest`
- Tests run: 6, Failures: 0, Errors: 0
- 结果：GREEN

## 后端全量回归测试

| 指标 | 值 |
|------|-----|
| 总测试数 | 133 |
| 通过 | 133 |
| 失败 | 0 |
| BUILD | SUCCESS |

## 服务器部署结果

- 服务器：Ubuntu 24.04.4 LTS
- 后端启动方式：`start_backend_deploy.sh` → source `/etc/campus-buddy/backend.env` → `java -jar` deploy profile
- OBS SDK 初始化日志确认：`[OBS SDK Version=3.24.3];[Endpoint=https://obs.cn-north-4.myhuaweicloud.com:443/];[Access Mode=Virtual Hosting]`
- 启动成功，耗时 9.5s
- 健康检查：localhost 200 UP，Nginx 公网 200 UP

## OBS PUT/GET/DELETE Smoke Test

执行方式：服务器本地 Python 探针脚本，使用 AWS SigV4 签名（OBS 兼容），凭据从 `/etc/campus-buddy/backend.env` 读取

| 操作 | 对象 key | HTTP 状态 | 结果 |
|------|----------|-----------|------|
| PUT | `technical-spike/round22/cb36ad8936dc90e2.txt` | 200 | 成功上传 |
| GET | 同上 | 200 | 成功读取，SHA-256 匹配 |
| DELETE | 同上 | 204 | 成功删除 |
| GET (验证删除) | 同上 | 404 | 确认已删除 |

SHA-256: `cb36ad8936dc90e2bdc4ef316319eebf6d4437c7524b75d01731996d640e1699`

测试对象已删除，无残留。

## 私有配置检查

- `JWT_SECRET` 是否仅在服务器私有配置：是（`/etc/campus-buddy/backend.env`，权限 600）
- 数据库密码是否仅在服务器私有配置：是（同上）
- 对象存储凭据是否仅在服务器私有配置：是（`OBJECT_STORAGE_ACCESS_KEY_ID` 和 `OBJECT_STORAGE_SECRET_ACCESS_KEY` 在 `/etc/campus-buddy/backend.env`，权限 600）
- Qt 客户端是否不持有对象存储凭据：是
- 仓库 / 文档 / 聊天中是否未出现敏感明文：是
  - `application-deploy.properties` 只引用 `${OBS_ACCESS_KEY_ID}` 和 `${OBS_SECRET_ACCESS_KEY}` 环境变量
  - `CampBuddyProperties.java` 只有属性名和 getter/setter，无默认值
  - 探针脚本不打印凭据
  - 启动脚本只 source env 文件，不记录内容
- 检查结论：通过

## 敏感信息检查

- 未记录 OBS AK/SK 明文
- 未记录 JWT secret 明文
- 未记录数据库密码明文
- 未记录 SSH 私钥
- 探针脚本 SHA-256 值为测试对象内容哈希，非敏感
- 测试对象 key 前缀 `technical-spike/round22/` 为非敏感技术标识

## 结论

- deploy profile 现在使用真实华为云 OBS，不再使用 InMemoryObjectStorageService 兜底
- 缺少 OBS 凭据时后端启动失败，不会静默退回内存存储
- 服务器 OBS PUT/GET/DELETE 最小闭环验证通过
- 后端全量回归测试 133/133 通过

## 未覆盖风险

1. OBS 临时凭证（IAM 委托）模式未实现，当前仍使用长期 AK/SK
2. 对象 key 前缀校验未在后端代码中强制执行（当前依赖业务层约定）
3. OBS 连接池/超时/重试配置未调优（使用 SDK 默认值）
4. getObject 返回的 InputStream 未做自动关闭包装（依赖调用方正确关闭）
5. ObsClient 关闭逻辑未集成到 Spring Bean 生命周期（PreDestroy）
6. HTTPS / 域名 / 备案未配置
7. 后端进程管理未使用 systemd
8. 邮件发送仍为 Noop 实现

## 下一步建议

1. **ObsClient 生命周期管理** — 添加 @PreDestroy 关闭 ObsClient，复用当前线程
2. **OBS IAM 委托模式** — 实现 ECS 绑定委托获取临时凭证，替换长期 AK/SK，新开线程
3. **后端 systemd 服务化** — 注册为 systemd service，新开线程
4. **Qt 客户端服务器联调** — 验证认证+附件上传完整闭环，新开线程
5. **邮件发送替换** — 接入 SMTP 或华为云邮件服务，新开线程
