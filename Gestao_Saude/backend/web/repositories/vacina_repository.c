#include "vacina_repository.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>

int vacina_criar(const char *nome, const char *fabricante,
                 const char *doencas_alvo, int doses_previstas,
                 int intervalo_dias, int reforco_dias)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int ok = 0;
    const char *sql =
        "INSERT INTO vacinas "
        "(nome, fabricante, doencas_alvo, doses_previstas, intervalo_dias, reforco_dias, ativo) "
        "VALUES (?, ?, ?, ?, ?, ?, 1);";

    if (nome == NULL || nome[0] == '\0' || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (doses_previstas <= 0) doses_previstas = 1;
    if (intervalo_dias < 0) intervalo_dias = 0;
    if (reforco_dias < 0) reforco_dias = 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, nome, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, fabricante != NULL ? fabricante : "", -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, doencas_alvo != NULL ? doencas_alvo : "", -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, doses_previstas);
        sqlite3_bind_int(stmt, 5, intervalo_dias);
        sqlite3_bind_int(stmt, 6, reforco_dias);
        ok = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
    }

    db_fechar(db);
    return ok ? 1 : 0;
}

int vacina_listar_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, nome, fabricante, doencas_alvo, doses_previstas, intervalo_dias, reforco_dias "
        "FROM vacinas WHERE ativo = 1 ORDER BY nome;";
    int usado = 0;
    int primeiro = 1;

    if (buffer == NULL || tamanho <= 0 || db_abrir(&db) == 0)
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
        char nomeJson[256];
        char fabricanteJson[160];
        char doencasJson[256];
        char objeto[900];
        int id = sqlite3_column_int(stmt, 0);
        int dosesPrevistas = sqlite3_column_int(stmt, 4);
        int intervaloDias = sqlite3_column_int(stmt, 5);
        int reforcoDias = sqlite3_column_int(stmt, 6);
        int escrito;

        if (repo_json_escapar(nomeJson, sizeof(nomeJson), (const char *)sqlite3_column_text(stmt, 1)) == 0 ||
            repo_json_escapar(fabricanteJson, sizeof(fabricanteJson), (const char *)sqlite3_column_text(stmt, 2)) == 0 ||
            repo_json_escapar(doencasJson, sizeof(doencasJson), (const char *)sqlite3_column_text(stmt, 3)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"id\":%d,\"nome\":%s,\"fabricante\":%s,\"doencasAlvo\":%s,"
            "\"dosesPrevistas\":%d,\"intervaloDias\":%d,\"reforcoDias\":%d}",
            primeiro ? "" : ",",
            id, nomeJson, fabricanteJson, doencasJson,
            dosesPrevistas, intervaloDias, reforcoDias);

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

int vacina_desativar(int id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int alteradas = 0;

    if (id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db,
            "UPDATE vacinas SET ativo = 0 WHERE id = ? AND ativo = 1;",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, id);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        alteradas = sqlite3_changes(db);
    }

    db_fechar(db);
    return alteradas > 0 ? 1 : 0;
}

int vacina_contar_ativas(void)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int total = -1;

    if (db_abrir(&db) == 0)
    {
        return -1;
    }

    if (sqlite3_prepare_v2(db,
            "SELECT COUNT(*) FROM vacinas WHERE ativo = 1;",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            total = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    db_fechar(db);
    return total;
}

int vacina_ativa(int id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int ativo = 0;

    if (id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db,
            "SELECT 1 FROM vacinas WHERE id = ? AND ativo = 1;",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, id);
        ativo = sqlite3_step(stmt) == SQLITE_ROW;
        sqlite3_finalize(stmt);
    }

    db_fechar(db);
    return ativo;
}
