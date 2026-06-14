#include "prontuario_repository.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>
#include <string.h>

int prontuario_repo_criar(int paciente_id, int medico_id, const char *data,
                          const char *observacoes, const char *diagnostico,
                          const char *conduta, int alerta_importante)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO prontuarios "
        "(paciente_id, medico_id, data, observacoes, diagnostico, conduta, "
        "alerta_importante, ativo) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, 1);";

    if (paciente_id <= 0 || medico_id <= 0)
    {
        return 0;
    }

    if (data == NULL || data[0] == '\0')
    {
        return 0;
    }

    if (observacoes == NULL || diagnostico == NULL || conduta == NULL)
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
    sqlite3_bind_text(stmt, 4, observacoes, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, diagnostico, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, conduta, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 7, alerta_importante);

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

int prontuario_repo_listar_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, paciente_id, medico_id, data, observacoes, diagnostico, "
        "conduta, alerta_importante "
        "FROM prontuarios WHERE ativo = 1 ORDER BY id;";
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
        char observacoesJson[640];
        char diagnosticoJson[448];
        char condutaJson[448];
        char objeto[2048];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int medicoId = sqlite3_column_int(stmt, 2);
        const char *data = (const char *)sqlite3_column_text(stmt, 3);
        const char *observacoes = (const char *)sqlite3_column_text(stmt, 4);
        const char *diagnostico = (const char *)sqlite3_column_text(stmt, 5);
        const char *conduta = (const char *)sqlite3_column_text(stmt, 6);
        int alerta = sqlite3_column_int(stmt, 7);
        int escrito;

        if (repo_json_escapar(dataJson, sizeof(dataJson), data) == 0 ||
            repo_json_escapar(observacoesJson, sizeof(observacoesJson), observacoes) == 0 ||
            repo_json_escapar(diagnosticoJson, sizeof(diagnosticoJson), diagnostico) == 0 ||
            repo_json_escapar(condutaJson, sizeof(condutaJson), conduta) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"id\":%d,\"pacienteId\":%d,\"medicoId\":%d,\"data\":%s,"
            "\"observacoes\":%s,\"diagnostico\":%s,\"conduta\":%s,"
            "\"alertaImportante\":%d}",
            primeiro ? "" : ",",
            id, pacienteId, medicoId, dataJson, observacoesJson,
            diagnosticoJson, condutaJson, alerta);

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

int prontuario_repo_desativar(int id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE prontuarios SET ativo = 0 WHERE id = ? AND ativo = 1;";
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

int prontuario_repo_contar_ativos(void)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "SELECT COUNT(*) FROM prontuarios WHERE ativo = 1;";
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
