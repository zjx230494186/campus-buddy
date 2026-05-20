#!/bin/bash
set -a
. /etc/campus-buddy/backend.env
set +a
python3 /tmp/obs_put_get_delete_smoke.py
