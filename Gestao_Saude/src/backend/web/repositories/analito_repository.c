#include "analito_repository.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>
#include <string.h>

int analito_criar(const char *codigo, const char *nome, const char *unidade,
                  double ref_min, double ref_max, const char *metodo)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int ok = 0;
    /* Insere apenas se nao houver outro analito ATIVO com o mesmo codigo. */
    const char *sql =
        "INSERT INTO analitos (codigo, nome, unidade, valor_ref_min, valor_ref_max, metodo, ativo) "
        "SELECT ?, ?, ?, ?, ?, ?, 1 "
        "WHERE NOT EXISTS (SELECT 1 FROM analitos WHERE codigo = ? AND ativo = 1);";

    if (codigo == NULL || codigo[0] == '\0' || nome == NULL || nome[0] == '\0' ||
        db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, codigo, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, nome, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, unidade != NULL ? unidade : "", -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 4, ref_min);
        sqlite3_bind_double(stmt, 5, ref_max);
        sqlite3_bind_text(stmt, 6, metodo != NULL ? metodo : "", -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 7, codigo, -1, SQLITE_STATIC);
        ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;
        sqlite3_finalize(stmt);
    }

    db_fechar(db);
    return ok ? 1 : 0;
}

int analito_listar_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, codigo, nome, unidade, valor_ref_min, valor_ref_max, metodo "
        "FROM analitos WHERE ativo = 1 ORDER BY codigo;";
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
        char codigoJson[64];
        char nomeJson[128];
        char unidadeJson[48];
        char metodoJson[96];
        char objeto[512];
        int id = sqlite3_column_int(stmt, 0);
        double refMin = sqlite3_column_double(stmt, 4);
        double refMax = sqlite3_column_double(stmt, 5);
        int escrito;

        if (repo_json_escapar(codigoJson, sizeof(codigoJson), (const char *)sqlite3_column_text(stmt, 1)) == 0 ||
            repo_json_escapar(nomeJson, sizeof(nomeJson), (const char *)sqlite3_column_text(stmt, 2)) == 0 ||
            repo_json_escapar(unidadeJson, sizeof(unidadeJson), (const char *)sqlite3_column_text(stmt, 3)) == 0 ||
            repo_json_escapar(metodoJson, sizeof(metodoJson), (const char *)sqlite3_column_text(stmt, 6)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"id\":%d,\"codigo\":%s,\"nome\":%s,\"unidade\":%s,"
            "\"refMin\":%g,\"refMax\":%g,\"metodo\":%s}",
            primeiro ? "" : ",",
            id, codigoJson, nomeJson, unidadeJson, refMin, refMax, metodoJson);

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

int analito_desativar(int id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int ok = 0;

    if (id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    if (sqlite3_prepare_v2(db,
            "UPDATE analitos SET ativo = 0 WHERE id = ? AND ativo = 1;",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, id);
        ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;
        sqlite3_finalize(stmt);
    }

    /* Tira o analito desativado de todos os paineis que o referenciam. */
    if (ok && sqlite3_prepare_v2(db,
            "DELETE FROM painel_analitos WHERE analito_id = ?;",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, id);
        ok = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
    }

    sqlite3_exec(db, ok ? "COMMIT;" : "ROLLBACK;", NULL, NULL, NULL);
    db_fechar(db);
    return ok ? 1 : 0;
}

int analito_contar_ativos(void)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int total = -1;

    if (db_abrir(&db) == 0)
    {
        return -1;
    }

    if (sqlite3_prepare_v2(db,
            "SELECT COUNT(*) FROM analitos WHERE ativo = 1;",
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

int painel_adicionar_analito(int tipo_exame, int analito_id, int ordem)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int ok = 0;
    /* So vincula analito ATIVO e ainda nao presente nesse painel. */
    const char *sql =
        "INSERT INTO painel_analitos (tipo_exame, analito_id, ordem) "
        "SELECT ?, ?, ? "
        "WHERE EXISTS (SELECT 1 FROM analitos WHERE id = ? AND ativo = 1) "
        "AND NOT EXISTS (SELECT 1 FROM painel_analitos WHERE tipo_exame = ? AND analito_id = ?);";

    if (tipo_exame <= 0 || analito_id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, tipo_exame);
        sqlite3_bind_int(stmt, 2, analito_id);
        sqlite3_bind_int(stmt, 3, ordem);
        sqlite3_bind_int(stmt, 4, analito_id);
        sqlite3_bind_int(stmt, 5, tipo_exame);
        sqlite3_bind_int(stmt, 6, analito_id);
        ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;
        sqlite3_finalize(stmt);
    }

    db_fechar(db);
    return ok ? 1 : 0;
}

int painel_remover_analito(int tipo_exame, int analito_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int ok = 0;

    if (tipo_exame <= 0 || analito_id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db,
            "DELETE FROM painel_analitos WHERE tipo_exame = ? AND analito_id = ?;",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, tipo_exame);
        sqlite3_bind_int(stmt, 2, analito_id);
        ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;
        sqlite3_finalize(stmt);
    }

    db_fechar(db);
    return ok ? 1 : 0;
}

int painel_listar_json(int tipo_exame, char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT a.id, a.codigo, a.nome, a.unidade, a.valor_ref_min, a.valor_ref_max, "
        "a.metodo, pa.ordem "
        "FROM painel_analitos pa "
        "JOIN analitos a ON a.id = pa.analito_id "
        "WHERE pa.tipo_exame = ? AND a.ativo = 1 "
        "ORDER BY pa.ordem, a.codigo;";
    int usado = 0;
    int primeiro = 1;

    if (buffer == NULL || tamanho <= 0 || tipo_exame <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, tipo_exame);

    buffer[0] = '\0';
    if (repo_json_anexar(buffer, tamanho, &usado, "[") == 0)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char codigoJson[64];
        char nomeJson[128];
        char unidadeJson[48];
        char metodoJson[96];
        char objeto[512];
        int id = sqlite3_column_int(stmt, 0);
        double refMin = sqlite3_column_double(stmt, 4);
        double refMax = sqlite3_column_double(stmt, 5);
        int ordem = sqlite3_column_int(stmt, 7);
        int escrito;

        if (repo_json_escapar(codigoJson, sizeof(codigoJson), (const char *)sqlite3_column_text(stmt, 1)) == 0 ||
            repo_json_escapar(nomeJson, sizeof(nomeJson), (const char *)sqlite3_column_text(stmt, 2)) == 0 ||
            repo_json_escapar(unidadeJson, sizeof(unidadeJson), (const char *)sqlite3_column_text(stmt, 3)) == 0 ||
            repo_json_escapar(metodoJson, sizeof(metodoJson), (const char *)sqlite3_column_text(stmt, 6)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"id\":%d,\"codigo\":%s,\"nome\":%s,\"unidade\":%s,"
            "\"refMin\":%g,\"refMax\":%g,\"metodo\":%s,\"ordem\":%d}",
            primeiro ? "" : ",",
            id, codigoJson, nomeJson, unidadeJson, refMin, refMax, metodoJson, ordem);

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
