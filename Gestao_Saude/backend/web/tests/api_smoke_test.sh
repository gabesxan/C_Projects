#!/usr/bin/env bash

# Ativa modo estrito: encerra em erro, em variavel indefinida e em falha de pipeline.
set -euo pipefail

# Descobre o diretorio absoluto onde este script esta salvo.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Sobe um nivel para chegar na pasta backend/.
BACKEND_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

# Verifica se a pasta encontrada realmente parece ser o backend esperado.
if [[ ! -f "${BACKEND_DIR}/Makefile" || ! -f "${BACKEND_DIR}/api/server.c" ]]; then
    # Informa claramente que o backend nao foi localizado.
    echo "[ERRO] Nao foi possivel localizar o diretorio backend/ do projeto."
    # Encerra com falha para evitar executar no lugar errado.
    exit 1
fi

# Se o usuario chamou o script fora de backend/, entra automaticamente no diretorio correto.
if [[ "${PWD}" != "${BACKEND_DIR}" ]]; then
    # Mostra qual diretorio sera usado durante o teste.
    echo "[INFO] Entrando em ${BACKEND_DIR}"
    # Muda o diretorio atual para backend/.
    cd "${BACKEND_DIR}"
fi

# Guarda o PID do servidor quando ele for iniciado; comeca vazio.
API_PID=""
# Cria um diretorio temporario exclusivo para os arquivos auxiliares do teste.
TMP_DIR="$(mktemp -d "${TMPDIR:-/tmp}/sigeh_api_smoke.XXXXXX")"
# Define o arquivo temporario que recebera o corpo das respostas HTTP.
RESP_FILE="${TMP_DIR}/response.json"
# Define o arquivo temporario que recebera o log do servidor.
SERVER_LOG="${TMP_DIR}/api.log"
# Aponta para o banco padrao usado pela API dentro de backend/.
DB_FILE="../data/sigeh_v3.db"
# Guarda o caminho do backup do banco, caso ele exista antes do teste.
DB_BACKUP=""
# Permite reaproveitar o mesmo caminho de OpenSSL usado pelo Makefile do backend.
OPENSSL_DIR="${OPENSSL_DIR:-/opt/homebrew/opt/openssl@3}"

# Funcao chamada ao sair do script para sempre desligar a API e restaurar o ambiente.
cleanup() {
    # Recebe o codigo de saida original do script.
    local exit_code="${1:-0}"

    # Remove os traps para evitar chamada recursiva desta mesma limpeza.
    trap - EXIT INT TERM

    # Se a API foi iniciada e ainda esta viva, encerra o processo.
    if [[ -n "${API_PID}" ]] && kill -0 "${API_PID}" 2>/dev/null; then
        # Envia sinal de termino ao processo da API.
        kill "${API_PID}" 2>/dev/null || true
        # Aguarda o processo finalizar para nao deixar zumbi.
        wait "${API_PID}" 2>/dev/null || true
    fi

    # Se havia um banco anterior salvo, restaura esse banco original.
    if [[ -n "${DB_BACKUP}" && -f "${DB_BACKUP}" ]]; then
        # Move o backup de volta para o caminho oficial do banco.
        mv "${DB_BACKUP}" "${DB_FILE}"
    else
        # Caso nao existisse banco antes do teste, remove o banco temporario criado.
        rm -f "${DB_FILE}"
    fi

    # Remove todos os arquivos temporarios usados durante o smoke test.
    rm -rf "${TMP_DIR}"
    # Finaliza com o mesmo codigo de saida capturado anteriormente.
    exit "${exit_code}"
}

# Garante limpeza automatica na saida normal do script.
trap 'cleanup $?' EXIT
# Garante limpeza automatica tambem em interrupcoes comuns.
trap 'cleanup $?' INT TERM

# Funcao utilitaria para imprimir erro detalhado e encerrar o teste.
fail() {
    # Mostra a mensagem principal de erro recebida.
    echo "[ERRO] $1"
    # Se houver uma resposta HTTP salva, exibe essa resposta em uma linha.
    if [[ -s "${RESP_FILE}" ]]; then
        echo "[ERRO] Resposta: $(tr -d '\r\n' < "${RESP_FILE}")"
    fi
    # Se houver log do servidor, exibe um trecho para facilitar diagnostico.
    if [[ -s "${SERVER_LOG}" ]]; then
        echo "[ERRO] Log da API:"
        sed -n '1,120p' "${SERVER_LOG}"
    fi
    # Encerra imediatamente com falha.
    exit 1
}

# Valida se a resposta atual contem um trecho textual obrigatorio.
assert_contains() {
    # Texto minimo esperado no corpo da resposta.
    local needle="$1"
    # Nome amigavel da rota em validacao.
    local label="$2"

    # Procura o texto esperado no arquivo de resposta.
    if ! grep -Fq "${needle}" "${RESP_FILE}"; then
        # Falha se o texto obrigatorio nao estiver presente.
        fail "${label} nao contem ${needle}"
    fi
}

