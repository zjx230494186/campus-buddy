# Windows Internal Beta Package Record

日期：2026-05-26

## 1. 本轮目标

打包 Qt 桌面端 Windows 内测版，交付为 zip。用户解压后进入目录，双击 exe 即可打开，并默认连接华为云服务器。

## 2. 交付物

- 打包目录：
  - `deliverables/internal_beta/CampusBuddyInternalBeta_20260526/`
- zip 包：
  - `deliverables/internal_beta/CampusBuddyInternalBeta_20260526.zip`
- 启动程序：
  - `campus_buddy_desktop.exe`

## 3. 本轮代码配置

- `desktop/src/domain/ApiClientConfig.cpp`
  - 默认 API base URL 改为：`http://114.116.203.78/api`
  - 仍保留覆盖方式：
    - 环境变量：`CAMPUS_BUDDY_API_BASE_URL`
    - 启动参数：`--api-base-url=...`
- `desktop/tests/ApiClientConfigTest.cpp`
  - 同步更新默认 URL 断言。

## 4. 构建命令

```powershell
$env:PATH='G:\Qt\6.10.3\mingw_64\bin;G:\Qt\Tools\mingw1310_64\bin;G:\Qt\Tools\CMake_64\bin;G:\Qt\Tools\Ninja;' + $env:PATH
G:\Qt\Tools\CMake_64\bin\cmake.exe -S desktop -B desktop\build-qt6103-internal-release -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=G:\Qt\6.10.3\mingw_64
G:\Qt\Tools\CMake_64\bin\cmake.exe --build desktop\build-qt6103-internal-release --target campus_buddy_desktop api_client_config_test --config Release
```

## 5. 打包命令

```powershell
Copy-Item desktop\build-qt6103-internal-release\campus_buddy_desktop.exe deliverables\internal_beta\CampusBuddyInternalBeta_20260526\campus_buddy_desktop.exe
G:\Qt\6.10.3\mingw_64\bin\windeployqt.exe --release --compiler-runtime --no-translations deliverables\internal_beta\CampusBuddyInternalBeta_20260526\campus_buddy_desktop.exe
Compress-Archive -LiteralPath deliverables\internal_beta\CampusBuddyInternalBeta_20260526 -DestinationPath deliverables\internal_beta\CampusBuddyInternalBeta_20260526.zip -CompressionLevel Optimal
```

## 6. 验证结果

服务器健康检查：

```text
GET http://114.116.203.78/api/health
{"status":"UP"}
```

Qt 配置测试：

```text
api_client_config_test exit code = 0
```

打包目录独立启动 smoke：

```text
campus_buddy_desktop.exe --smoke-test
package_smoke_exit=0
```

zip 内容关键项：

```text
CampusBuddyInternalBeta_20260526\campus_buddy_desktop.exe
CampusBuddyInternalBeta_20260526\Qt6Core.dll
CampusBuddyInternalBeta_20260526\Qt6Network.dll
CampusBuddyInternalBeta_20260526\Qt6Widgets.dll
CampusBuddyInternalBeta_20260526\platforms\qwindows.dll
CampusBuddyInternalBeta_20260526\tls\qcertonlybackend.dll
CampusBuddyInternalBeta_20260526\tls\qschannelbackend.dll
CampusBuddyInternalBeta_20260526\README.txt
```

zip 文件大小：

```text
24762992 bytes
```

本机服务器密钥依赖检查：

```text
源码命中：
desktop/src/domain/ApiClientConfig.cpp: DEFAULT_BASE_URL = "http://114.116.203.78/api"
desktop/src/domain/ApiClientConfig.cpp: ENV_KEY = "CAMPUS_BUDDY_API_BASE_URL"

交付包命中：
campus_buddy_desktop.exe 内包含 CAMPUS_BUDDY_API_BASE_URL、--api-base-url=、http://114.116.203.78/api
README.txt 内包含 http://114.116.203.78/api
Qt TLS DLL 内包含通用证书/私钥解析提示字符串
```

结论：

- 客户端连接华为云服务器只依赖公网 HTTP API：`http://114.116.203.78/api`
- 客户端不依赖本机 SSH 私钥、服务器私钥、数据库密码、OBS AK/SK、SMTP 授权码或项目外私有 env 文件。
- Qt TLS DLL 中的 `BEGIN PRIVATE KEY` 等字符串是 Qt 官方 TLS 组件用于解析证书/密钥格式的内置文本，不是项目密钥，也不包含实际密钥材料。
- 打包目录在剥离 Qt/MinGW PATH 的环境下通过 `--smoke-test`，说明运行依赖已随包携带。

## 7. 边界与风险

- 本轮只打包 Windows Qt 客户端。
- 没有修改后端、Flyway migration 或 deploy 脚本。
- 未将服务器私有配置、SMTP 授权码、OBS AK/SK、数据库密码或账号密码写入交付包。
- 服务器公网仍为 HTTP，内测可用；生产环境应升级 HTTPS。
- `windeployqt` 输出提示未找到 `dxcompiler.dll` / `dxil.dll`，当前应用未使用 Qt Quick/Shader 路径，打包 smoke 已通过。
