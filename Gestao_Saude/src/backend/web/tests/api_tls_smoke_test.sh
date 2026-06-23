#!/usr/bin/env bash

# Smoke test do transporte TLS (HTTPS) do SIGEH-DF: sobe a API com TLS
# habilitado (certificado autoassinado de dev) e valida health + login sobre
# HTTPS, alem de checar que HTTP puro nao atravessa a porta TLS.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
if [[ ! -f "${BACKEND_DIR}/Makefile" || ! -f "${BACKEND_DIR}/api/server.c" ]]; then
    echo "[ERRO] diretorio src/backend/ nao localizado."; exit 1
fi
[[ "${PWD}" != "${BACKEND_DIR}" ]] && cd "${BACKEND_DIR}"

API_PID=""
TMP_DIR="$(mktemp -d "${TMPDIR:-/tmp}/sigeh_tls.XXXXXX")"
RESP_FILE="${TMP_DIR}/response.json"
SERVER_LOG="${TMP_DIR}/api.log"
DB_FILE="../data/sigeh_v3.db"
DB_BACKUP=""
CERT_DIR="${TMP_DIR}/certs"
PORT=8443
BASE="https://localhost:${PORT}"

cleanup() {
    local code="${1:-0}"
    trap - EXIT INT TERM
    if [[ -n "${API_PID}" ]] && kill -0 "${API_PID}" 2>/dev/null; then
        kill "${API_PID}" 2>/dev/null || true
        wait "${API_PID}" 2>/dev/null || true
    fi
    if [[ -n "${DB_BACKUP}" && -f "${DB_BACKUP}" ]]; then mv "${DB_BACKUP}" "${DB_FILE}"; else rm -f "${DB_FILE}"; fi
    rm -rf "${TMP_DIR}"
    exit "${code}"
}
trap 'cleanup $?' EXIT INT TERM

fail() {
    echo "[ERRO] $1"
    [[ -s "${RESP_FILE}" ]] && echo "[ERRO] Resposta: $(tr -d '\r\n' < "${RESP_FILE}")"
    [[ -s "${SERVER_LOG}" ]] && { echo "[ERRO] Log:"; sed -n '1,40p' "${SERVER_LOG}"; }
    exit 1
}

echo "[INFO] Compilando API"
make api >/dev/null

echo "[INFO] Gerando certificado TLS de teste"
mkdir -p "${CERT_DIR}"
openssl req -x509 -newkey rsa:2048 -nodes -days 1 \
    -keyout "${CERT_DIR}/server.key" -out "${CERT_DIR}/server.crt" \
    -subj "/CN=localhost" 2>/dev/null

echo "[INFO] Preparando banco temporario"
[[ -f "${DB_FILE}" ]] && { DB_BACKUP="${TMP_DIR}/db.backup"; cp "${DB_FILE}" "${DB_BACKUP}"; }
make seed >/dev/null

echo "[INFO] Subindo API em HTTPS (porta ${PORT})"
SIGEH_TLS_CERT="${CERT_DIR}/server.crt" SIGEH_TLS_KEY="${CERT_DIR}/server.key" \
    ./build/sigeh_api "${PORT}" > "${SERVER_LOG}" 2>&1 &
API_PID=$!

# Espera o handshake TLS responder em /health.
ready=0
for _ in $(seq 1 15); do
    kill -0 "${API_PID}" 2>/dev/null || fail "a API encerrou antes de subir"
    if curl -sk -o "${RESP_FILE}" "${BASE}/health" 2>/dev/null && grep -Fq '"status"' "${RESP_FILE}"; then
        ready=1; break
    fi
    sleep 1
done
[[ "${ready}" == "1" ]] || fail "API TLS nao respondeu em /health"
echo "[OK] /health sobre HTTPS"

# Login sobre HTTPS, credenciais no corpo.
code="$(curl -sk -o "${RESP_FILE}" -w '%{http_code}' -X POST "${BASE}/sessao" \
    -H 'Content-Type: application/json' -d '{"login":"admin","senha":"admin123"}' || true)"
[[ "${code}" == "200" ]] || fail "login HTTPS retornou ${code}"
grep -Fq '"token"' "${RESP_FILE}" || fail "login HTTPS nao devolveu token"
TOKEN="$(sed -n 's/.*"token":"\([0-9a-f]*\)".*/\1/p' "${RESP_FILE}")"
echo "[OK] login sobre HTTPS (token recebido)"

# Rota autenticada sobre HTTPS.
code="$(curl -sk -o "${RESP_FILE}" -w '%{http_code}' "${BASE}/me" \
    -H "Authorization: Bearer ${TOKEN}" || true)"
[[ "${code}" == "200" ]] || fail "/me HTTPS retornou ${code}"
grep -Fq '"papel":"ADMIN"' "${RESP_FILE}" || fail "/me HTTPS sem papel ADMIN"
echo "[OK] rota autenticada sobre HTTPS"

# HTTP puro na porta TLS nao deve ser atendido como HTTP.
http_code="$(curl -s -o /dev/null -w '%{http_code}' --max-time 3 "http://localhost:${PORT}/health" || true)"
[[ "${http_code}" == "000" ]] || fail "HTTP puro foi atendido na porta TLS (codigo ${http_code})"
echo "[OK] HTTP puro rejeitado na porta TLS"

echo "[OK] Smoke test TLS concluido com sucesso"
