-- Local PostgreSQL bootstrap template.
-- Run as a PostgreSQL admin user, for example:
--   psql -U postgres -f deploy/local-postgres/00_create_database.sql
--
-- Replace CHANGE_ME_LOCAL_PASSWORD before running.

CREATE ROLE campus_buddy WITH LOGIN PASSWORD 'CHANGE_ME_LOCAL_PASSWORD';

CREATE DATABASE campus_buddy
    WITH OWNER = campus_buddy
         ENCODING = 'UTF8';