# Valida a resposta atual com uma expressao regular simples.
assert_matches() {
    # Padrao regex esperado para o corpo retornado.
    local pattern="$1"
    # Nome amigavel da rota em validacao.
    local label="$2"

    # Confere se o corpo da resposta casa com o padrao informado.
    if ! grep -Eq "${pattern}" "${RESP_FILE}"; then
        # Falha quando o formato nao corresponde ao esperado.
        fail "${label} nao retornou o formato esperado"
    fi
}

# Faz login via POST /sessao (corpo JSON) e ecoa o token de sessao retornado.
obter_token() {
    local login="$1"
    local senha="$2"
    curl -sS -X POST "http://localhost:8080/sessao" \
        -H 'Content-Type: application/json' \
        -d "{\"login\":\"${login}\",\"senha\":\"${senha}\"}" \
        | sed -n 's/.*"token":"\([0-9a-f]*\)".*/\1/p'
}

# Executa uma requisicao HTTP e valida status e conteudo minimo.
request_and_assert() {
    # Caminho da rota a ser testada.
    local path="$1"
    # Status HTTP esperado para a rota.
    local expected_status="$2"
    # Rotulo mostrado nas mensagens de sucesso ou erro.
    local label="$3"
    # Token de sessao opcional (Authorization: Bearer).
    local token="${4:-}"
    # Tipo de validacao do corpo: contains, matches ou vazio.
    local body_check_type="${5:-}"
    # Valor associado ao tipo de validacao escolhido.
    local body_check_value="${6:-}"
    # Variavel que recebera o status HTTP real devolvido pelo curl.
    local http_code

    # Remove qualquer resposta anterior antes de chamar a proxima rota.
    rm -f "${RESP_FILE}"

    # Se houver token, envia no cabecalho Authorization: Bearer.
    if [[ -n "${token}" ]]; then
        http_code="$(curl -sS -o "${RESP_FILE}" -w "%{http_code}" -H "Authorization: Bearer ${token}" "http://localhost:8080${path}" || true)"
    else
        # Caso contrario, chama a rota sem credenciais.
        http_code="$(curl -sS -o "${RESP_FILE}" -w "%{http_code}" "http://localhost:8080${path}" || true)"
    fi

    # Compara o status retornado com o status esperado.
    if [[ "${http_code}" != "${expected_status}" ]]; then
        # Falha se o status HTTP estiver diferente do contrato minimo esperado.
        fail "${label} retornou HTTP ${http_code}, esperado ${expected_status}"
    fi

    # Escolhe como validar o corpo da resposta.
    case "${body_check_type}" in
        contains)
            # Exige um trecho textual minimo no JSON.
            assert_contains "${body_check_value}" "${label}"
            ;;
        matches)
            # Exige que a resposta obedece a um formato simples por regex.
            assert_matches "${body_check_value}" "${label}"
            ;;
        "")
            # Nao faz validacao extra quando nenhum tipo foi informado.
            ;;
        *)
            # Protege contra uso incorreto da propria funcao auxiliar.
            fail "Tipo de validacao desconhecido para ${label}: ${body_check_type}"
            ;;
    esac

    # Informa que a rota passou em todas as verificacoes.
    echo "[OK] ${label}"
}

# Executa uma requisicao JSON autenticada/anonima e valida status e conteudo.
request_json_and_assert() {
    local method="$1"
    local path="$2"
    local body="$3"
    local expected_status="$4"
    local label="$5"
    local token="${6:-}"
    local body_check_type="${7:-}"
    local body_check_value="${8:-}"
    local http_code

    rm -f "${RESP_FILE}"

    if [[ -n "${token}" ]]; then
        http_code="$(
            curl -sS -o "${RESP_FILE}" -w "%{http_code}" \
                -H "Authorization: Bearer ${token}" \
                -H 'Content-Type: application/json' \
                -X "${method}" "http://localhost:8080${path}" -d "${body}" || true
        )"
    else
        http_code="$(
            curl -sS -o "${RESP_FILE}" -w "%{http_code}" \
                -H 'Content-Type: application/json' \
                -X "${method}" "http://localhost:8080${path}" -d "${body}" || true
        )"
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

