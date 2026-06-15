#include "exame_repository.h"
#include "paciente_repository.h"
#include "medico_repository.h"
#include "prontuario_repository.h"
#include "database.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *BANCO_TESTE = "tests/test_sigeh_repository.db";
static const char *SCHEMA = "data/schema_v2.sql";

int main(void)
{
    char json[8192];
    int antes;

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    /* Pais (pacientes, medicos e prontuarios ids 1 e 2) exigidos pelas FKs. */
    assert(paciente_repo_criar("Ana", "111", 20, "61", "F", 1) == 1);
    assert(paciente_repo_criar("Bia", "222", 30, "61", "F", 2) == 1);
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

    assert(exame_repo_desativar(1) == 1);
    assert(exame_repo_contar_ativos() == antes - 1);
    assert(exame_repo_desativar(9999) == 0);

    printf("test_exame_repository: OK\n");
    return 0;
}
