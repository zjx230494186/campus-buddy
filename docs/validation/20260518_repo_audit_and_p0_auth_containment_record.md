# 仓库审计与 P0 认证收束验证记录

- 日期：2026-05-18
- 线程：仓库审计与 P0 认证开发收束批次

## Git 污染审计

### 审计发现
- 基线提交 `447b0af` 跟踪了不应提交的文件：
  - `desktop/build-qt6103/` 目录：24 个 Qt 构建产物（.ninja_deps, .ninja_log, build.ninja, autogen 等）
  - `latex_work/build_pdf_rrfix/` 目录：3 个 LaTeX 构建产物（main.docx, main.pdf, main_docx_plain.txt）
  - `.arts/settings.json`：IDE 工具元数据
  - `ppt_extracted.txt`：临时提取文件

### 清理操作
- `git rm --cached` 移除上述 29 个文件
- 文件本身保留在本地，仅取消 Git 跟踪

### .gitignore 补充
新增忽略规则：
- `desktop/build-*/` — Qt 构建输出目录
- `.ninja_deps`, `.ninja_log`, `build.ninja` — Ninja 构建文件
- `*_autogen/` — Qt autogen 目录
- `latex_work/build_*/` — LaTeX 构建输出目录
- `.arts/` — IDE 工具元数据
- `test_output*.txt`, `test_error*.txt`, `test_flyway_out*.txt` — 调试输出文件
- `ppt_extracted.txt` — 临时提取文件

## 敏感信息审计

### 审计结论：通过
- `application-local.properties`：JWT secret 为非敏感测试密钥 `local-dev-secret-key-...`，可接受。
- `application-test.properties`：JWT secret 为非敏感测试密钥 `test-secret-key-...`，可接受。
- `application-deploy.properties`：JWT secret 通过 `${JWT_SECRET}` 环境变量注入，数据库密码通过环境变量引用，符合安全要求。
- `JwtProperties.java`：默认值为非敏感开发密钥，仅用于 fallback。
- 未发现 AccessKey/SecretKey/数据库密码/JWT 真实密钥泄露到仓库。

### 风险项：无

## 测试修复

### 修复项
1. `AuthRegistrationService`：调整逻辑顺序，先检查 `existsByCampusEmail` 再 `consumeRegistrationTicket`，修复重复注册时错误码。
2. `SecurityProbeEndpointTest`：从占位 token 改为使用 `JwtService.issueAccessToken()` 生成真实 JWT。
3. `SecureProbeController`：`authenticationMode` 从 `"jwt-placeholder"` 更新为 `"jwt"`。

### 测试结果
- 非容器快速测试：32/32 通过
- 命令：`.\mvnw.cmd test '-Dspring.testcontainers.enabled=false' '-Dtest=!DatabaseMigrationTest'`
- Testcontainers 测试：需要 Docker 可用，本轮未执行

## 未完成项
- Git 提交（需用户确认）
- Testcontainers/Docker 测试（需 Docker 可用）
