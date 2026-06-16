#include "triagem_repository.h"
#include "paciente_repository.h"
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

    /* Pacientes pais (ids 1 e 2) exigidos pela FK de triagens. */
    assert(paciente_repo_criar("Ana", "111", 20, "61", "F", 1) == 1);
    assert(paciente_repo_criar("Bia", "222", 30, "61", "F", 2) == 1);

    assert(triagem_repo_contar_ativos() == 0);

    assert(triagem_repo_criar(1, 1, 8, "Emergencia") == 1);
    assert(triagem_repo_contar_ativos() == 1);

    /* Falhas de validacao. */
    assert(triagem_repo_criar(0, 1, 8, "Emergencia") == 0);
    assert(triagem_repo_criar(1, 0, 8, "Emergencia") == 0);
    assert(triagem_repo_criar(1, 1, 8, "") == 0);
    assert(triagem_repo_contar_ativos() == 1);

    assert(triagem_repo_listar_json(json, sizeof(json)) == 1);
    assert(json[0] == '[');
    assert(strstr(json, "Emergencia") != NULL);

    assert(triagem_repo_criar(2, 3, 4, "Prioritario") == 1);
    antes = triagem_repo_contar_ativos();
    assert(antes == 2);

    /* Filtro por tipos: triagem 1 e tipo 1; triagem 2 e tipo 3. */
    {
        int tiposCardio[1] = {3};
        int tiposGeral[1] = {1};
        int tiposVarios[2] = {1, 3};

        assert(triagem_repo_listar_por_tipos_json(tiposCardio, 1, json, sizeof(json)) == 1);
        assert(strstr(json, "\"tipoTriagem\":3") != NULL);
        assert(strstr(json, "\"tipoTriagem\":1") == NULL);

        assert(triagem_repo_listar_por_tipos_json(tiposVarios, 2, json, sizeof(json)) == 1);
        assert(strstr(json, "\"tipoTriagem\":1") != NULL);
        assert(strstr(json, "\"tipoTriagem\":3") != NULL);

        /* n <= 0 devolve uma lista vazia valida. */
        assert(triagem_repo_listar_por_tipos_json(tiposGeral, 0, json, sizeof(json)) == 1);
        assert(strcmp(json, "[]") == 0);
    }

    assert(triagem_repo_desativar(1) == 1);
    assert(triagem_repo_contar_ativos() == antes - 1);
    assert(triagem_repo_desativar(9999) == 0);

    printf("test_triagem_repository: OK\n");
    return 0;
}
