#include "exame_repository.h"
#include "paciente_repository.h"
#include "medico_repository.h"
#include "prontuario_repository.h"
#include "database.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *BANCO_TESTE = "build/test_sigeh_repository.db";
static const char *SCHEMA = "../data/schema_v3.sql";

int main(void)
{
    char json[8192];
    int antes;

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    /* Pais (pacientes, medicos e prontuarios ids 1 e 2) exigidos pelas FKs. */
    assert(paciente_repo_criar("Ana", "1990-01-01", "111", "CPF", "61", "F", 1, "", "") == 1);
    assert(paciente_repo_criar("Bia", "1980-01-01", "222", "CPF", "61", "F", 2, "", "") == 1);
    assert(medico_repo_criar("Dr X", "CRM1", "Cardiologia", 1) == 1);
    assert(medico_repo_criar("Dr Y", "CRM2", "Ortopedia", 2) == 1);
    assert(prontuario_repo_criar(1, 1, "2026-06-01", "o", "d", "c", 0) == 1);
    assert(prontuario_repo_criar(2, 2, "2026-06-01", "o", "d", "c", 0) == 1);

    assert(exame_repo_contar_ativos() == 0);

    assert(exame_repo_criar(1, 1, 1, 1, "2026-06-14", 0) == 1);
    assert(exame_repo_contar_ativos() == 1);

    /* Falhas de validacao. */
    assert(exame_repo_criar(0, 1, 1, 1, "2026-06-14", 0) == 0);
    assert(exame_repo_criar(1, 1, 1, 0, "2026-06-14", 0) == 0);
    assert(exame_repo_criar(1, 1, 1, 1, "", 0) == 0);
    assert(exame_repo_contar_ativos() == 1);

    assert(exame_repo_listar_json(json, sizeof(json)) == 1);
    assert(json[0] == '[');
    assert(strstr(json, "SOLICITADO") != NULL);

    assert(exame_repo_criar(2, 2, 2, 5, "2026-06-15", 1) == 1);
    antes = exame_repo_contar_ativos();
    assert(antes == 2);

    /* Escopo por medico: cada medico ve apenas os exames que solicitou. */
    assert(exame_repo_listar_por_medico_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "\"medicoId\":1") != NULL);
    assert(strstr(json, "\"medicoId\":2") == NULL);
    assert(exame_repo_listar_por_medico_json(2, json, sizeof(json)) == 1);
    assert(strstr(json, "\"medicoId\":2") != NULL);
    assert(strstr(json, "\"medicoId\":1") == NULL);

    /* Maquina de estados: SOLICITADO -> AUTORIZADO -> COLETADO -> EM_ANALISE. */
    assert(exame_repo_atualizar_status(1, "COLETADO") == 0); /* pula etapa */
    assert(exame_repo_atualizar_status(1, "AUTORIZADO") == 1);
    assert(exame_repo_atualizar_status(1, "COLETADO") == 1);
    /* Resultado so apos coleta; marca CONCLUIDO e o flag de critico. */
    assert(exame_repo_registrar_resultado(1, "Alterado", 1) == 1);
    assert(exame_repo_listar_json(json, sizeof(json)) == 1);
    assert(strstr(json, "CONCLUIDO") != NULL);
    /* Exame concluido nao avanca status nem e cancelado. */
    assert(exame_repo_atualizar_status(1, "EM_ANALISE") == 0);
    assert(exame_repo_cancelar(1, "tentativa") == 0);

    /* Cancelamento exige motivo; com motivo, sai dos ativos. */
    assert(exame_repo_cancelar(2, "") == 0);
    assert(exame_repo_cancelar(2, "Duplicado") == 1);
    assert(exame_repo_desativar(9999) == 0);

    printf("test_exame_repository: OK\n");
    return 0;
}