# Recria um banco limpo e semeia um usuario admin temporario para o smoke test.
prepare_database() {
    # Arquivo-fonte temporario de um pequeno helper em C para reset e seed.
    local seed_source="${TMP_DIR}/api_smoke_seed.c"
    # Binario temporario gerado a partir desse helper em C.
    local seed_binary="${TMP_DIR}/api_smoke_seed"

    # Se ja existir banco da API, faz backup para restaurar depois.
    if [[ -f "${DB_FILE}" ]]; then
        DB_BACKUP="${TMP_DIR}/sigeh_v3.db.backup"
        cp "${DB_FILE}" "${DB_BACKUP}"
    fi

    # Escreve um programa C minimo que reaproveita as funcoes reais do backend.
    cat > "${seed_source}" <<'EOF'
/* Importa a API de banco para resetar o SQLite a partir do schema. */
#include "database.h"
/* Importa o repository de usuarios para criar o admin temporario. */
#include "usuario_repository.h"

/* Importa fprintf para relatar erros no stderr. */
#include <stdio.h>

/* Funcao principal do helper temporario. */
int main(void)
{
    /* Recria o banco oficial da API usando o schema versionado do projeto. */
    if (db_resetar_com_schema("../data/schema_v3.sql") != 1)
    {
        /* Explica a falha caso o reset nao funcione. */
        fprintf(stderr, "falha ao resetar banco com schema\n");
        /* Retorna erro para o script abortar o smoke test. */
        return 1;
    }

    /* Cria o usuario admin padrao documentado para autenticar as rotas. */
    if (usuario_repo_criar("Administrador", "admin", "secreta", "ADMIN", 0, 0) != 1)
    {
        /* Explica a falha caso o usuario nao possa ser criado. */
        fprintf(stderr, "falha ao criar usuario admin padrao\n");
        /* Retorna erro para impedir um teste com ambiente inconsistente. */
        return 1;
    }

    /* Indica sucesso ao finalizar o preparo do banco. */
    return 0;
}
EOF

    # Compila o helper temporario com os mesmos includes e bibliotecas do backend.
    gcc -Wall -Wextra -pedantic \
        -Idatabase \
        -Irepositories \
        -Iservices \
        -Iutil \
        -I"${OPENSSL_DIR}/include" \
        "${seed_source}" \
        database/database.c \
        repositories/usuario_repository.c \
        util/senha_util.c \
        util/repo_json.c \
        -o "${seed_binary}" \
        -lsqlite3 \
        -L"${OPENSSL_DIR}/lib" \
        -lcrypto

    # Executa o helper compilado para resetar o banco e inserir o admin.
    "${seed_binary}"
}

# Espera a API ficar pronta consultando /health por ate 15 segundos.
wait_for_api() {
    # Contador de tentativas.
    local attempt
    # Variavel para receber o status HTTP do /health.
    local http_code

    # Faz varias tentativas para dar tempo de a API abrir o socket.
    for attempt in $(seq 1 15); do
        # Se o processo morreu antes de responder, aborta imediatamente.
        if ! kill -0 "${API_PID}" 2>/dev/null; then
            fail "A API encerrou antes de ficar disponivel"
        fi

        # Consulta o endpoint de health e salva o corpo retornado.
        http_code="$(curl -sS -o "${RESP_FILE}" -w "%{http_code}" "http://localhost:8080/health" || true)"
        # Considera a API pronta quando /health responde 200 e contem o campo status.
        if [[ "${http_code}" == "200" ]] && grep -Fq '"status"' "${RESP_FILE}"; then
            return 0
        fi

        # Aguarda um segundo antes da proxima tentativa.
        sleep 1
    done

    # Falha se o servidor nao ficou pronto dentro do tempo limite.
    fail "A API nao ficou disponivel em /health apos 15 segundos"
}

# Informa o inicio da etapa de build.
echo "[INFO] Compilando API"
# Compila a API antes de rodar qualquer verificacao HTTP.
make api

# Informa o inicio da etapa de preparo do banco.
echo "[INFO] Preparando banco temporario para o smoke test"
# Recria o banco e insere o admin temporario.
prepare_database

# Informa que o processo da API sera iniciado em background.
echo "[INFO] Subindo API em background"
# Sobe a API redirecionando a saida para um log temporario.
./build/sigeh_api > "${SERVER_LOG}" 2>&1 &
# Guarda o PID do processo que acabou de ser iniciado.
API_PID=$!

# Espera o servidor ficar pronto antes de testar as demais rotas.
wait_for_api

# Faz login do admin e guarda o token de sessao usado nas rotas autenticadas.
ADMIN_TOKEN="$(obter_token admin secreta)"

