#include "lote_repository.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>
#include <string.h>

int lote_criar(int convenio_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int novo = 0;

    if (convenio_id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    /* So cria lote para convenio ativo. */
    if (sqlite3_prepare_v2(db,
            "INSERT INTO lotes (convenio_id) "
            "SELECT ? WHERE EXISTS (SELECT 1 FROM convenios WHERE id = ? AND ativo = 1);",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, convenio_id);
        sqlite3_bind_int(stmt, 2, convenio_id);
        if (sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0)
        {
            novo = (int)sqlite3_last_insert_rowid(db);
        }
        sqlite3_finalize(stmt);
    }

    db_fechar(db);
    return novo;
}

int lote_adicionar_cobranca(int lote_id, int cobranca_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int ok = 0;
    /* A cobranca precisa: estar sem lote, ser CONVENIO AUTORIZADA e do mesmo
     * convenio de um lote ABERTO. Tudo verificado na propria clausula. */
    const char *sql =
        "UPDATE cobrancas SET lote_id = ? "
        "WHERE id = ? AND lote_id = 0 AND forma = 'CONVENIO' AND status = 'AUTORIZADA' "
        "AND convenio_id = (SELECT convenio_id FROM lotes WHERE id = ? AND status = 'ABERTO');";

    if (lote_id <= 0 || cobranca_id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, lote_id);
        sqlite3_bind_int(stmt, 2, cobranca_id);
        sqlite3_bind_int(stmt, 3, lote_id);
        ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;
        sqlite3_finalize(stmt);
    }

    db_fechar(db);
    return ok ? 1 : 0;
}

int lote_remover_cobranca(int lote_id, int cobranca_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int ok = 0;
    const char *sql =
        "UPDATE cobrancas SET lote_id = 0 "
        "WHERE id = ? AND lote_id = ? "
        "AND (SELECT status FROM lotes WHERE id = ?) = 'ABERTO';";

    if (lote_id <= 0 || cobranca_id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, cobranca_id);
        sqlite3_bind_int(stmt, 2, lote_id);
        sqlite3_bind_int(stmt, 3, lote_id);
        ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;
        sqlite3_finalize(stmt);
    }

    db_fechar(db);
    return ok ? 1 : 0;
}

int lote_fechar(int lote_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int ok = 0;
    /* Fecha somente se ABERTO e com ao menos uma cobranca vinculada. */
    const char *sql =
        "UPDATE lotes SET status = 'FECHADO', fechado_em = datetime('now') "
        "WHERE id = ? AND status = 'ABERTO' "
        "AND (SELECT COUNT(*) FROM cobrancas WHERE lote_id = ?) > 0;";

    if (lote_id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, lote_id);
        sqlite3_bind_int(stmt, 2, lote_id);
        ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;
        sqlite3_finalize(stmt);
    }

    db_fechar(db);
    return ok ? 1 : 0;
}

