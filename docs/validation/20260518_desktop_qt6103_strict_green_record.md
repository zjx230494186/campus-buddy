# 测试记录：Qt 6.10.3 批次 E 严格绿灯复跑

## 基本信息

- 日期：2026-05-18
- 线程：Qt 6.10.3 kit 安装/定位后批次 E 再复跑
- 模块：桌面端基础工程
- 功能：Qt Widgets 最小启动与 Qt Test 严格版本复跑
- 对应需求/设计编号：技术基线验证；对应 `docs/13_detailed_design_v1.md` 第 3 章中 Qt 6.10.3、Qt Widgets、Qt Test 与桌面端分层约束
- 测试文件：`D:\big_homework\desktop\tests\ApiClientConfigTest.cpp`
- 实现文件：
  - `D:\big_homework\desktop\CMakeLists.txt`
  - `D:\big_homework\desktop\src\main.cpp`
  - `D:\big_homework\desktop\src\domain\ApiClientConfig.h`
  - `D:\big_homework\desktop\src\domain\ApiClientConfig.cpp`

## 本次任务边界

- 本次只补齐/定位 Qt 6.10.3 kit，并基于 `D:\big_homework\desktop` 当前最小工程复跑批次 E。
- 不实现完整业务系统。
- 不新增完整页面、完整 API Client、登录、token 存储或业务流程。
- 不使用 Qt 6.10.1、Qt 6.11.0、Anaconda Qt 6.10.2 或其他版本冒充 Qt 6.10.3 严格绿灯。
- 不使用 `-DCAMPUS_BUDDY_REQUIRED_QT_VERSION=6.10.1` 覆盖版本要求。

## 环境补齐与定位记录

- 初始扫描结果：
  - `G:\Qt` 下已存在 `6.10.1`、`6.11.0`、`6.9.3`、`6.9.1`、`6.8.3`。
  - 全盘扫描还发现 Anaconda 中的 Qt 6.10.2 / 6.9.1，但不符合本次严格版本要求。
  - 初始未发现 `G:\Qt\6.10.3`。
- 使用 `aqtinstall` 查询 Qt 6.10.3 可用架构：

```powershell
python -m aqt list-qt windows desktop --arch 6.10.3
```

- 查询结果包含：
  - `win64_mingw`
  - `win64_llvm_mingw`
  - `win64_msvc2022_64`
  - `win64_msvc2022_arm64_cross_compiled`
- 本次选择 `win64_mingw`，与既有 `G:\Qt\Tools\mingw1310_64` 和上一轮桌面端工程验证路线保持一致。
- 安装命令：

```powershell
python -m aqt install-qt windows desktop 6.10.3 win64_mingw --outputdir 'G:\Qt'
```

- 安装结果：
  - 成功生成 `G:\Qt\6.10.3\mingw_64`
  - 安装日志显示 `Finished installation`
- 版本确认命令：

```powershell
& 'G:\Qt\6.10.3\mingw_64\bin\qmake.exe' -query QT_VERSION
```

- 版本确认结果：`6.10.3`

## 严格配置记录

- 配置命令未使用 `-DCAMPUS_BUDDY_REQUIRED_QT_VERSION=6.10.1` 或其他版本覆盖。

```powershell
$env:PATH='G:\Qt\Tools\Ninja;G:\Qt\Tools\mingw1310_64\bin;G:\Qt\6.10.3\mingw_64\bin;' + $env:PATH
& 'G:\Qt\Tools\CMake_64\bin\cmake.exe' -S 'D:\big_homework\desktop' -B 'D:\big_homework\desktop\build-qt6103' -G Ninja '-DCMAKE_PREFIX_PATH=G:\Qt\6.10.3\mingw_64\lib\cmake'
```

- 配置结果：通过。
- CMake 关键确认：
  - `CAMPUS_BUDDY_REQUIRED_QT_VERSION:STRING=6.10.3`
  - `CMAKE_PREFIX_PATH=G:\Qt\6.10.3\mingw_64\lib\cmake`
  - `Qt6_DIR=G:/Qt/6.10.3/mingw_64/lib/cmake/Qt6`
  - `CMAKE_CXX_COMPILER=G:/Qt/Tools/mingw1310_64/bin/c++.exe`

## 构建记录

```powershell
$env:PATH='G:\Qt\Tools\Ninja;G:\Qt\Tools\mingw1310_64\bin;G:\Qt\6.10.3\mingw_64\bin;' + $env:PATH
& 'G:\Qt\Tools\CMake_64\bin\cmake.exe' --build 'D:\big_homework\desktop\build-qt6103'
```

- 构建结果：通过。
- 生成目标：
  - `D:\big_homework\desktop\build-qt6103\campus_buddy_desktop.exe`
  - `D:\big_homework\desktop\build-qt6103\api_client_config_test.exe`

## Qt Test 记录

```powershell
$env:PATH='G:\Qt\Tools\Ninja;G:\Qt\Tools\mingw1310_64\bin;G:\Qt\6.10.3\mingw_64\bin;' + $env:PATH
& 'G:\Qt\Tools\CMake_64\bin\ctest.exe' --test-dir 'D:\big_homework\desktop\build-qt6103' --output-on-failure
```

- 测试结果：通过。
- 关键输出：

```text
1/1 Test #1: api_client_config_test ...........   Passed
100% tests passed, 0 tests failed out of 1
```

## smoke test 记录

```powershell
$env:PATH='G:\Qt\Tools\Ninja;G:\Qt\Tools\mingw1310_64\bin;G:\Qt\6.10.3\mingw_64\bin;' + $env:PATH
& 'D:\big_homework\desktop\build-qt6103\campus_buddy_desktop.exe' --smoke-test -platform minimal
```

- 运行结果：通过。
- 进程退出码：`0`

## 结论

- 批次 E 已在 Qt 6.10.3 严格版本口径下完成配置、构建、Qt Test 与 smoke test。
- 本次未修改桌面端工程代码，只新增严格复跑构建目录与验证记录。
- `D:\big_homework\desktop\CMakeLists.txt` 继续保持默认 `CAMPUS_BUDDY_REQUIRED_QT_VERSION = 6.10.3`。
- 批次 E 可以标记为 Qt 6.10.3 严格绿灯。

## 本批明确未覆盖内容

- 未实现完整页面。
- 未实现完整 API Client。
- 未实现登录、注册、JWT 解析、refresh token 或 token 存储。
- 未写入 QSettings，也未保存任何敏感信息。
- 未接入后端接口、消息轮询、业务 ViewModel 或 Presenter。

## 后续风险

- 当前 `ApiClientConfig` 只有默认值，不包含配置加载、校验或用户自定义能力。
- 当前 Widgets 启动入口仅为工程探路入口，不代表正式 UI 架构、导航结构或视觉设计已经开始。
- `--smoke-test` 仅用于自动化最小启动验证，不应被误认为正式业务启动模式。

## 下一步建议

- 候选 1：进入批次 F：Qt API Client 骨架。建议复用当前桌面端技术探路上下文，但仍需先写 Qt Test，验证 GET JSON、错误对象转换和 Widget 层不直接调用 `QNetworkAccessManager`。
- 候选 2：进入批次 G：Docker Compose 本地部署探路。建议新开或复用后端技术探路上下文，验证 PostgreSQL 与后端服务本地 Compose 启动。