# Testa a rota publica de saude e exige o campo status.
request_and_assert "/health" "200" "/health" "" contains '"status"'
# Testa a rota de sessao autenticada e exige o papel ADMIN no JSON.
request_and_assert "/me" "200" "/me" "${ADMIN_TOKEN}" contains '"papel":"ADMIN"'
# /me renova a sessao (TTL deslizante) e expoe a nova expiracao.
request_and_assert "/me" "200" "/me renova sessao (expiraEm)" "${ADMIN_TOKEN}" contains '"expiraEm":"20'
# Testa a listagem de pacientes e exige um JSON em formato de array.
request_and_assert "/pacientes" "200" "/pacientes" "${ADMIN_TOKEN}" matches '^\[[^[:cntrl:]]*\]$'
# Testa a listagem de medicos e exige um JSON em formato de array.
request_and_assert "/medicos" "200" "/medicos" "${ADMIN_TOKEN}" matches '^\[[^[:cntrl:]]*\]$'
# Testa o relatorio de indicadores e exige um campo chave do JSON retornado.
request_and_assert "/relatorios/indicadores" "200" "/relatorios/indicadores" "${ADMIN_TOKEN}" contains '"pacientesAtivos"'
# Testa o relatorio de distribuicao e exige um campo chave do JSON retornado.
request_and_assert "/relatorios/distribuicao" "200" "/relatorios/distribuicao" "${ADMIN_TOKEN}" contains '"pacientesPorRegiao"'
# Testa o relatorio por periodo com datas e exige o total no JSON retornado.
request_and_assert "/relatorios/agendamentos?inicio=2026-06-01&fim=2026-06-30" "200" "/relatorios/agendamentos" "${ADMIN_TOKEN}" contains '"total"'
# Testa que o relatorio por periodo sem datas retorna 400.
request_and_assert "/relatorios/agendamentos" "400" "/relatorios/agendamentos (sem datas)" "${ADMIN_TOKEN}"

# --- Catalogo de laboratorio: analitos e paineis ---
# Cria dois analitos para o painel do hemograma (tipo 1).
request_json_and_assert "POST" "/analitos" \
    '{"codigo":"HGB","nome":"Hemoglobina","unidade":"g/dL","ref_min":"12","ref_max":"16","metodo":"Automacao"}' \
    "201" "/analitos POST (HGB)" "${ADMIN_TOKEN}" contains '"status":"criado"'
request_json_and_assert "POST" "/analitos" \
    '{"codigo":"GLI","nome":"Glicose","unidade":"mg/dL","ref_min":"70","ref_max":"99","metodo":"Hexoquinase"}' \
    "201" "/analitos POST (GLI)" "${ADMIN_TOKEN}" contains '"status":"criado"'
# Lista e contagem dos analitos ativos.
request_and_assert "/analitos" "200" "/analitos" "${ADMIN_TOKEN}" contains '"codigo":"HGB"'
request_and_assert "/analitos/contar" "200" "/analitos/contar" "${ADMIN_TOKEN}" contains '"ativos":2'
# Monta um painel do tipo 1 e garante a ordenacao no laudo.
request_json_and_assert "POST" "/paineis/1/analitos" '{"analito_id":"1","ordem":"2"}' \
    "201" "/paineis/1/analitos POST (HGB)" "${ADMIN_TOKEN}" contains '"status":"criado"'
request_json_and_assert "POST" "/paineis/1/analitos" '{"analito_id":"2","ordem":"1"}' \
    "201" "/paineis/1/analitos POST (GLI)" "${ADMIN_TOKEN}" contains '"status":"criado"'
request_and_assert "/paineis/1/analitos" "200" "/paineis/1/analitos" "${ADMIN_TOKEN}" contains '"ordem":1'
# Remocao destrutiva exige motivo no corpo.
request_json_and_assert "POST" "/paineis/1/remover" '{"analito_id":"2"}' \
    "404" "/paineis/1/remover (sem motivo)" "${ADMIN_TOKEN}" contains 'motivo ausente'
request_json_and_assert "POST" "/paineis/1/remover" '{"analito_id":"2","motivo":"painel simplificado"}' \
    "200" "/paineis/1/remover" "${ADMIN_TOKEN}" contains '"status":"removido"'
request_json_and_assert "DELETE" "/analitos/2" '{"motivo":"catalogo duplicado"}' \
    "200" "/analitos/2 DELETE" "${ADMIN_TOKEN}" contains '"status":"removido"'
request_and_assert "/analitos/contar" "200" "/analitos/contar apos delete" "${ADMIN_TOKEN}" contains '"ativos":1'

# --- Escopo de dados por papel: o MEDICO ve apenas os proprios dados ---
# Endereco base reutilizado nas criacoes via POST (parametros vao no CORPO JSON).
BASE="http://localhost:8080"
# Helper: POST autenticado (Basic) enviando o corpo como JSON.
post_json() {
    curl -sS -H "Authorization: Bearer ${ADMIN_TOKEN}" -H 'Content-Type: application/json' \
        -X POST "${BASE}$1" -d "$2" >/dev/null
}
# Informa o inicio da preparacao do cenario de escopo.
echo "[INFO] Preparando cenario de escopo por papel (MEDICO)"
# Cria um medico (sera o id 1 no banco recem-resetado).
post_json "/medicos" '{"nome":"DrSmoke","crm":"CRM-SMOKE","especialidade":"Cardiologia","regiao":"1"}'
# Cria dois pacientes: o primeiro sera vinculado ao medico, o segundo nao.
post_json "/pacientes" '{"nome":"AnaSmoke","nascimento":"1990-01-01","documento":"900900","tipo_documento":"CPF","telefone":"61","sexo":"F","regiao":"1"}'
post_json "/pacientes" '{"nome":"BiaSmoke","nascimento":"1980-01-01","documento":"900901","tipo_documento":"CPF","telefone":"61","sexo":"F","regiao":"2"}'
# Agenda a paciente 1 (Ana) com o medico 1, criando o vinculo de escopo.
post_json "/agendamentos" '{"paciente_id":"1","medico_id":"1","data":"2026-06-14","horario":"09:00"}'
# Cria um usuario MEDICO ligado ao medico 1 para autenticar com escopo.
post_json "/usuarios" '{"nome":"MedSmoke","login":"medsmoke","senha":"med123","papel":"MEDICO","medico_id":"1"}'
# Credencial do medico recem-criado.
MED_TOKEN="$(obter_token medsmoke med123)"

