# 测试记录：Qt 6.10.3 环境定位与批次 E 严格复跑

## 基本信息

- 日期：2026-05-18
- 线程：Qt 6.10.3 环境补齐/定位与批次 E 复跑
- 模块：桌面端基础工程
- 功能：Qt Widgets 最小启动与 Qt Test 严格环境复跑
- 对应需求/设计编号：技术基线验证；对应 `docs/13_detailed_design_v1.md` 第 3 章中 Qt 6.10.3、Qt Widgets、Qt Test 与桌面端分层约束
- 测试文件：`D:\big_homework\desktop\tests\ApiClientConfigTest.cpp`
- 实现文件：
  - `D:\big_homework\desktop\CMakeLists.txt`
  - `D:\big_homework\desktop\src\main.cpp`
  - `D:\big_homework\desktop\src\domain\ApiClientConfig.h`
  - `D:\big_homework\desktop\src\domain\ApiClientConfig.cpp`

## 本次任务边界

- 本次只定位 Qt 6.10.3 环境并尝试严格复跑批次 E。
- 不实现完整业务系统。
- 不新增完整页面、完整 API Client、登录、token 存储或业务流程。
- 不使用 Qt 6.10.1、Qt 6.11.0 或其他版本冒充 Qt 6.10.3 严格绿灯。

## 环境定位记录

- 已检查 Qt 安装根目录：`G:\Qt`
- 已发现版本目录：
  - `G:\Qt\6.10.1`
  - `G:\Qt\6.11.0`
  - `G:\Qt\6.9.3`
  - `G:\Qt\6.9.1`
  - `G:\Qt\6.8.3`
- 未发现版本目录：`G:\Qt\6.10.3`
- 已检查常见 Qt 根目录：当前仅发现 `G:\Qt`
- 已通过 `qmake -query QT_VERSION` 抽查：
  - `G:\Qt\6.10.1\mingw_64\bin\qmake.exe` 返回 `6.10.1`
  - `G:\Qt\6.11.0\mingw_64\bin\qmake.exe` 返回 `6.11.0`
  - `G:\Qt\6.9.3\mingw_64\bin\qmake.exe` 返回 `6.9.3`
- 已扫描 `G:\Qt` 下 `Qt6ConfigVersion.cmake`，未发现 Qt `6.10.3` kit。

## 严格复跑记录

- 复跑命令：未使用 `-DCAMPUS_BUDDY_REQUIRED_QT_VERSION=6.10.1` 覆盖。

```powershell
$env:PATH='G:\Qt\Tools\Ninja;G:\Qt\Tools\mingw1310_64\bin;G:\Qt\6.10.1\mingw_64\bin;' + $env:PATH
& 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' -S 'D:\big_homework\desktop' -B 'D:\big_homework\desktop\build-qt6103-rerun' -G Ninja '-DCMAKE_PREFIX_PATH=G:\Qt\6.10.1\mingw_64\lib\cmake'
```

- 结果：失败，符合预期。
- CMake 关键错误：

```text
Could not find a configuration file for package "Qt6" that is compatible
with requested version "6.10.3".

The following configuration files were considered but not accepted:
  G:/Qt/6.10.1/mingw_64/lib/cmake/Qt6/Qt6Config.cmake, version: 6.10.1
```

## Qt Test 与 smoke test 记录

- 本次未运行 Qt Test。
- 本次未运行 `campus_buddy_desktop.exe --smoke-test -platform minimal`。
- 原因：本机未定位到 Qt 6.10.3 kit，严格环境配置阶段已经阻塞；继续使用 Qt 6.10.1 或 Qt 6.11.0 构建会违反本次任务的版本约束。

## 结论

- 当前机器尚未具备批次 E 的 Qt 6.10.3 严格复跑条件。
- `D:\big_homework\desktop\CMakeLists.txt` 仍保持默认 `CAMPUS_BUDDY_REQUIRED_QT_VERSION = 6.10.3`。
- 本次没有修改桌面端工程代码。
- 批次 E 在 Qt 6.10.3 严格版本口径下仍处于环境阻塞状态，不能标记为严格绿灯。

## 后续风险

- 若继续使用 Qt 6.10.1 的历史构建结果，只能说明最小工程在 Qt 6.10.1 下曾经可构建和运行，不能说明 Qt 6.10.3 基线通过。
- 若使用 Qt 6.11.0 通过 CMake 配置，也不能替代 Qt 6.10.3 严格绿灯。
- 后续补齐 Qt 6.10.3 后，需要重新配置全新 build 目录，运行构建、Qt Test 和 smoke test，并追加新的验证记录。

## 下一步建议

- 优先安装或定位 Qt `6.10.3` 对应 Win11 MinGW kit。
- 安装完成后复跑：

```powershell
$env:PATH='G:\Qt\Tools\Ninja;G:\Qt\Tools\mingw1310_64\bin;G:\Qt\6.10.3\mingw_64\bin;' + $env:PATH
& 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' -S 'D:\big_homework\desktop' -B 'D:\big_homework\desktop\build-qt6103' -G Ninja '-DCMAKE_PREFIX_PATH=G:\Qt\6.10.3\mingw_64\lib\cmake'
& 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build 'D:\big_homework\desktop\build-qt6103'
& 'C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe' --test-dir 'D:\big_homework\desktop\build-qt6103' --output-on-failure
& 'D:\big_homework\desktop\build-qt6103\campus_buddy_desktop.exe' --smoke-test -platform minimal
```
