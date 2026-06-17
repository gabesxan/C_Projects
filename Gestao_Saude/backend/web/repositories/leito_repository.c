#include "leito_repository.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>
#include <string.h>

/* Status validos de um leito. */
static int statusValido(const char *s)
{
    return s != NULL && (
        strcmp(s, "DISPONIVEL") == 0 ||
        strcmp(s, "OCUPADO") == 0 ||
        strcmp(s, "HIGIENIZACAO") == 0 ||
        strcmp(s, "MANUTENCAO") == 0 ||
        strcmp(s, "BLOQUEADO") == 0);
}

/* Insere uma linha no historico de status usando uma conexao ja aberta. */
static void inserirHistorico(sqlite3 *db, int leito_id, const char *status,
                            const char *responsavel)
{
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO leito_status_historico (leito_id, status, responsavel) "
        "VALUES (?, ?, ?);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        return;
    }

    sqlite3_bind_int(stmt, 1, leito_id);
    sqlite3_bind_text(stmt, 2, status, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, responsavel != NULL ? responsavel : "",
                      -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

int leito_repo_criar(int ala_id, int numero)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO leitos (ala_id, numero, status, paciente_id, ativo) "
        "VALUES (?, ?, 'DISPONIVEL', 0, 1);";
    int id;

    if (ala_id <= 0 || numero <= 0)
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

    sqlite3_bind_int(stmt, 1, ala_id);
    sqlite3_bind_int(stmt, 2, numero);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    sqlite3_finalize(stmt);
    id = (int)sqlite3_last_insert_rowid(db);
    inserirHistorico(db, id, "DISPONIVEL", "cadastro");
    db_fechar(db);
    return 1;
}

int leito_repo_listar_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, ala_id, numero, status, paciente_id "
        "FROM leitos WHERE ativo = 1 ORDER BY id;";
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
        char statusJson[32];
        char objeto[256];
        int id = sqlite3_column_int(stmt, 0);
        int alaId = sqlite3_column_int(stmt, 1);
        int numero = sqlite3_column_int(stmt, 2);
        const char *status = (const char *)sqlite3_column_text(stmt, 3);
        int pacienteId = sqlite3_column_int(stmt, 4);
        int escrito;

        if (repo_json_escapar(statusJson, sizeof(statusJson), status) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"id\":%d,\"alaId\":%d,\"numero\":%d,"
                           "\"status\":%s,\"pacienteId\":%d}",
                           primeiro ? "" : ",",
                           id, alaId, numero, statusJson, pacienteId);

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

int leito_repo_status(int id, char *destino, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "SELECT status FROM leitos WHERE id = ? AND ativo = 1;";
    int ok = 0;

    if (destino == NULL || tamanho <= 0 || id <= 0)
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

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *st = (const char *)sqlite3_column_text(stmt, 0);
        snprintf(destino, (size_t)tamanho, "%s", st != NULL ? st : "");
        ok = 1;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return ok;
}

int leito_repo_ocupar(int leito_id, int paciente_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE leitos SET status = 'OCUPADO', paciente_id = ? "
        "WHERE id = ? AND ativo = 1 AND status = 'DISPONIVEL';";
    int ok = 0;

    if (leito_id <= 0 || paciente_id <= 0)
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
    sqlite3_bind_int(stmt, 2, leito_id);
    ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;

    if (ok)
    {
        inserirHistorico(db, leito_id, "OCUPADO", "internacao");
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return ok ? 1 : 0;
}

int leito_repo_liberar(int leito_id, const char *responsavel)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE leitos SET status = 'HIGIENIZACAO', paciente_id = 0 "
        "WHERE id = ? AND ativo = 1;";
    int ok = 0;

    if (leito_id <= 0)
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

    sqlite3_bind_int(stmt, 1, leito_id);
    ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;

    if (ok)
    {
        inserirHistorico(db, leito_id, "HIGIENIZACAO",
                         responsavel != NULL ? responsavel : "");
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return ok ? 1 : 0;
}

int leito_repo_registrar_status(int id, const char *novo_status,
                                const char *responsavel)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE leitos SET status = ?, paciente_id = 0 "
        "WHERE id = ? AND ativo = 1;";
    int ok = 0;

    /* OCUPADO so e definido pela internacao, nunca manualmente. */
    if (statusValido(novo_status) == 0 || strcmp(novo_status, "OCUPADO") == 0)
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

    sqlite3_bind_text(stmt, 1, novo_status, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, id);
    ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;

    if (ok)
    {
        inserirHistorico(db, id, novo_status,
                         responsavel != NULL ? responsavel : "");
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return ok ? 1 : 0;
}

int leito_repo_ocupacao_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT "
        "COUNT(*), "
        "SUM(status = 'OCUPADO'), "
        "SUM(status = 'DISPONIVEL'), "
        "SUM(status = 'HIGIENIZACAO'), "
        "SUM(status IN ('MANUTENCAO','BLOQUEADO')) "
        "FROM leitos WHERE ativo = 1;";
    int total = 0, ocupados = 0, disponiveis = 0, higienizacao = 0, indisponiveis = 0;
    int taxa = 0;
    int escrito;

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

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        total = sqlite3_column_int(stmt, 0);
        ocupados = sqlite3_column_int(stmt, 1);
        disponiveis = sqlite3_column_int(stmt, 2);
        higienizacao = sqlite3_column_int(stmt, 3);
        indisponiveis = sqlite3_column_int(stmt, 4);
    }

    sqlite3_finalize(stmt);
    db_fechar(db);

    if (total > 0)
    {
        taxa = (ocupados * 100) / total;
    }

    escrito = snprintf(buffer, (size_t)tamanho,
        "{\"total\":%d,\"ocupados\":%d,\"disponiveis\":%d,"
        "\"higienizacao\":%d,\"indisponiveis\":%d,\"taxaOcupacao\":%d}",
        total, ocupados, disponiveis, higienizacao, indisponiveis, taxa);

    return (escrito > 0 && escrito < tamanho) ? 1 : 0;
}

int leito_repo_desativar(int id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "UPDATE leitos SET ativo = 0 WHERE id = ? AND ativo = 1;";
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

int leito_repo_contar_ativos(void)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "SELECT COUNT(*) FROM leitos WHERE ativo = 1;";
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
