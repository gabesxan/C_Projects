#include "estoque_repository.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>
#include <string.h>

/* Capacidade maxima de lotes considerados numa baixa FIFO (suficiente para o
 * estoque de um unico medicamento; lotes alem disso sao raros). */
#define MAX_LOTES 256

/* Grava uma movimentacao na conexao 'db' ja aberta (usada dentro de transacao
 * ou de chamada simples). Retorna 1 em sucesso. */
static int registrar_movimentacao(sqlite3 *db, int medicamento_id,
                                  int estoque_item_id, const char *tipo,
                                  int quantidade, const char *motivo,
                                  int usuario_id, const char *usuario_login)
{
    sqlite3_stmt *stmt = NULL;
    int ok = 0;
    const char *sql =
        "INSERT INTO movimentacoes "
        "(medicamento_id, estoque_item_id, tipo, quantidade, motivo, usuario_id, usuario_login) "
        "VALUES (?, ?, ?, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        return 0;
    }
    sqlite3_bind_int(stmt, 1, medicamento_id);
    if (estoque_item_id > 0)
    {
        sqlite3_bind_int(stmt, 2, estoque_item_id);
    }
    else
    {
        sqlite3_bind_null(stmt, 2);
    }
    sqlite3_bind_text(stmt, 3, tipo, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, quantidade);
    sqlite3_bind_text(stmt, 5, motivo != NULL ? motivo : "", -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, usuario_id);
    sqlite3_bind_text(stmt, 7, usuario_login != NULL ? usuario_login : "", -1, SQLITE_STATIC);
    ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok ? 1 : 0;
}

