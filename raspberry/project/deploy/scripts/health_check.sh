#!/usr/bin/env bash
set -euo pipefail

HEARTBEAT_FILE="${HEALTH_HEARTBEAT_FILE:-/tmp/wheelchair_heartbeat}"
HEARTBEAT_TIMEOUT="${HEALTH_HEARTBEAT_TIMEOUT:-20}"
GATEWAY_IP="${GATEWAY_IP:-}"
SERVICE_NAME="${SERVICE_NAME:-wheelchair.service}"

check_heartbeat() {
  if [[ ! -f "${HEARTBEAT_FILE}" ]]; then
    echo "heartbeat file missing: ${HEARTBEAT_FILE}"
    return 1
  fi

  local now ts delta
  now="$(date +%s)"
  ts="$(cat "${HEARTBEAT_FILE}" 2>/dev/null || echo 0)"
  delta=$((now - ts))
  if (( delta > HEARTBEAT_TIMEOUT )); then
    echo "heartbeat stale: ${delta}s > ${HEARTBEAT_TIMEOUT}s"
    return 1
  fi
  return 0
}

check_network() {
  if [[ -z "${GATEWAY_IP}" ]]; then
    return 0
  fi
  ping -c 1 -W 2 "${GATEWAY_IP}" >/dev/null 2>&1
}

restart_service() {
  if [[ "${EUID}" -eq 0 ]]; then
    systemctl restart "${SERVICE_NAME}"
    return
  fi

  if command -v sudo >/dev/null 2>&1 && sudo -n true >/dev/null 2>&1; then
    sudo -n systemctl restart "${SERVICE_NAME}"
    return
  fi

  echo "cannot restart ${SERVICE_NAME}: run as root or grant passwordless sudo for systemctl restart" >&2
  return 1
}

if ! check_heartbeat || ! check_network; then
  echo "health check failed, restarting ${SERVICE_NAME}"
  restart_service
fi
