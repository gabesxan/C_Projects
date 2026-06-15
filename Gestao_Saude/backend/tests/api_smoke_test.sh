#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

if [[ ! -f "${BACKEND_DIR}/Makefile" || ! -f "${BACKEND_DIR}/api/health.c" ]]; then
    echo "[ERRO] Nao foi possivel localizar o diretorio backend/ do projeto."
    exit 1
fi

if [[ "${PWD}" != "${BACKEND_DIR}" ]]; then
    echo "[INFO] Entrando em ${BACKEND_DIR}"
    cd "${BACKEND_DIR}"
fi

API_PID=""
TMP_DIR="$(mktemp -d "${TMPDIR:-/tmp}/sigeh_api_smoke.XXXXXX")"
RESP_FILE="${TMP_DIR}/response.json"
SERVER_LOG="${TMP_DIR}/api.log"
DB_FILE="data/sigeh_v2.db"
DB_BACKUP=""
OPENSSL_DIR="${OPENSSL_DIR:-/opt/homebrew/opt/openssl@3}"

cleanup() {
    local exit_code="${1:-0}"

    trap - EXIT INT TERM

    if [[ -n "${API_PID}" ]] && kill -0 "${API_PID}" 2>/dev/null; then
        kill "${API_PID}" 2>/dev/null || true
        wait "${API_PID}" 2>/dev/null || true
    fi

    if [[ -n "${DB_BACKUP}" && -f "${DB_BACKUP}" ]]; then
        mv "${DB_BACKUP}" "${DB_FILE}"
    else
        rm -f "${DB_FILE}"
    fi

    rm -rf "${TMP_DIR}"
    exit "${exit_code}"
}

trap 'cleanup $?' EXIT
trap 'cleanup $?' INT TERM

fail() {
    echo "[ERRO] $1"
    if [[ -s "${RESP_FILE}" ]]; then
        echo "[ERRO] Resposta: $(tr -d '\r\n' < "${RESP_FILE}")"
    fi
    if [[ -s "${SERVER_LOG}" ]]; then
        echo "[ERRO] Log da API:"
        sed -n '1,120p' "${SERVER_LOG}"
    fi
    exit 1
}

assert_contains() {
    local needle="$1"
    local label="$2"

    if ! grep -Fq "${needle}" "${RESP_FILE}"; then
        fail "${label} nao contem ${needle}"
    fi
}

assert_matches() {
    local pattern="$1"
    local label="$2"

    if ! grep -Eq "${pattern}" "${RESP_FILE}"; then
        fail "${label} nao retornou o formato esperado"
    fi
}

request_and_assert() {
    local path="$1"
    local expected_status="$2"
    local label="$3"
    local auth="${4:-}"
    local body_check_type="${5:-}"
    local body_check_value="${6:-}"
    local http_code

    rm -f "${RESP_FILE}"

    if [[ -n "${auth}" ]]; then
        http_code="$(curl -sS -o "${RESP_FILE}" -w "%{http_code}" -u "${auth}" "http://localhost:8080${path}" || true)"
    else
        http_code="$(curl -sS -o "${RESP_FILE}" -w "%{http_code}" "http://localhost:8080${path}" || true)"
    fi

    if [[ "${http_code}" != "${expected_status}" ]]; then
        fail "${label} retornou HTTP ${http_code}, esperado ${expected_status}"
    fi

    case "${body_check_type}" in
        contains)
            assert_contains "${body_check_value}" "${label}"
            ;;
        matches)
            assert_matches "${body_check_value}" "${label}"
            ;;
        "")
            ;;
        *)
            fail "Tipo de validacao desconhecido para ${label}: ${body_check_type}"
            ;;
    esac

    echo "[OK] ${label}"
}

prepare_database() {
    local seed_source="${TMP_DIR}/api_smoke_seed.c"
    local seed_binary="${TMP_DIR}/api_smoke_seed"

    if [[ -f "${DB_FILE}" ]]; then
        DB_BACKUP="${TMP_DIR}/sigeh_v2.db.backup"
        cp "${DB_FILE}" "${DB_BACKUP}"
    fi

    cat > "${seed_source}" <<'EOF'
#include "database.h"
#include "usuario_repository.h"

#include <stdio.h>

int main(void)
{
    if (db_resetar_com_schema("data/schema_v2.sql") != 1)
    {
        fprintf(stderr, "falha ao resetar banco com schema\n");
        return 1;
    }

    if (usuario_repo_criar("admin", "secreta", "ADMIN", 0, 0) != 1)
    {
        fprintf(stderr, "falha ao criar usuario admin padrao\n");
        return 1;
    }

    return 0;
}
EOF

    gcc -Wall -Wextra -pedantic \
        -Idatabase \
        -Irepositories \
        -Iservices \
        -I"${OPENSSL_DIR}/include" \
        "${seed_source}" \
        database/database.c \
        repositories/usuario_repository.c \
        repositories/senha_util.c \
        repositories/repo_json.c \
        -o "${seed_binary}" \
        -lsqlite3 \
        -L"${OPENSSL_DIR}/lib" \
        -lcrypto

    "${seed_binary}"
}

wait_for_api() {
    local attempt
    local http_code

    for attempt in $(seq 1 15); do
        if ! kill -0 "${API_PID}" 2>/dev/null; then
            fail "A API encerrou antes de ficar disponivel"
        fi

        http_code="$(curl -sS -o "${RESP_FILE}" -w "%{http_code}" "http://localhost:8080/health" || true)"
        if [[ "${http_code}" == "200" ]] && grep -Fq '"status"' "${RESP_FILE}"; then
            return 0
        fi

        sleep 1
    done

    fail "A API nao ficou disponivel em /health apos 15 segundos"
}

echo "[INFO] Compilando API"
make api

echo "[INFO] Preparando banco temporario para o smoke test"
prepare_database

echo "[INFO] Subindo API em background"
./api/sigeh_api > "${SERVER_LOG}" 2>&1 &
API_PID=$!

wait_for_api

ADMIN_AUTH="admin:secreta"

request_and_assert "/health" "200" "/health" "" contains '"status"'
request_and_assert "/me" "200" "/me" "${ADMIN_AUTH}" contains '"papel":"ADMIN"'
request_and_assert "/pacientes" "200" "/pacientes" "${ADMIN_AUTH}" matches '^\[[^[:cntrl:]]*\]$'
request_and_assert "/medicos" "200" "/medicos" "${ADMIN_AUTH}" matches '^\[[^[:cntrl:]]*\]$'
request_and_assert "/relatorios/indicadores" "200" "/relatorios/indicadores" "${ADMIN_AUTH}" contains '"pacientesAtivos"'

echo "[OK] Smoke test da API concluido com sucesso"