int estoque_entrada(int medicamento_id, const char *lote, const char *validade,
                    int quantidade, const char *localizacao,
                    int usuario_id, const char *usuario_login)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int item_id = 0;

    if (medicamento_id <= 0 || quantidade <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (lote == NULL) lote = "";
    if (validade == NULL) validade = "";
    if (localizacao == NULL) localizacao = "";

    if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    /* Reaproveita um lote identico (mesmo medicamento/lote/validade) se houver. */
    if (sqlite3_prepare_v2(db,
            "SELECT id FROM estoque_itens "
            "WHERE medicamento_id = ? AND lote = ? AND validade = ? LIMIT 1;",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, medicamento_id);
        sqlite3_bind_text(stmt, 2, lote, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, validade, -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            item_id = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (item_id > 0)
    {
        stmt = NULL;
        if (sqlite3_prepare_v2(db,
                "UPDATE estoque_itens SET quantidade = quantidade + ? WHERE id = ?;",
                -1, &stmt, NULL) != SQLITE_OK)
        {
            sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
            db_fechar(db);
            return 0;
        }
        sqlite3_bind_int(stmt, 1, quantidade);
        sqlite3_bind_int(stmt, 2, item_id);
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            sqlite3_finalize(stmt);
            sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
            db_fechar(db);
            return 0;
        }
        sqlite3_finalize(stmt);
    }
    else
    {
        stmt = NULL;
        if (sqlite3_prepare_v2(db,
                "INSERT INTO estoque_itens "
                "(medicamento_id, lote, validade, quantidade, localizacao) "
                "VALUES (?, ?, ?, ?, ?);",
                -1, &stmt, NULL) != SQLITE_OK)
        {
            sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
            db_fechar(db);
            return 0;
        }
        sqlite3_bind_int(stmt, 1, medicamento_id);
        sqlite3_bind_text(stmt, 2, lote, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, validade, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, quantidade);
        sqlite3_bind_text(stmt, 5, localizacao, -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            sqlite3_finalize(stmt);
            sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
            db_fechar(db);
            return 0;
        }
        sqlite3_finalize(stmt);
        item_id = (int)sqlite3_last_insert_rowid(db);
    }

    if (registrar_movimentacao(db, medicamento_id, item_id, "ENTRADA",
                               quantidade, "", usuario_id, usuario_login) == 0)
    {
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        db_fechar(db);
        return 0;
    }

    sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
    db_fechar(db);
    return 1;
}

int estoque_baixar(int medicamento_id, int quantidade, const char *tipo,
                   const char *motivo, int usuario_id, const char *usuario_login)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int ids[MAX_LOTES];
    int qtds[MAX_LOTES];
    int n = 0;
    int total = 0;
    int restante;
    int i;

    if (medicamento_id <= 0 || quantidade <= 0 || tipo == NULL ||
        db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    /* Coleta os lotes com saldo, por validade mais proxima primeiro (lotes sem
     * validade por ultimo). Materializa antes de atualizar para nao mexer no
     * cursor durante os UPDATEs. */
    if (sqlite3_prepare_v2(db,
            "SELECT id, quantidade FROM estoque_itens "
            "WHERE medicamento_id = ? AND quantidade > 0 "
            "ORDER BY (validade = ''), validade, id;",
            -1, &stmt, NULL) != SQLITE_OK)
    {
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, medicamento_id);
    while (sqlite3_step(stmt) == SQLITE_ROW && n < MAX_LOTES)
    {
        ids[n] = sqlite3_column_int(stmt, 0);
        qtds[n] = sqlite3_column_int(stmt, 1);
        total += qtds[n];
        n++;
    }
    sqlite3_finalize(stmt);

    if (total < quantidade)
    {
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        db_fechar(db);
        return 0;
    }

    restante = quantidade;
    for (i = 0; i < n && restante > 0; i++)
    {
        int tira = qtds[i] < restante ? qtds[i] : restante;

        stmt = NULL;
        if (sqlite3_prepare_v2(db,
                "UPDATE estoque_itens SET quantidade = quantidade - ? WHERE id = ?;",
                -1, &stmt, NULL) != SQLITE_OK)
        {
            sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
            db_fechar(db);
            return 0;
        }
        sqlite3_bind_int(stmt, 1, tira);
        sqlite3_bind_int(stmt, 2, ids[i]);
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            sqlite3_finalize(stmt);
            sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
            db_fechar(db);
            return 0;
        }
        sqlite3_finalize(stmt);
        restante -= tira;
    }

    if (registrar_movimentacao(db, medicamento_id, 0, tipo, quantidade, motivo,
                               usuario_id, usuario_login) == 0)
    {
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        db_fechar(db);
        return 0;
    }

    sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
    db_fechar(db);
    return 1;
}

