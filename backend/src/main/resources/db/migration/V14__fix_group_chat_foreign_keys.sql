-- 修复群聊表的外键约束
-- 注意：由于V13已经创建了表，这里使用ALTER TABLE添加外键约束

-- group_chat表的creator_id外键
ALTER TABLE group_chat 
ADD CONSTRAINT fk_group_chat_creator_id 
FOREIGN KEY (creator_id) REFERENCES user_account(user_id) ON DELETE CASCADE;

-- group_chat表的related_post_uuid外键
ALTER TABLE group_chat 
ADD CONSTRAINT fk_group_chat_related_post_uuid 
FOREIGN KEY (related_post_uuid) REFERENCES partner_post(id) ON DELETE SET NULL;

-- group_chat_member表的user_id外键
ALTER TABLE group_chat_member 
ADD CONSTRAINT fk_group_chat_member_user_id 
FOREIGN KEY (user_id) REFERENCES user_account(user_id) ON DELETE CASCADE;

-- group_chat_message表的sender_id外键
ALTER TABLE group_chat_message 
ADD CONSTRAINT fk_group_chat_message_sender_id 
FOREIGN KEY (sender_id) REFERENCES user_account(user_id) ON DELETE CASCADE;

-- 清理可能的孤儿数据（可选，根据实际数据情况决定是否执行）
-- DELETE FROM group_chat_member WHERE user_id NOT IN (SELECT user_id FROM user_account);
-- DELETE FROM group_chat_message WHERE sender_id NOT IN (SELECT user_id FROM user_account);
-- DELETE FROM group_chat WHERE creator_id NOT IN (SELECT user_id FROM user_account);
-- DELETE FROM group_chat WHERE related_post_uuid IS NOT NULL AND related_post_uuid NOT IN (SELECT id FROM partner_post);