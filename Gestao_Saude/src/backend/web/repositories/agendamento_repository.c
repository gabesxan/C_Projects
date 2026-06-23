#include "agendamento_repository.h"
#include "medico_repository.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>
#include <string.h>

/* Politica de agenda (grade de slots fixos + janela de expediente). */
#define AGENDA_MIN_INICIO (8 * 60) /* 08:00, inclusivo */
#define AGENDA_MIN_FIM (18 * 60)   /* 18:00, exclusivo */
#define AGENDA_SLOT_MIN 30         /* passo da grade, em minutos */
#define MAX_MEDICOS_AGENDA 256

/* Converte "HH:MM" em minutos do dia; -1 se invalido. */
static int minutosDoHorario(const char *horario)
{
    int horas;
    int minutos;

    if (horario == NULL || sscanf(horario, "%d:%d", &horas, &minutos) != 2)
    {
        return -1;
    }

    if (horas < 0 || horas > 23 || minutos < 0 || minutos > 59)
    {
        return -1;
    }

    return horas * 60 + minutos;
}

int agendamento_repo_horario_valido(const char *horario)
{
    int minutos = minutosDoHorario(horario);

    if (minutos < 0)
    {
        return 0;
    }

    if (minutos % AGENDA_SLOT_MIN != 0)
    {
        return 0;
    }

    if (minutos < AGENDA_MIN_INICIO || minutos >= AGENDA_MIN_FIM)
    {
        return 0;
    }

    return 1;
}

/* 1 se o paciente ja tem atendimento nao-cancelado nesse data/horario (em
 * qualquer setor/medico); 0 se livre; -1 em erro. */
int agendamento_repo_paciente_ocupado(int paciente_id, const char *data,
                                      const char *horario)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT COUNT(*) FROM agendamentos "
        "WHERE paciente_id = ? AND data = ? AND horario = ? "
        "AND status != 'CANCELADO';";
    int ocupado = -1;

    if (paciente_id <= 0 || data == NULL || horario == NULL)
    {
        return -1;
    }

    if (db_abrir(&db) == 0)
    {
        return -1;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return -1;
    }

    sqlite3_bind_int(stmt, 1, paciente_id);
    sqlite3_bind_text(stmt, 2, data, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, horario, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        ocupado = sqlite3_column_int(stmt, 0) > 0 ? 1 : 0;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return ocupado;
}

int agendamento_repo_criar(int paciente_id, int medico_id,
                           const char *data, const char *horario)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    char especialidade[64];
    const char *sql =
        "INSERT INTO agendamentos "
        "(paciente_id, medico_id, especialidade, data, horario, status) "
        "VALUES (?, ?, ?, ?, ?, 'AGENDADO');";

    if (paciente_id <= 0 || medico_id <= 0)
    {
        return 0;
    }

    if (data == NULL || data[0] == '\0' || horario == NULL || horario[0] == '\0')
    {
        return 0;
    }

    if (agendamento_repo_horario_valido(horario) == 0)
    {
        return 0;
    }

    /* So agenda com medico ativo. */
    if (medico_repo_ativo(medico_id) == 0)
    {
        return 0;
    }
    if (medico_repo_especialidade(medico_id, especialidade, sizeof(especialidade)) == 0)
    {
        return 0;
    }

    /* Sem dois atendimentos no mesmo horario para o mesmo medico... */
    if (agendamento_repo_medico_ocupado(medico_id, data, horario) != 0)
    {
        return 0;
    }

    /* ...nem para o mesmo paciente (em qualquer setor). */
    if (agendamento_repo_paciente_ocupado(paciente_id, data, horario) != 0)
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_int(stmt, 1, paciente_id);
    sqlite3_bind_int(stmt, 2, medico_id);
    sqlite3_bind_text(stmt, 3, especialidade, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, data, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, horario, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return 1;
}

static int medico_livre_por_especialidade(const char *especialidade,
                                          const char *data,
                                          const char *horario)
{
    int ids[MAX_MEDICOS_AGENDA];
    int total;
    int i;

    if (especialidade == NULL || especialidade[0] == '\0' ||
        data == NULL || data[0] == '\0' ||
        agendamento_repo_horario_valido(horario) == 0)
    {
        return 0;
    }

    total = medico_repo_ids_por_especialidade(especialidade, ids, MAX_MEDICOS_AGENDA);
    for (i = 0; i < total; i++)
    {
        if (agendamento_repo_medico_ocupado(ids[i], data, horario) == 0)
        {
            return ids[i];
        }
    }

    return 0;
}

