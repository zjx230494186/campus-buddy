# 负面用户行为测试矩阵 v1

> 本文档覆盖系统各模块的异常/边界/恶意输入场景，确保系统在非理想操作下仍能正确拒绝或优雅降级。

## 汇总统计

| 模块 | 用例数 | P0 | P1 | P2 | P3 |
|------|--------|-----|-----|-----|-----|
| Login/Register/Token | 22 | 6 | 8 | 6 | 2 |
| Identity verification | 16 | 4 | 6 | 4 | 2 |
| Post draft save | 17 | 5 | 6 | 4 | 2 |
| Post submit review | 28 | 10 | 10 | 6 | 2 |
| My posts list/detail/withdraw/unpublish | 16 | 4 | 6 | 4 | 2 |
| Admin post review | 12 | 3 | 4 | 3 | 2 |
| Plaza search/filter/detail/contact | 16 | 4 | 5 | 5 | 2 |
| Conversation/messaging | 22 | 6 | 8 | 6 | 2 |
| Contact card/unlock | 16 | 4 | 6 | 4 | 2 |
| Review/credit | 16 | 4 | 6 | 4 | 2 |
| Admin identity review | 11 | 3 | 4 | 2 | 2 |
| Network/HTTP errors | 16 | 5 | 5 | 4 | 2 |
| UI experience | 22 | 5 | 8 | 7 | 2 |
| **合计** | **230** | **53** | **82** | **59** | **24** |

---

## 1. Login/Register/Token（22 条）

| caseId | module | role | precondition | operation | input | expectedResult | automationStatus | priority |
|--------|--------|------|--------------|-----------|-------|----------------|-----------------|----------|
| LR-001 | Login/Register/Token | 未注册用户 | 无 | 注册时邮箱格式非法 | email: "abc@ | 返回 400，提示邮箱格式不合法 | AUTOMATED_RED | P0_BLOCKER |
| LR-002 | Login/Register/Token | 未注册用户 | 无 | 注册时密码为空 | password: "" | 返回 400，提示密码不能为空 | AUTOMATED_RED | P0_BLOCKER |
| LR-003 | Login/Register/Token | 未注册用户 | 无 | 注册时所有字段为空 | email:"", password:"", realName:"" | 返回 400，提示必填字段缺失 | AUTOMATED_RED | P0_BLOCKER |
| LR-004 | Login/Register/Token | 已注册用户 | 正确账号 | 登录时密码错误 | password: "wrongPwd123" | 返回 401，提示密码错误 | AUTOMATED_RED | P0_BLOCKER |
| LR-005 | Login/Register/Token | 已注册用户 | 正确账号 | 登录时邮箱不存在 | email: "nobody@test.com" | 返回 401，提示用户不存在 | AUTOMATED_RED | P0_BLOCKER |
| LR-006 | Login/Register/Token | 已登录用户 | Token 已过期 | 携带过期 Token 访问受保护接口 | Authorization: Bearer <expired> | 返回 401，提示 Token 已过期 | AUTOMATED_RED | P0_BLOCKER |
| LR-007 | Login/Register/Token | 已登录用户 | 有效 Token | 携带被篡改的 Token | Authorization: Bearer <tampered> | 返回 401，提示 Token 无效 | AUTOMATED_RED | P1_DEMO_RISK |
| LR-008 | Login/Register/Token | 已登录用户 | 有效 Token | Token 刷新接口传入无效 refreshToken | refreshToken: "invalid" | 返回 401，刷新失败 | AUTOMATED_RED | P1_DEMO_RISK |
| LR-009 | Login/Register/Token | 已登录用户 | 有效 Token | refreshToken 已过期时刷新 | refreshToken: <expired> | 返回 401，需重新登录 | AUTOMATED_RED | P1_DEMO_RISK |
| LR-010 | Login/Register/Token | 已登录用户 | 同一账号已在设备A登录 | 在设备B用同一账号登录 | 正常凭据 | 设备A Token 失效或允许多端登录（视业务策略） | AUTOMATED_GREEN | P1_DEMO_RISK |
| LR-011 | Login/Register/Token | 已登录用户 | 无 | 同一账号并发登录（10并发） | 相同凭据×10 | 仅允许合法登录，不产生重复 Token 泄漏 | AUTOMATED_GREEN | P1_DEMO_RISK |
| LR-012 | Login/Register/Token | 未注册用户 | 无 | 注册时邮箱已存在 | 已注册邮箱 | 返回 409，提示邮箱已注册 | AUTOMATED_RED | P1_DEMO_RISK |
| LR-013 | Login/Register/Token | 已登录用户 | 网络超时环境 | 登录请求触发网络超时 | 正常凭据，timeout=1ms | 客户端提示网络超时，不崩溃 | AUTOMATED_RED | P1_DEMO_RISK |
| LR-014 | Login/Register/Token | 已登录用户 | 服务端返回 401 | 携带 Token 请求但后端返回 401 | 任意受保护接口 | 客户端清除本地 Token，跳转登录页 | AUTOMATED_GREEN | P1_DEMO_RISK |
| LR-015 | Login/Register/Token | 未注册用户 | 无 | 注册时 realName 含特殊字符 | realName: "<script>alert(1)</script>" | 返回 400 或存储时转义，不执行 XSS | AUTOMATED_RED | P1_DEMO_RISK |
| LR-016 | Login/Register/Token | 已登录用户 | 有效 Token | 请求头不携带 Authorization | 无 Authorization 头 | 返回 401 | AUTOMATED_RED | P1_DEMO_RISK |
| LR-017 | Login/Register/Token | 已登录用户 | 有效 Token | 登出后用原 Token 请求 | 已登出的 Token | 返回 401 | AUTOMATED_GREEN | P2_UX |
| LR-018 | Login/Register/Token | 已登录用户 | 无 | 密码含前后空格 | password: "  pwd123  " | 服务端 trim 或拒绝，行为明确 | AUTOMATED_RED | P2_UX |
| LR-019 | Login/Register/Token | 已登录用户 | 无 | 注册时密码过短（<6位） | password: "abc" | 返回 400，提示密码长度不足 | AUTOMATED_RED | P2_UX |
| LR-020 | Login/Register/Token | 已登录用户 | 无 | 注册时学号格式非法 | studentNumber: "abc!@#" | 返回 400，提示学号格式不合法 | AUTOMATED_RED | P2_UX |
| LR-021 | Login/Register/Token | 已登录用户 | 无 | 连续5次密码错误后登录 | 错误密码×5 | 触发账号锁定或验证码，不无限尝试 | MANUAL_ONLY | P2_UX |
| LR-022 | Login/Register/Token | 已登录用户 | 无 | 注册接口 SQL 注入尝试 | email: "a'; DROP TABLE users;--" | 返回 400 或参数化查询拦截，不破坏数据 | AUTOMATED_RED | P3_BACKLOG |

## 2. Identity verification（16 条）