int estoque_saldo(int medicamento_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int saldo = -1;

    if (medicamento_id <= 0 || db_abrir(&db) == 0)
    {
        return -1;
    }

    if (sqlite3_prepare_v2(db,
            "SELECT COALESCE(SUM(quantidade), 0) FROM estoque_itens "
            "WHERE medicamento_id = ?;",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, medicamento_id);
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            saldo = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    db_fechar(db);
    return saldo;
}

int estoque_itens_listar_json(int medicamento_id, char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, lote, validade, quantidade, localizacao FROM estoque_itens "
        "WHERE medicamento_id = ? AND quantidade > 0 "
        "ORDER BY (validade = ''), validade, id;";
    int usado = 0;
    int primeiro = 1;

    if (buffer == NULL || tamanho <= 0 || medicamento_id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, medicamento_id);

    buffer[0] = '\0';
    if (repo_json_anexar(buffer, tamanho, &usado, "[") == 0)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char loteJson[96];
        char validadeJson[32];
        char localizacaoJson[96];
        char objeto[400];
        int id = sqlite3_column_int(stmt, 0);
        int quantidade = sqlite3_column_int(stmt, 3);
        int escrito;

        if (repo_json_escapar(loteJson, sizeof(loteJson), (const char *)sqlite3_column_text(stmt, 1)) == 0 ||
            repo_json_escapar(validadeJson, sizeof(validadeJson), (const char *)sqlite3_column_text(stmt, 2)) == 0 ||
            repo_json_escapar(localizacaoJson, sizeof(localizacaoJson), (const char *)sqlite3_column_text(stmt, 4)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"id\":%d,\"lote\":%s,\"validade\":%s,\"quantidade\":%d,\"localizacao\":%s}",
            primeiro ? "" : ",",
            id, loteJson, validadeJson, quantidade, localizacaoJson);

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

int movimentacao_listar_json(int medicamento_id, char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, tipo, quantidade, motivo, usuario_login, criado_em "
        "FROM movimentacoes WHERE medicamento_id = ? ORDER BY id DESC;";
    int usado = 0;
    int primeiro = 1;

    if (buffer == NULL || tamanho <= 0 || medicamento_id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, medicamento_id);

    buffer[0] = '\0';
    if (repo_json_anexar(buffer, tamanho, &usado, "[") == 0)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char tipoJson[24];
        char motivoJson[256];
        char loginJson[96];
        char criadoJson[40];
        char objeto[560];
        int id = sqlite3_column_int(stmt, 0);
        int quantidade = sqlite3_column_int(stmt, 2);
        int escrito;

        if (repo_json_escapar(tipoJson, sizeof(tipoJson), (const char *)sqlite3_column_text(stmt, 1)) == 0 ||
            repo_json_escapar(motivoJson, sizeof(motivoJson), (const char *)sqlite3_column_text(stmt, 3)) == 0 ||
            repo_json_escapar(loginJson, sizeof(loginJson), (const char *)sqlite3_column_text(stmt, 4)) == 0 ||
            repo_json_escapar(criadoJson, sizeof(criadoJson), (const char *)sqlite3_column_text(stmt, 5)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"id\":%d,\"tipo\":%s,\"quantidade\":%d,\"motivo\":%s,"
            "\"usuario\":%s,\"criadoEm\":%s}",
            primeiro ? "" : ",",
            id, tipoJson, quantidade, motivoJson, loginJson, criadoJson);

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

static int anexar_saldos_json(sqlite3 *db, char *buffer, int tamanho, int *usado,
                              const char *sql)
{
    sqlite3_stmt *stmt = NULL;
    int primeiro = 1;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char nomeJson[256];
        char apresentacaoJson[256];
        char unidadeJson[64];
        char objeto[760];
        int id = sqlite3_column_int(stmt, 0);
        int estoqueMinimo = sqlite3_column_int(stmt, 4);
        int saldo = sqlite3_column_int(stmt, 5);
        int escrito;

        if (repo_json_escapar(nomeJson, sizeof(nomeJson), (const char *)sqlite3_column_text(stmt, 1)) == 0 ||
            repo_json_escapar(apresentacaoJson, sizeof(apresentacaoJson), (const char *)sqlite3_column_text(stmt, 2)) == 0 ||
            repo_json_escapar(unidadeJson, sizeof(unidadeJson), (const char *)sqlite3_column_text(stmt, 3)) == 0)
        {
            sqlite3_finalize(stmt);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"medicamentoId\":%d,\"nome\":%s,\"apresentacao\":%s,"
            "\"unidade\":%s,\"estoqueMinimo\":%d,\"saldo\":%d}",
            primeiro ? "" : ",",
            id, nomeJson, apresentacaoJson, unidadeJson, estoqueMinimo, saldo);

        if (escrito < 0 || escrito >= (int)sizeof(objeto) ||
            repo_json_anexar(buffer, tamanho, usado, objeto) == 0)
        {
            sqlite3_finalize(stmt);
            return 0;
        }
        primeiro = 0;
    }

    sqlite3_finalize(stmt);
    return 1;
}

static int anexar_validade_proxima_json(sqlite3 *db, char *buffer, int tamanho,
                                        int *usado)
{
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT e.id, m.id, m.nome, e.lote, e.validade, e.quantidade, e.localizacao "
        "FROM estoque_itens e "
        "JOIN medicamentos m ON m.id = e.medicamento_id "
        "WHERE m.ativo = 1 AND e.quantidade > 0 AND e.validade <> '' "
        "AND date(e.validade) BETWEEN date('now') AND date('now', '+30 days') "
        "ORDER BY e.validade, m.nome, e.id;";
    int primeiro = 1;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char nomeJson[256];
        char loteJson[96];
        char validadeJson[32];
        char localizacaoJson[96];
        char objeto[760];
        int id = sqlite3_column_int(stmt, 0);
        int medicamentoId = sqlite3_column_int(stmt, 1);
        int quantidade = sqlite3_column_int(stmt, 5);
        int escrito;

        if (repo_json_escapar(nomeJson, sizeof(nomeJson), (const char *)sqlite3_column_text(stmt, 2)) == 0 ||
            repo_json_escapar(loteJson, sizeof(loteJson), (const char *)sqlite3_column_text(stmt, 3)) == 0 ||
            repo_json_escapar(validadeJson, sizeof(validadeJson), (const char *)sqlite3_column_text(stmt, 4)) == 0 ||
            repo_json_escapar(localizacaoJson, sizeof(localizacaoJson), (const char *)sqlite3_column_text(stmt, 6)) == 0)
        {
            sqlite3_finalize(stmt);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"id\":%d,\"medicamentoId\":%d,\"nome\":%s,\"lote\":%s,"
            "\"validade\":%s,\"quantidade\":%d,\"localizacao\":%s}",
            primeiro ? "" : ",",
            id, medicamentoId, nomeJson, loteJson, validadeJson, quantidade, localizacaoJson);

        if (escrito < 0 || escrito >= (int)sizeof(objeto) ||
            repo_json_anexar(buffer, tamanho, usado, objeto) == 0)
        {
            sqlite3_finalize(stmt);
            return 0;
        }
        primeiro = 0;
    }

    sqlite3_finalize(stmt);
    return 1;
}

int estoque_alertas_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    int usado = 0;
    const char *sqlSaldos =
        "SELECT m.id, m.nome, m.apresentacao, m.unidade, m.estoque_minimo, "
        "COALESCE(SUM(e.quantidade), 0) AS saldo "
        "FROM medicamentos m "
        "LEFT JOIN estoque_itens e ON e.medicamento_id = m.id "
        "WHERE m.ativo = 1 "
        "GROUP BY m.id, m.nome, m.apresentacao, m.unidade, m.estoque_minimo "
        "ORDER BY m.nome;";
    const char *sqlBaixo =
        "SELECT m.id, m.nome, m.apresentacao, m.unidade, m.estoque_minimo, "
        "COALESCE(SUM(e.quantidade), 0) AS saldo "
        "FROM medicamentos m "
        "LEFT JOIN estoque_itens e ON e.medicamento_id = m.id "
        "WHERE m.ativo = 1 "
        "GROUP BY m.id, m.nome, m.apresentacao, m.unidade, m.estoque_minimo "
        "HAVING saldo < m.estoque_minimo "
        "ORDER BY m.nome;";

    if (buffer == NULL || tamanho <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    buffer[0] = '\0';
    if (repo_json_anexar(buffer, tamanho, &usado, "{\"saldos\":[") == 0 ||
        anexar_saldos_json(db, buffer, tamanho, &usado, sqlSaldos) == 0 ||
        repo_json_anexar(buffer, tamanho, &usado, "],\"estoqueBaixo\":[") == 0 ||
        anexar_saldos_json(db, buffer, tamanho, &usado, sqlBaixo) == 0 ||
        repo_json_anexar(buffer, tamanho, &usado, "],\"validadeProxima\":[") == 0 ||
        anexar_validade_proxima_json(db, buffer, tamanho, &usado) == 0 ||
        repo_json_anexar(buffer, tamanho, &usado, "]}") == 0)
    {
        db_fechar(db);
        return 0;
    }

    db_fechar(db);
    return 1;
}
