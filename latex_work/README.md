# 校园搭子需求分析报告 LaTeX 工程

本目录用于维护《校园搭子平台需求分析报告》的 LaTeX 版本。

## 文件说明

- `CampusBuddy_Requirements_Analysis_Report_Coded.docx`：原始内容来源。
- `tools/build_latex_report.py`：从 docx 抽取正文并生成 LaTeX 正文的脚本。
- `main.tex`：报告主模板，包含封面、目录、页眉页脚、标题样式、长表格样式和插图占位命令。
- `body.tex`：由脚本生成的正文内容。
- `build/body_raw.tex`：Pandoc 从 docx 直接导出的中间 LaTeX，便于排查原始转换结果。
- `assets/`：后续放置正式截图、建模图和流程图。

## 生成正文

```powershell
cd D:\big_homework
$env:PATH='C:\Users\zjx230494186\anaconda3\envs\campus_latex\Library\bin;C:\Users\zjx230494186\anaconda3\envs\campus_latex;' + $env:PATH
C:\Users\zjx230494186\anaconda3\envs\campus_latex\python.exe latex_work\tools\build_latex_report.py
```

## 编译 PDF

已创建专用 conda 环境 `campus_latex`，其中包含 `python`、`pandoc` 和 `tectonic`。推荐用 Tectonic 编译：

```powershell
cd D:\big_homework\latex_work
$env:PATH='C:\Users\zjx230494186\anaconda3\envs\campus_latex\Library\bin;C:\Users\zjx230494186\anaconda3\envs\campus_latex;' + $env:PATH
tectonic main.tex --keep-logs --keep-intermediates
```

输出文件为 `D:\big_homework\latex_work\main.pdf`。

## 插图替换方式

当前正文中的图片位置使用：

```latex
\FigurePlaceholder{图 1-1 问卷核心数据截图}{建议插入问卷报告中……}
```

拿到正式图片后，可以替换为：

```latex
\begin{figure}[htbp]
  \centering
  \includegraphics[width=0.92\textwidth]{assets/figure-1-1.png}
  \caption{图 1-1 问卷核心数据截图}
\end{figure}
```
