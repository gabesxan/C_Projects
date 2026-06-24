#include "notificacao_repository.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

int notificacao_criar_para_usuario(int usuario_id, const char *titulo,
                                   const char *mensagem, const char *tipo,
                                   const char *entidade, int entidade_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO notificacoes "
        "(usuario_id, papel, titulo, mensagem, tipo, entidade, entidade_id) "
        "VALUES (?, '', ?, ?, ?, ?, ?);";
    int ok = 0;

    if (usuario_id <= 0 || titulo == NULL || titulo[0] == '\0' ||
        db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, usuario_id);
        sqlite3_bind_text(stmt, 2, titulo, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, mensagem != NULL ? mensagem : "", -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, tipo != NULL && tipo[0] ? tipo : "INFO", -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, entidade != NULL ? entidade : "", -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 6, entidade_id);
        ok = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
    }

    db_fechar(db);
    return ok ? 1 : 0;
}

int notificacao_criar_para_papel(const char *papel, const char *titulo,
                                 const char *mensagem, const char *tipo,
                                 const char *entidade, int entidade_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    /* Fan-out: uma linha por usuario ativo do papel-alvo. */
    const char *sql =
        "INSERT INTO notificacoes "
        "(usuario_id, papel, titulo, mensagem, tipo, entidade, entidade_id) "
        "SELECT u.id, ?, ?, ?, ?, ?, ? FROM usuarios u "
        "WHERE u.papel = ? AND u.ativo = 1;";
    int ok = 0;

    if (papel == NULL || papel[0] == '\0' || titulo == NULL || titulo[0] == '\0' ||
        db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, papel, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, titulo, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, mensagem != NULL ? mensagem : "", -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, tipo != NULL && tipo[0] ? tipo : "INFO", -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, entidade != NULL ? entidade : "", -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 6, entidade_id);
        sqlite3_bind_text(stmt, 7, papel, -1, SQLITE_STATIC);
        ok = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
    }

    db_fechar(db);
    return ok ? 1 : 0;
}

int notificacao_listar_por_usuario_json(int usuario_id, char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, titulo, mensagem, tipo, entidade, entidade_id, lida, criado_em "
        "FROM notificacoes WHERE usuario_id = ? ORDER BY id DESC LIMIT 100;";
    int usado = 0;
    int primeiro = 1;

    if (buffer == NULL || tamanho <= 0 || usuario_id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, usuario_id);

    buffer[0] = '\0';
    if (repo_json_anexar(buffer, tamanho, &usado, "[") == 0)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char tituloJson[256];
        char mensagemJson[600];
        char tipoJson[40];
        char entidadeJson[40];
        char criadoJson[40];
        char objeto[1100];
        int id = sqlite3_column_int(stmt, 0);
        int entidadeId = sqlite3_column_int(stmt, 5);
        int lida = sqlite3_column_int(stmt, 6);
        int escrito;

        if (repo_json_escapar(tituloJson, sizeof(tituloJson), (const char *)sqlite3_column_text(stmt, 1)) == 0 ||
            repo_json_escapar(mensagemJson, sizeof(mensagemJson), (const char *)sqlite3_column_text(stmt, 2)) == 0 ||
            repo_json_escapar(tipoJson, sizeof(tipoJson), (const char *)sqlite3_column_text(stmt, 3)) == 0 ||
            repo_json_escapar(entidadeJson, sizeof(entidadeJson), (const char *)sqlite3_column_text(stmt, 4)) == 0 ||
            repo_json_escapar(criadoJson, sizeof(criadoJson), (const char *)sqlite3_column_text(stmt, 7)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"id\":%d,\"titulo\":%s,\"mensagem\":%s,\"tipo\":%s,"
            "\"entidade\":%s,\"entidadeId\":%d,\"lida\":%d,\"criadoEm\":%s}",
            primeiro ? "" : ",",
            id, tituloJson, mensagemJson, tipoJson, entidadeJson, entidadeId,
            lida, criadoJson);

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

int notificacao_contar_nao_lidas(int usuario_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT COUNT(*) FROM notificacoes WHERE usuario_id = ? AND lida = 0;";
    int total = -1;

    if (usuario_id <= 0 || db_abrir(&db) == 0)
    {
        return -1;
    }
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, usuario_id);
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            total = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    db_fechar(db);
    return total;
}

int notificacao_marcar_lida(int id, int usuario_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE notificacoes SET lida = 1 WHERE id = ? AND usuario_id = ?;";
    int ok = 0;

    if (id <= 0 || usuario_id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, id);
        sqlite3_bind_int(stmt, 2, usuario_id);
        ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;
        sqlite3_finalize(stmt);
    }
    db_fechar(db);
    return ok ? 1 : 0;
}

int notificacao_marcar_todas_lidas(int usuario_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE notificacoes SET lida = 1 WHERE usuario_id = ? AND lida = 0;";
    int ok = 0;

    if (usuario_id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, usuario_id);
        ok = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
    }
    db_fechar(db);
    return ok ? 1 : 0;
}
