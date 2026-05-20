#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "Installing campus-buddy-backend systemd service..."

cp "${SCRIPT_DIR}/campus-buddy-backend.service" /etc/systemd/system/campus-buddy-backend.service
cp "${SCRIPT_DIR}/start_backend_service.sh" /srv/campus-buddy/start_backend_service.sh
chmod +x /srv/campus-buddy/start_backend_service.sh

systemctl daemon-reload
systemctl enable campus-buddy-backend

echo "Service installed and enabled."
echo "Use: systemctl start campus-buddy-backend"
echo "Use: systemctl status campus-buddy-backend"
