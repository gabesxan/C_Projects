#include "agendamento_repository.h"
#include "paciente_repository.h"
#include "medico_repository.h"
#include "database.h"

#include <string.h>

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

    /* Pacientes e medicos pais (ids 1 e 2) exigidos pela FK de agendamentos. */
    assert(paciente_repo_criar("Ana", "1990-01-01", "111", "CPF", "61", "F", 1, "", "") == 1);
    assert(paciente_repo_criar("Bia", "1980-01-01", "222", "CPF", "61", "F", 2, "", "") == 1);
    assert(medico_repo_criar("Dr X", "CRM1", "Cardiologia", 1) == 1);
    assert(medico_repo_criar("Dr Y", "CRM2", "Ortopedia", 2) == 1);

    assert(agendamento_repo_contar_ativos() == 0);

    assert(agendamento_repo_criar(1, 1, "2026-06-14", "09:00") == 1);
    assert(agendamento_repo_contar_ativos() == 1);

    /* Falhas de validacao. */
    assert(agendamento_repo_criar(0, 1, "2026-06-14", "09:00") == 0);
    assert(agendamento_repo_criar(1, 0, "2026-06-14", "09:00") == 0);
    assert(agendamento_repo_criar(1, 1, "", "09:00") == 0);
    assert(agendamento_repo_criar(1, 1, "2026-06-14", "") == 0);
    assert(agendamento_repo_contar_ativos() == 1);

    /* Agenda: grade de slots fixos + janela de expediente. */
    assert(agendamento_repo_horario_valido("08:00") == 1);
    assert(agendamento_repo_horario_valido("17:30") == 1);
    assert(agendamento_repo_horario_valido("08:15") == 0); /* fora da grade */
    assert(agendamento_repo_horario_valido("07:30") == 0); /* antes do expediente */
    assert(agendamento_repo_horario_valido("18:00") == 0); /* fim exclusivo */
    assert(agendamento_repo_horario_valido("xx:yy") == 0); /* invalido */
    /* Criacao recusa horario fora da grade. */
    assert(agendamento_repo_criar(1, 1, "2026-06-14", "08:15") == 0);
    assert(agendamento_repo_contar_ativos() == 1);

    assert(agendamento_repo_listar_json(json, sizeof(json)) == 1);
    assert(json[0] == '[');
    assert(strstr(json, "2026-06-14") != NULL);
    assert(strstr(json, "AGENDADO") != NULL);

    /* Conflito: mesmo medico, mesmo data/horario (paciente 1 ja ocupa). */
    assert(agendamento_repo_criar(2, 1, "2026-06-14", "09:00") == 0);
    /* Conflito: mesmo paciente, mesmo data/horario, outro medico/setor. */
    assert(agendamento_repo_criar(1, 2, "2026-06-14", "09:00") == 0);
    /* Medico inexistente/inativo nao agenda. */
    assert(agendamento_repo_criar(1, 999, "2026-06-14", "10:00") == 0);
    assert(agendamento_repo_contar_ativos() == 1);

    assert(agendamento_repo_criar(2, 2, "2026-06-15", "10:30") == 1);
    antes = agendamento_repo_contar_ativos();
    assert(antes == 2);

    /* Reagendamento valido (status AGENDADO) para slot livre. */
    assert(agendamento_repo_reagendar(2, "2026-06-15", "11:00") == 1);
    /* Reagendar para horario fora da grade falha. */
    assert(agendamento_repo_reagendar(2, "2026-06-15", "11:15") == 0);

    /* Escopo por medico: agenda e pacientes do medico 1 (paciente Ana). */
    assert(agendamento_repo_listar_por_medico_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "\"medicoId\":1") != NULL);
    assert(strstr(json, "\"medicoId\":2") == NULL);
    assert(paciente_repo_listar_por_medico_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "Ana") != NULL);
    assert(strstr(json, "Bia") == NULL);
    assert(paciente_repo_listar_por_medico_json(2, json, sizeof(json)) == 1);
    assert(strstr(json, "Bia") != NULL);
    assert(strstr(json, "Ana") == NULL);

    /* Cancelar exige motivo; com motivo, status passa a CANCELADO. */
    assert(agendamento_repo_cancelar(1, "") == 0);
    assert(agendamento_repo_cancelar(1, "Paciente faltou") == 1);
    assert(agendamento_repo_contar_ativos() == antes - 1);
    assert(agendamento_repo_cancelar(9999, "x") == 0);

    printf("test_agendamento_repository: OK\n");
    return 0;
}