int agendamento_repo_criar_por_especialidade(int paciente_id,
                                             const char *especialidade,
                                             const char *data,
                                             const char *horario,
                                             int *agendamento_id_out,
                                             int *medico_id_out)
{
    int medico_id;
    int ag_id = 0;
    int pac_id = 0;

    if (paciente_id <= 0 || especialidade == NULL || especialidade[0] == '\0' ||
        data == NULL || data[0] == '\0' ||
        agendamento_repo_horario_valido(horario) == 0)
    {
        return 0;
    }

    if (agendamento_repo_paciente_ocupado(paciente_id, data, horario) != 0)
    {
        return 0;
    }

    medico_id = medico_livre_por_especialidade(especialidade, data, horario);
    if (medico_id <= 0)
    {
        return 0;
    }

    if (agendamento_repo_criar(paciente_id, medico_id, data, horario) == 0)
    {
        return 0;
    }

    if (agendamento_repo_buscar_no_slot(medico_id, data, horario, &ag_id, &pac_id) == 1 &&
        pac_id == paciente_id)
    {
        if (agendamento_id_out != NULL) *agendamento_id_out = ag_id;
        if (medico_id_out != NULL) *medico_id_out = medico_id;
        return 1;
    }

    if (medico_id_out != NULL) *medico_id_out = medico_id;
    return 1;
}

