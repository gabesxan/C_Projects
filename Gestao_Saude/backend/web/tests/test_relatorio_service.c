#include "relatorio_service.h"
#include "paciente_repository.h"
#include "medico_repository.h"
#include "triagem_repository.h"
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

    printf("test_relatorio_service: OK\n");
    return 0;
}
