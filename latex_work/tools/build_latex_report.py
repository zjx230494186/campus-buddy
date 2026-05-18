from __future__ import annotations

import re
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SOURCE_DOCX = ROOT / "CampusBuddy_Requirements_Analysis_Report_Coded.docx"
BODY_TEX = ROOT / "body.tex"

FIGURE_MAP = {
    "图 1-1 问卷核心数据截图": [
        "picture/用户附件.png",
        "picture/1-1.png",
    ],
    "图 5-2 总体用例图": ["picture/5-2.png"],
    "图 5-3 模块用例图一：需求发布与匹配": ["picture/5-3.png"],
    "图 5-4 模块用例图二：邀约与双向确认": ["picture/5-4.png"],
    "图 5-5 模块用例图三：信用评价与历史记录": ["picture/5-5.png"],
    "图 6-1 需求树总览图": ["picture/785092e90d6cbd4ba66ae0bac2c809f3.png"],
    "图 6-2 系统特性截图": ["picture/e9063c6533924a0086e104c235025ee6.png"],
    "图 6-3 研发需求截图": ["picture/6-3.png"],
    "图 6-4 用户故事截图": ["picture/6-4.png"],
}

FIGURE_CAPTION = {
    "图 6-1 需求树总览图": "图 6-1 原始需求截图",
}

FIGURE_CLEAR_BEFORE = {
    "图 5-2 总体用例图",
}

FIGURE_CLEAR_AFTER = {
    "图 5-5 模块用例图三：信用评价与历史记录",
}


def run(cmd: list[str]) -> None:
    subprocess.run(cmd, cwd=ROOT, check=True)


def x_columns(count: int) -> str:
    return "@{}" + "".join(
        [r">{\centering\arraybackslash}X" for _ in range(count)]
    ) + "@{}"


def normalize_latex_tables(text: str) -> str:
    def repl_begin(match: re.Match[str]) -> str:
        spec = match.group(1)
        count = len(re.findall(r"\\arraybackslash\}p\{", spec))
        if count == 0:
            count = sum(1 for ch in spec if ch in "lcrpmbX")
        count = max(count, 1)
        return rf"\begin{{xltabular}}{{\textwidth}}{{{x_columns(count)}}}"

    text = re.sub(
        r"\\begin\{longtable\}\[\]\{@\{\}([lcrpmbX]+)@\{\}\}",
        repl_begin,
        text,
    )
    text = re.sub(
        r"\\begin\{longtable\}\[\]\{@\{\}(.*?)@\{\}\}",
        repl_begin,
        text,
        flags=re.DOTALL,
    )
    text = text.replace(r"\end{longtable}", r"\end{xltabular}")
    return text


def normalize_placeholders(text: str) -> str:
    def clean_text(raw: str) -> tuple[str, str]:
        raw = raw.replace(r"\textbf{", "")
        raw = raw.replace(r"\strut", "")
        raw = raw.replace(r"\,", "")
        raw = raw.replace(r"\\", "\n")
        raw = raw.replace(r"\%", "%")
        raw = raw.replace("}", "")
        lines = [line.strip() for line in raw.splitlines() if line.strip()]
        title = lines[0].replace("【插图占位】", "").strip()
        hint = " ".join(lines[1:]).strip() if len(lines) > 1 else "待补充图片。"
        title = title.replace("%", r"\%").replace("&", r"\&")
        hint = hint.replace("%", r"\%").replace("&", r"\&")
        return title, hint

    def repl_raw_placeholder(match: re.Match[str]) -> str:
        title, hint = clean_text(match.group(1))
        return f"\n\\FigurePlaceholder{{{title}}}{{{hint}}}\n"

    def repl_table_placeholder(match: re.Match[str]) -> str:
        title, hint = clean_text(match.group(1))
        return f"\n\\FigurePlaceholder{{{title}}}{{{hint}}}\n"

    text = re.sub(
        r"\\begin\{longtable\}\[\]\{@\{\}l@\{\}\}\s*\\toprule\s*\\endhead\s*(\\textbf\{【插图占位】.*?\})\s*\\\\\s*\\bottomrule\s*\\end\{longtable\}",
        repl_raw_placeholder,
        text,
        flags=re.DOTALL,
    )
    text = re.sub(
        r"\{\\def\\LTcaptype\{none\}.*?\\begin\{longtable\}\[\].*?\\endhead\s*\\bottomrule\\noalign\{\}\s*\\endlastfoot\s*(\\textbf\{【插图占位】.*?\})\s*\\\\\s*\\end\{longtable\}\s*\}",
        repl_raw_placeholder,
        text,
        flags=re.DOTALL,
    )
    return text