int agendamento_repo_especialidades_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT e.nome, COUNT(m.id) "
        "FROM especialidades_clinicas e "
        "LEFT JOIN medicos m ON m.ativo = 1 AND m.especialidade = e.nome "
        "WHERE e.ativo = 1 "
        "GROUP BY e.id, e.nome "
        "ORDER BY e.id;";
    int usado = 0;
    int primeiro = 1;

    if (buffer == NULL || tamanho <= 0)
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    buffer[0] = '\0';
    if (repo_json_anexar(buffer, tamanho, &usado, "[") == 0)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char especialidadeJson[128];
        char objeto[220];
        int total = sqlite3_column_int(stmt, 1);
        int escrito;

        if (repo_json_escapar(especialidadeJson, sizeof(especialidadeJson),
                              (const char *)sqlite3_column_text(stmt, 0)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"especialidade\":%s,\"total\":%d}",
                           primeiro ? "" : ",", especialidadeJson, total);

        if (escrito < 0 || escrito >= (int)sizeof(objeto) ||
            repo_json_anexar(buffer, tamanho, &usado, objeto) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        primeiro = 0;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return repo_json_anexar(buffer, tamanho, &usado, "]");
}

int agendamento_repo_slots_disponiveis_json(int paciente_id,
                                            const char *especialidade,
                                            const char *data,
                                            char *buffer, int tamanho)
{
    int usado = 0;
    int primeiro = 1;
    int min;

    if (buffer == NULL || tamanho <= 0 || paciente_id <= 0 ||
        especialidade == NULL || especialidade[0] == '\0' ||
        data == NULL || data[0] == '\0')
    {
        return 0;
    }

    buffer[0] = '\0';
    if (repo_json_anexar(buffer, tamanho, &usado, "[") == 0)
    {
        return 0;
    }

    for (min = AGENDA_MIN_INICIO; min < AGENDA_MIN_FIM; min += AGENDA_SLOT_MIN)
    {
        char horario[8];
        char item[48];
        snprintf(horario, sizeof(horario), "%02d:%02d", min / 60, min % 60);

        if (agendamento_repo_paciente_ocupado(paciente_id, data, horario) == 0 &&
            medico_livre_por_especialidade(especialidade, data, horario) > 0)
        {
            int escrito = snprintf(item, sizeof(item), "%s\"%s\"",
                                   primeiro ? "" : ",", horario);
            if (escrito < 0 || escrito >= (int)sizeof(item) ||
                repo_json_anexar(buffer, tamanho, &usado, item) == 0)
            {
                return 0;
            }
            primeiro = 0;
        }
    }

    return repo_json_anexar(buffer, tamanho, &usado, "]");
}

int agendamento_repo_listar_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT a.id, a.paciente_id, a.medico_id, a.data, a.horario, a.status, "
        "a.motivo_cancelamento, COALESCE(p.nome, ''), COALESCE(m.nome, ''), "
        "COALESCE(NULLIF(a.especialidade, ''), m.especialidade, '') "
        "FROM agendamentos a "
        "LEFT JOIN pacientes p ON p.id = a.paciente_id "
        "LEFT JOIN medicos m ON m.id = a.medico_id ORDER BY a.id;";
    int usado = 0;
    int primeiro = 1;

    if (buffer == NULL || tamanho <= 0)
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    buffer[0] = '\0';

    if (repo_json_anexar(buffer, tamanho, &usado, "[") == 0)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char dataJson[32];
        char horarioJson[24];
        char statusJson[48];
        char motivoJson[280];
        char pacienteNomeJson[256];
        char medicoNomeJson[256];
        char especialidadeJson[128];
        char objeto[1250];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int medicoId = sqlite3_column_int(stmt, 2);
        const char *data = (const char *)sqlite3_column_text(stmt, 3);
        const char *horario = (const char *)sqlite3_column_text(stmt, 4);
        const char *status = (const char *)sqlite3_column_text(stmt, 5);
        const char *motivo = (const char *)sqlite3_column_text(stmt, 6);
        int escrito;

        if (repo_json_escapar(dataJson, sizeof(dataJson), data) == 0 ||
            repo_json_escapar(horarioJson, sizeof(horarioJson), horario) == 0 ||
            repo_json_escapar(statusJson, sizeof(statusJson), status) == 0 ||
            repo_json_escapar(motivoJson, sizeof(motivoJson), motivo) == 0 ||
            repo_json_escapar(pacienteNomeJson, sizeof(pacienteNomeJson), (const char *)sqlite3_column_text(stmt, 7)) == 0 ||
            repo_json_escapar(medicoNomeJson, sizeof(medicoNomeJson), (const char *)sqlite3_column_text(stmt, 8)) == 0 ||
            repo_json_escapar(especialidadeJson, sizeof(especialidadeJson), (const char *)sqlite3_column_text(stmt, 9)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"id\":%d,\"pacienteId\":%d,\"medicoId\":%d,"
                           "\"pacienteNome\":%s,\"medicoNome\":%s,"
                           "\"especialidade\":%s,\"data\":%s,\"horario\":%s,\"status\":%s,"
                           "\"motivoCancelamento\":%s}",
                           primeiro ? "" : ",",
                           id, pacienteId, medicoId, pacienteNomeJson, medicoNomeJson,
                           especialidadeJson, dataJson, horarioJson, statusJson, motivoJson);

        if (escrito < 0 || escrito >= (int)sizeof(objeto))
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        if (repo_json_anexar(buffer, tamanho, &usado, objeto) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        primeiro = 0;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);

    if (repo_json_anexar(buffer, tamanho, &usado, "]") == 0)
    {
        return 0;
    }

    return 1;
}

int agendamento_repo_listar_por_medico_json(int medico_id, char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT a.id, a.paciente_id, a.medico_id, a.data, a.horario, a.status, "
        "a.motivo_cancelamento, COALESCE(p.nome, ''), COALESCE(m.nome, ''), "
        "COALESCE(NULLIF(a.especialidade, ''), m.especialidade, '') "
        "FROM agendamentos a "
        "LEFT JOIN pacientes p ON p.id = a.paciente_id "
        "LEFT JOIN medicos m ON m.id = a.medico_id "
        "WHERE a.medico_id = ? ORDER BY a.id;";
    int usado = 0;
    int primeiro = 1;

    if (buffer == NULL || tamanho <= 0 || medico_id <= 0)
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_int(stmt, 1, medico_id);

    buffer[0] = '\0';

    if (repo_json_anexar(buffer, tamanho, &usado, "[") == 0)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char dataJson[32];
        char horarioJson[24];
        char statusJson[48];
        char motivoJson[280];
        char pacienteNomeJson[256];
        char medicoNomeJson[256];
        char especialidadeJson[128];
        char objeto[1250];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int medicoId = sqlite3_column_int(stmt, 2);
        const char *data = (const char *)sqlite3_column_text(stmt, 3);
        const char *horario = (const char *)sqlite3_column_text(stmt, 4);
        const char *status = (const char *)sqlite3_column_text(stmt, 5);
        const char *motivo = (const char *)sqlite3_column_text(stmt, 6);
        int escrito;

        if (repo_json_escapar(dataJson, sizeof(dataJson), data) == 0 ||
            repo_json_escapar(horarioJson, sizeof(horarioJson), horario) == 0 ||
            repo_json_escapar(statusJson, sizeof(statusJson), status) == 0 ||
            repo_json_escapar(motivoJson, sizeof(motivoJson), motivo) == 0 ||
            repo_json_escapar(pacienteNomeJson, sizeof(pacienteNomeJson), (const char *)sqlite3_column_text(stmt, 7)) == 0 ||
            repo_json_escapar(medicoNomeJson, sizeof(medicoNomeJson), (const char *)sqlite3_column_text(stmt, 8)) == 0 ||
            repo_json_escapar(especialidadeJson, sizeof(especialidadeJson), (const char *)sqlite3_column_text(stmt, 9)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"id\":%d,\"pacienteId\":%d,\"medicoId\":%d,"
                           "\"pacienteNome\":%s,\"medicoNome\":%s,"
                           "\"especialidade\":%s,\"data\":%s,\"horario\":%s,\"status\":%s,"
                           "\"motivoCancelamento\":%s}",
                           primeiro ? "" : ",",
                           id, pacienteId, medicoId, pacienteNomeJson, medicoNomeJson,
                           especialidadeJson, dataJson, horarioJson, statusJson, motivoJson);

        if (escrito < 0 || escrito >= (int)sizeof(objeto))
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        if (repo_json_anexar(buffer, tamanho, &usado, objeto) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        primeiro = 0;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);

    if (repo_json_anexar(buffer, tamanho, &usado, "]") == 0)
    {
        return 0;
    }

    return 1;
}

int agendamento_repo_listar_por_paciente_json(int paciente_id, char *buffer,
                                              int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT a.id, a.paciente_id, a.medico_id, a.data, a.horario, a.status, "
        "a.motivo_cancelamento, COALESCE(NULLIF(a.especialidade, ''), m.especialidade, '') "
        "FROM agendamentos a "
        "LEFT JOIN medicos m ON m.id = a.medico_id "
        "WHERE a.paciente_id = ? ORDER BY a.id;";
    int usado = 0;
    int primeiro = 1;

    if (buffer == NULL || tamanho <= 0 || paciente_id <= 0)
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_int(stmt, 1, paciente_id);

    buffer[0] = '\0';

    if (repo_json_anexar(buffer, tamanho, &usado, "[") == 0)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char dataJson[32];
        char horarioJson[24];
        char statusJson[48];
        char motivoJson[280];
        char especialidadeJson[128];
        char objeto[780];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int medicoId = sqlite3_column_int(stmt, 2);
        const char *data = (const char *)sqlite3_column_text(stmt, 3);
        const char *horario = (const char *)sqlite3_column_text(stmt, 4);
        const char *status = (const char *)sqlite3_column_text(stmt, 5);
        const char *motivo = (const char *)sqlite3_column_text(stmt, 6);
        int escrito;

        if (repo_json_escapar(dataJson, sizeof(dataJson), data) == 0 ||
            repo_json_escapar(horarioJson, sizeof(horarioJson), horario) == 0 ||
            repo_json_escapar(statusJson, sizeof(statusJson), status) == 0 ||
            repo_json_escapar(motivoJson, sizeof(motivoJson), motivo) == 0 ||
            repo_json_escapar(especialidadeJson, sizeof(especialidadeJson), (const char *)sqlite3_column_text(stmt, 7)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"id\":%d,\"pacienteId\":%d,\"medicoId\":%d,"
                           "\"especialidade\":%s,\"data\":%s,\"horario\":%s,\"status\":%s,"
                           "\"motivoCancelamento\":%s}",
                           primeiro ? "" : ",",
                           id, pacienteId, medicoId, especialidadeJson, dataJson,
                           horarioJson, statusJson, motivoJson);

        if (escrito < 0 || escrito >= (int)sizeof(objeto))
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        if (repo_json_anexar(buffer, tamanho, &usado, objeto) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        primeiro = 0;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);

    return repo_json_anexar(buffer, tamanho, &usado, "]");
}

int agendamento_repo_listar_por_paciente_detalhe_json(int paciente_id, char *buffer,
                                                      int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    /* Mesma lista por paciente, porem com o nome e a especialidade do medico
     * (portal do paciente: evita exibir "Medico #3"). */
    const char *sql =
        "SELECT a.id, a.paciente_id, a.medico_id, a.data, a.horario, a.status, "
        "a.motivo_cancelamento, COALESCE(m.nome, ''), "
        "COALESCE(NULLIF(a.especialidade, ''), m.especialidade, '') "
        "FROM agendamentos a "
        "LEFT JOIN medicos m ON m.id = a.medico_id "
        "WHERE a.paciente_id = ? ORDER BY a.id;";
    int usado = 0;
    int primeiro = 1;

    if (buffer == NULL || tamanho <= 0 || paciente_id <= 0)
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_int(stmt, 1, paciente_id);

    buffer[0] = '\0';

    if (repo_json_anexar(buffer, tamanho, &usado, "[") == 0)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char dataJson[32];
        char horarioJson[24];
        char statusJson[48];
        char motivoJson[280];
        char medicoNomeJson[256];
        char especialidadeJson[128];
        char objeto[1024];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int medicoId = sqlite3_column_int(stmt, 2);
        int escrito;

        if (repo_json_escapar(dataJson, sizeof(dataJson), (const char *)sqlite3_column_text(stmt, 3)) == 0 ||
            repo_json_escapar(horarioJson, sizeof(horarioJson), (const char *)sqlite3_column_text(stmt, 4)) == 0 ||
            repo_json_escapar(statusJson, sizeof(statusJson), (const char *)sqlite3_column_text(stmt, 5)) == 0 ||
            repo_json_escapar(motivoJson, sizeof(motivoJson), (const char *)sqlite3_column_text(stmt, 6)) == 0 ||
            repo_json_escapar(medicoNomeJson, sizeof(medicoNomeJson), (const char *)sqlite3_column_text(stmt, 7)) == 0 ||
            repo_json_escapar(especialidadeJson, sizeof(especialidadeJson), (const char *)sqlite3_column_text(stmt, 8)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"id\":%d,\"pacienteId\":%d,\"medicoId\":%d,"
                           "\"medicoNome\":%s,\"especialidade\":%s,"
                           "\"data\":%s,\"horario\":%s,\"status\":%s,"
                           "\"motivoCancelamento\":%s}",
                           primeiro ? "" : ",",
                           id, pacienteId, medicoId, medicoNomeJson, especialidadeJson,
                           dataJson, horarioJson, statusJson, motivoJson);

        if (escrito < 0 || escrito >= (int)sizeof(objeto) ||
            repo_json_anexar(buffer, tamanho, &usado, objeto) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        primeiro = 0;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);

    return repo_json_anexar(buffer, tamanho, &usado, "]");
}

int agendamento_repo_cancelar(int id, const char *motivo)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE agendamentos SET status = 'CANCELADO', motivo_cancelamento = ? "
        "WHERE id = ? AND status != 'CANCELADO';";
    int alteradas;

    /* Cancelamento exige motivo (a consulta permanece no historico). */
    if (motivo == NULL || motivo[0] == '\0')
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_text(stmt, 1, motivo, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, id);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    sqlite3_finalize(stmt);
    alteradas = sqlite3_changes(db);
    db_fechar(db);

    return alteradas > 0 ? 1 : 0;
}

int agendamento_repo_reagendar(int id, const char *data, const char *horario)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sqlBusca =
        "SELECT paciente_id, medico_id, status FROM agendamentos WHERE id = ?;";
    const char *sqlUpd =
        "UPDATE agendamentos SET data = ?, horario = ? WHERE id = ?;";
    int pacienteId = 0;
    int medicoId = 0;
    char status[32] = "";
    int ok = 0;

    if (data == NULL || data[0] == '\0' || horario == NULL)
    {
        return 0;
    }

    if (agendamento_repo_horario_valido(horario) == 0)
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sqlBusca, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *st;
        pacienteId = sqlite3_column_int(stmt, 0);
        medicoId = sqlite3_column_int(stmt, 1);
        st = (const char *)sqlite3_column_text(stmt, 2);
        snprintf(status, sizeof(status), "%s", st != NULL ? st : "");
    }

    sqlite3_finalize(stmt);

    /* So reagenda antes da realizacao (status AGENDADO) e sem conflito. */
    if (pacienteId <= 0 || strcmp(status, "AGENDADO") != 0 ||
        agendamento_repo_medico_ocupado(medicoId, data, horario) != 0 ||
        agendamento_repo_paciente_ocupado(pacienteId, data, horario) != 0)
    {
        db_fechar(db);
        return 0;
    }

    if (sqlite3_prepare_v2(db, sqlUpd, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_text(stmt, 1, data, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, horario, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, id);

    ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;

    sqlite3_finalize(stmt);
    db_fechar(db);
    return ok ? 1 : 0;
}

int agendamento_repo_medico_ocupado(int medico_id, const char *data,
                                    const char *horario)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT COUNT(*) FROM agendamentos "
        "WHERE medico_id = ? AND data = ? AND horario = ? "
        "AND status != 'CANCELADO';";
    int ocupado = -1;

    if (medico_id <= 0 || data == NULL || horario == NULL)
    {
        return -1;
    }

    if (db_abrir(&db) == 0)
    {
        return -1;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return -1;
    }

    sqlite3_bind_int(stmt, 1, medico_id);
    sqlite3_bind_text(stmt, 2, data, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, horario, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        ocupado = sqlite3_column_int(stmt, 0) > 0 ? 1 : 0;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return ocupado;
}

int agendamento_repo_buscar_no_slot(int medico_id, const char *data,
                                    const char *horario, int *ag_id,
                                    int *paciente_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, paciente_id FROM agendamentos "
        "WHERE medico_id = ? AND data = ? AND horario = ? "
        "AND status != 'CANCELADO' LIMIT 1;";
    int achou = 0;

    if (medico_id <= 0 || data == NULL || horario == NULL)
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_int(stmt, 1, medico_id);
    sqlite3_bind_text(stmt, 2, data, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, horario, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (ag_id != NULL)
        {
            *ag_id = sqlite3_column_int(stmt, 0);
        }
        if (paciente_id != NULL)
        {
            *paciente_id = sqlite3_column_int(stmt, 1);
        }
        achou = 1;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return achou;
}

int agendamento_repo_contar_por_periodo(const char *inicio, const char *fim)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT COUNT(*) FROM agendamentos "
        "WHERE status != 'CANCELADO' AND data BETWEEN ? AND ?;";
    int total = -1;

    if (inicio == NULL || inicio[0] == '\0' || fim == NULL || fim[0] == '\0')
    {
        return -1;
    }

    if (db_abrir(&db) == 0)
    {
        return -1;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return -1;
    }

    sqlite3_bind_text(stmt, 1, inicio, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, fim, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        total = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return total;
}

int agendamento_repo_distribuicao_por_dia_json(const char *inicio,
                                               const char *fim,
                                               char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT data, COUNT(*) FROM agendamentos "
        "WHERE status != 'CANCELADO' AND data BETWEEN ? AND ? "
        "GROUP BY data ORDER BY data;";
    int usado = 0;
    int primeiro = 1;

    if (buffer == NULL || tamanho <= 0 ||
        inicio == NULL || inicio[0] == '\0' || fim == NULL || fim[0] == '\0')
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_text(stmt, 1, inicio, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, fim, -1, SQLITE_STATIC);

    buffer[0] = '\0';

    if (repo_json_anexar(buffer, tamanho, &usado, "[") == 0)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char dataJson[32];
        char objeto[80];
        const char *data = (const char *)sqlite3_column_text(stmt, 0);
        int total = sqlite3_column_int(stmt, 1);
        int escrito;

        if (repo_json_escapar(dataJson, sizeof(dataJson), data) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"data\":%s,\"total\":%d}",
                           primeiro ? "" : ",", dataJson, total);

        if (escrito < 0 || escrito >= (int)sizeof(objeto))
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        if (repo_json_anexar(buffer, tamanho, &usado, objeto) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        primeiro = 0;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);

    if (repo_json_anexar(buffer, tamanho, &usado, "]") == 0)
    {
        return 0;
    }

    return 1;
}

int agendamento_repo_contar_por_medico(int medico_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT COUNT(*) FROM agendamentos "
        "WHERE medico_id = ? AND status != 'CANCELADO';";
    int total = -1;

    if (medico_id <= 0)
    {
        return -1;
    }

    if (db_abrir(&db) == 0)
    {
        return -1;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return -1;
    }

    sqlite3_bind_int(stmt, 1, medico_id);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        total = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return total;
}

int agendamento_repo_contar_ativos(void)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT COUNT(*) FROM agendamentos WHERE status != 'CANCELADO';";
    int total = -1;

    if (db_abrir(&db) == 0)
    {
        return -1;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return -1;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        total = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return total;
}
