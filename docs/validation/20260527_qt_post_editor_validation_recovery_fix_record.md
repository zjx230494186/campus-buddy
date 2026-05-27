# 2026-05-27 Qt 发布编辑页校验失败后按钮恢复修复记录

## 问题

用户在发布帖子草稿后点击提交审核，服务端返回 `VALIDATION_FAILED` 字段缺失提示。随后用户补齐字段，但发布编辑页仍可能无法继续“更新草稿”或“提交审核”。

## 根因

- 后端当前草稿可编辑动作返回 `EDIT` + `SUBMIT_REVIEW`。
- Qt `PostEditorWidget` 只识别旧动作名 `UPDATE_DRAFT`，导致加载或回写草稿后“更新草稿”可能被错误禁用。
- 提交审核失败路径只恢复了“提交审核”按钮的 busy 状态，没有统一按当前草稿状态恢复“更新草稿 / 提交审核”两个动作按钮。

## 修复

- `PostEditorWidget` 增加当前草稿状态缓存和统一动作恢复逻辑。
- 草稿更新动作兼容 `EDIT` 与历史 `UPDATE_DRAFT`。
- 保存、更新、提交审核成功后统一根据返回草稿状态恢复按钮。
- 更新或提交审核失败后，保留字段级错误提示，同时恢复当前草稿可执行动作。
- 新增 `post_editor_widget_test`，覆盖：
  - 后端真实 `EDIT + SUBMIT_REVIEW` 动作下，加载草稿后可更新、可提交。
  - 提交审核返回 `VALIDATION_FAILED` 后，更新草稿和提交审核按钮恢复可用。
- 修正 `MyPartnerPostApiServiceTest` 中草稿响应样例，避免继续用过期 `UPDATE_DRAFT` 掩盖问题。

## 验证

本地 Qt 工具链：

```powershell
$env:PATH='G:\Qt\6.10.3\mingw_64\bin;G:\Qt\Tools\mingw1310_64\bin;G:\Qt\Tools\CMake_64\bin;G:\Qt\Tools\Ninja;' + $env:PATH
G:\Qt\Tools\CMake_64\bin\cmake.exe -S desktop -B desktop\build-qt6103-internal-release -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=G:\Qt\6.10.3\mingw_64
G:\Qt\Tools\CMake_64\bin\cmake.exe --build desktop\build-qt6103-internal-release --config Release
G:\Qt\Tools\CMake_64\bin\ctest.exe --test-dir desktop\build-qt6103-internal-release --output-on-failure
```

结果：

- `ctest`：11/11 passed。
- `post_editor_widget_test.exe`：passed。
- `my_partner_post_api_service_test.exe`：passed。
- `campus_buddy_desktop.exe --smoke-test`：passed。
- 第一次 `ctest` 曾因只构建了本轮目标，其他测试 exe 尚未生成而 Not Run；补齐全量构建后 11/11 passed。

交付包更新：

- 已备份旧包和旧解压目录：
  - `D:\big_homework\deliverables\internal_beta\backups\post_editor_fix_20260527_135738`
- 已用修复后的 Release exe 更新：
  - `D:\big_homework\deliverables\internal_beta\CampusBuddyInternalBeta_20260526\campus_buddy_desktop.exe`
- 已重新生成 zip：
  - `D:\big_homework\deliverables\internal_beta\CampusBuddyInternalBeta_20260526.zip`
- 剥离 Qt/CMake/MinGW PATH 后运行包内 exe：
  - `campus_buddy_desktop.exe --smoke-test` passed。
- zip 内容检查确认包含：
  - `campus_buddy_desktop.exe`
  - `Qt6Core.dll`
  - `Qt6Network.dll`
  - `platforms\qwindows.dll`
  - `README.txt`

服务器状态：

- `GET http://114.116.203.78/api/health` 返回 `{"status":"UP"}`。

## 边界

- 本轮只修复 Qt 桌面端。
- 未修改后端业务逻辑。
- 未修改 Flyway。
- 未修改 deploy 脚本。
- 未写入或泄露服务器私钥、数据库密码、OBS AK/SK、SMTP 授权码、验证码、token 或真实联系方式。
- 由于这是桌面端客户端 bug，服务器后端 jar 不需要重新部署；内测用户需要使用更新后的 zip。
