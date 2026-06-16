-- Campus Buddy local PostgreSQL schema.
-- Source: Flyway migrations V1-V11 under backend/src/main/resources/db/migration.
-- Target: clean local PostgreSQL database.
--
-- Run after creating and selecting the campus_buddy database:
--   psql -U campus_buddy -d campus_buddy -f deploy/local-postgres/01_schema.sql

CREATE EXTENSION IF NOT EXISTS pgcrypto;

CREATE TABLE IF NOT EXISTS technical_spike_marker (
    marker_key varchar(64) PRIMARY KEY,
    marker_value varchar(128) NOT NULL,
    created_at timestamp with time zone NOT NULL DEFAULT now()
);

INSERT INTO technical_spike_marker (marker_key, marker_value)
VALUES ('database_migration', 'flyway_testcontainers_postgresql')
ON CONFLICT (marker_key) DO NOTHING;

CREATE TABLE IF NOT EXISTS user_account (
    user_id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
    campus_email varchar(255) NOT NULL UNIQUE,
    password_hash varchar(255) NOT NULL,
    display_name varchar(100) NOT NULL,
    authentication_status varchar(30) NOT NULL DEFAULT 'UNVERIFIED',
    campus_email_verification_status varchar(30) NOT NULL DEFAULT 'UNVERIFIED',
    created_at timestamp with time zone NOT NULL DEFAULT now(),
    updated_at timestamp with time zone NOT NULL DEFAULT now(),
    account_role varchar(30) NOT NULL DEFAULT 'STUDENT'
);

CREATE INDEX IF NOT EXISTS idx_user_account_campus_email
    ON user_account (campus_email);

CREATE TABLE IF NOT EXISTS campus_email_verification_code (
    email_purpose_key varchar(255) PRIMARY KEY,
    campus_email varchar(255) NOT NULL,
    code_hash varchar(64) NOT NULL,
    expires_at timestamp with time zone NOT NULL,
    next_allowed_at timestamp with time zone NOT NULL,
    created_at timestamp with time zone NOT NULL DEFAULT now()
);

CREATE TABLE IF NOT EXISTS campus_email_verification_ticket (
    email_purpose_key varchar(255) PRIMARY KEY,
    ticket_hash varchar(64) NOT NULL,
    expires_at timestamp with time zone NOT NULL,
    created_at timestamp with time zone NOT NULL DEFAULT now()
);

CREATE TABLE IF NOT EXISTS identity_verification_submission (
    submission_id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
    user_id uuid NOT NULL REFERENCES user_account(user_id),
    real_name varchar(100) NOT NULL,
    student_number varchar(50) NOT NULL,
    college varchar(100) NOT NULL,
    major varchar(100) NOT NULL,
    grade varchar(20) NOT NULL,
    review_status varchar(30) NOT NULL DEFAULT 'PENDING_REVIEW',
    reject_reason varchar(500),
    submitted_at timestamp with time zone NOT NULL DEFAULT now(),
    reviewed_at timestamp with time zone,
    material_attachment_id uuid
);

CREATE INDEX IF NOT EXISTS idx_identity_verification_user
    ON identity_verification_submission (user_id);

CREATE TABLE IF NOT EXISTS identity_material_attachment (
    attachment_id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
    owner_user_id uuid NOT NULL REFERENCES user_account(user_id),
    object_key varchar(500) NOT NULL UNIQUE,
    content_type varchar(100) NOT NULL,
    original_filename varchar(255) NOT NULL,
    size_bytes bigint NOT NULL,
    sha256 varchar(64) NOT NULL,
    business_type varchar(30) NOT NULL DEFAULT 'IDENTITY_MATERIAL',
    status varchar(30) NOT NULL DEFAULT 'ACTIVE',
    created_at timestamp with time zone NOT NULL DEFAULT now()
);

CREATE INDEX IF NOT EXISTS idx_material_attachment_owner
    ON identity_material_attachment (owner_user_id);

