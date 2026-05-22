CREATE TABLE partner_post (
    id              UUID        NOT NULL PRIMARY KEY,
    publisher_id    UUID        NOT NULL REFERENCES user_account(user_id),
    scene_type      VARCHAR(30),
    status          VARCHAR(30) NOT NULL DEFAULT 'DRAFT',
    title           VARCHAR(40),
    description     VARCHAR(500),
    time_mode       VARCHAR(30),
    time_text       VARCHAR(100),
    start_at        TIMESTAMP WITH TIME ZONE,
    end_at          TIMESTAMP WITH TIME ZONE,
    location_text   VARCHAR(80),
    participant_count INTEGER,
    target_requirement VARCHAR(120),
    contact_preference VARCHAR(80),
    tags            JSONB,
    attachment_ids  JSONB,
    scene_payload   JSONB,
    reject_reason   VARCHAR(500),
    reviewed_by     UUID,
    reviewed_at     TIMESTAMP WITH TIME ZONE,
    published_at    TIMESTAMP WITH TIME ZONE,
    created_at      TIMESTAMP WITH TIME ZONE NOT NULL,
    updated_at      TIMESTAMP WITH TIME ZONE NOT NULL
);

CREATE INDEX idx_partner_post_publisher ON partner_post(publisher_id);
CREATE INDEX idx_partner_post_status ON partner_post(status);