| caseId | module | role | precondition | operation | input | expectedResult | automationStatus | priority |
|--------|--------|------|--------------|-----------|-------|----------------|-----------------|----------|
| IV-001 | Identity verification | 未认证用户 | 无 | 提交认证时缺少姓名 | realName: null | 返回 400，提示姓名必填 | AUTOMATED_RED | P0_BLOCKER |
| IV-002 | Identity verification | 未认证用户 | 无 | 提交认证时缺少学号 | studentNumber: null | 返回 400，提示学号必填 | AUTOMATED_RED | P0_BLOCKER |
| IV-003 | Identity verification | 未认证用户 | 无 | 上传材料文件过大（>10MB） | file: 15MB图片 | 返回 400，提示文件过大 | AUTOMATED_RED | P0_BLOCKER |
| IV-004 | Identity verification | 未认证用户 | 无 | 上传材料文件类型非法 | file: .exe文件 | 返回 400，提示文件类型不支持 | AUTOMATED_RED | P0_BLOCKER |
| IV-005 | Identity verification | 未认证用户 | 无 | 提交认证时未上传任何材料 | materialAttachmentIds: [] | 返回 400，提示至少上传一份材料 | AUTOMATED_RED | P1_DEMO_RISK |
| IV-006 | Identity verification | 已认证用户 | authenticationStatus=VERIFIED | 重复提交认证申请 | 完整认证数据 | 返回 409 或 400，提示已认证不可重复提交 | AUTOMATED_RED | P1_DEMO_RISK |
| IV-007 | Identity verification | 已认证用户 | authenticationStatus=PENDING | 认证审核中再次提交 | 完整认证数据 | 返回 409，提示审核中不可重复提交 | AUTOMATED_RED | P1_DEMO_RISK |
| IV-008 | Identity verification | 普通用户 | 存在他人材料 | 尝试删除他人上传的材料 | 其他用户的 materialAttachmentId | 返回 403，无权操作 | AUTOMATED_RED | P1_DEMO_RISK |
| IV-009 | Identity verification | 未认证用户 | 无 | 上传材料文件名为空 | file: 无文件名的流 | 返回 400，提示文件名非法 | AUTOMATED_RED | P1_DEMO_RISK |
| IV-010 | Identity verification | 未认证用户 | 无 | 上传材料图片分辨率过低 | file: 1×1像素图片 | 返回 400 或接受但提示模糊（视业务） | MANUAL_ONLY | P1_DEMO_RISK |
| IV-011 | Identity verification | 未认证用户 | 无 | 上传材料为恶意 SVG（含脚本） | file: evil.svg | 服务端拒绝或剥离脚本，不执行 | AUTOMATED_RED | P2_UX |
| IV-012 | Identity verification | 未认证用户 | 无 | 上传材料并发10次 | 同一文件×10 | 不产生重复附件或仅保留最后一份 | AUTOMATED_GREEN | P2_UX |
| IV-013 | Identity verification | 未认证用户 | 已上传3份材料 | 上传第4份超出限制 | file: 第4份 | 返回 400，提示材料数量超限 | AUTOMATED_RED | P2_UX |
| IV-014 | Identity verification | 未认证用户 | 无 | 上传文件名含路径穿越 | filename: "../../etc/passwd" | 返回 400，不泄露文件系统信息 | AUTOMATED_RED | P2_UX |
| IV-015 | Identity verification | 未认证用户 | 无 | 提交认证时学号已被他人使用 | 已被其他用户占用的学号 | 返回 409，提示学号已被占用 | AUTOMATED_RED | P3_BACKLOG |
| IV-016 | Identity verification | 未认证用户 | 无 | 上传材料后网络中断 | 上传中途断网 | 客户端提示上传失败，不残留脏数据 | MANUAL_ONLY | P3_BACKLOG |

## 3. Post draft save（17 条）

| caseId | module | role | precondition | operation | input | expectedResult | automationStatus | priority |
|--------|--------|------|--------------|-----------|-------|----------------|-----------------|----------|
| DS-001 | Post draft save | 已认证用户 | 无 | 保存草稿时标题为空 | title: "" | 返回 400，提示标题不能为空 | AUTOMATED_RED | P0_BLOCKER |
| DS-002 | Post draft save | 已认证用户 | 无 | 保存草稿时标题超长（>100字） | title: 101字字符串 | 返回 400，提示标题长度超限 | AUTOMATED_RED | P0_BLOCKER |
| DS-003 | Post draft save | 已认证用户 | 无 | 保存草稿时描述为空 | description: "" | 返回 400，提示描述不能为空 | AUTOMATED_RED | P0_BLOCKER |
| DS-004 | Post draft save | 已认证用户 | 无 | 保存草稿时 sceneType 非法 | sceneType: "INVALID_TYPE" | 返回 400，提示场景类型不合法 | AUTOMATED_RED | P0_BLOCKER |
| DS-005 | Post draft save | 已认证用户 | sceneType=MEAL | MEAL场景缺少 canteen 字段 | scenePayload: {无canteen} | 返回 400，提示 MEAL 场景必须填写食堂 | AUTOMATED_RED | P0_BLOCKER |
| DS-006 | Post draft save | 已认证用户 | sceneType=STUDY | STUDY场景缺少 studyGoal 字段 | scenePayload: {无studyGoal} | 返回 400，提示 STUDY 场景必须填写学习目标 | AUTOMATED_RED | P0_BLOCKER |
| DS-007 | Post draft save | 已认证用户 | sceneType=SPORT | SPORT场景缺少 sportType 字段 | scenePayload: {无sportType} | 返回 400，提示 SPORT 场景必须填写运动类型 | AUTOMATED_RED | P1_DEMO_RISK |
| DS-008 | Post draft save | 已认证用户 | sceneType=COURSE_TEAM | COURSE_TEAM场景缺少 courseName 字段 | scenePayload: {无courseName} | 返回 400，提示课程组队必须填写课程名称 | AUTOMATED_RED | P1_DEMO_RISK |
| DS-009 | Post draft save | 已认证用户 | sceneType=INNOVATION_PROJECT | INNOVATION_PROJECT场景缺少 projectDirection 字段 | scenePayload: {无projectDirection} | 返回 400，提示创新项目必须填写项目方向 | AUTOMATED_RED | P1_DEMO_RISK |
| DS-010 | Post draft save | 已认证用户 | 无 | 正文含联系方式（手机号） | description: "联系13800138000" | 返回 400 VALIDATION_FAILED，提示正文禁止包含联系方式 | AUTOMATED_RED | P1_DEMO_RISK |
| DS-011 | Post draft save | 已认证用户 | 无 | 正文含微信号 | description: "加微信abc123" | 返回 400 VALIDATION_FAILED，提示正文禁止包含联系方式 | AUTOMATED_RED | P1_DEMO_RISK |
| DS-012 | Post draft save | 已认证用户 | 无 | 正文含QQ号 | description: "QQ123456789" | 返回 400 VALIDATION_FAILED，提示正文禁止包含联系方式 | AUTOMATED_RED | P1_DEMO_RISK |
| DS-013 | Post draft save | 未认证用户 | authenticationStatus≠VERIFIED | 尝试保存草稿 | 合法草稿数据 | 返回 403，提示未认证不可发帖 | AUTOMATED_RED | P1_DEMO_RISK |
| DS-014 | Post draft save | 已认证用户 | 无 | description 超长（>2000字） | description: 2001字 | 返回 400，提示描述长度超限 | AUTOMATED_RED | P2_UX |
| DS-015 | Post draft save | 已认证用户 | 无 | contactPreference 非法值 | contactPreference: "INVALID" | 返回 400，提示联系方式偏好不合法 | AUTOMATED_RED | P2_UX |
| DS-016 | Post draft save | 已认证用户 | 无 | 并发保存同一草稿 | 同一草稿ID并发PUT×5 | 返回 409 或最终一致，不数据损坏 | AUTOMATED_GREEN | P2_UX |
| DS-017 | Post draft save | 已认证用户 | 无 | sceneType 为 null | sceneType: null | 返回 400，提示场景类型必填 | AUTOMATED_RED | P3_BACKLOG |

