# Qt UI Visual Polish Record

日期：2026-05-25

## 1. 本轮目标

将 Qt 桌面端从默认控件堆叠状态，提升到答辩可展示的清爽工具型界面。范围严格限定在 Qt 桌面端 UI 视觉打磨与体验修复，不修改后端、Flyway、deploy 或服务器配置。

## 2. UI 审计摘要

### 当前最影响答辩的页面

- `HomePageWidget`：首页只有标题、状态文本、Tab 和退出按钮，缺少应用标题区、用户模式说明和主次入口层级。
- `PostEditorWidget`：发布表单字段连续堆叠，基础信息、时间地点、偏好标签和场景扩展字段没有分区。
- `PlazaWidget`：列表、筛选、详情、邀约输入和联系按钮纵向堆叠，详情是一整块文本，发布者认证与信用摘要不够醒目。
- `ConversationsWidget`：会话、消息、会话动作、联系方式卡片和解锁状态都在同一竖列里，像调试表单。
- `ReviewCreditWidget`：信用摘要、提交评价、修改评价、评价列表混在一页中，内部 ID 输入不可避免但缺少分区说明。

### 布局混乱点

- 页面普遍缺少统一页头、分区卡片和主次按钮。
- 主链路页面没有左右布局，列表和详情的关系不清楚。
- 状态提示和详情文本样式相同，用户不容易区分“数据内容”和“操作反馈”。
- 登录/注册作为答辩第一屏，原先是大留白 + 默认按钮，第一眼观感偏粗糙。

### 文案/错误提示问题

- 部分成功提示仍带内部 ID，例如 `conversationId`、`reviewId`，已在本轮弱化为更接近用户语言的提示。
- 场景、状态、标签存在英文枚举裸露问题，本轮在广场/会话/发帖状态中增加中文展示。
- `qt_server_integration_smoke` 的联系方式字段仍不打印真实值，本轮没有新增敏感值输出。

### 组件风格不统一点

- 按钮缺少 primary/secondary/danger/ghost 角色。
- 卡片圆角、输入框边框、Tab 样式和列表项间距缺少统一 QSS。
- 页面标题字号、状态标签背景、列表项选中态不统一。

### 优先修复页面

第一批：`HomePageWidget`、`PostEditorWidget`、`PlazaWidget`、`ConversationsWidget`、`ReviewCreditWidget`，并顺手收口登录/注册第一屏。  
第二批：`IdentityVerificationWidget`、`MyPostsWidget`、`AdminReviewWidget`。  
第三批：更细的表单字段级错误贴近、空状态插图/占位、弱网 loading、防重复交互一致化。

## 3. 实际修改文件

### Qt 桌面端

- `desktop/CMakeLists.txt`
- `desktop/src/main.cpp`
- `desktop/src/ui/AppStyles.h`
- `desktop/src/ui/AppStyles.cpp`
- `desktop/src/ui/UiHelpers.h`
- `desktop/src/ui/UiHelpers.cpp`
- `desktop/src/ui/HomePageWidget.cpp`
- `desktop/src/ui/LoginWidget.cpp`
- `desktop/src/ui/RegisterWidget.cpp`
- `desktop/src/ui/PostEditorWidget.cpp`
- `desktop/src/ui/PlazaWidget.cpp`
- `desktop/src/ui/ConversationsWidget.cpp`
- `desktop/src/ui/ReviewCreditWidget.cpp`
- `desktop/tests/StudentPostPlazaWidgetTest.cpp`

### Validation 产物

- `docs/validation/20260525_qt_ui_visual_polish_record.md`
- `docs/validation/20260525_qt_ui_visual_polish_login_screenshot.png`

## 4. 是否修改后端/Flyway/deploy

- 修改 `backend/**`：否
- 修改 `backend/src/main/resources/db/migration/**`：否
- 修改 `deploy/**`：否
- 修改服务器配置：否
- 修改后端 API 合同：否

## 5. 改造页面列表

- `HomePageWidget`：新增清晰页头、当前模式/状态区域、退出按钮 danger 样式，Tab 容器统一风格。
- `LoginWidget` / `RegisterWidget`：改为页头 + 表单卡片 + 主按钮/ghost 链接，提升第一屏观感。
- `PostEditorWidget`：按基础信息、时间地点、偏好标签分组，主次按钮分离，状态提示统一。
- `PlazaWidget`：筛选区卡片化，列表/详情改左右分栏，详情改富文本层级，联系按钮主按钮化。
- `ConversationsWidget`：会话列表与消息区左右分栏，消息动作、联系方式卡片和解锁动作分组。
- `ReviewCreditWidget`：信用摘要、提交评价、修改评价、评价列表分区，提示语更贴近演示语境。

## 6. 统一样式/布局 helper

