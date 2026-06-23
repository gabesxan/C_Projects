#include "vacina_repository.h"
#include "database.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *BANCO_TESTE = "build/test_sigeh_vacina.db";
static const char *SCHEMA = "../data/schema_v3.sql";

int main(void)
{
    char json[4096];

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    assert(vacina_contar_ativas() == 0);

    assert(vacina_criar("", "Fiocruz", "COVID-19", 2, 28, 180) == 0);
    assert(vacina_criar(NULL, "Fiocruz", "COVID-19", 2, 28, 180) == 0);

    assert(vacina_criar("COVID-19 bivalente", "Fiocruz", "COVID-19", 2, 28, 180) == 1);
    assert(vacina_criar("Influenza", "Butantan", "Gripe", 1, 0, 365) == 1);
    assert(vacina_criar("Hepatite B", "", "Hepatite B", -2, -1, -5) == 1);
    assert(vacina_contar_ativas() == 3);

    assert(vacina_listar_json(json, sizeof(json)) == 1);
    assert(strstr(json, "\"nome\":\"COVID-19 bivalente\"") != NULL);
    assert(strstr(json, "\"fabricante\":\"Fiocruz\"") != NULL);
    assert(strstr(json, "\"doencasAlvo\":\"COVID-19\"") != NULL);
    assert(strstr(json, "\"dosesPrevistas\":2") != NULL);
    assert(strstr(json, "\"intervaloDias\":28") != NULL);
    assert(strstr(json, "\"reforcoDias\":180") != NULL);
    assert(strstr(json, "\"nome\":\"Hepatite B\"") != NULL);
    assert(strstr(json, "\"dosesPrevistas\":1") != NULL);
    assert(strstr(json, "\"intervaloDias\":0") != NULL);
    assert(strstr(json, "\"reforcoDias\":0") != NULL);
    assert(strstr(json, "COVID-19 bivalente") < strstr(json, "Hepatite B"));

    assert(vacina_ativa(1) == 1);
    assert(vacina_ativa(999) == 0);
    assert(vacina_ativa(0) == 0);

    assert(vacina_desativar(1) == 1);
    assert(vacina_desativar(1) == 0);
    assert(vacina_desativar(999) == 0);
    assert(vacina_contar_ativas() == 2);
    assert(vacina_ativa(1) == 0);

    printf("test_vacina_repository: OK\n");
    return 0;
}