## 4. Post submit review（28 条）

| caseId | module | role | precondition | operation | input | expectedResult | automationStatus | priority |
|--------|--------|------|--------------|-----------|-------|----------------|-----------------|----------|
| SR-001 | Post submit review | 已认证用户 | 草稿标题为空 | 提交审核 | title: "" | 返回 400 VALIDATION_FAILED，details.title 提示不能为空 | AUTOMATED_RED | P0_BLOCKER |
| SR-002 | Post submit review | 已认证用户 | 草稿标题超长 | 提交审核 | title: 101字 | 返回 400 VALIDATION_FAILED，details.title 提示长度超限 | AUTOMATED_RED | P0_BLOCKER |
| SR-003 | Post submit review | 已认证用户 | 草稿描述为空 | 提交审核 | description: "" | 返回 400 VALIDATION_FAILED，details.description 提示不能为空 | AUTOMATED_RED | P0_BLOCKER |
| SR-004 | Post submit review | 已认证用户 | 草稿描述超长 | 提交审核 | description: 2001字 | 返回 400 VALIDATION_FAILED，details.description 提示长度超限 | AUTOMATED_RED | P0_BLOCKER |
| SR-005 | Post submit review | 已认证用户 | sceneType=STUDY 缺少 studyGoal | 提交审核 | scenePayload: {无studyGoal} | 返回 400 VALIDATION_FAILED，details.scenePayload.studyGoal 提示必填 | AUTOMATED_RED | P0_BLOCKER |
| SR-006 | Post submit review | 已认证用户 | sceneType=MEAL 缺少 canteen | 提交审核 | scenePayload: {无canteen} | 返回 400 VALIDATION_FAILED，details.scenePayload.canteen 提示必填 | AUTOMATED_RED | P0_BLOCKER |
| SR-007 | Post submit review | 已认证用户 | contactPreference 为空或非法 | 提交审核 | contactPreference: null | 返回 400 VALIDATION_FAILED，details.contactPreference 提示必填 | AUTOMATED_RED | P0_BLOCKER |
| SR-008 | Post submit review | 已认证用户 | 帖子状态=PENDING_REVIEW | 再次提交审核 | 已在审核中的帖子 | 返回 409 POST_STATUS_CONFLICT，提示帖子已在审核中 | AUTOMATED_RED | P0_BLOCKER |
| SR-009 | Post submit review | 已认证用户 | 帖子状态=PUBLISHED | 尝试提交审核 | 已发布的帖子 | 返回 409 POST_STATUS_CONFLICT，提示已发布帖子不可提交 | AUTOMATED_RED | P0_BLOCKER |
| SR-010 | Post submit review | 已认证用户 | 帖子状态=REJECTED | 尝试提交审核（未修改） | 被拒绝的帖子 | 返回 409 POST_STATUS_CONFLICT，提示需修改后重新提交 | AUTOMATED_RED | P0_BLOCKER |
| SR-011 | Post submit review | 未认证用户 | authenticationStatus≠VERIFIED | 尝试提交审核 | 合法草稿 | 返回 403，提示未认证不可提交 | AUTOMATED_RED | P0_BLOCKER |
| SR-012 | Post submit review | 已认证用户 | 提交审核时网络错误 | 点击提交按钮 | 正常草稿数据，网络断开 | 客户端提示网络错误，帖子状态保持 DRAFT 不变 | AUTOMATED_RED | P1_DEMO_RISK |
| SR-013 | Post submit review | 已认证用户 | 提交审核按钮可点击 | 双击提交按钮 | 快速连续点击2次 | 仅提交1次，第2次被防重复机制拦截 | AUTOMATED_RED | P1_DEMO_RISK |
| SR-014 | Post submit review | 已认证用户 | 服务端返回 500 | 提交审核 | 正常数据，服务端异常 | 客户端提示服务器错误，不改变本地状态 | AUTOMATED_RED | P1_DEMO_RISK |
| SR-015 | Post submit review | 已认证用户 | 草稿正文含联系方式 | 提交审核 | description含手机号 | 返回 400 VALIDATION_FAILED，提示正文禁止包含联系方式 | AUTOMATED_RED | P1_DEMO_RISK |
| SR-016 | Post submit review | 已认证用户 | sceneType=SPORT 缺少 sportType | 提交审核 | scenePayload: {无sportType} | 返回 400 VALIDATION_FAILED，details.scenePayload.sportType 提示必填 | AUTOMATED_RED | P1_DEMO_RISK |
| SR-017 | Post submit review | 已认证用户 | sceneType=COURSE_TEAM 缺少 courseName | 提交审核 | scenePayload: {无courseName} | 返回 400 VALIDATION_FAILED，details.scenePayload.courseName 提示必填 | AUTOMATED_RED | P1_DEMO_RISK |
| SR-018 | Post submit review | 已认证用户 | sceneType=INNOVATION_PROJECT 缺少 projectDirection | 提交审核 | scenePayload: {无projectDirection} | 返回 400 VALIDATION_FAILED，details.scenePayload.projectDirection 提示必填 | AUTOMATED_RED | P1_DEMO_RISK |
| SR-019 | Post submit review | 已认证用户 | 返回 VALIDATION_FAILED | 提交审核失败 | 多字段不合法 | UI 显示字段级错误信息，至少显示 errorCode+errorMessage | AUTOMATED_GREEN | P1_DEMO_RISK |
| SR-020 | Post submit review | 已认证用户 | 帖子状态=CLOSED | 尝试提交审核 | 已关闭帖子 | 返回 409 POST_STATUS_CONFLICT | AUTOMATED_RED | P1_DEMO_RISK |
| SR-021 | Post submit review | 已认证用户 | 无 | 提交他人草稿 | 其他用户的postId | 返回 403，无权操作 | AUTOMATED_RED | P1_DEMO_RISK |
| SR-022 | Post submit review | 已认证用户 | 无 | 提交不存在的帖子 | postId: 999999 | 返回 404，帖子不存在 | AUTOMATED_RED | P1_DEMO_RISK |
| SR-023 | Post submit review | 已认证用户 | 无 | 提交时 body 为空 JSON | {} | 返回 400，提示必填字段缺失 | AUTOMATED_RED | P2_UX |
| SR-024 | Post submit review | 已认证用户 | 无 | sceneType 为空提交 | sceneType: null | 返回 400 VALIDATION_FAILED，提示场景类型必填 | AUTOMATED_RED | P2_UX |
| SR-025 | Post submit review | 已认证用户 | 无 | 提交时附加多余未知字段 | ...+unknownField: "x" | 服务端忽略或拒绝，不产生副作用 | AUTOMATED_RED | P2_UX |
| SR-026 | Post submit review | 已认证用户 | 无 | 提交审核后立即返回再提交 | 短时间内2次提交 | 第2次返回 409 POST_STATUS_CONFLICT | AUTOMATED_GREEN | P2_UX |
| SR-027 | Post submit review | 已认证用户 | 无 | VALIDATION_FAILED 时 UI 不显示错误 | 提交不合法数据 | UI 必须至少显示 errorCode + errorMessage，不能静默失败 | MANUAL_ONLY | P3_BACKLOG |
| SR-028 | Post submit review | 已认证用户 | 无 | scenePayload 含 sceneType 不需要的字段 | MEAL场景传入studyGoal | 服务端忽略多余字段或拒绝，行为明确 | AUTOMATED_RED | P3_BACKLOG |

