ALTER TABLE conversation ADD COLUMN related_post_uuid UUID REFERENCES partner_post(id);

CREATE INDEX idx_conversation_related_post_uuid ON conversation(related_post_uuid);