CREATE TABLE IF NOT EXISTS partner_post (
    id uuid NOT NULL PRIMARY KEY,
    publisher_id uuid NOT NULL REFERENCES user_account(user_id),
    scene_type varchar(30),
    status varchar(30) NOT NULL DEFAULT 'DRAFT',
    title varchar(40),
    description varchar(500),
    time_mode varchar(30),
    time_text varchar(100),
    start_at timestamp with time zone,
    end_at timestamp with time zone,
    location_text varchar(80),
    participant_count integer,
    target_requirement varchar(120),
    contact_preference varchar(80),
    tags jsonb,
    attachment_ids jsonb,
    scene_payload jsonb,
    reject_reason varchar(500),
    reviewed_by uuid,
    reviewed_at timestamp with time zone,
    published_at timestamp with time zone,
    created_at timestamp with time zone NOT NULL,
    updated_at timestamp with time zone NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_partner_post_publisher
    ON partner_post (publisher_id);

CREATE INDEX IF NOT EXISTS idx_partner_post_status
    ON partner_post (status);

CREATE TABLE IF NOT EXISTS conversation (
    id bigserial PRIMARY KEY,
    participant1_id uuid NOT NULL REFERENCES user_account(user_id),
    participant2_id uuid NOT NULL REFERENCES user_account(user_id),
    status varchar(30) NOT NULL DEFAULT 'ACTIVE',
    related_post_id bigint,
    created_at timestamp with time zone NOT NULL DEFAULT now(),
    updated_at timestamp with time zone NOT NULL DEFAULT now(),
    related_post_uuid uuid REFERENCES partner_post(id),
    participant1_last_read_message_id bigint,
    participant2_last_read_message_id bigint,
    CONSTRAINT chk_different_participants CHECK (participant1_id != participant2_id)
);

CREATE INDEX IF NOT EXISTS idx_conversation_participant1
    ON conversation (participant1_id);

CREATE INDEX IF NOT EXISTS idx_conversation_participant2
    ON conversation (participant2_id);

CREATE INDEX IF NOT EXISTS idx_conversation_related_post_uuid
    ON conversation (related_post_uuid);

CREATE TABLE IF NOT EXISTS conversation_message (
    id bigserial PRIMARY KEY,
    conversation_id bigint NOT NULL REFERENCES conversation(id),
    sender_id uuid,
    message_type varchar(30) NOT NULL,
    content text,
    created_at timestamp with time zone NOT NULL DEFAULT now()
);

CREATE INDEX IF NOT EXISTS idx_conversation_message_conv_sender
    ON conversation_message (conversation_id, sender_id);

CREATE INDEX IF NOT EXISTS idx_conversation_message_conv_type
    ON conversation_message (conversation_id, message_type);

CREATE TABLE IF NOT EXISTS contact_unlock_record (
    id bigserial PRIMARY KEY,
    conversation_id bigint NOT NULL UNIQUE REFERENCES conversation(id),
    unlocked_at timestamp with time zone NOT NULL,
    unlocked_by_user_id uuid,
    created_at timestamp with time zone NOT NULL DEFAULT now()
);

CREATE INDEX IF NOT EXISTS idx_contact_unlock_conversation
    ON contact_unlock_record (conversation_id);

CREATE TABLE IF NOT EXISTS review (
    id bigserial PRIMARY KEY,
    conversation_id bigint NOT NULL REFERENCES conversation(id),
    reviewer_id uuid NOT NULL REFERENCES user_account(user_id),
    reviewee_id uuid NOT NULL REFERENCES user_account(user_id),
    rating integer NOT NULL,
    review_tags text,
    status varchar(30) NOT NULL DEFAULT 'ACTIVE',
    modified_once boolean NOT NULL DEFAULT false,
    created_at timestamp with time zone NOT NULL DEFAULT now(),
    updated_at timestamp with time zone NOT NULL DEFAULT now(),
    CONSTRAINT chk_rating_range CHECK (rating >= 1 AND rating <= 6),
    CONSTRAINT chk_reviewer_not_reviewee CHECK (reviewer_id != reviewee_id),
    CONSTRAINT uq_review_unique UNIQUE (conversation_id, reviewer_id, reviewee_id)
);

CREATE INDEX IF NOT EXISTS idx_review_reviewee
    ON review (reviewee_id);

CREATE INDEX IF NOT EXISTS idx_review_reviewer
    ON review (reviewer_id);

CREATE INDEX IF NOT EXISTS idx_review_conversation
    ON review (conversation_id);

CREATE TABLE IF NOT EXISTS contact_card (
    id bigserial PRIMARY KEY,
    user_id uuid NOT NULL UNIQUE,
    wechat_id varchar(64),
    phone_number varchar(20),
    qq_number varchar(32),
    created_at timestamp NOT NULL DEFAULT now(),
    updated_at timestamp NOT NULL DEFAULT now()
);

CREATE TABLE IF NOT EXISTS contact_unlock_confirm (
    id bigserial PRIMARY KEY,
    conversation_id bigint NOT NULL,
    user_id uuid NOT NULL,
    confirmed_at timestamp NOT NULL DEFAULT now(),
    CONSTRAINT uq_unlock_confirm_conv_user UNIQUE (conversation_id, user_id)
);

