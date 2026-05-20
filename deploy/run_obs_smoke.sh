#!/bin/bash
set -a
. /etc/campus-buddy/backend.env
set +a
exec python3 /srv/campus-buddy/obs_put_get_delete_smoke.py
