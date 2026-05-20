create table review (
    id bigserial primary key,
    conversation_id bigint not null references conversation(id),
    reviewer_id uuid not null references user_account(user_id),
    reviewee_id uuid not null references user_account(user_id),
    rating integer not null,
    review_tags text,
    status varchar(30) not null default 'ACTIVE',
    modified_once boolean not null default false,
    created_at timestamp with time zone not null default now(),
    updated_at timestamp with time zone not null default now(),
    constraint chk_rating_range check (rating >= 1 and rating <= 6),
    constraint chk_reviewer_not_reviewee check (reviewer_id != reviewee_id),
    constraint uq_review_unique unique (conversation_id, reviewer_id, reviewee_id)
);

create index idx_review_reviewee on review (reviewee_id);
create index idx_review_reviewer on review (reviewer_id);
create index idx_review_conversation on review (conversation_id);
