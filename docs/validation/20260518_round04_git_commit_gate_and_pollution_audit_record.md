# Git 提交门禁与剩余污染复核验证记录

- 日期：2026-05-18
- 线程：Round 04 — Git 提交门禁与剩余污染复核

## 阶段 A：Git 状态审计

### 剩余跟踪污染发现
- `desktop/build-f-red/`：4 个 Qt 构建产物（上一轮遗漏）
- `latex_work/build_pdf_*`：10 个 LaTeX 构建产物（上一轮遗漏 `build_pdf_pictures_fixed/main.pdf`）
- 合计 14 个文件仍被跟踪

### 清理操作
- `git rm --cached` 移除上述 14 个文件
- 最终验证：`git ls-files | grep -E 'desktop/build|\.ninja_|backend/target|\.arts|test_output|test_error|ppt_extracted|latex_work/build'` 返回空

### 分类记录
- 明确应移出的构建/缓存/临时文件：全部 14 个（Qt 构建产物 + LaTeX 构建产物）
- 需用户确认的课程交付物：9 个 LaTeX 产物，用户确认全部移除跟踪
- 无风险文件

## 阶段 B：测试与敏感信息门禁

### 非容器快速测试
- 命令：`.\mvnw.cmd test '-Dspring.testcontainers.enabled=false' '-Dtest=!DatabaseMigrationTest'`
- 结果：32/32 通过，BUILD SUCCESS

### 敏感信息复查
- 检查项：AccessKey、SecretKey、AK/SK、JWT_SECRET、DB_PASSWORD、SSH 私钥、.env、.pem、.key、.p12、.pfx
- 结果：所有匹配均为安全规则声明或 `${}` 环境变量引用，无真实密钥值
- 无 .env/.pem/.key/.p12/.pfx/id_rsa/id_ed25519 文件被跟踪
- 结论：通过

## 阶段 C：提交

### 提交 1：`18e2a01` chore(repo): clean generated artifacts and record git hygiene
- 文件数：51 files changed (+683/-8580)
- 内容类型：
  - `.gitignore` 修改（补充忽略规则）
  - 38 个构建产物删除（.arts/, desktop/build-*/, latex_work/build_*/, ppt_extracted.txt）
  - 7 个文档新增/修改（审计记录、prompt 记录、current_plan、handoff）

### 提交 2：`157abc0` feat(auth): persist authentication flow and introduce jwt
- 文件数：31 files changed (+748/-295)
- 内容类型：
  - 7 个新增 JPA 实体/Repository
  - 3 个新增 JWT 服务（JwtService, JwtProperties, JwtAuthenticationFilter）
  - 1 个新增 FlywayConfiguration
  - 1 个新增 Flyway V2 迁移
  - 1 个新增 TestcontainersConfiguration
  - 1 个新增 AuthPersistenceIntegrationTest
  - 1 个新增 test application.properties
  - 8 个修改的源码（AuthLoginService, AuthRegistrationService, CampusEmailVerificationService, SecurityConfiguration, SecureProbeController, CampusBuddyBackendApplication, pom.xml）
  - 8 个修改的测试
  - 3 个修改的配置模板（application-deploy/local/test.properties）

### 提交后状态
- `git status --short --branch`：`## main`（工作区干净）
- `git log --oneline -3`：`157abc0`, `18e2a01`, `447b0af`
- 无构建产物、缓存、临时日志或密钥文件在跟踪中

## 停止条件检查
- 非容器快速测试失败？否
- 敏感信息疑似进入仓库？否
- Git 状态中出现新的构建产物？否
- 仍有大量无法分类的历史生成物？否
- 需要重写历史/reset hard/checkout？否
- 提交边界无法拆清？否
- 结论：所有停止条件均未触发
