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
    char status[32];
    int antes;

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    /* Pais exigidos pelas FKs de internacoes: pacientes, ala e leitos. */
    assert(paciente_repo_criar("Ana", "1990-01-01", "111", "CPF", "61", "F", 1, "", "") == 1);
    assert(paciente_repo_criar("Bia", "1980-01-01", "222", "CPF", "61", "F", 2, "", "") == 1);
    assert(ala_repo_criar("Ala A", 1, 10) == 1);
    assert(leito_repo_criar(1, 101) == 1);
    assert(leito_repo_criar(1, 102) == 1);
    assert(leito_repo_criar(1, 103) == 1);

    assert(internacao_repo_contar_ativos() == 0);

    /* Internar ocupa o leito. */
    assert(internacao_repo_criar(1, 1, 1, "2026-06-14", "Dr X") == 1);
    assert(internacao_repo_contar_ativos() == 1);
    assert(leito_repo_status(1, status, sizeof(status)) == 1);
    assert(strcmp(status, "OCUPADO") == 0);

    /* Falhas de validacao. */
    assert(internacao_repo_criar(0, 1, 1, "2026-06-14", "x") == 0);
    assert(internacao_repo_criar(1, 0, 1, "2026-06-14", "x") == 0);
    assert(internacao_repo_criar(1, 1, 1, "", "x") == 0);
    /* Leito ja ocupado nao recebe outro paciente. */
    assert(internacao_repo_criar(2, 1, 1, "2026-06-14", "x") == 0);
    assert(internacao_repo_contar_ativos() == 1);

    assert(internacao_repo_listar_json(json, sizeof(json)) == 1);
    assert(json[0] == '[');
    assert(strstr(json, "INTERNADO") != NULL);
    assert(strstr(json, "Dr X") != NULL);

    assert(internacao_repo_criar(2, 1, 2, "2026-06-15", "Dr Y") == 1);
    antes = internacao_repo_contar_ativos();
    assert(antes == 2);

    /* Transferencia: internacao 2 vai do leito 2 para o leito 3 (livre). */
    assert(internacao_repo_transferir(2, 3, "2026-06-15", "Enf Z") == 1);
    assert(leito_repo_status(3, status, sizeof(status)) == 1);
    assert(strcmp(status, "OCUPADO") == 0);
    assert(leito_repo_status(2, status, sizeof(status)) == 1);
    assert(strcmp(status, "HIGIENIZACAO") == 0);
    /* Transferir para leito ocupado falha. */
    assert(internacao_repo_transferir(2, 1, "2026-06-15", "Enf Z") == 0);

    /* Alta exige resumo e diagnostico; libera o leito para HIGIENIZACAO. */
    assert(internacao_repo_dar_alta(1, "2026-06-16", "", "", "") == 0);
    assert(internacao_repo_dar_alta(1, "2026-06-16", "Melhora clinica",
                                    "Pneumonia resolvida", "Repouso") == 1);
    assert(internacao_repo_contar_ativos() == antes - 1);
    assert(leito_repo_status(1, status, sizeof(status)) == 1);
    assert(strcmp(status, "HIGIENIZACAO") == 0);
    /* Ja teve alta / inexistente. */
    assert(internacao_repo_dar_alta(1, "2026-06-16", "r", "d", "o") == 0);
    assert(internacao_repo_dar_alta(9999, "2026-06-16", "r", "d", "o") == 0);

    /* Mudanca manual de status de leito (enfermagem) com historico. */
    assert(leito_repo_registrar_status(1, "DISPONIVEL", "Enf Z") == 1);
    assert(leito_repo_status(1, status, sizeof(status)) == 1);
    assert(strcmp(status, "DISPONIVEL") == 0);
    /* OCUPADO nao e definido manualmente. */
    assert(leito_repo_registrar_status(1, "OCUPADO", "Enf Z") == 0);

    /* Ocupacao em JSON. */
    assert(leito_repo_ocupacao_json(json, sizeof(json)) == 1);
    assert(strstr(json, "taxaOcupacao") != NULL);

    printf("test_internacao_repository: OK\n");
    return 0;
}
