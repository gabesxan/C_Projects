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
    char json[8192];
    int antes;

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    /* Pacientes e medicos pais (ids 1..3) exigidos pelas FKs. Paciente 3 tem
     * alergia registrada para validar a checagem na prescricao. */
    assert(paciente_repo_criar("Ana", "1990-01-01", "111", "CPF", "61", "F", 1, "", "") == 1);
    assert(paciente_repo_criar("Bia", "1980-01-01", "222", "CPF", "61", "F", 2, "", "") == 1);
    assert(paciente_repo_criar("Cleo", "1975-01-01", "333", "CPF", "61", "F", 1, "", "Penicilina") == 1);
    assert(medico_repo_criar("Dr X", "CRM1", "Cardiologia", 1) == 1);
    assert(medico_repo_criar("Dr Y", "CRM2", "Ortopedia", 2) == 1);

    assert(prescricao_repo_contar_ativos() == 0);

    assert(prescricao_repo_criar(1, 1, "Dipirona", "500mg", "8/8h",
                                 "Oral", "5 dias", "Apos as refeicoes") == 1);
    assert(prescricao_repo_contar_ativos() == 1);

    /* Falhas de validacao. */
    assert(prescricao_repo_criar(0, 1, "X", "1", "1", "", "", "o") == 0);
    assert(prescricao_repo_criar(1, 0, "X", "1", "1", "", "", "o") == 0);
    assert(prescricao_repo_criar(1, 1, "", "1", "1", "", "", "o") == 0);
    /* Alergia: paciente 3 e alergico a Penicilina -> prescricao recusada. */
    assert(prescricao_repo_criar(3, 1, "Penicilina", "500mg", "8/8h",
                                 "Oral", "7 dias", "x") == 0);
    /* Outro medicamento para o mesmo paciente alergico e permitido. */
    assert(prescricao_repo_criar(3, 1, "Dipirona", "500mg", "8/8h",
                                 "Oral", "3 dias", "x") == 1);
    assert(prescricao_repo_contar_ativos() == 2);

    assert(prescricao_repo_listar_json(json, sizeof(json)) == 1);
    assert(json[0] == '[');
    assert(strstr(json, "Dipirona") != NULL);

    /* Segunda prescricao: outro paciente e outro medico. */
    assert(prescricao_repo_criar(2, 2, "Amoxicilina", "875mg", "12/12h",
                                 "Oral", "7 dias", "Por 7 dias") == 1);
    antes = prescricao_repo_contar_ativos();
    assert(antes == 3);

    /* Escopo por paciente. */
    assert(prescricao_repo_listar_por_paciente_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "Dipirona") != NULL);
    assert(strstr(json, "Amoxicilina") == NULL);

    /* Escopo por medico. */
    assert(prescricao_repo_listar_por_medico_json(2, json, sizeof(json)) == 1);
    assert(strstr(json, "Amoxicilina") != NULL);
    assert(strstr(json, "Dipirona") == NULL);

    /* Suspensao exige justificativa e diminui a contagem. */
    assert(prescricao_repo_desativar(1, "") == 0);
    assert(prescricao_repo_desativar(1, "Reacao adversa") == 1);
    assert(prescricao_repo_contar_ativos() == antes - 1);
    assert(prescricao_repo_desativar(9999, "x") == 0);

    printf("test_prescricao_repository: OK\n");
    return 0;
}
