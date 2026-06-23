#!/usr/bin/env bash

# Teste de INTEGRACAO da API do SIGEH-DF: exercita fluxos ponta-a-ponta
# (varias rotas encadeadas, com transicoes de estado), indo alem do smoke test
# de liveness/escopo. Toda a autenticacao usa token de sessao (Bearer).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BACKEND_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

if [[ ! -f "${BACKEND_DIR}/Makefile" || ! -f "${BACKEND_DIR}/api/server.c" ]]; then
    echo "[ERRO] Nao foi possivel localizar o diretorio backend/ do projeto."
    exit 1
fi
if [[ "${PWD}" != "${BACKEND_DIR}" ]]; then
    echo "[INFO] Entrando em ${BACKEND_DIR}"
    cd "${BACKEND_DIR}"
fi

API_PID=""
TMP_DIR="$(mktemp -d "${TMPDIR:-/tmp}/sigeh_api_integ.XXXXXX")"
RESP_FILE="${TMP_DIR}/response.json"
SERVER_LOG="${TMP_DIR}/api.log"
DB_FILE="../data/sigeh_v3.db"
DB_BACKUP=""
OPENSSL_DIR="${OPENSSL_DIR:-/opt/homebrew/opt/openssl@3}"
TOKEN=""
HTTP_CODE=""

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
        echo "[ERRO] Log da API:"; sed -n '1,80p' "${SERVER_LOG}"
    fi
    exit 1
}

# Recria o banco oficial a partir do schema e cria o admin do teste.
prepare_database() {
    local seed_source="${TMP_DIR}/integ_seed.c"
    local seed_binary="${TMP_DIR}/integ_seed"

    if [[ -f "${DB_FILE}" ]]; then
        DB_BACKUP="${TMP_DIR}/sigeh_v3.db.backup"
        cp "${DB_FILE}" "${DB_BACKUP}"
    fi

    cat > "${seed_source}" <<'EOF'
#include "database.h"
#include "usuario_repository.h"
#include <stdio.h>
int main(void)
{
    if (db_resetar_com_schema("../data/schema_v3.sql") != 1)
    {
        fprintf(stderr, "falha ao resetar banco com schema\n");
        return 1;
    }
    if (usuario_repo_criar("Administrador", "admin", "secreta", "ADMIN", 0, 0) != 1)
    {
        fprintf(stderr, "falha ao criar usuario admin padrao\n");
        return 1;
    }
    return 0;
}
EOF

    gcc -Wall -Wextra -pedantic -Idatabase -Irepositories -Iservices -Iutil \
        -I"${OPENSSL_DIR}/include" \
        "${seed_source}" database/database.c repositories/usuario_repository.c \
        util/senha_util.c util/repo_json.c \
        -o "${seed_binary}" -lsqlite3 -L"${OPENSSL_DIR}/lib" -lcrypto
    "${seed_binary}"
}

