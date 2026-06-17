#include "prontuario_repository.h"
#include "paciente_repository.h"
#include "medico_repository.h"
#include "database.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *BANCO_TESTE = "build/test_sigeh_repository.db";
static const char *SCHEMA = "../data/schema_v3.sql";

int main(void)
{
    char json[8192];
    int antes;

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    /* Pacientes e medicos pais (ids 1 e 2) exigidos pela FK de prontuarios. */
    assert(paciente_repo_criar("Ana", "1990-01-01", "111", "CPF", "61", "F", 1, "", "") == 1);
    assert(paciente_repo_criar("Bia", "1980-01-01", "222", "CPF", "61", "F", 2, "", "") == 1);
    assert(medico_repo_criar("Dr X", "CRM1", "Cardiologia", 1) == 1);
    assert(medico_repo_criar("Dr Y", "CRM2", "Ortopedia", 2) == 1);

    assert(prontuario_repo_contar_ativos() == 0);

    assert(prontuario_repo_criar(1, 1, "2026-06-14", "Paciente estavel",
                                 "Gripe", "Repouso e hidratacao", 0) == 1);
    assert(prontuario_repo_contar_ativos() == 1);

    /* Falhas de validacao. */
    assert(prontuario_repo_criar(0, 1, "2026-06-14", "x", "y", "z", 0) == 0);
    assert(prontuario_repo_criar(1, 0, "2026-06-14", "x", "y", "z", 0) == 0);
    assert(prontuario_repo_criar(1, 1, "", "x", "y", "z", 0) == 0);
    /* Nao finaliza atendimento sem conduta. */
    assert(prontuario_repo_criar(1, 1, "2026-06-14", "x", "y", "", 0) == 0);
    assert(prontuario_repo_contar_ativos() == 1);

    assert(prontuario_repo_listar_json(json, sizeof(json)) == 1);
    assert(json[0] == '[');
    assert(strstr(json, "Gripe") != NULL);

    assert(prontuario_repo_criar(2, 2, "2026-06-15", "Dor toracica",
                                 "Investigar", "Solicitar ECG", 1) == 1);
    antes = prontuario_repo_contar_ativos();
    assert(antes == 2);

    /* Escopo por medico: cada medico ve apenas os proprios prontuarios. */
    assert(prontuario_repo_listar_por_medico_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "Gripe") != NULL);
    assert(strstr(json, "Dor toracica") == NULL);
    assert(prontuario_repo_listar_por_medico_json(2, json, sizeof(json)) == 1);
    assert(strstr(json, "Dor toracica") != NULL);
    assert(strstr(json, "Gripe") == NULL);

    assert(prontuario_repo_desativar(1) == 1);
    assert(prontuario_repo_contar_ativos() == antes - 1);
    assert(prontuario_repo_desativar(9999) == 0);

    printf("test_prontuario_repository: OK\n");
    return 0;
}
