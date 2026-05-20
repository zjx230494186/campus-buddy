#!/bin/bash
# Wrapper script for systemd campus-buddy-backend service.
# Sources private env file and maps OBS variable names.
# Must NOT contain any real secrets - only references variable names.
set -a
. /etc/campus-buddy/backend.env
set +a

export OBS_ACCESS_KEY_ID="${OBJECT_STORAGE_ACCESS_KEY_ID}"
export OBS_SECRET_ACCESS_KEY="${OBJECT_STORAGE_SECRET_ACCESS_KEY}"

exec /usr/bin/java -jar /srv/campus-buddy/campus-buddy-backend-0.0.1-SNAPSHOT.jar \
  --spring.profiles.active=deploy
