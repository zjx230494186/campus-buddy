CREATE TABLE group_chat (
    id BIGSERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    description VARCHAR(500),
    creator_id UUID NOT NULL,
    related_post_uuid UUID,
    status VARCHAR(30) NOT NULL DEFAULT 'ACTIVE',
    max_members INTEGER NOT NULL DEFAULT 20,
    created_at TIMESTAMP NOT NULL,
    updated_at TIMESTAMP NOT NULL,
    last_message_at TIMESTAMP
);

CREATE TABLE group_chat_member (
    id BIGSERIAL PRIMARY KEY,
    group_chat_id BIGINT NOT NULL REFERENCES group_chat(id) ON DELETE CASCADE,
    user_id UUID NOT NULL,
    role VARCHAR(30) NOT NULL DEFAULT 'MEMBER',
    status VARCHAR(30) NOT NULL DEFAULT 'JOINED',
    joined_at TIMESTAMP NOT NULL,
    last_read_message_id BIGINT,
    left_at TIMESTAMP,
    UNIQUE (group_chat_id, user_id)
);

CREATE TABLE group_chat_message (
    id BIGSERIAL PRIMARY KEY,
    group_chat_id BIGINT NOT NULL REFERENCES group_chat(id) ON DELETE CASCADE,
    sender_id UUID NOT NULL,
    message_type VARCHAR(30) NOT NULL,
    content VARCHAR(1000),
    created_at TIMESTAMP NOT NULL
);

CREATE INDEX idx_group_chat_creator_id ON group_chat(creator_id);
CREATE INDEX idx_group_chat_related_post_uuid ON group_chat(related_post_uuid);
CREATE INDEX idx_group_chat_member_group_chat_id ON group_chat_member(group_chat_id);
CREATE INDEX idx_group_chat_member_user_id ON group_chat_member(user_id);
CREATE INDEX idx_group_chat_message_group_chat_id ON group_chat_message(group_chat_id);
CREATE INDEX idx_group_chat_message_sender_id ON group_chat_message(sender_id);
CREATE INDEX idx_group_chat_message_created_at ON group_chat_message(created_at);