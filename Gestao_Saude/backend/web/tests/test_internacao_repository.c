#include "internacao_repository.h"
#include "paciente_repository.h"
#include "ala_repository.h"
#include "leito_repository.h"
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

    /* Pais exigidos pelas FKs de internacoes: pacientes, ala e leitos. */
    assert(paciente_repo_criar("Ana", "111", 20, "61", "F", 1) == 1);
    assert(paciente_repo_criar("Bia", "222", 30, "61", "F", 2) == 1);
    assert(ala_repo_criar("Ala A", 1, 10) == 1);
    assert(leito_repo_criar(1, 101) == 1);
    assert(leito_repo_criar(1, 102) == 1);

    assert(internacao_repo_contar_ativos() == 0);

    assert(internacao_repo_criar(1, 1, 1, "2026-06-14") == 1);
    assert(internacao_repo_contar_ativos() == 1);

    /* Falhas de validacao. */
    assert(internacao_repo_criar(0, 1, 1, "2026-06-14") == 0);
    assert(internacao_repo_criar(1, 0, 1, "2026-06-14") == 0);
    assert(internacao_repo_criar(1, 1, 1, "") == 0);
    assert(internacao_repo_contar_ativos() == 1);

    assert(internacao_repo_listar_json(json, sizeof(json)) == 1);
    assert(json[0] == '[');
    assert(strstr(json, "INTERNADO") != NULL);

    assert(internacao_repo_criar(2, 1, 2, "2026-06-15") == 1);
    antes = internacao_repo_contar_ativos();
    assert(antes == 2);

    /* Dar alta tira a internacao da contagem de ativos. */
    assert(internacao_repo_dar_alta(1, "2026-06-16") == 1);
    assert(internacao_repo_contar_ativos() == antes - 1);
    assert(internacao_repo_dar_alta(1, "2026-06-16") == 0); /* ja teve alta */
    assert(internacao_repo_dar_alta(9999, "2026-06-16") == 0);

    printf("test_internacao_repository: OK\n");
    return 0;
}
