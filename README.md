# Big Homework Project Workspace

这个目录用于承载接下来真实项目开发所需的项目文档、线程交接和可复用 prompt。

它的目标不是堆资料，而是让后续每个新线程都能：

- 快速读取当前项目状态
- 明确本线程该维护哪些文档
- 在结束前完成最小交接

## 目录导航

- `AGENTS.md`
  为什么存在：保存这个项目的高频、长期有效协作规则。
- `docs/`
  为什么存在：保存项目稳定信息、阶段计划和线程使用说明。
- `handoff/latest.md`
  为什么存在：保存当前线程最新可接手状态与下一线程 prompt。
- `prompts/new_thread_prompt_template.md`
  为什么存在：提供下一条项目线程可直接复制填写的启动 prompt。

## 新线程默认先读

- `AGENTS.md`
- `docs/00_project_map.md`
- `docs/03_current_plan.md`
- `handoff/latest.md`

如当前任务有直接相关材料，再额外补读对应文件，例如：

- 需求文档
- 原型图
- 课程材料
- 代码目录下相关实现文件

## 线程结束前默认维护

- `handoff/latest.md`
- `docs/03_current_plan.md`
- 与本线程直接相关的项目文档

## 当前已知输入材料

- `软件系统分析与设计课程作业02(4)(1).pptx`