# O MEDICO deve ver a propria paciente (Ana) na listagem ampla de pacientes.
request_and_assert "/pacientes" "200" "/pacientes (MEDICO escopado)" "${MED_TOKEN}" contains 'AnaSmoke'
# Valida que a paciente fora do escopo (Bia) NAO aparece para o medico.
rm -f "${RESP_FILE}"
curl -sS -o "${RESP_FILE}" -H "Authorization: Bearer ${MED_TOKEN}" "${BASE}/pacientes" >/dev/null
if grep -Fq 'BiaSmoke' "${RESP_FILE}"; then
    fail "/pacientes (MEDICO) vazou paciente fora do escopo"
fi
echo "[OK] /pacientes escopado por papel (MEDICO nao ve paciente de fora)"
# O MEDICO deve ver o proprio agendamento na listagem ampla de agendamentos.
request_and_assert "/agendamentos" "200" "/agendamentos (MEDICO escopado)" "${MED_TOKEN}" contains '"medicoId":1'

# Cria um segundo medico (id 2) para gerar registros fora do escopo do primeiro.
post_json "/medicos" '{"nome":"DrOutro","crm":"CRM-OUTRO","especialidade":"Ortopedia","regiao":"2"}'
# Prontuario do medico 1 (escopo do MEDICO logado) e do medico 2 (fora do escopo).
post_json "/prontuarios" '{"paciente_id":"1","medico_id":"1","data":"2026-06-14","observacoes":"obs","diagnostico":"DiagMed1","conduta":"c","alerta_importante":"0"}'
post_json "/prontuarios" '{"paciente_id":"2","medico_id":"2","data":"2026-06-14","observacoes":"obs","diagnostico":"DiagMed2","conduta":"c","alerta_importante":"0"}'
# Exame do medico 1 e do medico 2 (vinculados aos prontuarios 1 e 2).
post_json "/exames" '{"paciente_id":"1","medico_id":"1","prontuario_id":"1","tipo":"1","data_solicitacao":"2026-06-14","urgente":"0"}'
post_json "/exames" '{"paciente_id":"2","medico_id":"2","prontuario_id":"2","tipo":"5","data_solicitacao":"2026-06-14","urgente":"1"}'

# O MEDICO ve o proprio prontuario (DiagMed1) e nao o do outro medico (DiagMed2).
request_and_assert "/prontuarios" "200" "/prontuarios (MEDICO escopado)" "${MED_TOKEN}" contains 'DiagMed1'
rm -f "${RESP_FILE}"
curl -sS -o "${RESP_FILE}" -H "Authorization: Bearer ${MED_TOKEN}" "${BASE}/prontuarios" >/dev/null
if grep -Fq 'DiagMed2' "${RESP_FILE}"; then
    fail "/prontuarios (MEDICO) vazou prontuario de outro medico"
fi
echo "[OK] /prontuarios escopado por papel (MEDICO nao ve prontuario de outro)"

# O MEDICO ve o proprio exame (medicoId 1) e nao o do outro medico (medicoId 2).
request_and_assert "/exames" "200" "/exames (MEDICO escopado)" "${MED_TOKEN}" contains '"medicoId":1'
rm -f "${RESP_FILE}"
curl -sS -o "${RESP_FILE}" -H "Authorization: Bearer ${MED_TOKEN}" "${BASE}/exames" >/dev/null
if grep -Fq '"medicoId":2' "${RESP_FILE}"; then
    fail "/exames (MEDICO) vazou exame de outro medico"
fi
echo "[OK] /exames escopado por papel (MEDICO nao ve exame de outro)"

# Triagens de tipos diferentes: tipo 3 -> Cardiologia (do DrSmoke), tipo 2 -> Ortopedia.
post_json "/triagens" '{"paciente_id":"1","tipo":"3","itens":"dor_toracica"}'
post_json "/triagens" '{"paciente_id":"2","tipo":"2","itens":"dor_moderada"}'