def latex_figure(title: str, paths: list[str]) -> str:
    caption = FIGURE_CAPTION.get(title, title)
    graphics = []
    for path in paths:
        width = r"0.96\textwidth"
        graphics.append(rf"\includegraphics[width={width},height=0.78\textheight,keepaspectratio]{{{path}}}")
    body = "\n\\vspace{0.4em}\n".join(graphics)
    before = "\\clearpage\n" if title in FIGURE_CLEAR_BEFORE else ""
    after = "\\clearpage\n" if title in FIGURE_CLEAR_AFTER else ""
    return (
        "\n"
        f"{before}"
        "\\begin{figure}[H]\n"
        "\\centering\n"
        f"{body}\n"
        f"\\caption{{{caption}}}\n"
        "\\end{figure}\n"
        f"{after}"
    )


def replace_placeholder_tables(text: str) -> str:
    lines = text.splitlines()
    out: list[str] = []
    i = 0
    while i < len(lines):
        line = lines[i]
        if line.startswith(r"{\def\LTcaptype{none}"):
            block = [line]
            j = i + 1
            saw_end = False
            while j < len(lines):
                block.append(lines[j])
                if lines[j].strip() == r"\end{xltabular}":
                    saw_end = True
                elif saw_end and lines[j].strip() == "}":
                    break
                j += 1
            block_text = "\n".join(block)
            if "【插图占位】" in block_text:
                match = re.search(r"【插图占位】(图\s+\d+-\d+[^\n\\}]*)", block_text)
                if match:
                    title = match.group(1).strip()
                    if title in FIGURE_MAP:
                        out.extend(latex_figure(title, FIGURE_MAP[title]).splitlines())
                    # If there is no supplied image for this placeholder, drop it.
                    i = j + 1
                    continue
            out.extend(block)
            i = j + 1
            continue
        out.append(line)
        i += 1
    return "\n".join(out) + "\n"


def move_rr_figure_after_problem_table(text: str) -> str:
    target_image = "picture/785092e90d6cbd4ba66ae0bac2c809f3.png"
    target_caption = r"\caption{图 6-1 原始需求截图}"
    figure_start = text.find(r"\begin{figure}[H]")
    rr_figure = ""
    text_without_figure = text

    while figure_start != -1:
        figure_end = text.find(r"\end{figure}", figure_start)
        if figure_end == -1:
            break
        figure_end += len(r"\end{figure}")
        if figure_end < len(text) and text[figure_end] == "\n":
            figure_end += 1
        block = text[figure_start:figure_end]
        if target_image in block and target_caption in block:
            rr_figure = "\n" + block.strip() + "\n"
            text_without_figure = text[:figure_start] + "\n" + text[figure_end:]
            break
        figure_start = text.find(r"\begin{figure}[H]", figure_end)

    if not rr_figure:
        return text

    table_pattern = re.compile(
        r"(\{\\def\\LTcaptype\{none\} % do not increment counter\n"
        r"\\begin\{xltabular\}.*?"
        r"RR1 & 现有找搭子渠道分散且触达效率低.*?"
        r"RR6 & 平台缺少违规与纠纷治理抓手.*?"
        r"\\end\{xltabular\}\n"
        r"\})",
        flags=re.DOTALL,
    )
    return table_pattern.sub(
        lambda table_match: table_match.group(1) + rr_figure,
        text_without_figure,
        count=1,
    )


