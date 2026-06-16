#include "auditoria_repository.h"
#include "database.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *BANCO_TESTE = "build/test_sigeh_repository.db";
static const char *SCHEMA = "../data/schema_v3.sql";

int main(void)
{
    char json[4096];

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    assert(auditoria_contar() == 0);

    /* Acao sem nome -> falha; nao registra. */
    assert(auditoria_registrar(1, "admin", "", "usuario", 0, "x") == 0);
    assert(auditoria_contar() == 0);

    /* Registro valido. */
    assert(auditoria_registrar(1, "admin", "LOGIN", "usuario", 1,
                               "login bem-sucedido") == 1);
    assert(auditoria_contar() == 1);

    /* usuario_login e detalhe podem ser nulos/vazios. */
    assert(auditoria_registrar(0, NULL, "ALTA", "internacao", 7, NULL) == 1);
    assert(auditoria_contar() == 2);

    /* Listagem traz os campos esperados e mais recentes primeiro. */
    assert(auditoria_listar_json(json, sizeof(json)) == 1);
    assert(strstr(json, "\"acao\":\"LOGIN\"") != NULL);
    assert(strstr(json, "\"acao\":\"ALTA\"") != NULL);
    assert(strstr(json, "\"entidade\":\"internacao\"") != NULL);
    /* ALTA foi inserida por ultimo, entao aparece antes de LOGIN. */
    assert(strstr(json, "ALTA") < strstr(json, "LOGIN"));

    printf("test_auditoria_repository: OK\n");
    return 0;
}
