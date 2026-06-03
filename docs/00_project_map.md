# 00 Project Map

这份文档回答的不是“项目做了多少”，而是“项目信息应该放在哪里”。

## 信息分层

### 1. 稳定层

适合放长期不常变的信息：

- 项目目标
- 协作规则
- 文档使用规则
- 关键术语

主要位置：

- `AGENTS.md`
- `docs/01_project_brief.md`

### 2. 当前阶段层

适合放“现在最重要”的信息：

- 当前阶段主题
- 当前最关键目标
- 当前明确不做什么
- 进入下一阶段的条件

主要位置：

- `docs/03_current_plan.md`
- `handoff/latest.md`

### 3. 线程交接层

适合放当前线程最新状态：

- 已完成事项
- 未完成事项
- 下一线程建议
- 下一线程启动 prompt

主要位置：

- `handoff/latest.md`

### 4. 任务材料层

适合放具体任务输入材料、实现文件和验证产物：

- 需求草稿
- 课程材料
- 原型图
- 代码
- 输出结果

主要位置：

- 项目根目录下相关业务目录
- `docs/`

当前可直接交付外部协作者的任务文档：

- `docs/30_submission_detailed_design_v1.md`：课程作业正式提交版详细设计文档正文，面向老师、助教和小组成员，覆盖架构、模块、流程、数据库、接口、权限、部署、测试与限制；对应 LaTeX 源位于 `deliverables/submission/submission_detailed_design_20260530.tex`。
- `docs/mobile_api_reference_20260527.md`：移动端 UI 接入后端 API 文档，覆盖 base URL、鉴权、错误体、注册认证、发帖广场、对话联系、评价信用和管理员接口。

## 当前线程默认阅读清单

必读：

- `AGENTS.md`
- `docs/00_project_map.md`
- `docs/03_current_plan.md`
- `handoff/latest.md`

按任务补读：

- `docs/01_project_brief.md`
- 直接相关需求材料
- 直接相关代码与配置文件

## 当前线程默认维护清单

必维护：

- `handoff/latest.md`

视情况维护：

- `docs/03_current_plan.md`
- `docs/01_project_brief.md`
- 与当前任务直接相关的实现文档
