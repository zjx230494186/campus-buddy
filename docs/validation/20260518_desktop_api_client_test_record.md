# 测试记录：桌面端 Qt API Client 骨架

## 基本信息

- 日期：2026-05-18
- 线程：技术探路批次 F：Qt API Client 骨架
- 模块：桌面端 API Client 层
- 功能：基础 GET JSON、统一错误对象转换、Widget 层不直接调用 `QNetworkAccessManager`
- 对应需求/设计编号：技术基线验证；对应 `docs/13_detailed_design_v1.md` 第 3 章中 Qt 6.10.3、Qt Widgets、API Client 层、HTTP 通信与客户端分层约束
- 测试文件：
  - `D:\big_homework\desktop\tests\CampusApiClientTest.cpp`
  - `D:\big_homework\desktop\tests\ApiClientConfigTest.cpp`
- 实现文件：
  - `D:\big_homework\desktop\src\api\CampusApiClient.h`
  - `D:\big_homework\desktop\src\api\CampusApiClient.cpp`
  - `D:\big_homework\desktop\src\domain\ApiClientConfig.h`
  - `D:\big_homework\desktop\src\domain\ApiClientConfig.cpp`
  - `D:\big_homework\desktop\CMakeLists.txt`

## 本次任务边界

- 本批只实现最小 API Client 骨架。
- 不实现完整业务系统。
- 不新增完整页面、登录、refresh token、真实 token 存储或业务流程。
- 不接入正式后端运行链路；测试使用本地 `QTcpServer` 模拟可控 HTTP JSON 响应。
- 必须使用 Qt 6.10.3，未使用 Qt 6.10.1、Qt 6.11.0 或其他版本冒充绿灯。

## 测试设计

- `getJsonParsesSuccessPayload`：使用本地 `QTcpServer` 返回 `HTTP 200` 与 JSON 响应体，断言 `CampusApiClient::getJson` 能解析 JSON。
- `errorResponseConvertsToClientError`：使用本地 `QTcpServer` 返回 `HTTP 404` 与统一错误结构，断言客户端错误对象包含 `type/httpStatus/code/message/details/traceId`。
- `widgetLayerDoesNotDirectlyUseNetworkAccessManager`：静态读取 `D:\big_homework\desktop\src\main.cpp`，断言 Widget/启动入口层不直接出现 `QNetworkAccessManager`。

## 红灯记录

- 红灯运行命令：

```powershell
$env:PATH='G:\Qt\Tools\Ninja;G:\Qt\Tools\mingw1310_64\bin;G:\Qt\6.10.3\mingw_64\bin;' + $env:PATH
& 'G:\Qt\Tools\CMake_64\bin\cmake.exe' -S 'D:\big_homework\desktop' -B 'D:\big_homework\desktop\build-f-red' -G Ninja '-DCMAKE_PREFIX_PATH=G:\Qt\6.10.3\mingw_64\lib\cmake'
```

- 红灯结果：失败，符合预期。
- 失败原因：测试和 CMake 已引用 `D:\big_homework\desktop\src\api\CampusApiClient.cpp` 与 `CampusApiClient.h`，但 API Client 尚未实现，CMake 生成阶段报找不到源文件。
- 关键错误：

```text
Cannot find source file:
  D:/big_homework/desktop/src/api/CampusApiClient.cpp
```

- 是否符合预期：符合。该失败证明 API Client 层在实现前确实不存在，本批新增测试不是无效绿灯。

## 实现摘要

- 新增 `CampusApiClient`：
  - API Client 层内部持有 `QNetworkAccessManager`。
  - 暴露 `getJson(path, callback)` 最小 GET JSON 入口。
  - 自动拼接 `ApiClientConfig::apiBaseUrl()` 与相对路径。
  - 解析 `2xx` JSON 响应为 `ApiClientResponse::json`。
  - 将非 `2xx` JSON 错误响应转换为 `ApiClientError`。
- 新增客户端错误对象：
  - `ApiClientError::NetworkError`
  - `ApiClientError::HttpError`
  - `ApiClientError::InvalidJson`
  - 字段包含 `httpStatus/code/message/details/traceId`。
- 扩展 `ApiClientConfig`：
  - 保留默认配置。
  - 新增测试可注入 base URL 的构造函数，不涉及敏感凭据存储。
- 更新 CMake：
  - 加入 Qt `Network` 组件。
  - 新增 `campus_api_client_test` 测试目标。

## 绿灯记录

- 配置与构建命令：

```powershell
$env:PATH='G:\Qt\Tools\Ninja;G:\Qt\Tools\mingw1310_64\bin;G:\Qt\6.10.3\mingw_64\bin;' + $env:PATH
& 'G:\Qt\Tools\CMake_64\bin\cmake.exe' -S 'D:\big_homework\desktop' -B 'D:\big_homework\desktop\build-qt6103' -G Ninja '-DCMAKE_PREFIX_PATH=G:\Qt\6.10.3\mingw_64\lib\cmake'
& 'G:\Qt\Tools\CMake_64\bin\cmake.exe' --build 'D:\big_homework\desktop\build-qt6103'
```

- Qt Test 命令：

```powershell
$env:PATH='G:\Qt\Tools\Ninja;G:\Qt\Tools\mingw1310_64\bin;G:\Qt\6.10.3\mingw_64\bin;' + $env:PATH
& 'G:\Qt\Tools\CMake_64\bin\ctest.exe' --test-dir 'D:\big_homework\desktop\build-qt6103' --output-on-failure
```

- Qt Test 结果：

```text
1/2 Test #1: api_client_config_test ...........   Passed
2/2 Test #2: campus_api_client_test ...........   Passed
100% tests passed, 0 tests failed out of 2
```

- smoke test 命令：

```powershell
$env:PATH='G:\Qt\Tools\Ninja;G:\Qt\Tools\mingw1310_64\bin;G:\Qt\6.10.3\mingw_64\bin;' + $env:PATH
& 'D:\big_homework\desktop\build-qt6103\campus_buddy_desktop.exe' --smoke-test -platform minimal
```

- smoke test 结果：通过，进程退出码为 `0`。

## 本批明确未覆盖内容

- 未实现完整业务系统。
- 未实现正式登录、注册、JWT 解析、refresh token 或真实 token 存储。
- 未实现 `SecureTokenStore`。
- 未使用 `QSettings` 存储任何 token 或敏感信息。
- 未新增正式页面、ViewModel/Presenter 或业务流程。
- 未实现请求重试、取消、统一超时中断、认证头注入、分页、上传或下载能力。
- 未接入真实后端联调；仅验证 API Client 层最小 HTTP/JSON 行为。

## 后续风险

- 当前 `getJson` 只覆盖最小 GET JSON，不代表完整 REST 客户端能力。
- `ApiClientError` 目前只按后端统一错误结构做最小映射，后续需要结合正式 OpenAPI 契约扩展错误码体系。
- 当前 Widget 层静态防护只覆盖 `src/main.cpp`；后续新增 `src/widgets` 或正式页面后，需要扩大静态检查范围。
- 认证头、token 读取和刷新必须等 `SecureTokenStore` 抽象确认后另开批次实现，不能直接写入明文文件或 `QSettings`。

## 下一步建议

- 候选 1：执行技术探路批次 G：Docker Compose 本地部署探路，建议新开或复用后端技术探路上下文。
- 候选 2：继续桌面端 API Client 小修，补充超时、请求取消或认证头占位，建议复用当前桌面端上下文，但必须继续测试先行。
- 候选 3：回到详细设计，补全正式 API Client 与后端 OpenAPI 的接口追踪矩阵，建议新开设计线程。
