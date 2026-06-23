#include "consentimento_repository.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>
#include <string.h>

int consentimento_criar(int paciente_id, const char *finalidade,
                        const char *versao_termo, int *novo_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int ok = 0;
    const char *sql =
        "INSERT INTO consentimentos "
        "(paciente_id, finalidade, versao_termo, status, concedido_em, ativo) "
        "VALUES (?, ?, ?, 'CONCEDIDO', datetime('now'), 1);";

    if (novo_id != NULL) *novo_id = 0;

    if (paciente_id <= 0 ||
        finalidade == NULL || finalidade[0] == '\0' ||
        versao_termo == NULL || versao_termo[0] == '\0' ||
        db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, paciente_id);
        sqlite3_bind_text(stmt, 2, finalidade, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, versao_termo, -1, SQLITE_STATIC);
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

int consentimento_listar_por_paciente_json(int paciente_id, char *buffer,
                                           int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, paciente_id, finalidade, versao_termo, status, "
        "concedido_em, revogado_em, motivo_revogacao, criado_em, ativo "
        "FROM consentimentos WHERE paciente_id = ? ORDER BY id DESC;";
    int usado = 0;
    int primeiro = 1;

    if (buffer == NULL || tamanho <= 0 || paciente_id <= 0 || db_abrir(&db) == 0)
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
        char finalidadeJson[320];
        char versaoJson[160];
        char statusJson[64];
        char concedidoJson[80];
        char revogadoJson[80];
        char motivoJson[520];
        char criadoJson[80];
        char objeto[1800];
        int id = sqlite3_column_int(stmt, 0);
        int pid = sqlite3_column_int(stmt, 1);
        int ativo = sqlite3_column_int(stmt, 9);
        int escrito;

        if (repo_json_escapar(finalidadeJson, sizeof(finalidadeJson), (const char *)sqlite3_column_text(stmt, 2)) == 0 ||
            repo_json_escapar(versaoJson, sizeof(versaoJson), (const char *)sqlite3_column_text(stmt, 3)) == 0 ||
            repo_json_escapar(statusJson, sizeof(statusJson), (const char *)sqlite3_column_text(stmt, 4)) == 0 ||
            repo_json_escapar(concedidoJson, sizeof(concedidoJson), (const char *)sqlite3_column_text(stmt, 5)) == 0 ||
            repo_json_escapar(revogadoJson, sizeof(revogadoJson), (const char *)sqlite3_column_text(stmt, 6)) == 0 ||
            repo_json_escapar(motivoJson, sizeof(motivoJson), (const char *)sqlite3_column_text(stmt, 7)) == 0 ||
            repo_json_escapar(criadoJson, sizeof(criadoJson), (const char *)sqlite3_column_text(stmt, 8)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"id\":%d,\"pacienteId\":%d,\"finalidade\":%s,\"versaoTermo\":%s,"
            "\"status\":%s,\"concedidoEm\":%s,\"revogadoEm\":%s,"
            "\"motivoRevogacao\":%s,\"criadoEm\":%s,\"ativo\":%d}",
            primeiro ? "" : ",",
            id, pid, finalidadeJson, versaoJson, statusJson, concedidoJson,
            revogadoJson, motivoJson, criadoJson, ativo);

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

int consentimento_buscar(int id, int *paciente_id, char *status, int tstatus)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int achou = 0;

    if (paciente_id != NULL) *paciente_id = 0;
    if (status != NULL && tstatus > 0) status[0] = '\0';

    if (id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db,
            "SELECT paciente_id, status FROM consentimentos WHERE id = ?;",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, id);
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const char *st = (const char *)sqlite3_column_text(stmt, 1);
            if (paciente_id != NULL)
                *paciente_id = sqlite3_column_int(stmt, 0);
            if (status != NULL && tstatus > 0)
                snprintf(status, (size_t)tstatus, "%s", st != NULL ? st : "");
            achou = 1;
        }
        sqlite3_finalize(stmt);
    }

    db_fechar(db);
    return achou;
}

int consentimento_revogar(int id, const char *motivo)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int alteradas = 0;
    /* So revoga quem esta CONCEDIDO: garante a transicao valida e a
     * idempotencia (revogar de novo nao altera nada). Linha preservada. */
    const char *sql =
        "UPDATE consentimentos "
        "SET status = 'REVOGADO', revogado_em = datetime('now'), "
        "    motivo_revogacao = ?, ativo = 0 "
        "WHERE id = ? AND status = 'CONCEDIDO';";

    if (id <= 0 || motivo == NULL || motivo[0] == '\0' || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, motivo, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, id);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        alteradas = sqlite3_changes(db);
    }

    db_fechar(db);
    return alteradas > 0 ? 1 : 0;
}
