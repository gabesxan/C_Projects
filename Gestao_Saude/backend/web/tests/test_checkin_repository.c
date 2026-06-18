#include "checkin_repository.h"
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
    char senha[16];

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    assert(paciente_repo_criar("Ana", "1990-01-01", "111", "CPF", "61", "F", 1, "", "") == 1);
    assert(paciente_repo_criar("Bia", "1980-01-01", "222", "CPF", "61", "F", 2, "", "") == 1);

    assert(checkin_repo_contar_aguardando() == 0);

    /* Check-in para triagem gera senha T001. */
    assert(checkin_repo_criar(1, "TRIAGEM", senha, sizeof(senha)) == 1);
    assert(strcmp(senha, "T001") == 0);
    /* Segundo check-in de triagem -> T002. */
    assert(checkin_repo_criar(2, "TRIAGEM", senha, sizeof(senha)) == 1);
    assert(strcmp(senha, "T002") == 0);
    /* Consulta usa outra serie -> C001. */
    assert(checkin_repo_criar(1, "CONSULTA", senha, sizeof(senha)) == 1);
    assert(strcmp(senha, "C001") == 0);

    /* Destino invalido falha. */
    assert(checkin_repo_criar(1, "QUALQUER", senha, sizeof(senha)) == 0);

    assert(checkin_repo_contar_aguardando() == 3);

    /* Fila lista os nao encerrados com nome do paciente. */
    assert(checkin_repo_listar_json(json, sizeof(json)) == 1);
    assert(strstr(json, "T001") != NULL);
    assert(strstr(json, "Ana") != NULL);
    assert(strstr(json, "\"destino\":\"TRIAGEM\"") != NULL);

    /* Chamar tira de AGUARDANDO; encerrar sai da fila. */
    assert(checkin_repo_chamar(1) == 1);
    assert(checkin_repo_contar_aguardando() == 2);
    assert(checkin_repo_encerrar(1) == 1);
    assert(checkin_repo_listar_json(json, sizeof(json)) == 1);
    assert(strstr(json, "T001") == NULL); /* encerrado saiu da fila */

    /* Chamar id inexistente falha. */
    assert(checkin_repo_chamar(999) == 0);

    printf("test_checkin_repository: OK\n");
    return 0;
}