## 5. My posts list/detail/withdraw/unpublish（16 条）

| caseId | module | role | precondition | operation | input | expectedResult | automationStatus | priority |
|--------|--------|------|--------------|-----------|-------|----------------|-----------------|----------|
| MP-001 | My posts | 已认证用户 | 帖子状态=DRAFT | 尝试撤回 | DRAFT帖子 | 返回 409 POST_STATUS_CONFLICT，DRAFT 不可撤回 | AUTOMATED_RED | P0_BLOCKER |
| MP-002 | My posts | 已认证用户 | 帖子状态=PUBLISHED | 尝试撤回 | PUBLISHED帖子 | 返回 409 POST_STATUS_CONFLICT，PUBLISHED 不可撤回 | AUTOMATED_RED | P0_BLOCKER |
| MP-003 | My posts | 已认证用户 | 帖子状态=DRAFT | 尝试取消发布 | DRAFT帖子 | 返回 409 POST_STATUS_CONFLICT，DRAFT 不可取消发布 | AUTOMATED_RED | P0_BLOCKER |
| MP-004 | My posts | 已认证用户 | 帖子状态=PENDING_REVIEW | 尝试取消发布 | PENDING_REVIEW帖子 | 返回 409 POST_STATUS_CONFLICT | AUTOMATED_RED | P0_BLOCKER |
| MP-005 | My posts | 已认证用户 | 帖子状态=PENDING_REVIEW | 撤回帖子 | 合法操作 | 帖子状态变为 DRAFT | AUTOMATED_GREEN | P1_DEMO_RISK |
| MP-006 | My posts | 已认证用户 | 帖子状态=PUBLISHED | 取消发布 | 合法操作 | 帖子状态变为 DRAFT 或 CLOSED（视业务） | AUTOMATED_GREEN | P1_DEMO_RISK |
| MP-007 | My posts | 已认证用户 | 无 | 查看他人帖子详情 | 其他用户的postId | 返回 403 或仅返回公开字段 | AUTOMATED_RED | P1_DEMO_RISK |
| MP-008 | My posts | 已认证用户 | 无 | 撤回他人帖子 | 其他用户的postId | 返回 403，无权操作 | AUTOMATED_RED | P1_DEMO_RISK |
| MP-009 | My posts | 已认证用户 | 无 | 取消发布他人帖子 | 其他用户的postId | 返回 403，无权操作 | AUTOMATED_RED | P1_DEMO_RISK |
| MP-010 | My posts | 已认证用户 | 帖子列表为空 | 查看我的帖子 | 无帖子 | UI 显示空状态提示 | MANUAL_ONLY | P1_DEMO_RISK |
| MP-011 | My posts | 已认证用户 | 帖子状态=CLOSED | 尝试撤回 | CLOSED帖子 | 返回 409 POST_STATUS_CONFLICT | AUTOMATED_RED | P2_UX |
| MP-012 | My posts | 已认证用户 | 帖子状态=CLOSED | 尝试取消发布 | CLOSED帖子 | 返回 409 POST_STATUS_CONFLICT | AUTOMATED_RED | P2_UX |
| MP-013 | My posts | 已认证用户 | 无 | 查看不存在的帖子详情 | postId: 999999 | 返回 404 | AUTOMATED_RED | P2_UX |
| MP-014 | My posts | 已认证用户 | 无 | 列表分页越界 | page=999, size=10 | 返回空列表 | AUTOMATED_RED | P2_UX |
| MP-015 | My posts | 已认证用户 | 无 | 并发撤回和取消发布 | 同一帖子同时撤回+取消发布 | 返回 409 或仅一个操作成功 | AUTOMATED_GREEN | P3_BACKLOG |
| MP-016 | My posts | 已认证用户 | 帖子状态=REJECTED | 尝试撤回 | REJECTED帖子 | 返回 409 POST_STATUS_CONFLICT | AUTOMATED_RED | P3_BACKLOG |

## 6. Admin post review（12 条）

| caseId | module | role | precondition | operation | input | expectedResult | automationStatus | priority |
|--------|--------|------|--------------|-----------|-------|----------------|-----------------|----------|
| AR-001 | Admin post review | 管理员 | 帖子状态=PUBLISHED | 尝试审核已发布帖子 | approve操作 | 返回 409 POST_STATUS_CONFLICT | AUTOMATED_RED | P0_BLOCKER |
| AR-002 | Admin post review | 管理员 | 帖子状态=REJECTED | 再次审核已拒绝帖子 | approve操作 | 返回 409 POST_STATUS_CONFLICT | AUTOMATED_RED | P0_BLOCKER |
| AR-003 | Admin post review | 管理员 | 帖子状态=DRAFT | 审核草稿帖子 | approve操作 | 返回 409 POST_STATUS_CONFLICT | AUTOMATED_RED | P0_BLOCKER |
| AR-004 | Admin post review | 管理员 | 帖子状态=PENDING_REVIEW | 拒绝时不填原因 | reject操作, reason: null | 返回 400，提示拒绝原因必填 | AUTOMATED_RED | P0_BLOCKER |
| AR-005 | Admin post review | 管理员 | 帖子状态=PENDING_REVIEW | 正常通过 | approve操作 | 帖子状态变为 PUBLISHED | AUTOMATED_GREEN | P1_DEMO_RISK |
| AR-006 | Admin post review | 管理员 | 帖子状态=PENDING_REVIEW | 正常拒绝并填原因 | reject操作, reason: "内容不当" | 帖子状态变为 REJECTED | AUTOMATED_GREEN | P1_DEMO_RISK |
| AR-007 | Admin post review | 普通用户 | 无 | 普通用户尝试审核帖子 | approve操作 | 返回 403，无管理员权限 | AUTOMATED_RED | P1_DEMO_RISK |
| AR-008 | Admin post review | 管理员 | 无 | 审核不存在的帖子 | postId: 999999 | 返回 404 | AUTOMATED_RED | P1_DEMO_RISK |
| AR-009 | Admin post review | 管理员 | 帖子状态=CLOSED | 审核已关闭帖子 | approve操作 | 返回 409 POST_STATUS_CONFLICT | AUTOMATED_RED | P2_UX |
| AR-010 | Admin post review | 管理员 | 帖子状态=PENDING_REVIEW | 并发审核同一帖子 | 两个管理员同时approve | 仅一个成功，另一个返回 409 | AUTOMATED_GREEN | P2_UX |
| AR-011 | Admin post review | 管理员 | 无 | 拒绝原因超长 | reason: 500字 | 返回 400，提示原因长度超限 | AUTOMATED_RED | P3_BACKLOG |
| AR-012 | Admin post review | 管理员 | 帖子状态=PENDING_REVIEW | 审核时携带非法 action | action: "invalid" | 返回 400，提示操作类型不合法 | AUTOMATED_RED | P3_BACKLOG |

