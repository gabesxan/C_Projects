#include "sessao_repository.h"
#include "database.h"

#include <stdio.h>
#include <string.h>
#include <openssl/rand.h>

/* Bytes de entropia do token; em hex viram o dobro de caracteres. */
#define TOKEN_BYTES 32

/* Gera um token opaco aleatorio em hex (NUL-terminado). tam >= 65. */
static int gerar_token(char *destino, int tam)
{
    unsigned char bytes[TOKEN_BYTES];
    int i;

    if (destino == NULL || tam < TOKEN_BYTES * 2 + 1)
    {
        return 0;
    }

    if (RAND_bytes(bytes, sizeof(bytes)) != 1)
    {
        return 0;
    }

    for (i = 0; i < (int)sizeof(bytes); i++)
    {
        sprintf(destino + i * 2, "%02x", bytes[i]);
    }

    destino[sizeof(bytes) * 2] = '\0';
    return 1;
}

int sessao_repo_criar(int usuario_id, int validade_horas,
                      char *token_out, int token_tam)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    char token[TOKEN_BYTES * 2 + 1];
    char janela[32];
    int ok = 0;

    if (usuario_id <= 0 || validade_horas <= 0 ||
        token_out == NULL || token_tam < (int)sizeof(token))
    {
        return 0;
    }

    if (gerar_token(token, sizeof(token)) == 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    /* Faxina oportunista: descarta sessoes ja expiradas. */
    db_executar("DELETE FROM sessoes WHERE expira_em <= datetime('now');");

    snprintf(janela, sizeof(janela), "+%d hours", validade_horas);

    if (sqlite3_prepare_v2(db,
            "INSERT INTO sessoes (token, usuario_id, expira_em) "
            "VALUES (?, ?, datetime('now', ?));",
            -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_text(stmt, 1, token, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, usuario_id);
    sqlite3_bind_text(stmt, 3, janela, -1, SQLITE_STATIC);
    ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    db_fechar(db);

    if (ok)
    {
        strncpy(token_out, token, (size_t)token_tam - 1);
        token_out[token_tam - 1] = '\0';
    }

    return ok ? 1 : 0;
}

int sessao_repo_validar(const char *token,
                        char *papel, int papel_tam,
                        int *paciente_id, int *medico_id, int *usuario_id,
                        char *login_out, int login_tam)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT u.papel, u.paciente_id, u.medico_id, u.id, u.login "
        "FROM sessoes s JOIN usuarios u ON u.id = s.usuario_id "
        "WHERE s.token = ? AND s.expira_em > datetime('now') AND u.ativo = 1;";
    int ok = 0;

    if (token == NULL || token[0] == '\0' || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_text(stmt, 1, token, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *papelArmazenado = (const char *)sqlite3_column_text(stmt, 0);
        const char *login = (const char *)sqlite3_column_text(stmt, 4);

        if (papel != NULL && papel_tam > 0 && papelArmazenado != NULL)
        {
            strncpy(papel, papelArmazenado, (size_t)papel_tam - 1);
            papel[papel_tam - 1] = '\0';
        }
        if (paciente_id != NULL) *paciente_id = sqlite3_column_int(stmt, 1);
        if (medico_id != NULL) *medico_id = sqlite3_column_int(stmt, 2);
        if (usuario_id != NULL) *usuario_id = sqlite3_column_int(stmt, 3);
        if (login_out != NULL && login_tam > 0 && login != NULL)
        {
            strncpy(login_out, login, (size_t)login_tam - 1);
            login_out[login_tam - 1] = '\0';
        }
        ok = 1;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return ok;
}

int sessao_repo_remover(const char *token)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int alteradas = 0;

    if (token == NULL || token[0] == '\0' || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, "DELETE FROM sessoes WHERE token = ?;",
                           -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_text(stmt, 1, token, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    alteradas = sqlite3_changes(db);
    db_fechar(db);

    return alteradas > 0 ? 1 : 0;
}
