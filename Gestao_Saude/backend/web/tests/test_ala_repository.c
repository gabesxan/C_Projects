#include "ala_repository.h"
#include "database.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *BANCO_TESTE = "tests/test_sigeh_repository.db";
static const char *SCHEMA = "data/schema_v2.sql";

int main(void)
{
    char json[4096];
    int antes;

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    assert(ala_repo_contar_ativos() == 0);

    assert(ala_repo_criar("Ala A", 1, 10) == 1);
    assert(ala_repo_contar_ativos() == 1);

    /* Falhas de validacao. */
    assert(ala_repo_criar("", 1, 10) == 0);
    assert(ala_repo_criar("Ala B", 0, 10) == 0);
    assert(ala_repo_criar("Ala B", 1, -1) == 0);
    assert(ala_repo_contar_ativos() == 1);

    assert(ala_repo_listar_json(json, sizeof(json)) == 1);
    assert(json[0] == '[');
    assert(strstr(json, "Ala A") != NULL);

    assert(ala_repo_criar("Ala B", 2, 5) == 1);
    antes = ala_repo_contar_ativos();
    assert(antes == 2);

    assert(ala_repo_desativar(1) == 1);
    assert(ala_repo_contar_ativos() == antes - 1);
    assert(ala_repo_desativar(9999) == 0);

    printf("test_ala_repository: OK\n");
    return 0;
}