## 7. Plaza search/filter/detail/contact（16 条）

| caseId | module | role | precondition | operation | input | expectedResult | automationStatus | priority |
|--------|--------|------|--------------|-----------|-------|----------------|-----------------|----------|
| PL-001 | Plaza | 已认证用户 | 无 | 搜索关键词为空 | keyword: "" | 返回全部帖子或提示请输入关键词 | AUTOMATED_RED | P0_BLOCKER |
| PL-002 | Plaza | 已认证用户 | 无 | 过滤 sceneType 非法 | sceneType: "INVALID" | 返回 400，提示场景类型不合法 | AUTOMATED_RED | P0_BLOCKER |
| PL-003 | Plaza | 已认证用户 | 无 | 查看帖子详情 postId 不存在 | postId: 999999 | 返回 404 | AUTOMATED_RED | P0_BLOCKER |
| PL-004 | Plaza | 已认证用户 | 帖子状态=CLOSED | 联系已关闭帖子 | contact操作 | 返回 409 或提示帖子已关闭不可联系 | AUTOMATED_RED | P0_BLOCKER |
| PL-005 | Plaza | 已认证用户 | 无 | 联系自己的帖子 | 自己的postId | 返回 400，不可联系自己 | AUTOMATED_RED | P1_DEMO_RISK |
| PL-006 | Plaza | 已认证用户 | 无 | 搜索关键词含特殊字符 | keyword: "<script>" | 正常处理或拒绝，不执行 XSS | AUTOMATED_RED | P1_DEMO_RISK |
| PL-007 | Plaza | 已认证用户 | 无 | 搜索关键词超长 | keyword: 500字 | 返回 400 或截断处理 | AUTOMATED_RED | P1_DEMO_RISK |
| PL-008 | Plaza | 未认证用户 | authenticationStatus≠VERIFIED | 浏览广场 | 无 | 可浏览但联系时提示需认证 | AUTOMATED_GREEN | P1_DEMO_RISK |
| PL-009 | Plaza | 已认证用户 | 无 | 分页参数越界 | page=-1 | 返回 400，提示分页参数非法 | AUTOMATED_RED | P2_UX |
| PL-010 | Plaza | 已认证用户 | 无 | pageSize 过大 | size=10000 | 返回 400 或限制最大值 | AUTOMATED_RED | P2_UX |
| PL-011 | Plaza | 已认证用户 | 无 | 多个过滤条件组合无结果 | sceneType=MEAL + keyword="不存在的词" | 返回空列表，UI 显示无结果 | AUTOMATED_GREEN | P2_UX |
| PL-012 | Plaza | 已认证用户 | 帖子状态=DRAFT | 尝试查看草稿帖子详情 | DRAFT的postId | 返回 404，草稿不在广场展示 | AUTOMATED_RED | P2_UX |
| PL-013 | Plaza | 已认证用户 | 无 | 快速切换筛选条件 | 连续切换5种sceneType | 最终结果与最后一次筛选一致 | MANUAL_ONLY | P2_UX |
| PL-014 | Plaza | 已认证用户 | 无 | 联系帖子时网络超时 | 正常postId，网络timeout | 客户端提示超时，不创建重复会话 | AUTOMATED_RED | P3_BACKLOG |
| PL-015 | Plaza | 已认证用户 | 无 | 联系同一帖子两次 | 同一postId联系2次 | 返回已有会话ID，不创建重复会话 | AUTOMATED_GREEN | P3_BACKLOG |
| PL-016 | Plaza | 已认证用户 | 无 | 排序参数非法 | sort: "invalid_sort" | 返回 400 或使用默认排序 | AUTOMATED_RED | P3_BACKLOG |

## 8. Conversation/messaging（22 条）

