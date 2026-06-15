#include "exame_repository.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>
#include <string.h>

int exame_repo_criar(int paciente_id, int medico_id, int prontuario_id,
                     int tipo_exame, const char *data_solicitacao, int urgente)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO exames "
        "(paciente_id, medico_id, prontuario_id, tipo_exame, data_solicitacao, "
        "data_resultado, resultado, status, urgente, ativo) "
        "VALUES (?, ?, ?, ?, ?, '', '', 'SOLICITADO', ?, 1);";

    if (paciente_id <= 0 || medico_id <= 0 || prontuario_id <= 0 || tipo_exame <= 0)
    {
        return 0;
    }

    if (data_solicitacao == NULL || data_solicitacao[0] == '\0')
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
    sqlite3_bind_int(stmt, 3, prontuario_id);
    sqlite3_bind_int(stmt, 4, tipo_exame);
    sqlite3_bind_text(stmt, 5, data_solicitacao, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, urgente);

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

int exame_repo_listar_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, paciente_id, medico_id, prontuario_id, tipo_exame, "
        "data_solicitacao, data_resultado, resultado, status, urgente "
        "FROM exames WHERE ativo = 1 ORDER BY id;";
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
        char dataSolicitacaoJson[32];
        char dataResultadoJson[32];
        char resultadoJson[640];
        char statusJson[48];
        char objeto[1024];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int medicoId = sqlite3_column_int(stmt, 2);
        int prontuarioId = sqlite3_column_int(stmt, 3);
        int tipoExame = sqlite3_column_int(stmt, 4);
        const char *dataSolic = (const char *)sqlite3_column_text(stmt, 5);
        const char *dataResul = (const char *)sqlite3_column_text(stmt, 6);
        const char *resultado = (const char *)sqlite3_column_text(stmt, 7);
        const char *status = (const char *)sqlite3_column_text(stmt, 8);
        int urgente = sqlite3_column_int(stmt, 9);
        int escrito;

        if (repo_json_escapar(dataSolicitacaoJson, sizeof(dataSolicitacaoJson), dataSolic) == 0 ||
            repo_json_escapar(dataResultadoJson, sizeof(dataResultadoJson), dataResul) == 0 ||
            repo_json_escapar(resultadoJson, sizeof(resultadoJson), resultado) == 0 ||
            repo_json_escapar(statusJson, sizeof(statusJson), status) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"id\":%d,\"pacienteId\":%d,\"medicoId\":%d,\"prontuarioId\":%d,"
            "\"tipoExame\":%d,\"dataSolicitacao\":%s,\"dataResultado\":%s,"
            "\"resultado\":%s,\"status\":%s,\"urgente\":%d}",
            primeiro ? "" : ",",
            id, pacienteId, medicoId, prontuarioId, tipoExame,
            dataSolicitacaoJson, dataResultadoJson, resultadoJson, statusJson,
            urgente);

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

int exame_repo_listar_por_paciente_json(int paciente_id, char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, paciente_id, medico_id, prontuario_id, tipo_exame, "
        "data_solicitacao, data_resultado, resultado, status, urgente "
        "FROM exames WHERE paciente_id = ? AND ativo = 1 ORDER BY id;";
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
        char dataSolicitacaoJson[32];
        char dataResultadoJson[32];
        char resultadoJson[640];
        char statusJson[48];
        char objeto[1024];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int medicoId = sqlite3_column_int(stmt, 2);
        int prontuarioId = sqlite3_column_int(stmt, 3);
        int tipoExame = sqlite3_column_int(stmt, 4);
        const char *dataSolic = (const char *)sqlite3_column_text(stmt, 5);
        const char *dataResul = (const char *)sqlite3_column_text(stmt, 6);
        const char *resultado = (const char *)sqlite3_column_text(stmt, 7);
        const char *status = (const char *)sqlite3_column_text(stmt, 8);
        int urgente = sqlite3_column_int(stmt, 9);
        int escrito;

        if (repo_json_escapar(dataSolicitacaoJson, sizeof(dataSolicitacaoJson), dataSolic) == 0 ||
            repo_json_escapar(dataResultadoJson, sizeof(dataResultadoJson), dataResul) == 0 ||
            repo_json_escapar(resultadoJson, sizeof(resultadoJson), resultado) == 0 ||
            repo_json_escapar(statusJson, sizeof(statusJson), status) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"id\":%d,\"pacienteId\":%d,\"medicoId\":%d,\"prontuarioId\":%d,"
            "\"tipoExame\":%d,\"dataSolicitacao\":%s,\"dataResultado\":%s,"
            "\"resultado\":%s,\"status\":%s,\"urgente\":%d}",
            primeiro ? "" : ",",
            id, pacienteId, medicoId, prontuarioId, tipoExame,
            dataSolicitacaoJson, dataResultadoJson, resultadoJson, statusJson,
            urgente);

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

int exame_repo_desativar(int id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "UPDATE exames SET ativo = 0 WHERE id = ? AND ativo = 1;";
    int alteradas;

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_int(stmt, 1, id);

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

int exame_repo_contar_ativos(void)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "SELECT COUNT(*) FROM exames WHERE ativo = 1;";
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
