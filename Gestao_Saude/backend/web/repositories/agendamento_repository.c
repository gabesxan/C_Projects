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
    const char *sql =
        "INSERT INTO agendamentos "
        "(paciente_id, medico_id, data, horario, status) "
        "VALUES (?, ?, ?, ?, 'AGENDADO');";

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
    sqlite3_bind_text(stmt, 3, data, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, horario, -1, SQLITE_STATIC);

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

int agendamento_repo_listar_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, paciente_id, medico_id, data, horario, status "
        "FROM agendamentos ORDER BY id;";
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
        char objeto[384];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int medicoId = sqlite3_column_int(stmt, 2);
        const char *data = (const char *)sqlite3_column_text(stmt, 3);
        const char *horario = (const char *)sqlite3_column_text(stmt, 4);
        const char *status = (const char *)sqlite3_column_text(stmt, 5);
        int escrito;

        if (repo_json_escapar(dataJson, sizeof(dataJson), data) == 0 ||
            repo_json_escapar(horarioJson, sizeof(horarioJson), horario) == 0 ||
            repo_json_escapar(statusJson, sizeof(statusJson), status) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"id\":%d,\"pacienteId\":%d,\"medicoId\":%d,"
                           "\"data\":%s,\"horario\":%s,\"status\":%s}",
                           primeiro ? "" : ",",
                           id, pacienteId, medicoId, dataJson, horarioJson, statusJson);

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
        "SELECT id, paciente_id, medico_id, data, horario, status "
        "FROM agendamentos WHERE medico_id = ? ORDER BY id;";
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
        char objeto[384];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int medicoId = sqlite3_column_int(stmt, 2);
        const char *data = (const char *)sqlite3_column_text(stmt, 3);
        const char *horario = (const char *)sqlite3_column_text(stmt, 4);
        const char *status = (const char *)sqlite3_column_text(stmt, 5);
        int escrito;

        if (repo_json_escapar(dataJson, sizeof(dataJson), data) == 0 ||
            repo_json_escapar(horarioJson, sizeof(horarioJson), horario) == 0 ||
            repo_json_escapar(statusJson, sizeof(statusJson), status) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"id\":%d,\"pacienteId\":%d,\"medicoId\":%d,"
                           "\"data\":%s,\"horario\":%s,\"status\":%s}",
                           primeiro ? "" : ",",
                           id, pacienteId, medicoId, dataJson, horarioJson, statusJson);

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
        "SELECT id, paciente_id, medico_id, data, horario, status "
        "FROM agendamentos WHERE paciente_id = ? ORDER BY id;";
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
        char objeto[384];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int medicoId = sqlite3_column_int(stmt, 2);
        const char *data = (const char *)sqlite3_column_text(stmt, 3);
        const char *horario = (const char *)sqlite3_column_text(stmt, 4);
        const char *status = (const char *)sqlite3_column_text(stmt, 5);
        int escrito;

        if (repo_json_escapar(dataJson, sizeof(dataJson), data) == 0 ||
            repo_json_escapar(horarioJson, sizeof(horarioJson), horario) == 0 ||
            repo_json_escapar(statusJson, sizeof(statusJson), status) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"id\":%d,\"pacienteId\":%d,\"medicoId\":%d,"
                           "\"data\":%s,\"horario\":%s,\"status\":%s}",
                           primeiro ? "" : ",",
                           id, pacienteId, medicoId, dataJson, horarioJson, statusJson);

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
