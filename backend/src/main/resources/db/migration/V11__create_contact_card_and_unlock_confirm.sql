CREATE TABLE contact_card (
    id              BIGSERIAL PRIMARY KEY,
    user_id         UUID NOT NULL UNIQUE,
    wechat_id       VARCHAR(64),
    phone_number    VARCHAR(20),
    qq_number       VARCHAR(32),
    created_at      TIMESTAMP NOT NULL DEFAULT NOW(),
    updated_at      TIMESTAMP NOT NULL DEFAULT NOW()
);

CREATE TABLE contact_unlock_confirm (
    id              BIGSERIAL PRIMARY KEY,
    conversation_id BIGINT NOT NULL,
    user_id         UUID NOT NULL,
    confirmed_at    TIMESTAMP NOT NULL DEFAULT NOW(),
    CONSTRAINT uq_unlock_confirm_conv_user UNIQUE (conversation_id, user_id)
);
