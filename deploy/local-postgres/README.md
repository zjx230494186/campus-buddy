# 本地 PostgreSQL 初始化脚本

用途：把当前 Flyway V1-V11 对应的数据库结构部署到本地 PostgreSQL。

## 脚本说明

- `00_create_database.sql`：建本地角色与数据库的模板，需用 PostgreSQL 管理员账号执行。
- `01_schema.sql`：创建当前项目完整业务表结构，需在 `campus_buddy` 数据库内执行。

## 执行顺序

### 方式 A：推荐，保留 Flyway 自动迁移

如果你是为了让后端连接本地 PostgreSQL，推荐只执行建库脚本，然后让后端启动时自动执行现有 `backend/src/main/resources/db/migration/V1__...` 到 `V11__...`：

```powershell
psql -U postgres -f deploy/local-postgres/00_create_database.sql
```

这种方式保留 `spring.flyway.enabled=true`，最贴近项目当前运行方式。

### 方式 B：手工一次性建最终态 schema

如果你只是想直接得到当前最终表结构，执行以下步骤：

1. 编辑 `00_create_database.sql`，把 `CHANGE_ME_LOCAL_PASSWORD` 替换成本地密码。
2. 执行建库脚本：

```powershell
psql -U postgres -f deploy/local-postgres/00_create_database.sql
```

3. 执行 schema 脚本：

```powershell
psql -U campus_buddy -d campus_buddy -f deploy/local-postgres/01_schema.sql
```

注意：`01_schema.sql` 是手工合并后的最终态 schema，不会创建 Flyway 的 `flyway_schema_history`。如果用这种方式初始化后再启动后端，请关闭 Flyway，或改用方式 A 让 Flyway 自己迁移空库。

后端本地连接示例环境变量：

```powershell
$env:DB_HOST="localhost"
$env:DB_PORT="5432"
$env:DB_NAME="campus_buddy"
$env:DB_USERNAME="campus_buddy"
$env:DB_PASSWORD="<本地密码>"
```

注意：本目录只放结构脚本和占位符，不保存真实数据库密码、云端连接串或生产数据。
