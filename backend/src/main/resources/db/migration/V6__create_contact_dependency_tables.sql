create table conversation (
    id bigserial primary key,
    participant1_id uuid not null references user_account(user_id),
    participant2_id uuid not null references user_account(user_id),
    status varchar(30) not null default 'ACTIVE',
    related_post_id bigint,
    created_at timestamp with time zone not null default now(),
    updated_at timestamp with time zone not null default now(),
    constraint chk_different_participants check (participant1_id != participant2_id)
);

create index idx_conversation_participant1 on conversation (participant1_id);
create index idx_conversation_participant2 on conversation (participant2_id);

create table conversation_message (
    id bigserial primary key,
    conversation_id bigint not null references conversation(id),
    sender_id uuid,
    message_type varchar(30) not null,
    content text,
    created_at timestamp with time zone not null default now()
);

create index idx_conversation_message_conv_sender on conversation_message (conversation_id, sender_id);
create index idx_conversation_message_conv_type on conversation_message (conversation_id, message_type);

create table contact_unlock_record (
    id bigserial primary key,
    conversation_id bigint not null unique references conversation(id),
    unlocked_at timestamp with time zone not null,
    unlocked_by_user_id uuid,
    created_at timestamp with time zone not null default now()
);

create index idx_contact_unlock_conversation on contact_unlock_record (conversation_id);
