# 测试记录：桌面端 Qt Widgets 最小启动 / Qt Test

## 基本信息

- 日期：2026-05-18
- 线程：技术探路批次 E：Qt Widgets 最小启动与 Qt Test
- 模块：桌面端基础工程
- 功能：Qt 6 + C++ + Qt Widgets 最小工程、Qt Test 验证非 UI 配置对象默认值
- 对应需求编号：技术基线验证；对应 `docs/13_detailed_design_v1.md` 第 3 章中 Qt Widgets、Qt Test、桌面端分层与普通配置约束
- 测试文件：`D:\big_homework\desktop\tests\ApiClientConfigTest.cpp`
- 实现文件：
  - `D:\big_homework\desktop\CMakeLists.txt`
  - `D:\big_homework\desktop\src\main.cpp`
  - `D:\big_homework\desktop\src\domain\ApiClientConfig.h`
  - `D:\big_homework\desktop\src\domain\ApiClientConfig.cpp`

## 测试设计

- 测试类型：Qt Test 单元测试
- 测试对象：非 UI 配置对象 `ApiClientConfig`
- 覆盖路径：
  - 默认 API base URL 为 `http://localhost:8080/api`
  - 默认请求超时时间为 `10000` 毫秒
  - 默认消息轮询间隔为 `1000` 毫秒
  - 技术探路模式默认开启
- 本批不覆盖：
  - 完整页面
  - 完整 API Client
  - 登录、JWT 解析或 token 存储
  - QSettings 持久化
  - 业务流程或网络请求

## 红灯记录

- 红灯运行命令：

```powershell
$env:PATH='G:\Qt\Tools\Ninja;G:\Qt\Tools\mingw1310_64\bin;G:\Qt\6.10.1\mingw_64\bin;' + $env:PATH
& 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' -S 'D:\big_homework\desktop' -B 'D:\big_homework\desktop\build-red' -G Ninja '-DCMAKE_PREFIX_PATH=G:\Qt\6.10.1\mingw_64\lib\cmake' '-DCAMPUS_BUDDY_REQUIRED_QT_VERSION=6.10.1'
```

- 红灯结果：失败。
- 失败原因：测试已引用 `domain/ApiClientConfig.h`，CMake 也已声明 `src/domain/ApiClientConfig.cpp`、`src/domain/ApiClientConfig.h` 和 `src/main.cpp`，但这些实现文件尚未创建，CMake 生成阶段报出找不到源文件。
- 是否符合预期：符合。该结果证明本批非 UI 配置对象和最小启动入口在实现前确实不存在。

## 实现摘要

- 新增 `desktop` 最小 CMake 工程。
- 新增 Qt Test 用例 `ApiClientConfigTest`。
- 新增非 UI 配置对象 `ApiClientConfig`，当前仅保存非敏感默认配置：
  - API base URL
  - 请求超时时间
  - 消息轮询间隔
  - 技术探路模式标记
- 新增最小 Qt Widgets 启动入口：
  - 创建 `QApplication`
  - 创建一个基础 `QWidget`
  - 显示最小窗口标题与 API base URL
  - 提供 `--smoke-test` 参数用于自动启动后立即退出
- CMake 默认要求 `CAMPUS_BUDDY_REQUIRED_QT_VERSION = 6.10.3`，与设计文档保持一致；本机验证时因未安装 6.10.3，临时覆盖为 `6.10.1` 执行红绿灯闭环。

## 绿灯记录

- 本机 Qt 环境：
  - 已发现 `G:\Qt\6.10.1\mingw_64`
  - 已发现 `G:\Qt\6.11.0`、`G:\Qt\6.9.3`、`G:\Qt\6.9.1`、`G:\Qt\6.8.3`
  - 未发现 `G:\Qt\6.10.3`
  - PATH 中的 Anaconda `qmake` 为 Qt `5.15.2`，未用于本批验证
- 绿灯构建命令：

```powershell
$env:PATH='G:\Qt\Tools\Ninja;G:\Qt\Tools\mingw1310_64\bin;G:\Qt\6.10.1\mingw_64\bin;' + $env:PATH
& 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' -S 'D:\big_homework\desktop' -B 'D:\big_homework\desktop\build' -G Ninja '-DCMAKE_PREFIX_PATH=G:\Qt\6.10.1\mingw_64\lib\cmake' '-DCAMPUS_BUDDY_REQUIRED_QT_VERSION=6.10.1'
& 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build 'D:\big_homework\desktop\build'
```

- 本批测试命令：

```powershell
& 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe' --test-dir 'D:\big_homework\desktop\build' --output-on-failure
```

- 本批测试结果：通过，`1/1 Test #1: api_client_config_test ... Passed`
- 最小启动验证命令：

```powershell
& 'D:\big_homework\desktop\build\campus_buddy_desktop.exe' --smoke-test -platform minimal
```

- 最小启动验证结果：进程正常启动并退出，退出码为 `0`。

## Qt 6.10.3 环境偏差记录

- 设计与 CMake 默认值均以 Qt `6.10.3` 为目标版本。
- 当前机器未发现 Qt `6.10.3` 安装目录。
- 未覆盖版本运行命令：

```powershell
& 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' -S 'D:\big_homework\desktop' -B 'D:\big_homework\desktop\build-qt6103-check' -G Ninja '-DCMAKE_PREFIX_PATH=G:\Qt\6.10.1\mingw_64\lib\cmake'
```

- 结果：失败，CMake 报告当前可用配置为 Qt `6.10.1`，不满足请求的 `6.10.3`。
- 结论：本批代码层面保持 Qt `6.10.3` 默认约束；本机实际绿灯验证使用 Qt `6.10.1`，需后续安装 Qt `6.10.3` 后重新运行同一批命令并去掉版本覆盖。

## 本批明确未覆盖内容

- 未实现完整桌面端页面。
- 未实现完整 API Client。
- 未实现登录、注册、JWT 解析、refresh token 或 token 存储。
- 未写入 QSettings，也未保存任何敏感信息。
- 未接入后端接口、消息轮询、业务 ViewModel 或 Presenter。

## 后续风险

- Qt `6.10.3` 未安装导致严格版本绿灯尚未完成；后续需要补齐环境后复跑。
- 当前 `ApiClientConfig` 只有默认值，不包含配置加载、校验或用户自定义能力。
- 当前 Widgets 启动入口仅为工程探路入口，不代表正式 UI 架构、导航结构或视觉设计已经开始。
- `--smoke-test` 仅用于自动化最小启动验证，不应被误认为正式业务启动模式。

## 下一步建议

- 若继续桌面端方向，进入批次 F：Qt API Client 骨架。仍需先写 Qt Test，验证 GET JSON、错误对象转换和 Widget 层不直接调用 `QNetworkAccessManager`。
- 若优先消除环境偏差，先安装 Qt `6.10.3` 对应 MinGW kit，然后使用 CMake 默认版本要求重新执行本批构建与测试。
