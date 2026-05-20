# 测试记录：P0 低压力联系最小后端依赖

## 基本信息

- 日期：2026-05-20
- 线程：CodeArts Round 15
- 模块：P0 低压力联系最小后端依赖
- 功能：Conversation + ConversationMessage + ContactUnlockRecord + ContactContextService
- 对应需求编号：IR9/IR10/IR12（评价前置依赖解除）
- 对应文档：docs/24_p0_contact_min_dependency_spec_v1.md
- 测试文件：
  - backend/src/test/java/com/campusbuddy/contact/ContactContextServiceTest.java（19 个测试）
  - backend/src/test/java/com/campusbuddy/contact/ContactPersistenceIntegrationTest.java（6 个测试）
- 实现文件：
  - backend/src/main/resources/db/migration/V6__create_contact_dependency_tables.sql
  - backend/src/main/java/com/campusbuddy/contact/Conversation.java
  - backend/src/main/java/com/campusbuddy/contact/ConversationMessage.java
  - backend/src/main/java/com/campusbuddy/contact/ContactUnlockRecord.java
  - backend/src/main/java/com/campusbuddy/contact/ConversationRepository.java
  - backend/src/main/java/com/campusbuddy/contact/ConversationMessageRepository.java
  - backend/src/main/java/com/campusbuddy/contact/ContactUnlockRecordRepository.java
  - backend/src/main/java/com/campusbuddy/contact/ContactContextService.java

## 红灯记录

- 运行命令：`.\mvnw.cmd test -Dspring.testcontainers.enabled=false -Dtest=ContactContextServiceTest,ContactPersistenceIntegrationTest`
- 失败测试：25 个全部编译失败
- 失败原因：Conversation、ConversationMessage、ContactUnlockRecord、ContactContextService、各 Repository 类不存在
- 是否符合预期：是

## 实现摘要

- 修改文件：8 个新增文件（见上方实现文件列表）
- 核心逻辑：
  - V6 迁移创建 conversation、conversation_message、contact_unlock_record 三张表，含 CHECK 约束（participant1_id != participant2_id）、UNIQUE 约束（conversation_id on contact_unlock_record）、外键和索引
  - Conversation 实体：Long ID（IDENTITY 策略）、UUID 参与者、String 状态、构造器校验参与者不同、isParticipant/getOtherParticipant 方法
  - ConversationMessage 实体：Long ID、Long conversationId、UUID senderId（可空）、String messageType、String content（不保存联系方式明文）
  - ContactUnlockRecord 实体：Long ID、Long conversationId（unique）、Instant unlockedAt
  - ContactContextService：isParticipant、isOtherParticipant、countUserMessages（排除 SYSTEM 和非参与者）、isValidConversation（双方各≥2条用户消息）、findConversationsByParticipant、isContactUnlocked
- 未覆盖内容：完整 IM、WebSocket、图片消息、联系方式明文、消息举报、Qt UI

## 绿灯记录

- 运行命令：`.\mvnw.cmd test -Dtest=ContactContextServiceTest,ContactPersistenceIntegrationTest`
- 通过测试：25/25（ContactContextServiceTest 19 + ContactPersistenceIntegrationTest 6）
- 回归测试命令：`.\mvnw.cmd test -Dspring.testcontainers.enabled=false -Dtest=!DatabaseMigrationTest`
- 回归测试结果：80/80 通过
- 结果：GREEN

## 服务器 smoke test 记录

- 是否适用：是（涉及数据库迁移 V6）
- 本地测试已通过
- 服务器 smoke test 未执行
- 原因：当前未配置 Ubuntu 24 服务器访问或用户未授权部署
- 因此本批次未达到完整完成门禁，只能视为本地开发闭环

## 敏感信息检查

- 未写入联系方式明文（手机号、微信号、QQ、私人邮箱等）
- 未写入 JWT_SECRET、DB_PASSWORD、OBJECT_STORAGE AK/SK
- 未写入 SSH 私钥、真实 token
- ConversationMessage.content 字段预留但不保存联系方式明文
- 检查结论：通过

## 后续风险

- 风险 1：服务器 smoke test 未执行，V6 迁移在 Ubuntu 24 + PostgreSQL 环境下尚未验证
- 风险 2：ConversationMessage.messageType 当前仅 USER_TEXT 和 SYSTEM，后续新增 USER_IMAGE 等类型时需确保 countUserMessages 仍正确排除非用户消息
