#include "leito_repository.h"
#include "ala_repository.h"
#include "database.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *BANCO_TESTE = "build/test_sigeh_repository.db";
static const char *SCHEMA = "../data/schema_v3.sql";

int main(void)
{
    char json[4096];
    int antes;

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    /* Ala pai (id 1) exigida pela FK de leitos. */
    assert(ala_repo_criar("Ala A", 1, 10) == 1);

    assert(leito_repo_contar_ativos() == 0);

    assert(leito_repo_criar(1, 101) == 1);
    assert(leito_repo_contar_ativos() == 1);

    /* Falhas de validacao. */
    assert(leito_repo_criar(0, 101) == 0);
    assert(leito_repo_criar(1, 0) == 0);
    assert(leito_repo_contar_ativos() == 1);

    assert(leito_repo_listar_json(json, sizeof(json)) == 1);
    assert(json[0] == '[');
    assert(strstr(json, "\"numero\":101") != NULL);

    assert(leito_repo_criar(1, 102) == 1);
    antes = leito_repo_contar_ativos();
    assert(antes == 2);

    assert(leito_repo_desativar(1) == 1);
    assert(leito_repo_contar_ativos() == antes - 1);
    assert(leito_repo_desativar(9999) == 0);

    printf("test_leito_repository: OK\n");
    return 0;
}