def insert_requirement_hierarchy_text(text: str) -> str:
    hierarchy_intro = (
        "为保证需求可追踪，本项目按``原始需求 - 系统特性 - 研发需求 - 用户故事 -\n"
        "验收标准''五层组织。原始需求定义问题域，系统特性定义能力域，研发需求定义系统应提供的能力边界，用户故事定义用户视角目标，验收标准定义可验证结果。\n"
    )
    hierarchy_rewrite = (
        "为保证需求可追踪，本项目在 CodeArts 工作项的需求管理中采用逐层分解的需求结构，而不是将 RR、SF、IR、US 作为互相并列的清单处理。整体逻辑是：先从调研和访谈中抽取原始需求（RR，Raw Requirement），明确用户痛点和业务问题；再将相近问题归并为系统特性（SF，System Feature），形成平台需要具备的能力域；随后把每个能力域细化为可开发、可验证的研发需求（IR，Initial Requirement）；最后从具体用户目标出发编写用户故事（US，User Story），并进一步连接验收标准。\n"
        "\n"
        "因此，本章的阅读顺序应理解为``问题来源 → 能力归并 → 开发项拆解 → 用户场景表达''。RR 说明为什么要做，SF 说明系统要覆盖哪些能力，IR 说明这些能力需要落成哪些实现条目，US 则说明用户将在什么场景下使用这些能力。后续各表格中的``承接问题''、``所属系统特性''和``对应需求''字段，均用于维持这一层级关系，避免需求在编写和实现过程中失去来源。\n"
    )
    text = text.replace(hierarchy_intro, hierarchy_rewrite, 1)

    sf_heading = r"\section{6.2 系统特性}\label{ux7cfbux7edfux7279ux6027}"
    sf_transition = (
        sf_heading
        + "\n\n"
        + "在完成 RR 层的问题定义后，系统特性用于把分散的问题归并为相对稳定的能力域。同一个系统特性可以承接多个原始需求，一个原始需求也可能通过不同特性共同解决。表中``承接问题''字段用于说明每个 SF 从哪些 RR 推导而来，优先级则体现该能力域对核心问题的支撑程度。"
    )
    text = text.replace(sf_heading, sf_transition, 1)

    module_heading = (
        "\\section{6.3\n"
        "功能模块总览}\\label{ux529fux80fdux6a21ux5757ux603bux89c8}"
    )
    module_transition = (
        module_heading
        + "\n\n"
        + "系统特性确定后，需要进一步落到产品与系统边界上。功能模块不是新的需求层级，而是对 SF 的工程化组织方式：它把能力域拆成可设计、可分工、可实现的模块，便于后续将研发需求挂接到具体模块和功能结构中。"
    )
    text = text.replace(module_heading, module_transition, 1)

    ir_heading = (
        "\\section{6.5\n"
        "研发需求总览}\\label{ux7814ux53d1ux9700ux6c42ux603bux89c8}"
    )
    ir_transition = (
        ir_heading
        + "\n\n"
        + "在 SF 明确系统能力边界后，IR 层将这些能力进一步拆解为开发团队可以实现和验证的需求条目。IR 不再描述抽象问题，而是描述系统应提供的功能、规则或处理机制；表中``所属系统特性''字段用于说明每条 IR 回溯到哪个能力域，从而保持从 RR 到 SF 再到 IR 的追踪链。"
    )
    text = text.replace(ir_heading, ir_transition, 1)

    us_heading = (
        "\\section{6.6\n"
        "用户故事总览}\\label{ux7528ux6237ux6545ux4e8bux603bux89c8}"
    )
    us_transition = (
        us_heading
        + "\n\n"
        + "用户故事用于把 IR 转换为用户视角下的目标、动机和收益表达。它不是对 IR 的重复罗列，而是说明具体角色为什么需要这些功能、在什么场景下使用这些功能，以及功能完成后应带来什么结果。表中``对应需求''字段用于把 US 与 IR 连接起来，后续验收标准可继续围绕这些用户故事展开。"
    )
    text = text.replace(us_heading, us_transition, 1)
    return text


def normalize_heading_levels(text: str) -> str:
    replacements = [
        (r"\subsubsection{", r"\TEMP_SUBSUB{"),
        (r"\subsection{", r"\TEMP_SUB{"),
        (r"\section{", r"\chapter{"),
        (r"\TEMP_SUB{", r"\section{"),
        (r"\TEMP_SUBSUB{", r"\subsection{"),
    ]
    for old, new in replacements:
        text = text.replace(old, new)
    return text


def clean_latex(text: str) -> str:
    text = text.replace("\r\n", "\n")

    # Drop Word-style title page metadata and the hand-written TOC. main.tex
    # owns the formal cover and the automatic table of contents.
    text = re.sub(
        r"^.*?(?=\\hypertarget\{ux6587ux6863ux63a7ux5236ux4fe1ux606f\})",
        "",
        text,
        flags=re.DOTALL,
    )
    text = re.sub(
        r"^.*?(?=\\section\{0 文档控制信息\})",
        "",
        text,
        flags=re.DOTALL,
    )

    text = text.replace(
        r"\section{术语、缩写与编号规则}",
        r"\subsection{0.1 术语、缩写与编号规则}",
    )

    # Remove empty headings introduced by direct docx editing.
    text = re.sub(
        r"\\hypertarget\{section\}\{%\s*\\subsection\{\}\\label\{section\}\}\s*",
        "",
        text,
        flags=re.DOTALL,
    )
    text = re.sub(
        r"\\hypertarget\{section-\d+\}\{%\s*\\subsection\{\}\\label\{section-\d+\}\}\s*",
        "",
        text,
        flags=re.DOTALL,
    )
    text = re.sub(
        r"\\section\{\}\\label\{section-\d+\}\s*",
        "",
        text,
    )

    text = normalize_placeholders(text)
    text = normalize_latex_tables(text)
    text = normalize_placeholders(text)
    text = replace_placeholder_tables(text)
    text = move_rr_figure_after_problem_table(text)
    text = insert_requirement_hierarchy_text(text)
    text = re.sub(r"\\section\{\}\\label\{section(?:-\d+)?\}\s*", "", text)
    text = normalize_heading_levels(text)
    text = re.sub(r"\\section\{\}\\label\{section(?:-\d+)?\}\s*", "", text)
    return text.strip() + "\n"


def main() -> None:
    build_dir = ROOT / "build"
    build_dir.mkdir(exist_ok=True)

    raw_tex = build_dir / "body_raw.tex"
    run(["pandoc", str(SOURCE_DOCX), "-t", "latex", "-o", str(raw_tex)])

    BODY_TEX.write_text(clean_latex(raw_tex.read_text(encoding="utf-8")), encoding="utf-8")


if __name__ == "__main__":
    main()
