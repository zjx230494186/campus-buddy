create table technical_spike_marker (
    marker_key varchar(64) primary key,
    marker_value varchar(128) not null,
    created_at timestamp with time zone not null default now()
);

insert into technical_spike_marker (marker_key, marker_value)
values ('database_migration', 'flyway_testcontainers_postgresql');
