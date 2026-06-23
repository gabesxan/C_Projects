#include "analito_repository.h"
#include "database.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *BANCO_TESTE = "build/test_sigeh_analito.db";
static const char *SCHEMA = "../data/schema_v3.sql";

int main(void)
{
    char json[8192];

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    assert(analito_contar_ativos() == 0);
    assert(analito_criar("", "Hemoglobina", "g/dL", 12.0, 16.0, "Automacao") == 0);
    assert(analito_criar("HGB", "", "g/dL", 12.0, 16.0, "Automacao") == 0);

    assert(analito_criar("HGB", "Hemoglobina", "g/dL", 12.0, 16.0, "Automacao") == 1);
    assert(analito_criar("GLI", "Glicose", "mg/dL", 70.0, 99.0, "Hexoquinase") == 1);
    assert(analito_criar("HGB", "Hemoglobina duplicada", "g/dL", 10.0, 15.0, "Outro") == 0);
    assert(analito_contar_ativos() == 2);

    assert(analito_listar_json(json, sizeof(json)) == 1);
    assert(strstr(json, "\"codigo\":\"GLI\"") != NULL);
    assert(strstr(json, "\"codigo\":\"HGB\"") != NULL);
    assert(strstr(json, "\"refMin\":12") != NULL);
    assert(strstr(json, "\"metodo\":\"Automacao\"") != NULL);

    assert(painel_adicionar_analito(1, 1, 2) == 1);
    assert(painel_adicionar_analito(1, 2, 1) == 1);
    assert(painel_adicionar_analito(1, 2, 3) == 0);
    assert(painel_adicionar_analito(1, 99, 4) == 0);
    assert(painel_adicionar_analito(0, 1, 1) == 0);

    assert(painel_listar_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "\"codigo\":\"GLI\"") != NULL);
    assert(strstr(json, "\"ordem\":1") != NULL);
    assert(strstr(json, "\"codigo\":\"HGB\"") != NULL);
    assert(strstr(json, "\"ordem\":2") != NULL);

    assert(painel_remover_analito(1, 2) == 1);
    assert(painel_remover_analito(1, 2) == 0);
    assert(painel_listar_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "\"codigo\":\"GLI\"") == NULL);
    assert(strstr(json, "\"codigo\":\"HGB\"") != NULL);

    assert(painel_adicionar_analito(1, 2, 1) == 1);
    assert(analito_desativar(2) == 1);
    assert(analito_desativar(2) == 0);
    assert(analito_contar_ativos() == 1);

    assert(analito_listar_json(json, sizeof(json)) == 1);
    assert(strstr(json, "\"codigo\":\"GLI\"") == NULL);
    assert(strstr(json, "\"codigo\":\"HGB\"") != NULL);

    assert(painel_listar_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "\"codigo\":\"GLI\"") == NULL);
    assert(strstr(json, "\"codigo\":\"HGB\"") != NULL);

    assert(analito_criar("GLI", "Glicose reativada", "mg/dL", 70.0, 99.0, "Hexoquinase") == 1);
    assert(analito_contar_ativos() == 2);

    printf("test_analito_repository: OK\n");
    return 0;
}
