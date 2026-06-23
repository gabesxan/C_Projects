#include "solicitacao_repository.h"
#include "repo_json.h"
#include "database.h"

#include <sqlite3.h>
#include <stdio.h>
#include <string.h>

static int tipo_valido(const char *tipo)
{
    return tipo != NULL &&
           (strcmp(tipo, "AGENDAMENTO") == 0 || strcmp(tipo, "AJUDA") == 0);
}

int solicitacao_repo_criar(int paciente_id, const char *tipo,
                           const char *mensagem, int *id_out)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO solicitacoes_paciente "
        "(paciente_id, tipo, mensagem, status) VALUES (?, ?, ?, 'ABERTA');";
    int ok = 0;

    if (paciente_id <= 0 || tipo_valido(tipo) == 0)
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
    sqlite3_bind_text(stmt, 2, tipo, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, mensagem != NULL ? mensagem : "", -1, SQLITE_STATIC);

    ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);

    if (ok && id_out != NULL)
    {
        *id_out = (int)sqlite3_last_insert_rowid(db);
    }

    db_fechar(db);
    return ok ? 1 : 0;
}

static int listar_json(const char *sql, int paciente_id, char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
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

    if (paciente_id > 0)
    {
        sqlite3_bind_int(stmt, 1, paciente_id);
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
        char tipoJson[32];
        char mensagemJson[512];
        char statusJson[32];
        char criadoJson[64];
        char objeto[1200];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int escrito;

        if (repo_json_escapar(nomeJson, sizeof(nomeJson), (const char *)sqlite3_column_text(stmt, 2)) == 0 ||
            repo_json_escapar(tipoJson, sizeof(tipoJson), (const char *)sqlite3_column_text(stmt, 3)) == 0 ||
            repo_json_escapar(mensagemJson, sizeof(mensagemJson), (const char *)sqlite3_column_text(stmt, 4)) == 0 ||
            repo_json_escapar(statusJson, sizeof(statusJson), (const char *)sqlite3_column_text(stmt, 5)) == 0 ||
            repo_json_escapar(criadoJson, sizeof(criadoJson), (const char *)sqlite3_column_text(stmt, 6)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"id\":%d,\"pacienteId\":%d,\"pacienteNome\":%s,"
            "\"tipo\":%s,\"mensagem\":%s,\"status\":%s,\"criadoEm\":%s}",
            primeiro ? "" : ",",
            id, pacienteId, nomeJson, tipoJson, mensagemJson, statusJson,
            criadoJson);

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

int solicitacao_repo_listar_por_paciente_json(int paciente_id, char *buffer,
                                              int tamanho)
{
    const char *sql =
        "SELECT s.id, s.paciente_id, COALESCE(p.nome, ''), s.tipo, "
        "s.mensagem, s.status, s.criado_em "
        "FROM solicitacoes_paciente s "
        "LEFT JOIN pacientes p ON p.id = s.paciente_id "
        "WHERE s.paciente_id = ? "
        "ORDER BY s.id DESC;";

    if (paciente_id <= 0)
    {
        return 0;
    }

    return listar_json(sql, paciente_id, buffer, tamanho);
}

int solicitacao_repo_listar_json(char *buffer, int tamanho)
{
    const char *sql =
        "SELECT s.id, s.paciente_id, COALESCE(p.nome, ''), s.tipo, "
        "s.mensagem, s.status, s.criado_em "
        "FROM solicitacoes_paciente s "
        "LEFT JOIN pacientes p ON p.id = s.paciente_id "
        "ORDER BY s.id DESC;";

    return listar_json(sql, 0, buffer, tamanho);
}

int solicitacao_repo_contar_abertas(void)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int total = -1;

    if (db_abrir(&db) == 0)
    {
        return -1;
    }

    if (sqlite3_prepare_v2(db,
            "SELECT COUNT(*) FROM solicitacoes_paciente WHERE status = 'ABERTA';",
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
