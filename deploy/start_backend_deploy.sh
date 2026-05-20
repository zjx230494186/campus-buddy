#!/bin/bash
set -a
. /etc/campus-buddy/backend.env
set +a

export OBS_ACCESS_KEY_ID="${OBJECT_STORAGE_ACCESS_KEY_ID}"
export OBS_SECRET_ACCESS_KEY="${OBJECT_STORAGE_SECRET_ACCESS_KEY}"

pkill -f campus-buddy-backend 2>/dev/null
sleep 2

cd /srv/campus-buddy
nohup java -jar campus-buddy-backend-0.0.1-SNAPSHOT.jar \
  --spring.profiles.active=deploy \
  > backend.log 2>&1 &

echo "Backend started with PID $!"
sleep 15
tail -5 /srv/campus-buddy/backend.log
