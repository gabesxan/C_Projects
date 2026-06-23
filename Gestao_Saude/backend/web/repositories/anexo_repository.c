#include "anexo_repository.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>
#include <string.h>

int anexo_criar(const char *entidade, int entidade_id, const char *nome,
                const char *mime, long tamanho, const char *caminho,
                int autor_id, const char *autor_login, int *novo_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int ok = 0;
    const char *sql =
        "INSERT INTO anexos "
        "(entidade, entidade_id, nome, mime, tamanho, caminho, autor_id, autor_login) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

    if (novo_id != NULL) *novo_id = 0;

    if (entidade == NULL || entidade[0] == '\0' || entidade_id <= 0 ||
        nome == NULL || nome[0] == '\0' ||
        mime == NULL || mime[0] == '\0' ||
        caminho == NULL || caminho[0] == '\0' ||
        tamanho < 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (autor_id < 0) autor_id = 0;
    if (autor_login == NULL) autor_login = "";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, entidade, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, entidade_id);
        sqlite3_bind_text(stmt, 3, nome, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, mime, -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 5, (sqlite3_int64)tamanho);
        sqlite3_bind_text(stmt, 6, caminho, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 7, autor_id);
        sqlite3_bind_text(stmt, 8, autor_login, -1, SQLITE_STATIC);
        ok = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
    }

    if (ok && novo_id != NULL)
    {
        *novo_id = (int)sqlite3_last_insert_rowid(db);
    }

    db_fechar(db);
    return ok ? 1 : 0;
}

int anexo_definir_caminho(int id, const char *caminho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int alteradas = 0;

    if (id <= 0 || caminho == NULL || caminho[0] == '\0' || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db,
            "UPDATE anexos SET caminho = ? WHERE id = ?;",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, caminho, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, id);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        alteradas = sqlite3_changes(db);
    }

    db_fechar(db);
    return alteradas > 0 ? 1 : 0;
}

int anexo_listar_por_entidade_json(const char *entidade, int entidade_id,
                                   char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, nome, mime, tamanho, autor_login, criado_em "
        "FROM anexos WHERE entidade = ? AND entidade_id = ? "
        "ORDER BY id DESC;";
    int usado = 0;
    int primeiro = 1;

    if (buffer == NULL || tamanho <= 0 ||
        entidade == NULL || entidade[0] == '\0' || entidade_id <= 0 ||
        db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_text(stmt, 1, entidade, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, entidade_id);

    buffer[0] = '\0';
    if (repo_json_anexar(buffer, tamanho, &usado, "[") == 0)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char nomeJson[512];
        char mimeJson[160];
        char loginJson[128];
        char criadoJson[80];
        char objeto[1200];
        int id = sqlite3_column_int(stmt, 0);
        long tamanhoBytes = (long)sqlite3_column_int64(stmt, 3);
        int escrito;

        if (repo_json_escapar(nomeJson, sizeof(nomeJson), (const char *)sqlite3_column_text(stmt, 1)) == 0 ||
            repo_json_escapar(mimeJson, sizeof(mimeJson), (const char *)sqlite3_column_text(stmt, 2)) == 0 ||
            repo_json_escapar(loginJson, sizeof(loginJson), (const char *)sqlite3_column_text(stmt, 4)) == 0 ||
            repo_json_escapar(criadoJson, sizeof(criadoJson), (const char *)sqlite3_column_text(stmt, 5)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"id\":%d,\"nome\":%s,\"mime\":%s,\"tamanho\":%ld,"
            "\"autorLogin\":%s,\"criadoEm\":%s}",
            primeiro ? "" : ",",
            id, nomeJson, mimeJson, tamanhoBytes, loginJson, criadoJson);

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

int anexo_buscar(int id, char *nome, int tnome, char *mime, int tmime,
                 char *caminho, int tcaminho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int achou = 0;

    if (nome != NULL && tnome > 0) nome[0] = '\0';
    if (mime != NULL && tmime > 0) mime[0] = '\0';
    if (caminho != NULL && tcaminho > 0) caminho[0] = '\0';

    if (id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db,
            "SELECT nome, mime, caminho FROM anexos WHERE id = ?;",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, id);
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const char *n = (const char *)sqlite3_column_text(stmt, 0);
            const char *m = (const char *)sqlite3_column_text(stmt, 1);
            const char *c = (const char *)sqlite3_column_text(stmt, 2);
            if (nome != NULL && tnome > 0)
                snprintf(nome, (size_t)tnome, "%s", n != NULL ? n : "");
            if (mime != NULL && tmime > 0)
                snprintf(mime, (size_t)tmime, "%s", m != NULL ? m : "");
            if (caminho != NULL && tcaminho > 0)
                snprintf(caminho, (size_t)tcaminho, "%s", c != NULL ? c : "");
            achou = 1;
        }
        sqlite3_finalize(stmt);
    }

    db_fechar(db);
    return achou;
}

int anexo_remover(int id, char *caminho_out, int tcaminho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int removidas = 0;

    if (caminho_out != NULL && tcaminho > 0) caminho_out[0] = '\0';

    if (id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    /* Captura o caminho antes de apagar, para o chamador remover o arquivo. */
    if (sqlite3_prepare_v2(db,
            "SELECT caminho FROM anexos WHERE id = ?;",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, id);
        if (sqlite3_step(stmt) == SQLITE_ROW && caminho_out != NULL && tcaminho > 0)
        {
            const char *c = (const char *)sqlite3_column_text(stmt, 0);
            snprintf(caminho_out, (size_t)tcaminho, "%s", c != NULL ? c : "");
        }
        sqlite3_finalize(stmt);
    }

    if (sqlite3_prepare_v2(db,
            "DELETE FROM anexos WHERE id = ?;",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, id);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        removidas = sqlite3_changes(db);
    }

    db_fechar(db);
    return removidas > 0 ? 1 : 0;
}
