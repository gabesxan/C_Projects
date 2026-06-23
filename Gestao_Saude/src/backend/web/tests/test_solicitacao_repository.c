#include "solicitacao_repository.h"
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
    int id = 0;

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    assert(paciente_repo_criar("Ana", "1990-01-01", "111", "CPF", "61", "F", 1, "", "") == 1);
    assert(paciente_repo_criar("Bia", "1980-01-01", "222", "CPF", "61", "F", 2, "", "") == 1);

    assert(solicitacao_repo_contar_abertas() == 0);

    assert(solicitacao_repo_criar(1, "AGENDAMENTO", "consulta comum", &id) == 1);
    assert(id > 0);
    assert(solicitacao_repo_criar(1, "AJUDA", "preciso de orientacao", NULL) == 1);
    assert(solicitacao_repo_criar(2, "AJUDA", "", NULL) == 1);

    assert(solicitacao_repo_criar(1, "TRIAGEM", "nao permitido", NULL) == 0);
    assert(solicitacao_repo_criar(0, "AJUDA", "", NULL) == 0);

    assert(solicitacao_repo_contar_abertas() == 3);

    assert(solicitacao_repo_listar_por_paciente_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "\"tipo\":\"AGENDAMENTO\"") != NULL);
    assert(strstr(json, "\"tipo\":\"AJUDA\"") != NULL);
    assert(strstr(json, "consulta comum") != NULL);
    assert(strstr(json, "\"pacienteId\":2") == NULL);

    assert(solicitacao_repo_listar_json(json, sizeof(json)) == 1);
    assert(strstr(json, "\"pacienteNome\":\"Ana\"") != NULL);
    assert(strstr(json, "\"pacienteNome\":\"Bia\"") != NULL);

    printf("test_solicitacao_repository: OK\n");
    return 0;
}