| caseId | module | role | precondition | operation | input | expectedResult | automationStatus | priority |
|--------|--------|------|--------------|-----------|-------|----------------|-----------------|----------|
| CM-001 | Conversation/messaging | 已认证用户 | 会话状态=CLOSED | 发送消息 | 正常消息内容 | 返回 409，提示会话已关闭 | AUTOMATED_RED | P0_BLOCKER |
| CM-002 | Conversation/messaging | 已认证用户 | 会话状态=OPEN | 发送空消息 | content: "" | 返回 400，提示消息不能为空 | AUTOMATED_RED | P0_BLOCKER |
| CM-003 | Conversation/messaging | 已认证用户 | 会话状态=OPEN | 发送超长消息 | content: 5001字 | 返回 400，提示消息长度超限 | AUTOMATED_RED | P0_BLOCKER |
| CM-004 | Conversation/messaging | 已认证用户 | 无 | 查询消息列表 afterMessageId 非法 | afterMessageId: "not_a_number" | 返回 400，提示参数格式非法 | AUTOMATED_RED | P0_BLOCKER |
| CM-005 | Conversation/messaging | 已认证用户 | 会话状态=CLOSED | 关闭已关闭会话 | close操作 | 返回 409，提示会话已关闭 | AUTOMATED_RED | P0_BLOCKER |
| CM-006 | Conversation/messaging | 已认证用户 | 会话状态=OPEN | 重新打开非关闭的会话 | reopen操作 | 返回 409，提示会话未关闭 | AUTOMATED_RED | P0_BLOCKER |
| CM-007 | Conversation/messaging | 已认证用户 | 无 | 标记已读但无未读消息 | markRead操作 | 成功但无副作用 | AUTOMATED_GREEN | P1_DEMO_RISK |
| CM-008 | Conversation/messaging | 已认证用户 | 无 | 查询不存在的会话 | conversationId: 999999 | 返回 404 | AUTOMATED_RED | P1_DEMO_RISK |
| CM-009 | Conversation/messaging | 已认证用户 | 无 | 发送消息到他人会话（非参与者） | 其他用户的conversationId | 返回 403，无权操作 | AUTOMATED_RED | P1_DEMO_RISK |
| CM-010 | Conversation/messaging | 已认证用户 | 会话状态=OPEN | 发送消息含 XSS 脚本 | content: "<script>alert(1)</script>" | 消息存储但前端转义显示，不执行脚本 | AUTOMATED_RED | P1_DEMO_RISK |
| CM-011 | Conversation/messaging | 已认证用户 | 会话状态=OPEN | 快速连续发送100条消息 | 短时间内100条 | 触发限流，超出部分返回 429 | AUTOMATED_GREEN | P1_DEMO_RISK |
| CM-012 | Conversation/messaging | 已认证用户 | 会话状态=OPEN | 关闭会话 | close操作 | 会话状态变为 CLOSED | AUTOMATED_GREEN | P1_DEMO_RISK |
| CM-013 | Conversation/messaging | 已认证用户 | 会话状态=CLOSED | 重新打开已关闭会话 | reopen操作 | 会话状态变为 OPEN | AUTOMATED_GREEN | P1_DEMO_RISK |
| CM-014 | Conversation/messaging | 已认证用户 | 无 | afterMessageId 大于最大消息ID | afterMessageId: 999999 | 返回空消息列表 | AUTOMATED_RED | P2_UX |
| CM-015 | Conversation/messaging | 已认证用户 | 会话状态=OPEN | 发送纯空格消息 | content: "   " | 返回 400，提示消息不能为空 | AUTOMATED_RED | P2_UX |
| CM-016 | Conversation/messaging | 已认证用户 | 无 | 会话列表分页参数非法 | page=-1 | 返回 400 | AUTOMATED_RED | P2_UX |
| CM-017 | Conversation/messaging | 已认证用户 | 无 | 标记他人会话已读 | 其他用户的conversationId | 返回 403 | AUTOMATED_RED | P2_UX |
| CM-018 | Conversation/messaging | 已认证用户 | 会话状态=OPEN | 并发发送消息 | 5并发同时发送 | 全部成功或按序处理，不丢失 | AUTOMATED_GREEN | P2_UX |
| CM-019 | Conversation/messaging | 已认证用户 | 无 | 删除不存在的消息 | messageId: 999999 | 返回 404 | AUTOMATED_RED | P3_BACKLOG |
| CM-020 | Conversation/messaging | 已认证用户 | 无 | 消息含非法文件附件 | attachment: .exe | 返回 400，提示文件类型不支持 | AUTOMATED_RED | P3_BACKLOG |
| CM-021 | Conversation/messaging | 已认证用户 | 无 | 网络中断时发送消息 | 网络断开 | 客户端提示发送失败，允许重试 | MANUAL_ONLY | P3_BACKLOG |
| CM-022 | Conversation/messaging | 已认证用户 | 会话状态=OPEN | 发送消息时服务端返回 500 | 正常消息，服务端异常 | 客户端提示服务器错误，消息不丢失（可重试） | AUTOMATED_RED | P3_BACKLOG |

## 9. Contact card/unlock（16 条）

| caseId | module | role | precondition | operation | input | expectedResult | automationStatus | priority |
|--------|--------|------|--------------|-----------|-------|----------------|-----------------|----------|
| CC-001 | Contact card/unlock | 已认证用户 | 无 | 保存名片时手机号填入 wechatId 字段 | wechatId: "13800138000" | 返回 400，提示微信号格式不合法 | AUTOMATED_RED | P0_BLOCKER |
| CC-002 | Contact card/unlock | 已认证用户 | 无 | 确认名片但未填写名片信息 | 空名片确认 | 返回 400，提示需先填写名片 | AUTOMATED_RED | P0_BLOCKER |
| CC-003 | Contact card/unlock | 已认证用户 | 对方名片状态=LOCKED | 查看对方名片 | conversationId | 返回锁定状态，不展示联系方式详情 | AUTOMATED_RED | P0_BLOCKER |
| CC-004 | Contact card/unlock | 已认证用户 | 会话状态=CLOSED | 查看对方名片 | CLOSED会话 | 返回 409 或仅展示锁定状态 | AUTOMATED_RED | P0_BLOCKER |
| CC-005 | Contact card/unlock | 已认证用户 | 无 | 双击确认名片 | 快速连续确认2次 | 仅确认1次，第2次被防重复机制拦截 | AUTOMATED_RED | P1_DEMO_RISK |
| CC-006 | Contact card/unlock | 已认证用户 | 已确认名片 | 重复确认名片 | confirm操作 | 返回 409 或幂等成功 | AUTOMATED_RED | P1_DEMO_RISK |
| CC-007 | Contact card/unlock | 已认证用户 | 无 | 保存名片时微信号格式非法 | wechatId: "a b c" | 返回 400，提示微信号格式不合法 | AUTOMATED_RED | P1_DEMO_RISK |
| CC-008 | Contact card/unlock | 已认证用户 | 无 | 保存名片时QQ号格式非法 | qqNumber: "abc" | 返回 400，提示QQ号必须为数字 | AUTOMATED_RED | P1_DEMO_RISK |
| CC-009 | Contact card/unlock | 已认证用户 | 无 | 查看自己名片 | 自己的cardId | 正常返回完整信息 | AUTOMATED_GREEN | P1_DEMO_RISK |
| CC-010 | Contact card/unlock | 已认证用户 | 对方名片状态=UNLOCKED | 查看对方名片 | conversationId | 返回完整联系方式 | AUTOMATED_GREEN | P2_UX |
| CC-011 | Contact card/unlock | 已认证用户 | 无 | 保存名片时所有联系方式为空 | phone:null, wechatId:null, qqNumber:null | 返回 400，提示至少填写一种联系方式 | AUTOMATED_RED | P2_UX |
| CC-012 | Contact card/unlock | 已认证用户 | 无 | 手机号格式非法 | phone: "123" | 返回 400，提示手机号格式不合法 | AUTOMATED_RED | P2_UX |
| CC-013 | Contact card/unlock | 已认证用户 | 无 | 保存他人名片 | 其他用户的cardId | 返回 403，无权操作 | AUTOMATED_RED | P2_UX |
| CC-014 | Contact card/unlock | 已认证用户 | 无 | 名片信息含 XSS | wechatId: "<script>alert(1)</script>" | 返回 400 或转义存储 | AUTOMATED_RED | P3_BACKLOG |
| CC-015 | Contact card/unlock | 已认证用户 | 无 | 确认名片时网络错误 | 正常确认，网络断开 | 客户端提示网络错误，可重试 | MANUAL_ONLY | P3_BACKLOG |
| CC-016 | Contact card/unlock | 已认证用户 | 无 | 并发保存同一名片 | 同一名片并发PUT×5 | 最终一致，不数据损坏 | AUTOMATED_GREEN | P3_BACKLOG |

## 10. Review/credit（16 条）

