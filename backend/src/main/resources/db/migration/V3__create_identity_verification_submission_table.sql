create table identity_verification_submission (
    submission_id uuid primary key default gen_random_uuid(),
    user_id uuid not null references user_account(user_id),
    real_name varchar(100) not null,
    student_number varchar(50) not null,
    college varchar(100) not null,
    major varchar(100) not null,
    grade varchar(20) not null,
    review_status varchar(30) not null default 'PENDING_REVIEW',
    reject_reason varchar(500),
    submitted_at timestamp with time zone not null default now(),
    reviewed_at timestamp with time zone
);

create index idx_identity_verification_user on identity_verification_submission (user_id);
