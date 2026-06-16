ALTER TABLE conversation ADD COLUMN closer_id UUID REFERENCES user_account(id);

ALTER TABLE conversation ADD COLUMN closed_at TIMESTAMP WITH TIME ZONE;

CREATE INDEX idx_conversation_closer_id ON conversation(closer_id);

CREATE INDEX idx_conversation_closed_at ON conversation(closed_at);