# O MEDICO (Cardiologia) ve a triagem cardiologica (tipo 3) na fila escopada.
request_and_assert "/triagens" "200" "/triagens (MEDICO escopado por especialidade)" "${MED_TOKEN}" contains '"tipoTriagem":3'
request_and_assert "/especialidades" "200" "/especialidades (MEDICO)" "${MED_TOKEN}" contains 'Cardiologia'
request_and_assert "/especialidades/3/problemas" "200" "/especialidades/3/problemas" "${MED_TOKEN}" contains 'dor no peito'
# Valida que a triagem de outra especialidade (tipo 2) NAO aparece para ele.
rm -f "${RESP_FILE}"
curl -sS -o "${RESP_FILE}" -H "Authorization: Bearer ${MED_TOKEN}" "${BASE}/triagens" >/dev/null
if grep -Fq '"tipoTriagem":2' "${RESP_FILE}"; then
    fail "/triagens (MEDICO) vazou triagem de outra especialidade"
fi
echo "[OK] /triagens escopado por especialidade (MEDICO nao ve fora da sua area)"

# Resumo do proprio medico: contagens escopadas (1 paciente, 1 prontuario, etc.).
request_and_assert "/me/resumo" "200" "/me/resumo (contagens do MEDICO)" "${MED_TOKEN}" contains '"medicoId":1'
# Reforca que o resumo traz os totais do medico (prontuarios assinados por ele).
request_and_assert "/me/resumo" "200" "/me/resumo (totais do MEDICO)" "${MED_TOKEN}" contains '"prontuarios":1'

# --- Prescricoes / medicacao: matriz de papeis ---
# Medico 1 prescreve para a paciente 1.
post_json "/prescricoes" '{"paciente_id":"1","medico_id":"1","medicamento":"DipironaSmoke","dosagem":"500mg","frequencia":"8/8h","observacoes":"apos refeicoes"}'
# Usuarios de enfermagem e do proprio paciente para validar os acessos.
post_json "/usuarios" '{"nome":"EnfSmoke","login":"enfsmoke","senha":"enf123","papel":"ENFERMAGEM"}'
post_json "/usuarios" '{"nome":"PacSmoke","login":"pacsmoke","senha":"pac123","papel":"PACIENTE","paciente_id":"1"}'
ENF_TOKEN="$(obter_token enfsmoke enf123)"
PAC_TOKEN="$(obter_token pacsmoke pac123)"

# ADMIN ve todas as prescricoes.
request_and_assert "/prescricoes" "200" "/prescricoes (ADMIN)" "${ADMIN_TOKEN}" contains 'DipironaSmoke'
# MEDICO ve as proprias prescricoes (escopo por identidade).
request_and_assert "/prescricoes" "200" "/prescricoes (MEDICO)" "${MED_TOKEN}" contains 'DipironaSmoke'
# ENFERMAGEM ve as prescricoes ativas (remedios a aplicar).
request_and_assert "/prescricoes" "200" "/prescricoes (ENFERMAGEM)" "${ENF_TOKEN}" contains 'DipironaSmoke'
# PACIENTE ve as proprias receitas via /me/receitas.
request_and_assert "/me/receitas" "200" "/me/receitas (PACIENTE)" "${PAC_TOKEN}" contains 'DipironaSmoke'
# PACIENTE cria solicitacoes administrativas pelo portal, sem acionar triagem.
request_json_and_assert "POST" "/me/solicitacoes" \
    '{"tipo":"AGENDAMENTO","mensagem":"consulta comum pelo portal"}' \
    "201" "/me/solicitacoes POST agendamento (PACIENTE)" "${PAC_TOKEN}" contains '"status":"ABERTA"'
request_json_and_assert "POST" "/me/solicitacoes" \
    '{"tipo":"AJUDA","mensagem":"preciso de orientacao"}' \
    "201" "/me/solicitacoes POST ajuda (PACIENTE)" "${PAC_TOKEN}" contains '"tipo":"AJUDA"'
request_and_assert "/me/solicitacoes" "200" "/me/solicitacoes (PACIENTE)" "${PAC_TOKEN}" contains 'consulta comum pelo portal'
request_and_assert "/solicitacoes-paciente" "200" "/solicitacoes-paciente (ADMIN)" "${ADMIN_TOKEN}" contains '"pacienteId":1'
# Laudo estruturado no portal: a equipe coleta e lanca um analito fora da faixa
# no exame 1 (paciente 1, tipo 1, painel com HGB 12-16), depois o PACIENTE ve o
# detalhamento por analito apenas do proprio exame.
request_json_and_assert "POST" "/exames/1/status" '{"valor":"AUTORIZADO"}' \
    "200" "/exames/1/status AUTORIZADO" "${ADMIN_TOKEN}" contains '"status":"removido"'
request_json_and_assert "POST" "/exames/1/status" '{"valor":"COLETADO"}' \
    "200" "/exames/1/status COLETADO" "${ADMIN_TOKEN}" contains '"status":"removido"'
request_json_and_assert "POST" "/exames/1/resultados-analitos" '{"analito_id":"1","valor":"18"}' \
    "201" "/exames/1/resultados-analitos (HGB fora da faixa)" "${ADMIN_TOKEN}" contains '"status":"criado"'
