create table user_account (
    user_id uuid primary key default gen_random_uuid(),
    campus_email varchar(255) not null unique,
    password_hash varchar(255) not null,
    display_name varchar(100) not null,
    authentication_status varchar(30) not null default 'UNVERIFIED',
    campus_email_verification_status varchar(30) not null default 'UNVERIFIED',
    created_at timestamp with time zone not null default now(),
    updated_at timestamp with time zone not null default now()
);

create table campus_email_verification_code (
    email_purpose_key varchar(255) primary key,
    campus_email varchar(255) not null,
    code_hash varchar(64) not null,
    expires_at timestamp with time zone not null,
    next_allowed_at timestamp with time zone not null,
    created_at timestamp with time zone not null default now()
);

create table campus_email_verification_ticket (
    email_purpose_key varchar(255) primary key,
    ticket_hash varchar(64) not null,
    expires_at timestamp with time zone not null,
    created_at timestamp with time zone not null default now()
);

create index idx_user_account_campus_email on user_account (campus_email);
