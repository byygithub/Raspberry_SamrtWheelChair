#!/usr/bin/env bash
set -euo pipefail

OUT_DIR="${1:-/tmp/wheelchair_diagnostics_$(date +%Y%m%d_%H%M%S)}"
SERVICE_NAME="${SERVICE_NAME:-wheelchair.service}"

mkdir -p "${OUT_DIR}"

{
  echo "==== datetime ===="
  date
  echo
  echo "==== uptime ===="
  uptime
  echo
  echo "==== free -h ===="
  free -h
  echo
  echo "==== df -h ===="
  df -h
  echo
  echo "==== vcgencmd measure_temp ===="
  (vcgencmd measure_temp || true)
  echo
  echo "==== top (snapshot) ===="
  top -b -n 1 | head -n 40
} > "${OUT_DIR}/system.txt"

journalctl -u "${SERVICE_NAME}" -n 500 --no-pager > "${OUT_DIR}/service_journal.txt" || true
cp -f /var/log/wheelchair/app.log "${OUT_DIR}/app.log" 2>/dev/null || true
cp -f /var/log/wheelchair/app.log.1 "${OUT_DIR}/app.log.1" 2>/dev/null || true

echo "diagnostics saved to ${OUT_DIR}"
