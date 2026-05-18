create table identity_material_attachment (
    attachment_id uuid primary key default gen_random_uuid(),
    owner_user_id uuid not null references user_account(user_id),
    object_key varchar(500) not null unique,
    content_type varchar(100) not null,
    original_filename varchar(255) not null,
    size_bytes bigint not null,
    sha256 varchar(64) not null,
    business_type varchar(30) not null default 'IDENTITY_MATERIAL',
    status varchar(30) not null default 'ACTIVE',
    created_at timestamp with time zone not null default now()
);

create index idx_material_attachment_owner on identity_material_attachment (owner_user_id);

ALTER TABLE identity_verification_submission ADD COLUMN material_attachment_id uuid;