request_and_assert "/me/exames/1/resultados" "200" "/me/exames/1/resultados (PACIENTE)" "${PAC_TOKEN}" contains '"foraReferencia":1'
request_and_assert "/me/exames/1/resultados" "200" "/me/exames/1/resultados nome do analito" "${PAC_TOKEN}" contains 'Hemoglobina'
# Posse: o PACIENTE 1 nao acessa o detalhamento de um exame de outro paciente.
request_and_assert "/me/exames/2/resultados" "404" "/me/exames/2/resultados (exame de outro paciente)" "${PAC_TOKEN}"
# Agendamentos do portal trazem o nome do medico (nao apenas o id).
request_and_assert "/me/agendamentos" "200" "/me/agendamentos com nome do medico (PACIENTE)" "${PAC_TOKEN}" contains '"medicoNome":"DrSmoke"'
request_and_assert "/me/agendamentos/especialidades" "200" "/me/agendamentos/especialidades (PACIENTE)" "${PAC_TOKEN}" contains '"especialidade":"Cardiologia"'
request_and_assert "/me/agendamentos/disponibilidade?especialidade=Cardiologia&data=2026-06-20" "200" "/me/agendamentos/disponibilidade (PACIENTE)" "${PAC_TOKEN}" contains '"08:00"'
request_json_and_assert "POST" "/me/agendamentos" \
    '{"especialidade":"Cardiologia","data":"2026-06-20","horario":"08:00"}' \
    "201" "/me/agendamentos POST autoagendamento (PACIENTE)" "${PAC_TOKEN}" contains '"status":"AGENDADO"'
request_and_assert "/agendamentos" "200" "/agendamentos recebe autoagendamento (ADMIN)" "${ADMIN_TOKEN}" contains '"especialidade":"Cardiologia"'
# Regra critica: PACIENTE nao executa nem lista fluxo de triagem clinica.
request_and_assert "/triagens" "403" "/triagens bloqueado para PACIENTE" "${PAC_TOKEN}"
request_json_and_assert "POST" "/triagens" '{"paciente_id":"1","tipo":"3","itens":"dor_toracica"}' \
    "403" "/triagens POST bloqueado para PACIENTE" "${PAC_TOKEN}"

# Farmacia / estoque (5a/5b): catalogo + movimentacao (ADMIN/ENFERMAGEM).
echo "--- Farmacia: catalogo e estoque ---"
request_json_and_assert "POST" "/medicamentos" \
    '{"nome":"Dipirona Smoke","apresentacao":"500mg comprimido","unidade":"comprimido","estoque_minimo":"20","preco_centavos":"150"}' \
    "201" "/medicamentos POST (ADMIN)" "${ADMIN_TOKEN}" contains '"status":"criado"'
request_and_assert "/medicamentos" "200" "/medicamentos (ADMIN)" "${ADMIN_TOKEN}" contains '"nome":"Dipirona Smoke"'
request_and_assert "/medicamentos" "200" "/medicamentos preco" "${ADMIN_TOKEN}" contains '"precoCentavos":150'
request_and_assert "/medicamentos/contar" "200" "/medicamentos/contar (ADMIN)" "${ADMIN_TOKEN}" contains '"ativos":1'
# Entrada de lote no estoque e saldo por medicamento.
request_json_and_assert "POST" "/estoque" \
    '{"medicamento_id":"1","lote":"L1","validade":"2026-12-31","quantidade":"100","localizacao":"Prateleira A"}' \
    "201" "/estoque POST entrada (ADMIN)" "${ADMIN_TOKEN}" contains '"status":"criado"'
request_and_assert "/medicamentos/1/estoque" "200" "/medicamentos/1/estoque" "${ADMIN_TOKEN}" contains '"quantidade":100'
request_and_assert "/estoque/alertas" "200" "/estoque/alertas (ADMIN)" "${ADMIN_TOKEN}" contains '"saldos"'
# Saida (ajuste/perda) debita o estoque.
request_json_and_assert "POST" "/movimentacoes" \
    '{"medicamento_id":"1","tipo":"SAIDA","quantidade":"10","motivo":"perda"}' \
    "201" "/movimentacoes POST SAIDA (ADMIN)" "${ADMIN_TOKEN}" contains '"status":"criado"'
request_and_assert "/medicamentos/1/movimentacoes" "200" "/medicamentos/1/movimentacoes" "${ADMIN_TOKEN}" contains '"tipo":"SAIDA"'
# Tipo invalido em /movimentacoes e recusado.
request_json_and_assert "POST" "/movimentacoes" \
    '{"medicamento_id":"1","tipo":"FOO","quantidade":"1"}' \
    "400" "/movimentacoes tipo invalido" "${ADMIN_TOKEN}" contains 'tipo invalido'
