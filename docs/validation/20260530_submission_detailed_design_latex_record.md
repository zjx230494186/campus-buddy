# 2026-05-30 作业正式提交版详细设计 LaTeX 记录

## 本轮产物

1. `docs/30_submission_detailed_design_v1.md`
   1. 正式详细设计正文。
   2. 面向课程老师、助教和小组成员。
   3. 覆盖需求、架构、模块、流程、数据库、接口、权限、安全、前端、部署、测试和后续限制。
2. `deliverables/submission/submission_detailed_design_20260530.tex`
   1. 由 Markdown 正文通过 pandoc 生成的 LaTeX 源。
   2. 目标文档类为 `ctexart`。
   3. 列表结构使用 `enumerate`，未发现正文生成 `itemize`。
3. `deliverables/submission/submission_detailed_design_20260530.pdf`
   1. 已使用 Conda 环境 `campus_latex` 中的 Tectonic 生成。
   2. PDF 页数为 16 页。
   3. 当前存在长 API 路径、长表名和部分表格导致的 `Overfull hbox` 排版警告，需人工版式验收或进一步调表格列宽。

## 执行命令

```powershell
New-Item -ItemType Directory -Force deliverables\submission | Out-Null
pandoc docs/30_submission_detailed_design_v1.md -s -t latex --pdf-engine=xelatex -V documentclass=ctexart -V geometry:margin=2.5cm -V colorlinks=true -V linkcolor=blue -V urlcolor=blue -o deliverables/submission/submission_detailed_design_20260530.tex
Select-String -Path docs/30_submission_detailed_design_v1.md -Pattern '^\s*- '
Select-String -Path deliverables/submission/submission_detailed_design_20260530.tex -Pattern '\\begin\{itemize\}|\\item\s'
pandoc docs/30_submission_detailed_design_v1.md --pdf-engine=xelatex -V documentclass=ctexart -V geometry:margin=2.5cm -V colorlinks=true -V linkcolor=blue -V urlcolor=blue -o deliverables/submission/submission_detailed_design_20260530.pdf
& 'C:\Users\zjx230494186\anaconda3\envs\campus_latex\Library\bin\tectonic.exe' -p --keep-logs --outdir deliverables\submission deliverables\submission\submission_detailed_design_20260530.tex
```

## 验证结果

1. Markdown 正文检查：
   1. 未发现以 `- ` 开头的黑点列表。
2. LaTeX 源检查：
   1. 未发现 `\begin{itemize}`。
   2. 正文列表使用 `\begin{enumerate}` 和 `\item`。
3. PDF 编译：
   1. 当前本机 PATH 未发现 `xelatex/pdflatex/lualatex`。
   2. pandoc 使用 `xelatex` 编译 PDF 失败，错误为 `xelatex not found. Please select a different --pdf-engine or install xelatex`。
   3. 在 Conda 环境 `campus_latex` 中找到 `tectonic 0.16.9` 和 `pandoc 3.9.0.2`。
   4. 直接调用 `C:\Users\zjx230494186\anaconda3\envs\campus_latex\Library\bin\tectonic.exe` 成功生成 PDF。
   5. PDF 输出路径为 `deliverables/submission/submission_detailed_design_20260530.pdf`。
   6. 使用 `pypdf` 读取确认 PDF 为 16 页；文本抽取因中文字体编码显示不可靠，不作为版式判断依据。
4. 排版警告：
   1. Tectonic 日志中存在多处 `Overfull hbox`。
   2. 主要来源是长接口路径、长数据库表名和宽表格。
   3. 当前未安装或发现 `pdftoppm/pdfinfo`，未完成 PNG 渲染版式检查。

## 后续生成 PDF 的推荐命令

可直接使用现有 Conda 环境中的 Tectonic，在项目根目录执行：

```powershell
& 'C:\Users\zjx230494186\anaconda3\envs\campus_latex\Library\bin\tectonic.exe' -p --keep-logs --outdir deliverables\submission deliverables\submission\submission_detailed_design_20260530.tex
```

如后续安装可用的 XeLaTeX，也可执行：

```powershell
pandoc docs/30_submission_detailed_design_v1.md --pdf-engine=xelatex -V documentclass=ctexart -V geometry:margin=2.5cm -V colorlinks=true -V linkcolor=blue -V urlcolor=blue -o deliverables/submission/submission_detailed_design_20260530.pdf
```

或直接编译 LaTeX 源：

```powershell
xelatex -interaction=nonstopmode -halt-on-error -output-directory deliverables/submission deliverables/submission/submission_detailed_design_20260530.tex
```

## 边界确认

1. 未修改后端代码。
2. 未修改 Qt 代码。
3. 未修改 Flyway 迁移。
4. 未修改 deploy 脚本。
5. 未写入真实密码、验证码、token、SMTP 授权码、数据库密码、OBS AK/SK 或服务器私钥。
6. 未安装新的 LaTeX 工具链；仅使用已存在的 Conda 环境 `campus_latex`。