wait_for_api() {
    local attempt http_code
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

# Login via POST /sessao (corpo JSON); ecoa o token.
obter_token() {
    curl -sS -X POST "http://localhost:8080/sessao" -H 'Content-Type: application/json' \
        -d "{\"login\":\"$1\",\"senha\":\"$2\"}" \
        | sed -n 's/.*"token":"\([0-9a-f]*\)".*/\1/p'
}

# api METODO CAMINHO [json] -> popula HTTP_CODE e RESP_FILE (auth Bearer).
api() {
    local method="$1" path="$2" body="${3:-}"
    rm -f "${RESP_FILE}"
    if [[ -n "${body}" ]]; then
        HTTP_CODE="$(curl -sS -o "${RESP_FILE}" -w '%{http_code}' -X "${method}" \
            -H "Authorization: Bearer ${TOKEN}" -H 'Content-Type: application/json' \
            -d "${body}" "http://localhost:8080${path}" || true)"
    else
        HTTP_CODE="$(curl -sS -o "${RESP_FILE}" -w '%{http_code}' -X "${method}" \
            -H "Authorization: Bearer ${TOKEN}" "http://localhost:8080${path}" || true)"
    fi
}

expect() { # expect <status_esperado> <rotulo>
    [[ "${HTTP_CODE}" == "$1" ]] || fail "$2: HTTP ${HTTP_CODE} (esperado $1)"
    echo "[OK] $2"
}
body_has() { grep -Fq "$1" "${RESP_FILE}" || fail "$2: resposta nao contem '$1'"; }

# --------------------------------------------------------------------------
echo "[INFO] Compilando API"
make api
echo "[INFO] Preparando banco temporario para o teste de integracao"
prepare_database
echo "[INFO] Subindo API em background"
./build/sigeh_api > "${SERVER_LOG}" 2>&1 &
API_PID=$!
wait_for_api

TOKEN="$(obter_token admin secreta)"
[[ -n "${TOKEN}" ]] || fail "nao obteve token do admin"
echo "[OK] login admin (token de sessao)"

# Fixtures comuns (IDs deterministicos no banco recem-resetado: tudo id 1).
api POST /medicos '{"nome":"Dr Cardio","crm":"CRM-INT-1","especialidade":"Cardiologia","regiao":"1"}'
expect 201 "fixture: medico cardiologista"
api POST /pacientes '{"nome":"Paciente Integra","nascimento":"1985-03-10","documento":"111222333","tipo_documento":"CPF","telefone":"61999990000","sexo":"M","regiao":"1"}'
expect 201 "fixture: paciente"
api POST /prontuarios '{"paciente_id":"1","medico_id":"1","data":"2026-07-01","observacoes":"obs","diagnostico":"Avaliacao inicial","conduta":"acompanhar","alerta_importante":"0"}'
expect 201 "fixture: prontuario"

echo "--- Fluxo 1: recepcao -> triagem -> agendamento ---"
api POST /checkins '{"paciente_id":"1","destino":"TRIAGEM"}'
expect 201 "checkin gera senha"; body_has '"senha"' "checkin"
api GET /checkins
expect 200 "fila de recepcao lista o checkin"
api POST /checkins/1/chamar
expect 200 "recepcao chama a senha"
api POST /triagens '{"paciente_id":"1","tipo":"3","itens":"dor_toracica"}'
expect 201 "triagem registrada"
api POST /agendamentos '{"paciente_id":"1","medico_id":"1","data":"2026-07-05","horario":"09:00"}'
expect 201 "agendamento criado"
api POST /checkins/1/encerrar
expect 200 "recepcao encerra a senha"

echo "--- Fluxo 2: triagem -> reclassificacao -> encaminhamento ---"
api POST /triagens/1/reclassificar '{"itens":"dor_leve","justificativa":"reavaliacao apos analgesia"}'
expect 200 "triagem reclassificada (versionada)"
api POST /triagem/1/encaminhar '{"especialidade":"Cardiologia","data":"2026-07-06","horario":"11:00"}'
expect 201 "encaminhamento gera agendamento na especialidade"

echo "--- Fluxo 3: exame -> resultado -> retificacao ---"
api POST /analitos '{"codigo":"HGB","nome":"Hemoglobina","unidade":"g/dL","ref_min":"12","ref_max":"16","metodo":"Automacao"}'
expect 201 "analito HGB criado"
api POST /paineis/1/analitos '{"analito_id":"1","ordem":"1"}'
expect 201 "analito vinculado ao painel do exame"
api POST /exames '{"paciente_id":"1","medico_id":"1","prontuario_id":"1","tipo":"1","data_solicitacao":"2026-07-01","urgente":"0"}'
expect 201 "exame solicitado"
api POST /exames/1/status '{"valor":"AUTORIZADO"}'
expect 200 "exame autorizado"
api POST /exames/1/status '{"valor":"COLETADO"}'
expect 200 "exame coletado"
api POST /exames/1/resultados-analitos '{"analito_id":"1","valor":"13.5","valor_texto":"13.5","observacao":"amostra integra"}'
expect 201 "resultado estruturado por analito"
api GET /exames/1/resultados-analitos
expect 200 "resultados estruturados listados"; body_has '"foraReferencia":0' "analito dentro da faixa"
api POST /exames/1/resultado '{"resultado":"Hemoglobina 13.5","critico":"0"}'
expect 200 "resultado registrado (CONCLUIDO)"
api GET /exames
expect 200 "exames listados"; body_has 'CONCLUIDO' "exame concluido"
api POST /exames/1/retificar '{"resultado":"Hemoglobina 13.8 (corrigido)","critico":"0","justificativa":"erro de digitacao"}'
expect 200 "resultado retificado (nova versao)"
api GET /exames
body_has '13.8' "exame retificado vigente"
api POST /exames/2/resultados-analitos/retificar '{"analito_id":"1","valor":"17.2","valor_texto":"17.2","observacao":"reprocessado","justificativa":"recalibracao do equipamento"}'
expect 200 "analito retificado com nova versao"
api GET /exames/3/resultados-analitos
expect 200 "resultado estruturado retificado listado"; body_has '"foraReferencia":1' "retificacao marca fora da faixa"

echo "--- Fluxo 4: financeiro -> convenio -> cobranca -> baixa ---"
api POST /convenios '{"nome":"Unimed Integracao"}'
expect 201 "convenio criado"
api POST /pacientes/1/convenio '{"convenio_id":"1"}'
expect 200 "convenio vinculado ao paciente"
api POST /cobrancas '{"paciente_id":"1","forma":"CONVENIO","convenio_id":"1","origem":"consulta","descricao":"Consulta cardiologia","valor_centavos":20000}'
expect 201 "cobranca lancada"
api POST /cobrancas/1/status '{"valor":"AUTORIZADA"}'
expect 200 "cobranca autorizada"
api POST /cobrancas/1/status '{"valor":"PAGA"}'
expect 200 "cobranca paga (baixa)"
api GET /cobrancas/demonstrativo
expect 200 "demonstrativo financeiro"; body_has '"recebidoCentavos":20000' "demonstrativo reflete a baixa"

echo "--- Fluxo 5: farmacia -> dispensacao debita estoque e gera cobranca ---"
api POST /medicamentos '{"nome":"Dipirona","apresentacao":"500mg comprimido","unidade":"comprimido","estoque_minimo":"5","preco_centavos":"150"}'
expect 201 "medicamento criado"
api POST /estoque '{"medicamento_id":"1","lote":"L1","validade":"2026-12-31","quantidade":"50","localizacao":"Prateleira A"}'
expect 201 "entrada de estoque (50)"
api GET /medicamentos/1/estoque
expect 200 "estoque do medicamento"; body_has '"quantidade":50' "saldo apos entrada"
# Dispensa 3 ao paciente 1: debita o estoque (50 -> 47) e gera cobranca
# PARTICULAR de 3 x 150 = 450 centavos (vinculo com o financeiro).
api POST /medicamentos/1/dispensar '{"paciente_id":"1","quantidade":"3","motivo":"pos-consulta"}'
expect 200 "dispensacao ao paciente"
body_has '"cobrancaGerada":true' "dispensacao gera cobranca"
body_has '"valorCentavos":450' "valor da dispensacao (3 x 150)"
body_has '"saldo":47' "estoque debitado pela dispensacao"
# A cobranca da farmacia aparece no financeiro.
api GET /cobrancas
expect 200 "cobrancas listadas"; body_has '"origem":"farmacia"' "cobranca de farmacia lancada"
# Saldo insuficiente nao dispensa nem altera o estoque.
api POST /medicamentos/1/dispensar '{"paciente_id":"1","quantidade":"9999","motivo":"x"}'
expect 400 "dispensacao sem saldo"; body_has 'estoque insuficiente' "erro de saldo insuficiente"
api GET /medicamentos/1/estoque
expect 200 "estoque inalterado apos falha"; body_has '"quantidade":47' "saldo preservado"
# Paciente inexistente e recusado.
api POST /medicamentos/1/dispensar '{"paciente_id":"9999","quantidade":"1"}'
expect 400 "dispensacao paciente invalido"; body_has 'paciente inexistente' "erro de paciente"

echo "--- Fluxo 6: validacoes de entrada invalida (400) ---"
# POST /sessao sem credenciais (cai antes da autenticacao; nao conta como falha
# de login para o rate-limit por IP).
api POST /sessao '{}'
expect 400 "sessao sem login/senha"; body_has "informe login e senha" "sessao 400 informativo"
# Cadastros com corpo vazio: o repository recusa e a rota devolve 400.
api POST /pacientes '{}'
expect 400 "paciente com dados invalidos"; body_has "dados invalidos" "paciente 400 informativo"
api POST /medicos '{}'
expect 400 "medico com dados invalidos"; body_has "dados invalidos para medico" "medico 400 informativo"
# Prontuario sem medico autor definido.
api POST /prontuarios '{}'
expect 400 "prontuario sem medico autor"; body_has "medico autor do prontuario indefinido" "prontuario 400 informativo"
# Relatorio por periodo exige inicio e fim.
api GET "/relatorios/agendamentos"
expect 400 "relatorio sem datas"; body_has "informe inicio e fim" "relatorio 400 informativo"

echo "[OK] Teste de integracao da API concluido com sucesso"
