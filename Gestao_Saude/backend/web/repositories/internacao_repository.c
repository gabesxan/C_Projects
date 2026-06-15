#include "internacao_repository.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>
#include <string.h>

int internacao_repo_criar(int paciente_id, int ala_id, int leito_id,
                          const char *data_entrada)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO internacoes "
        "(paciente_id, ala_id, leito_id, data_entrada, data_alta, status) "
        "VALUES (?, ?, ?, ?, '', 'INTERNADO');";

    if (paciente_id <= 0 || ala_id <= 0 || leito_id <= 0)
    {
        return 0;
    }

    if (data_entrada == NULL || data_entrada[0] == '\0')
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
    sqlite3_bind_int(stmt, 2, ala_id);
    sqlite3_bind_int(stmt, 3, leito_id);
    sqlite3_bind_text(stmt, 4, data_entrada, -1, SQLITE_STATIC);

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

int internacao_repo_listar_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, paciente_id, ala_id, leito_id, data_entrada, data_alta, status "
        "FROM internacoes ORDER BY id;";
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
        char dataEntradaJson[32];
        char dataAltaJson[32];
        char statusJson[48];
        char objeto[384];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int alaId = sqlite3_column_int(stmt, 2);
        int leitoId = sqlite3_column_int(stmt, 3);
        const char *dataEntrada = (const char *)sqlite3_column_text(stmt, 4);
        const char *dataAlta = (const char *)sqlite3_column_text(stmt, 5);
        const char *status = (const char *)sqlite3_column_text(stmt, 6);
        int escrito;

        if (repo_json_escapar(dataEntradaJson, sizeof(dataEntradaJson), dataEntrada) == 0 ||
            repo_json_escapar(dataAltaJson, sizeof(dataAltaJson), dataAlta) == 0 ||
            repo_json_escapar(statusJson, sizeof(statusJson), status) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"id\":%d,\"pacienteId\":%d,\"alaId\":%d,\"leitoId\":%d,"
            "\"dataEntrada\":%s,\"dataAlta\":%s,\"status\":%s}",
            primeiro ? "" : ",",
            id, pacienteId, alaId, leitoId, dataEntradaJson, dataAltaJson,
            statusJson);

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

int internacao_repo_dar_alta(int id, const char *data_alta)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE internacoes SET status = 'ALTA', data_alta = ? "
        "WHERE id = ? AND status = 'INTERNADO';";
    int alteradas;

    if (data_alta == NULL || data_alta[0] == '\0')
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

    sqlite3_bind_text(stmt, 1, data_alta, -1, SQLITE_STATIC);
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

int internacao_repo_contar_ativos(void)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT COUNT(*) FROM internacoes WHERE status = 'INTERNADO';";
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