- `AppStyles`：统一应用级 QSS，覆盖背景、页头卡片、输入框、按钮角色、Tab、列表项和 GroupBox。
- `UiHelpers`：提供页头、卡片、状态标签、按钮角色标记、场景/状态中文展示和标签压缩展示。
- `StudentPostPlazaWidgetTest` 新增 `coreDemoUiUsesSharedVisualHelpers`，确保桌面端加载统一样式并且主演示页面接入 helper。

## 7. 测试结果

### Red/Green 记录

- 新增 `coreDemoUiUsesSharedVisualHelpers` 后，当前代码红灯：`student_post_plaza_widget_test` 失败。
- 接入 `AppStyles`、`UiHelpers` 和主演示页面后，目标测试转绿。

### 构建

命令：

```powershell
cd D:\big_homework\desktop
$env:Path = "G:\Qt\Tools\CMake_64\bin;G:\Qt\Tools\Ninja;G:\Qt\Tools\mingw1310_64\bin;G:\Qt\6.10.3\mingw_64\bin;$env:Path"
& "G:\Qt\Tools\CMake_64\bin\cmake.exe" -S . -B build-qt6103-ui-polish -G Ninja -DCMAKE_PREFIX_PATH=G:\Qt\6.10.3\mingw_64
& "G:\Qt\Tools\CMake_64\bin\cmake.exe" --build build-qt6103-ui-polish
```

结果：通过。

### Qt ctest

命令：

```powershell
cd D:\big_homework\desktop\build-qt6103-ui-polish
ctest --output-on-failure -j4
```

结果：10/10 PASS。

### campus_buddy_desktop --smoke-test

命令：

```powershell
.\campus_buddy_desktop.exe --smoke-test
```

结果：通过，进程正常启动并退出。

### qt_server_integration_smoke

命令：

```powershell
.\qt_server_integration_smoke.exe
```

结果：阻塞，未运行服务器链路。原因是当前 shell 未注入以下私有环境变量：

- `CAMPUS_BUDDY_SMOKE_EMAIL`
- `CAMPUS_BUDDY_SMOKE_PASSWORD`
- `CAMPUS_BUDDY_SMOKE_ADMIN_EMAIL`
- `CAMPUS_BUDDY_SMOKE_ADMIN_PASSWORD`

该阻塞符合项目安全边界，本轮未在仓库或聊天中写入真实账号、密码或 token。

## 8. 人工视觉检查结果

- 启动桌面端到登录页并截图：`docs/validation/20260525_qt_ui_visual_polish_login_screenshot.png`
- 登录页可正常渲染统一 QSS、页头卡片、表单卡片、主按钮和 ghost 注册入口。
- 由于当前 shell 没有 smoke 账号环境变量，本轮未登录进入真实服务器数据页面做截图，主页面视觉检查以代码结构、构建和本地启动 smoke 为准。

## 9. 敏感信息检查结论

检查范围：

- `desktop/src/ui/*.cpp`
- `desktop/src/ui/*.h`
- `desktop/tests/StudentPostPlazaWidgetTest.cpp`

结果：

- 未新增真实 token、Authorization 值、OBS AK/SK、Secret、真实邮箱、真实联系方式。
- 命中项仅为局部变量名 `password`、测试中的安全扫描正则和源码路径字符串，不是具体敏感值。

## 10. Git status 摘要

本轮相关修改集中在：

- `desktop/CMakeLists.txt`
- `desktop/src/main.cpp`
- `desktop/src/ui/**`
- `desktop/tests/StudentPostPlazaWidgetTest.cpp`
- `docs/validation/20260525_qt_ui_visual_polish_*`

当前仓库仍存在若干前置未跟踪文件，例如 `deploy/*.sh`、`deploy/r47_smoke.py`、`tmp_*`、历史 prompts/docs 等。本轮不修改、不暂存、不提交这些前置脏文件。

## 11. 提交哈希

- 提交前基线：`92b1906`
- 本轮提交哈希：见最终回复中的 Git commit 结果。

## 12. 未覆盖风险

- `qt_server_integration_smoke` 未运行，原因是本地缺少私有 smoke 环境变量。
- 第一批没有深改 `IdentityVerificationWidget`、`MyPostsWidget`、`AdminReviewWidget`。
- 管理员审核页面仍然偏功能堆叠，适合第二批集中处理。
- 真实登录后的多数据页面没有截图，后续有 smoke 账号环境变量后应补一组主链路截图。

## 13. 下一批 UI 建议

1. 第二批：打磨 `IdentityVerificationWidget`、`MyPostsWidget`、`AdminReviewWidget`。
2. 第三批：增加空状态、加载状态、防重复按钮一致化和字段级错误贴近输入框展示。
3. 答辩前：注入私有 smoke 账号环境变量，跑 `qt_server_integration_smoke` 并补登录后主链路截图。
