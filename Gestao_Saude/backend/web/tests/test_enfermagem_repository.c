#include "enfermagem_repository.h"
#include "prescricao_repository.h"
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
    char json[4096];

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    assert(paciente_repo_criar("Ana", "1990-01-01", "111", "CPF", "61", "F", 1, "", "") == 1);
    assert(medico_repo_criar("Dr X", "CRM1", "Clinica Geral", 1) == 1);
    assert(prescricao_repo_criar(1, 1, "Dipirona", "500mg", "8/8h",
                                 "Oral", "3 dias", "x") == 1);

    /* Administracao de medicamento (MAR). */
    assert(administracao_criar(1, 1, "enfermagem", "Aplicado as 08h") == 1);
    /* Prescricao inexistente -> falha. */
    assert(administracao_criar(999, 1, "enfermagem", "x") == 0);

    assert(administracao_listar_por_paciente_json(1, json, sizeof(json)) == 1);
    assert(json[0] == '[');
    assert(strstr(json, "Dipirona") != NULL);
    assert(strstr(json, "enfermagem") != NULL);

    /* Evolucao de enfermagem. */
    assert(evolucao_criar(1, "enfermagem", "Paciente estavel, sem queixas",
                          "120/80", "36.5", "78", "98") == 1);
    /* Texto vazio -> falha. */
    assert(evolucao_criar(1, "enfermagem", "", "", "", "", "") == 0);

    assert(evolucao_listar_por_paciente_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "Paciente estavel") != NULL);
    assert(strstr(json, "120/80") != NULL);

    printf("test_enfermagem_repository: OK\n");
    return 0;
}