int lote_pagar(int lote_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int fechado = 0;
    int ok = 0;

    if (lote_id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    /* So paga lote FECHADO. */
    if (sqlite3_prepare_v2(db,
            "SELECT 1 FROM lotes WHERE id = ? AND status = 'FECHADO';",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, lote_id);
        fechado = sqlite3_step(stmt) == SQLITE_ROW;
        sqlite3_finalize(stmt);
    }

    if (fechado == 0)
    {
        db_fechar(db);
        return 0;
    }

    if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    /* Quita todas as cobrancas do lote e marca o lote como PAGO. */
    if (sqlite3_prepare_v2(db,
            "UPDATE cobrancas SET status = 'PAGA' WHERE lote_id = ?;",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, lote_id);
        ok = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
    }

    if (ok && sqlite3_prepare_v2(db,
            "UPDATE lotes SET status = 'PAGO' WHERE id = ?;",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, lote_id);
        ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;
        sqlite3_finalize(stmt);
    }
    else
    {
        ok = 0;
    }

    sqlite3_exec(db, ok ? "COMMIT;" : "ROLLBACK;", NULL, NULL, NULL);
    db_fechar(db);
    return ok ? 1 : 0;
}

int lote_listar_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT l.id, l.convenio_id, c.nome, l.status, l.criado_em, l.fechado_em, "
        "COUNT(cb.id), COALESCE(SUM(cb.valor_centavos), 0) "
        "FROM lotes l "
        "JOIN convenios c ON c.id = l.convenio_id "
        "LEFT JOIN cobrancas cb ON cb.lote_id = l.id "
        "GROUP BY l.id ORDER BY l.id DESC;";
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
        char nomeJson[128];
        char statusJson[24];
        char criadoJson[64];
        char fechadoJson[64];
        char objeto[512];
        int id = sqlite3_column_int(stmt, 0);
        int convenioId = sqlite3_column_int(stmt, 1);
        int qtd = sqlite3_column_int(stmt, 6);
        int total = sqlite3_column_int(stmt, 7);
        int escrito;

        if (repo_json_escapar(nomeJson, sizeof(nomeJson), (const char *)sqlite3_column_text(stmt, 2)) == 0 ||
            repo_json_escapar(statusJson, sizeof(statusJson), (const char *)sqlite3_column_text(stmt, 3)) == 0 ||
            repo_json_escapar(criadoJson, sizeof(criadoJson), (const char *)sqlite3_column_text(stmt, 4)) == 0 ||
            repo_json_escapar(fechadoJson, sizeof(fechadoJson), (const char *)sqlite3_column_text(stmt, 5)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"id\":%d,\"convenioId\":%d,\"convenioNome\":%s,\"status\":%s,"
            "\"criadoEm\":%s,\"fechadoEm\":%s,\"quantidade\":%d,\"totalCentavos\":%d}",
            primeiro ? "" : ",",
            id, convenioId, nomeJson, statusJson, criadoJson, fechadoJson, qtd, total);

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

/* Anexa os itens (cobrancas) do lote ao buffer da fatura. 1/0. */
static int fatura_itens(sqlite3 *db, int lote_id, char *buffer, int tamanho, int *usado)
{
    sqlite3_stmt *stmt = NULL;
    int primeiro = 1;
    const char *sql =
        "SELECT id, paciente_id, origem, descricao, status, valor_centavos "
        "FROM cobrancas WHERE lote_id = ? ORDER BY id;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        return 0;
    }
    sqlite3_bind_int(stmt, 1, lote_id);

    if (repo_json_anexar(buffer, tamanho, usado, "[") == 0)
    {
        sqlite3_finalize(stmt);
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char origemJson[128];
        char descricaoJson[256];
        char statusJson[24];
        char objeto[512];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int valor = sqlite3_column_int(stmt, 5);
        int escrito;

        if (repo_json_escapar(origemJson, sizeof(origemJson), (const char *)sqlite3_column_text(stmt, 2)) == 0 ||
            repo_json_escapar(descricaoJson, sizeof(descricaoJson), (const char *)sqlite3_column_text(stmt, 3)) == 0 ||
            repo_json_escapar(statusJson, sizeof(statusJson), (const char *)sqlite3_column_text(stmt, 4)) == 0)
        {
            sqlite3_finalize(stmt);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"id\":%d,\"pacienteId\":%d,\"origem\":%s,\"descricao\":%s,"
            "\"status\":%s,\"valorCentavos\":%d}",
            primeiro ? "" : ",",
            id, pacienteId, origemJson, descricaoJson, statusJson, valor);

        if (escrito < 0 || escrito >= (int)sizeof(objeto) ||
            repo_json_anexar(buffer, tamanho, usado, objeto) == 0)
        {
            sqlite3_finalize(stmt);
            return 0;
        }
        primeiro = 0;
    }

    sqlite3_finalize(stmt);
    return repo_json_anexar(buffer, tamanho, usado, "]");
}

int lote_fatura_json(int lote_id, char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    char nomeJson[128];
    char statusJson[24];
    char criadoJson[64];
    char fechadoJson[64];
    char cabecalho[640];
    int convenioId = 0;
    int qtd = 0;
    int total = 0;
    int achou = 0;
    int usado = 0;
    int escrito;
    const char *sql =
        "SELECT l.convenio_id, c.nome, l.status, l.criado_em, l.fechado_em, "
        "COUNT(cb.id), COALESCE(SUM(cb.valor_centavos), 0) "
        "FROM lotes l "
        "JOIN convenios c ON c.id = l.convenio_id "
        "LEFT JOIN cobrancas cb ON cb.lote_id = l.id "
        "WHERE l.id = ? GROUP BY l.id;";

    if (buffer == NULL || tamanho <= 0 || lote_id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, lote_id);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        convenioId = sqlite3_column_int(stmt, 0);
        qtd = sqlite3_column_int(stmt, 5);
        total = sqlite3_column_int(stmt, 6);
        if (repo_json_escapar(nomeJson, sizeof(nomeJson), (const char *)sqlite3_column_text(stmt, 1)) == 1 &&
            repo_json_escapar(statusJson, sizeof(statusJson), (const char *)sqlite3_column_text(stmt, 2)) == 1 &&
            repo_json_escapar(criadoJson, sizeof(criadoJson), (const char *)sqlite3_column_text(stmt, 3)) == 1 &&
            repo_json_escapar(fechadoJson, sizeof(fechadoJson), (const char *)sqlite3_column_text(stmt, 4)) == 1)
        {
            achou = 1;
        }
    }
    sqlite3_finalize(stmt);

    if (achou == 0)
    {
        db_fechar(db);
        return 0;
    }

    escrito = snprintf(cabecalho, sizeof(cabecalho),
        "{\"id\":%d,\"convenioId\":%d,\"convenioNome\":%s,\"status\":%s,"
        "\"criadoEm\":%s,\"fechadoEm\":%s,\"quantidade\":%d,\"totalCentavos\":%d,"
        "\"itens\":",
        lote_id, convenioId, nomeJson, statusJson, criadoJson, fechadoJson, qtd, total);

    buffer[0] = '\0';
    if (escrito < 0 || escrito >= (int)sizeof(cabecalho) ||
        repo_json_anexar(buffer, tamanho, &usado, cabecalho) == 0 ||
        fatura_itens(db, lote_id, buffer, tamanho, &usado) == 0 ||
        repo_json_anexar(buffer, tamanho, &usado, "}") == 0)
    {
        db_fechar(db);
        return 0;
    }

    db_fechar(db);
    return 1;
}