| caseId | module | role | precondition | operation | input | expectedResult | automationStatus | priority |
|--------|--------|------|--------------|-----------|-------|----------------|-----------------|----------|
| RC-001 | Review/credit | 已认证用户 | 已评价过该会话 | 对同一会话再次评价 | conversationId, rating: 5 | 返回 409，提示已评价 | AUTOMATED_RED | P0_BLOCKER |
| RC-002 | Review/credit | 已认证用户 | 双方各发消息<2条 | 尝试评价 | conversationId | 返回 400，提示消息数不足不可评价 | AUTOMATED_RED | P0_BLOCKER |
| RC-003 | Review/credit | 已认证用户 | 对方名片状态=LOCKED | 评7星评分 | rating: 7 | 返回 400，提示未解锁不可评7星 | AUTOMATED_RED | P0_BLOCKER |
| RC-004 | Review/credit | 已认证用户 | 无 | 评价不存在的会话 | conversationId: 999999 | 返回 404 | AUTOMATED_RED | P0_BLOCKER |
| RC-005 | Review/credit | 已认证用户 | 无 | 评分超出范围 | rating: 0 | 返回 400，提示评分范围非法 | AUTOMATED_RED | P1_DEMO_RISK |
| RC-006 | Review/credit | 已认证用户 | 无 | 评分超出范围 | rating: 8 | 返回 400，提示评分范围非法 | AUTOMATED_RED | P1_DEMO_RISK |
| RC-007 | Review/credit | 已认证用户 | 会话状态=OPEN | 评价进行中的会话 | conversationId | 返回 400 或 409，提示会话未结束 | AUTOMATED_RED | P1_DEMO_RISK |
| RC-008 | Review/credit | 已认证用户 | 无 | 评价他人会话（非参与者） | 其他用户的conversationId | 返回 403 | AUTOMATED_RED | P1_DEMO_RISK |
| RC-009 | Review/credit | 已认证用户 | 无 | 评价时评分为空 | rating: null | 返回 400，提示评分必填 | AUTOMATED_RED | P1_DEMO_RISK |
| RC-010 | Review/credit | 已认证用户 | 对方名片状态=UNLOCKED | 评7星评分 | rating: 7 | 成功，7星评分生效 | AUTOMATED_GREEN | P2_UX |
| RC-011 | Review/credit | 已认证用户 | 无 | 评价时评分为小数 | rating: 3.5 | 返回 400，提示评分必须为整数 | AUTOMATED_RED | P2_UX |
| RC-012 | Review/credit | 已认证用户 | 无 | 评价时评分为负数 | rating: -1 | 返回 400，提示评分范围非法 | AUTOMATED_RED | P2_UX |
| RC-013 | Review/credit | 已认证用户 | 无 | 评价内容含 XSS | comment: "<script>alert(1)</script>" | 存储转义，不执行脚本 | AUTOMATED_RED | P2_UX |
| RC-014 | Review/credit | 已认证用户 | 无 | 评价内容超长 | comment: 1001字 | 返回 400，提示评价内容长度超限 | AUTOMATED_RED | P3_BACKLOG |
| RC-015 | Review/credit | 已认证用户 | 无 | 查看信用分时服务端超时 | 正常请求，服务端慢 | 客户端超时提示，可重试 | MANUAL_ONLY | P3_BACKLOG |
| RC-016 | Review/credit | 未认证用户 | authenticationStatus≠VERIFIED | 尝试查看信用分 | 无 | 返回 403 | AUTOMATED_RED | P3_BACKLOG |

## 11. Admin identity review（11 条）

| caseId | module | role | precondition | operation | input | expectedResult | automationStatus | priority |
|--------|--------|------|--------------|-----------|-------|----------------|-----------------|----------|
| AI-001 | Admin identity review | 管理员 | 用户状态=VERIFIED | 审核已通过的用户 | approve操作 | 返回 409，提示已审核通过 | AUTOMATED_RED | P0_BLOCKER |
| AI-002 | Admin identity review | 管理员 | 用户状态=REJECTED | 审核已拒绝的用户 | approve操作 | 返回 409，提示已审核拒绝 | AUTOMATED_RED | P0_BLOCKER |
| AI-003 | Admin identity review | 管理员 | 用户状态=PENDING | 拒绝但不填原因 | reject操作, reason: null | 返回 400，提示拒绝原因必填 | AUTOMATED_RED | P0_BLOCKER |
| AI-004 | Admin identity review | 普通用户 | 无 | 普通用户尝试审核 | approve操作 | 返回 403，无管理员权限 | AUTOMATED_RED | P1_DEMO_RISK |
| AI-005 | Admin identity review | 管理员 | 用户状态=PENDING | 正常通过 | approve操作 | 用户状态变为 VERIFIED | AUTOMATED_GREEN | P1_DEMO_RISK |
| AI-006 | Admin identity review | 管理员 | 用户状态=PENDING | 正常拒绝并填原因 | reject操作, reason: "材料不清晰" | 用户状态变为 REJECTED | AUTOMATED_GREEN | P1_DEMO_RISK |
| AI-007 | Admin identity review | 管理员 | 无 | 审核不存在的用户 | userId: 999999 | 返回 404 | AUTOMATED_RED | P2_UX |
| AI-008 | Admin identity review | 管理员 | 用户状态=NOT_SUBMITTED | 审核未提交认证的用户 | approve操作 | 返回 409，提示用户未提交认证 | AUTOMATED_RED | P2_UX |
| AI-009 | Admin identity review | 管理员 | 无 | 并发审核同一用户 | 两个管理员同时approve | 仅一个成功，另一个返回 409 | AUTOMATED_GREEN | P3_BACKLOG |
| AI-010 | Admin identity review | 管理员 | 无 | 拒绝原因超长 | reason: 1001字 | 返回 400，提示原因长度超限 | AUTOMATED_RED | P3_BACKLOG |
| AI-011 | Admin identity review | 管理员 | 无 | 审核操作 action 非法 | action: "invalid" | 返回 400，提示操作类型不合法 | AUTOMATED_RED | P3_BACKLOG |

## 12. Network/HTTP errors（16 条）

