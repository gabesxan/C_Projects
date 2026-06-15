#include "relatorio_service.h"
#include "paciente_repository.h"
#include "medico_repository.h"
#include "triagem_repository.h"
#include "agendamento_repository.h"
#include "prontuario_repository.h"
#include "exame_repository.h"
#include "database.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *BANCO_TESTE = "build/test_sigeh_repository.db";
static const char *SCHEMA = "../data/schema_v2.sql";

int main(void)
{
    char json[1024];

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    /* Banco vazio: indicadores zerados, mas resposta valida. */
    assert(relatorio_service_indicadores_json(json, sizeof(json)) == 1);
    assert(strstr(json, "\"pacientesAtivos\":0") != NULL);
    assert(strstr(json, "\"casosGraves\":0") != NULL);

    /* Massa de dados. */
    assert(paciente_repo_criar("Ana", "111", 20, "61", "F", 1) == 1);
    assert(paciente_repo_criar("Bia", "222", 30, "61", "F", 2) == 1);
    assert(medico_repo_criar("Dr X", "CRM1", "Cardiologia", 1) == 1);
    assert(triagem_repo_criar(1, 3, 8, "Emergencia") == 1);
    assert(triagem_repo_criar(2, 1, 3, "Prioritario") == 1);

    assert(relatorio_service_indicadores_json(json, sizeof(json)) == 1);
    assert(strstr(json, "\"pacientesAtivos\":2") != NULL);
    assert(strstr(json, "\"medicosAtivos\":1") != NULL);
    assert(strstr(json, "\"triagensAtivas\":2") != NULL);
    assert(strstr(json, "\"emergencia\":1") != NULL);
    assert(strstr(json, "\"prioritario\":1") != NULL);
    assert(strstr(json, "\"casosGraves\":1") != NULL);

    /* Distribuicao: pacientes por regiao e medicos por especialidade.
     * Segundo medico na mesma especialidade testa o agrupamento (total 2). */
    assert(medico_repo_criar("Dr Y", "CRM2", "Cardiologia", 1) == 1);
    assert(relatorio_service_distribuicao_json(json, sizeof(json)) == 1);
    assert(strstr(json, "\"pacientesPorRegiao\"") != NULL);
    assert(strstr(json, "\"medicosPorEspecialidade\"") != NULL);
    assert(strstr(json, "{\"regiao\":1,\"total\":1}") != NULL);
    assert(strstr(json, "{\"regiao\":2,\"total\":1}") != NULL);
    assert(strstr(json, "{\"especialidade\":\"Cardiologia\",\"total\":2}") != NULL);

    /* Relatorio por periodo: agendamentos nao-cancelados agrupados por dia.
     * Dois no dia 14 (dentro do intervalo) e um no dia 20 (fora). */
    assert(agendamento_repo_criar(1, 1, "2026-06-14", "09:00") == 1);
    assert(agendamento_repo_criar(2, 1, "2026-06-14", "10:00") == 1);
    assert(agendamento_repo_criar(1, 1, "2026-06-20", "09:00") == 1);

    assert(relatorio_service_agendamentos_periodo_json(
               "2026-06-01", "2026-06-15", json, sizeof(json)) == 1);
    assert(strstr(json, "\"inicio\":\"2026-06-01\"") != NULL);
    assert(strstr(json, "\"fim\":\"2026-06-15\"") != NULL);
    assert(strstr(json, "\"total\":2") != NULL);
    assert(strstr(json, "{\"data\":\"2026-06-14\",\"total\":2}") != NULL);
    assert(strstr(json, "2026-06-20") == NULL); /* fora do intervalo */

    /* Datas ausentes sao recusadas. */
    assert(relatorio_service_agendamentos_periodo_json(
               "", "2026-06-15", json, sizeof(json)) == 0);

    /* --- Resumo do proprio medico (contagens escopadas) --- */
    assert(db_resetar_com_schema(SCHEMA) == 1);
    assert(paciente_repo_criar("Ana", "111", 20, "61", "F", 1) == 1);
    assert(paciente_repo_criar("Bia", "222", 30, "61", "F", 2) == 1);
    assert(medico_repo_criar("Dr X", "CRM1", "Cardiologia", 1) == 1); /* id 1 */
    assert(medico_repo_criar("Dr Y", "CRM2", "Ortopedia", 2) == 1);   /* id 2 */
    /* Medico 1: 1 paciente (via agendamento), 1 agendamento, 1 prontuario, 1 exame. */
    assert(agendamento_repo_criar(1, 1, "2026-06-14", "09:00") == 1);
    assert(prontuario_repo_criar(1, 1, "2026-06-14", "o", "d", "c", 0) == 1);
    assert(exame_repo_criar(1, 1, 1, 1, "2026-06-14", 0) == 1);
    /* Registros do medico 2 nao podem entrar no resumo do medico 1. */
    assert(agendamento_repo_criar(2, 2, "2026-06-15", "10:00") == 1);
    assert(prontuario_repo_criar(2, 2, "2026-06-15", "o", "d", "c", 0) == 1);

    assert(relatorio_service_resumo_medico_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "\"medicoId\":1") != NULL);
    assert(strstr(json, "\"pacientes\":1") != NULL);
    assert(strstr(json, "\"agendamentos\":1") != NULL);
    assert(strstr(json, "\"prontuarios\":1") != NULL);
    assert(strstr(json, "\"exames\":1") != NULL);

    /* medico_id invalido -> erro. */
    assert(relatorio_service_resumo_medico_json(0, json, sizeof(json)) == 0);

    printf("test_relatorio_service: OK\n");
    return 0;
}