# Escopo por papel: MEDICO e PACIENTE nao acessam a farmacia.
request_and_assert "/medicamentos" "403" "/medicamentos bloqueado para MEDICO" "${MED_TOKEN}"
request_and_assert "/medicamentos" "403" "/medicamentos bloqueado para PACIENTE" "${PAC_TOKEN}"
request_and_assert "/medicamentos/1/estoque" "403" "/estoque bloqueado para MEDICO" "${MED_TOKEN}"
request_and_assert "/estoque/alertas" "403" "/estoque/alertas bloqueado para MEDICO" "${MED_TOKEN}"

# Vacinacao (6b): catalogo vinculado ao estoque e aplicacao no paciente.
echo "--- Vacinacao: catalogo e aplicacoes ---"
request_json_and_assert "POST" "/vacinas" \
    '{"nome":"COVID-19 bivalente","fabricante":"Fiocruz","doencas_alvo":"COVID-19","doses_previstas":"2","intervalo_dias":"28","reforco_dias":"180","medicamento_id":"1"}' \
    "201" "/vacinas POST (ADMIN)" "${ADMIN_TOKEN}" contains '"status":"criado"'
request_and_assert "/vacinas" "200" "/vacinas (ADMIN)" "${ADMIN_TOKEN}" contains '"nome":"COVID-19 bivalente"'
request_and_assert "/vacinas" "200" "/vacinas medicamento vinculado" "${ADMIN_TOKEN}" contains '"medicamentoId":1'
request_and_assert "/vacinas/contar" "200" "/vacinas/contar (ADMIN)" "${ADMIN_TOKEN}" contains '"ativos":1'
request_json_and_assert "POST" "/aplicacoes-vacinas" \
    '{"paciente_id":"1","vacina_id":"1","dose_numero":"1","lote":"L1","validade":"2026-12-31","observacao":"smoke"}' \
    "201" "/aplicacoes-vacinas POST (ADMIN)" "${ADMIN_TOKEN}" contains '"status":"aplicada"'
request_and_assert "/aplicacoes-vacinas" "200" "/aplicacoes-vacinas (ADMIN)" "${ADMIN_TOKEN}" contains '"vacinaNome":"COVID-19 bivalente"'
request_and_assert "/me/vacinas" "200" "/me/vacinas (PACIENTE)" "${PAC_TOKEN}" contains '"vacinaNome":"COVID-19 bivalente"'
request_and_assert "/medicamentos/1/estoque" "200" "/estoque baixa vacina" "${ADMIN_TOKEN}" contains '"quantidade":89'
request_json_and_assert "DELETE" "/vacinas/1" '{}' \
    "200" "/vacinas/1 DELETE (ADMIN)" "${ADMIN_TOKEN}" contains '"status":"removido"'
request_and_assert "/vacinas" "403" "/vacinas bloqueado para MEDICO" "${MED_TOKEN}"
request_and_assert "/vacinas" "403" "/vacinas bloqueado para PACIENTE" "${PAC_TOKEN}"
request_and_assert "/aplicacoes-vacinas" "403" "/aplicacoes-vacinas bloqueado para MEDICO" "${MED_TOKEN}"
request_and_assert "/me/vacinas" "403" "/me/vacinas bloqueado para MEDICO" "${MED_TOKEN}"

# Rate-limit por IP no POST /sessao: apos LOGIN_IP_MAX_FALHAS (10) falhas do
# mesmo IP, novas tentativas sao barradas com 429 (independe do login alvo).
# DEVE ser o ultimo teste: deixa o IP 127.0.0.1 bloqueado pela janela.
echo "--- Rate-limit de login por IP (ultimo teste) ---"
for i in $(seq 1 10); do
    rm -f "${RESP_FILE}"
    # Login inexistente: cada falha conta para o IP sem bloquear nenhum usuario.
    code="$(curl -sS -o "${RESP_FILE}" -w "%{http_code}" -X POST "http://localhost:8080/sessao" \
        -H 'Content-Type: application/json' \
        -d '{"login":"naoexiste_ip","senha":"x"}' || true)"
    if [[ "${code}" != "401" ]]; then
        fail "rate-limit IP: tentativa ${i} retornou HTTP ${code}, esperado 401"
    fi
done
rm -f "${RESP_FILE}"
code="$(curl -sS -o "${RESP_FILE}" -w "%{http_code}" -X POST "http://localhost:8080/sessao" \
    -H 'Content-Type: application/json' \
    -d '{"login":"naoexiste_ip","senha":"x"}' || true)"
if [[ "${code}" != "429" ]]; then
    fail "rate-limit IP: apos 10 falhas esperava HTTP 429, obteve ${code}"
fi
assert_contains "muitas tentativas" "rate-limit de login por IP (429 apos limite)"
echo "[OK] rate-limit de login por IP (429 apos limite)"

# Informa sucesso final quando todas as rotas passaram.
echo "[OK] Smoke test da API concluido com sucesso"
