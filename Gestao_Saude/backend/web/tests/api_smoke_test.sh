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
DB_FILE="../data/sigeh_v2.db"
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

# Executa uma requisicao HTTP e valida status e conteudo minimo.
request_and_assert() {
    # Caminho da rota a ser testada.
    local path="$1"
    # Status HTTP esperado para a rota.
    local expected_status="$2"
    # Rotulo mostrado nas mensagens de sucesso ou erro.
    local label="$3"
    # Credencial Basic opcional no formato usuario:senha.
    local auth="${4:-}"
    # Tipo de validacao do corpo: contains, matches ou vazio.
    local body_check_type="${5:-}"
    # Valor associado ao tipo de validacao escolhido.
    local body_check_value="${6:-}"
    # Variavel que recebera o status HTTP real devolvido pelo curl.
    local http_code

    # Remove qualquer resposta anterior antes de chamar a proxima rota.
    rm -f "${RESP_FILE}"

    # Se houver autenticacao, chama o curl com -u usuario:senha.
    if [[ -n "${auth}" ]]; then
        http_code="$(curl -sS -o "${RESP_FILE}" -w "%{http_code}" -u "${auth}" "http://localhost:8080${path}" || true)"
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

# Recria um banco limpo e semeia um usuario admin temporario para o smoke test.
prepare_database() {
    # Arquivo-fonte temporario de um pequeno helper em C para reset e seed.
    local seed_source="${TMP_DIR}/api_smoke_seed.c"
    # Binario temporario gerado a partir desse helper em C.
    local seed_binary="${TMP_DIR}/api_smoke_seed"

    # Se ja existir banco da API, faz backup para restaurar depois.
    if [[ -f "${DB_FILE}" ]]; then
        DB_BACKUP="${TMP_DIR}/sigeh_v2.db.backup"
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
    if (db_resetar_com_schema("../data/schema_v2.sql") != 1)
    {
        /* Explica a falha caso o reset nao funcione. */
        fprintf(stderr, "falha ao resetar banco com schema\n");
        /* Retorna erro para o script abortar o smoke test. */
        return 1;
    }

    /* Cria o usuario admin padrao documentado para autenticar as rotas. */
    if (usuario_repo_criar("admin", "secreta", "ADMIN", 0, 0) != 1)
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

# Define a credencial admin documentada usada nas rotas autenticadas.
ADMIN_AUTH="admin:secreta"

# Testa a rota publica de saude e exige o campo status.
request_and_assert "/health" "200" "/health" "" contains '"status"'
# Testa a rota de sessao autenticada e exige o papel ADMIN no JSON.
request_and_assert "/me" "200" "/me" "${ADMIN_AUTH}" contains '"papel":"ADMIN"'
# Testa a listagem de pacientes e exige um JSON em formato de array.
request_and_assert "/pacientes" "200" "/pacientes" "${ADMIN_AUTH}" matches '^\[[^[:cntrl:]]*\]$'
# Testa a listagem de medicos e exige um JSON em formato de array.
request_and_assert "/medicos" "200" "/medicos" "${ADMIN_AUTH}" matches '^\[[^[:cntrl:]]*\]$'
# Testa o relatorio de indicadores e exige um campo chave do JSON retornado.
request_and_assert "/relatorios/indicadores" "200" "/relatorios/indicadores" "${ADMIN_AUTH}" contains '"pacientesAtivos"'
# Testa o relatorio de distribuicao e exige um campo chave do JSON retornado.
request_and_assert "/relatorios/distribuicao" "200" "/relatorios/distribuicao" "${ADMIN_AUTH}" contains '"pacientesPorRegiao"'
# Testa o relatorio por periodo com datas e exige o total no JSON retornado.
request_and_assert "/relatorios/agendamentos?inicio=2026-06-01&fim=2026-06-30" "200" "/relatorios/agendamentos" "${ADMIN_AUTH}" contains '"total"'
# Testa que o relatorio por periodo sem datas retorna 400.
request_and_assert "/relatorios/agendamentos" "400" "/relatorios/agendamentos (sem datas)" "${ADMIN_AUTH}"

# --- Escopo de dados por papel: o MEDICO ve apenas os proprios dados ---
# Endereco base reutilizado nas criacoes via POST (parametros vao na query string).
BASE="http://localhost:8080"
# Informa o inicio da preparacao do cenario de escopo.
echo "[INFO] Preparando cenario de escopo por papel (MEDICO)"
# Cria um medico (sera o id 1 no banco recem-resetado).
curl -sS -u "${ADMIN_AUTH}" -X POST "${BASE}/medicos?nome=DrSmoke&crm=CRM-SMOKE&especialidade=Cardiologia&regiao=1" >/dev/null
# Cria dois pacientes: o primeiro sera vinculado ao medico, o segundo nao.
curl -sS -u "${ADMIN_AUTH}" -X POST "${BASE}/pacientes?nome=AnaSmoke&cpf=900900&idade=20&telefone=61&sexo=F&regiao=1" >/dev/null
curl -sS -u "${ADMIN_AUTH}" -X POST "${BASE}/pacientes?nome=BiaSmoke&cpf=900901&idade=30&telefone=61&sexo=F&regiao=2" >/dev/null
# Agenda a paciente 1 (Ana) com o medico 1, criando o vinculo de escopo.
curl -sS -u "${ADMIN_AUTH}" -X POST "${BASE}/agendamentos?paciente_id=1&medico_id=1&data=2026-06-14&horario=09:00" >/dev/null
# Cria um usuario MEDICO ligado ao medico 1 para autenticar com escopo.
curl -sS -u "${ADMIN_AUTH}" -X POST "${BASE}/usuarios?login=medsmoke&senha=med123&papel=MEDICO&medico_id=1" >/dev/null
# Credencial do medico recem-criado.
MED_AUTH="medsmoke:med123"

# O MEDICO deve ver a propria paciente (Ana) na listagem ampla de pacientes.
request_and_assert "/pacientes" "200" "/pacientes (MEDICO escopado)" "${MED_AUTH}" contains 'AnaSmoke'
# Valida que a paciente fora do escopo (Bia) NAO aparece para o medico.
rm -f "${RESP_FILE}"
curl -sS -o "${RESP_FILE}" -u "${MED_AUTH}" "${BASE}/pacientes" >/dev/null
if grep -Fq 'BiaSmoke' "${RESP_FILE}"; then
    fail "/pacientes (MEDICO) vazou paciente fora do escopo"
fi
echo "[OK] /pacientes escopado por papel (MEDICO nao ve paciente de fora)"
# O MEDICO deve ver o proprio agendamento na listagem ampla de agendamentos.
request_and_assert "/agendamentos" "200" "/agendamentos (MEDICO escopado)" "${MED_AUTH}" contains '"medicoId":1'

# Informa sucesso final quando todas as rotas passaram.
echo "[OK] Smoke test da API concluido com sucesso"
