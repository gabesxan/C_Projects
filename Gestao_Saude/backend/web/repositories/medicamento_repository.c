#include "medicamento_repository.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>
#include <string.h>

int medicamento_criar(const char *nome, const char *apresentacao,
                      const char *unidade, int estoque_minimo)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int ok = 0;
    const char *sql =
        "INSERT INTO medicamentos (nome, apresentacao, unidade, estoque_minimo, ativo) "
        "VALUES (?, ?, ?, ?, 1);";

    if (nome == NULL || nome[0] == '\0' || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (estoque_minimo < 0)
    {
        estoque_minimo = 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, nome, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, apresentacao != NULL ? apresentacao : "", -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, unidade != NULL ? unidade : "", -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, estoque_minimo);
        ok = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
    }

    db_fechar(db);
    return ok ? 1 : 0;
}

int medicamento_listar_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, nome, apresentacao, unidade, estoque_minimo "
        "FROM medicamentos WHERE ativo = 1 ORDER BY nome;";
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
        char apresentacaoJson[256];
        char unidadeJson[64];
        char objeto[700];
        int id = sqlite3_column_int(stmt, 0);
        int estoqueMinimo = sqlite3_column_int(stmt, 4);
        int escrito;

        if (repo_json_escapar(nomeJson, sizeof(nomeJson), (const char *)sqlite3_column_text(stmt, 1)) == 0 ||
            repo_json_escapar(apresentacaoJson, sizeof(apresentacaoJson), (const char *)sqlite3_column_text(stmt, 2)) == 0 ||
            repo_json_escapar(unidadeJson, sizeof(unidadeJson), (const char *)sqlite3_column_text(stmt, 3)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"id\":%d,\"nome\":%s,\"apresentacao\":%s,\"unidade\":%s,"
            "\"estoqueMinimo\":%d}",
            primeiro ? "" : ",",
            id, nomeJson, apresentacaoJson, unidadeJson, estoqueMinimo);

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

int medicamento_desativar(int id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int alteradas = 0;

    if (id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db,
            "UPDATE medicamentos SET ativo = 0 WHERE id = ? AND ativo = 1;",
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

int medicamento_contar_ativos(void)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int total = -1;

    if (db_abrir(&db) == 0)
    {
        return -1;
    }

    if (sqlite3_prepare_v2(db,
            "SELECT COUNT(*) FROM medicamentos WHERE ativo = 1;",
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

int medicamento_ativo(int id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int ativo = 0;

    if (id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db,
            "SELECT 1 FROM medicamentos WHERE id = ? AND ativo = 1;",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, id);
        ativo = sqlite3_step(stmt) == SQLITE_ROW;
        sqlite3_finalize(stmt);
    }

    db_fechar(db);
    return ativo;
}