| caseId | module | role | precondition | operation | input | expectedResult | automationStatus | priority |
|--------|--------|------|--------------|-----------|-------|----------------|-----------------|----------|
| NE-001 | Network/HTTP errors | 已登录用户 | Token 已过期 | 请求受保护接口 | Authorization: Bearer <expired> | 返回 401，客户端清除 Token 并跳转登录 | AUTOMATED_RED | P0_BLOCKER |
| NE-002 | Network/HTTP errors | 普通用户 | 无 | 请求管理员接口 | 管理员专用API | 返回 403 Forbidden | AUTOMATED_RED | P0_BLOCKER |
| NE-003 | Network/HTTP errors | 已认证用户 | 无 | 请求不存在的资源 | /api/not-exist | 返回 404 Not Found | AUTOMATED_RED | P0_BLOCKER |
| NE-004 | Network/HTTP errors | 已认证用户 | 帖子状态冲突 | 操作冲突 | 冲突操作 | 返回 409 Conflict | AUTOMATED_RED | P0_BLOCKER |
| NE-005 | Network/HTTP errors | 已认证用户 | 服务端异常 | 请求触发500 | 正常请求 | 返回 500，客户端提示服务器错误 | AUTOMATED_RED | P0_BLOCKER |
| NE-006 | Network/HTTP errors | 已认证用户 | 无 | 服务端返回非法 JSON | 正常请求 | 客户端 JSON 解析失败，提示数据异常 | AUTOMATED_RED | P1_DEMO_RISK |
| NE-007 | Network/HTTP errors | 已认证用户 | 无 | 请求超时 | timeout=1ms | 客户端超时提示，不崩溃 | AUTOMATED_RED | P1_DEMO_RISK |
| NE-008 | Network/HTTP errors | 已认证用户 | 服务不可达 | 请求触发连接拒绝 | 服务关闭 | 客户端提示连接失败 | AUTOMATED_RED | P1_DEMO_RISK |
| NE-009 | Network/HTTP errors | 已认证用户 | 无 | 4xx 响应体为空 | 触发 400 空 body | 客户端提示请求错误，不崩溃 | AUTOMATED_RED | P1_DEMO_RISK |
| NE-010 | Network/HTTP errors | 已认证用户 | 无 | 5xx 响应体为空 | 触发 500 空 body | 客户端提示服务器错误，不崩溃 | AUTOMATED_RED | P1_DEMO_RISK |
| NE-011 | Network/HTTP errors | 已认证用户 | 无 | 服务端返回非 JSON 内容 | Content-Type: text/html | 客户端提示数据格式异常 | AUTOMATED_RED | P2_UX |
| NE-012 | Network/HTTP errors | 已认证用户 | 无 | 请求被限流 | 短时间大量请求 | 返回 429 Too Many Requests | AUTOMATED_GREEN | P2_UX |
| NE-013 | Network/HTTP errors | 已认证用户 | 无 | 服务端返回 502 Bad Gateway | 代理异常 | 客户端提示网关错误 | AUTOMATED_RED | P2_UX |
| NE-014 | Network/HTTP errors | 已认证用户 | 无 | 服务端返回 503 Service Unavailable | 维护中 | 客户端提示服务暂不可用 | AUTOMATED_RED | P2_UX |
| NE-015 | Network/HTTP errors | 已认证用户 | 无 | 网络间歇性断开 | 断开-恢复-断开 | 客户端自动重试或提示，不丢失用户数据 | MANUAL_ONLY | P3_BACKLOG |
| NE-016 | Network/HTTP errors | 已认证用户 | 无 | DNS 解析失败 | 无效域名 | 客户端提示网络错误 | AUTOMATED_RED | P3_BACKLOG |

## 13. UI experience（22 条）

| caseId | module | role | precondition | operation | input | expectedResult | automationStatus | priority |
|--------|--------|------|--------------|-----------|-------|----------------|-----------------|----------|
| UI-001 | UI experience | 已认证用户 | 表单数据不完整 | 提交按钮状态 | 必填字段未填 | 提交按钮禁用或点击后显示字段级错误 | MANUAL_ONLY | P0_BLOCKER |
| UI-002 | UI experience | 已认证用户 | 服务端返回 VALIDATION_FAILED | 查看错误显示 | 多字段错误 | UI 显示字段级错误信息，至少显示 errorCode+errorMessage | MANUAL_ONLY | P0_BLOCKER |
| UI-003 | UI experience | 已认证用户 | 列表数据为空 | 查看空列表 | 无数据 | UI 显示友好的空状态提示 | MANUAL_ONLY | P0_BLOCKER |
| UI-004 | UI experience | 已认证用户 | 无 | 快速切换标签页 | 连续切换5次 | 最终显示正确内容，无闪烁或数据错乱 | MANUAL_ONLY | P0_BLOCKER |
| UI-005 | UI experience | 已认证用户 | 列表已加载 | 下拉刷新 | 正常刷新 | 刷新后数据正确，不丢失滚动位置 | MANUAL_ONLY | P0_BLOCKER |
| UI-006 | UI experience | 已认证用户 | 提交按钮可点击 | 双击提交 | 快速连续点击2次 | 仅提交1次 | AUTOMATED_GREEN | P1_DEMO_RISK |
| UI-007 | UI experience | 已认证用户 | 无 | 并发操作（保存+提交） | 同时保存和提交 | 操作有序执行，不数据损坏 | MANUAL_ONLY | P1_DEMO_RISK |
| UI-008 | UI experience | 已认证用户 | 网络请求进行中 | 再次触发请求 | 请求未完成时再点击 | 按钮禁用或忽略重复请求 | MANUAL_ONLY | P1_DEMO_RISK |
| UI-009 | UI experience | 已认证用户 | 无 | 快速返回上一页 | 提交后立即返回 | 请求正常完成，不丢失 | MANUAL_ONLY | P1_DEMO_RISK |
| UI-010 | UI experience | 已认证用户 | 长列表 | 滚动加载更多 | 滚动到底部 | 分页加载正确，无重复数据 | MANUAL_ONLY | P1_DEMO_RISK |
| UI-011 | UI experience | 已认证用户 | 表单已填写 | 切换页面后返回 | 填写一半离开再回来 | 表单数据保留（视业务策略） | MANUAL_ONLY | P1_DEMO_RISK |
| UI-012 | UI experience | 已认证用户 | 错误提示可见 | 修正错误后重新提交 | 修正后提交 | 错误提示清除，提交成功 | MANUAL_ONLY | P1_DEMO_RISK |
| UI-013 | UI experience | 已认证用户 | 无 | 弱网环境下操作 | 3G网络 | 操作不超时不崩溃，适当loading提示 | MANUAL_ONLY | P1_DEMO_RISK |
| UI-014 | UI experience | 已认证用户 | 无 | 输入法组合输入中提交 | 拼音未确认时提交 | 提交时使用已确认文本，不包含未确认拼音 | MANUAL_ONLY | P1_DEMO_RISK |
| UI-015 | UI experience | 已认证用户 | 无 | 窗口尺寸变化 | 拖动调整窗口 | UI 自适应，不溢出或遮挡 | MANUAL_ONLY | P2_UX |
| UI-016 | UI experience | 已认证用户 | 无 | 长文本输入 | title输入100字 | 不截断显示，完整展示 | MANUAL_ONLY | P2_UX |
| UI-017 | UI experience | 已认证用户 | 无 | 快速输入搜索关键词 | 连续输入10个字符 | 防抖处理，不发起10次请求 | MANUAL_ONLY | P2_UX |
| UI-018 | UI experience | 已认证用户 | 无 | 会话列表快速滑动 | 快速上下滑动 | 不卡顿，不白屏 | MANUAL_ONLY | P2_UX |
| UI-019 | UI experience | 已认证用户 | 无 | 图片加载失败 | 头像URL失效 | 显示占位图，不显示裂图 | MANUAL_ONLY | P2_UX |
| UI-020 | UI experience | 已认证用户 | 无 | 剪贴板粘贴超长文本 | 粘贴5000字到输入框 | 截断或提示长度超限 | MANUAL_ONLY | P2_UX |
| UI-021 | UI experience | 已认证用户 | 无 | 多窗口同时操作 | 2个窗口同时操作 | 数据一致，不冲突 | MANUAL_ONLY | P3_BACKLOG |
| UI-022 | UI experience | 已认证用户 | 无 | 应用前后台切换 | 切到后台再回前台 | 数据自动刷新或保持一致 | MANUAL_ONLY | P3_BACKLOG |
