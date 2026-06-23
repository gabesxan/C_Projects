#include "auditoria_repository.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

int auditoria_registrar(int usuario_id, const char *usuario_login,
                        const char *acao, const char *entidade,
                        int entidade_id, const char *detalhe)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO auditoria "
        "(usuario_id, usuario_login, acao, entidade, entidade_id, detalhe) "
        "VALUES (?, ?, ?, ?, ?, ?);";

    if (acao == NULL || acao[0] == '\0' || entidade == NULL)
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

    sqlite3_bind_int(stmt, 1, usuario_id);
    sqlite3_bind_text(stmt, 2, usuario_login != NULL ? usuario_login : "",
                      -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, acao, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, entidade, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, entidade_id);
    sqlite3_bind_text(stmt, 6, detalhe != NULL ? detalhe : "",
                      -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return 1;
}

int auditoria_listar_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, usuario_id, usuario_login, acao, entidade, entidade_id, "
        "detalhe, criado_em FROM auditoria ORDER BY id DESC LIMIT 500;";
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
        char loginJson[128];
        char acaoJson[128];
        char entidadeJson[64];
        char detalheJson[1024];
        char criadoJson[64];
        char objeto[1536];
        int id = sqlite3_column_int(stmt, 0);
        int usuarioId = sqlite3_column_int(stmt, 1);
        int entidadeId = sqlite3_column_int(stmt, 5);
        int escrito;

        if (repo_json_escapar(loginJson, sizeof(loginJson),
                              (const char *)sqlite3_column_text(stmt, 2)) == 0 ||
            repo_json_escapar(acaoJson, sizeof(acaoJson),
                              (const char *)sqlite3_column_text(stmt, 3)) == 0 ||
            repo_json_escapar(entidadeJson, sizeof(entidadeJson),
                              (const char *)sqlite3_column_text(stmt, 4)) == 0 ||
            repo_json_escapar(detalheJson, sizeof(detalheJson),
                              (const char *)sqlite3_column_text(stmt, 6)) == 0 ||
            repo_json_escapar(criadoJson, sizeof(criadoJson),
                              (const char *)sqlite3_column_text(stmt, 7)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"id\":%d,\"usuarioId\":%d,\"usuarioLogin\":%s,\"acao\":%s,"
                           "\"entidade\":%s,\"entidadeId\":%d,\"detalhe\":%s,\"criadoEm\":%s}",
                           primeiro ? "" : ",",
                           id, usuarioId, loginJson, acaoJson, entidadeJson, entidadeId,
                           detalheJson, criadoJson);

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

int auditoria_acessos_paciente_json(int paciente_id, char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    /* ?1 (o paciente) e reutilizado em todas as condicoes: as entidades
     * clinicas/administrativas sao ligadas ao paciente pela sua chave proria.
     * Entidades cujo id auditado nao identifica um paciente (vacina, anexo,
     * leito, estoque) ficam de fora deste recorte. */
    const char *sql =
        "SELECT id, usuario_id, usuario_login, acao, entidade, entidade_id, "
        "detalhe, criado_em FROM auditoria WHERE "
        "(entidade = 'paciente' AND entidade_id = ?1) OR "
        "(entidade = 'exame' AND entidade_id IN (SELECT id FROM exames WHERE paciente_id = ?1)) OR "
        "(entidade = 'prontuario' AND entidade_id IN (SELECT id FROM prontuarios WHERE paciente_id = ?1)) OR "
        "(entidade = 'prescricao' AND entidade_id IN (SELECT id FROM prescricoes WHERE paciente_id = ?1)) OR "
        "(entidade = 'triagem' AND entidade_id IN (SELECT id FROM triagens WHERE paciente_id = ?1)) OR "
        "(entidade = 'agendamento' AND entidade_id IN (SELECT id FROM agendamentos WHERE paciente_id = ?1)) OR "
        "(entidade = 'consentimento' AND entidade_id IN (SELECT id FROM consentimentos WHERE paciente_id = ?1)) OR "
        "(entidade = 'cobranca' AND entidade_id IN (SELECT id FROM cobrancas WHERE paciente_id = ?1)) OR "
        "(entidade = 'checkin' AND entidade_id IN (SELECT id FROM checkins WHERE paciente_id = ?1)) OR "
        "(entidade = 'internacao' AND entidade_id IN (SELECT id FROM internacoes WHERE paciente_id = ?1)) OR "
        "(entidade = 'solicitacao_paciente' AND entidade_id IN (SELECT id FROM solicitacoes_paciente WHERE paciente_id = ?1)) "
        "ORDER BY id DESC LIMIT 500;";
    int usado = 0;
    int primeiro = 1;

    if (buffer == NULL || tamanho <= 0 || paciente_id <= 0)
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

    buffer[0] = '\0';

    if (repo_json_anexar(buffer, tamanho, &usado, "[") == 0)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char loginJson[128];
        char acaoJson[128];
        char entidadeJson[64];
        char detalheJson[1024];
        char criadoJson[64];
        char objeto[1536];
        int id = sqlite3_column_int(stmt, 0);
        int usuarioId = sqlite3_column_int(stmt, 1);
        int entidadeId = sqlite3_column_int(stmt, 5);
        int escrito;

        if (repo_json_escapar(loginJson, sizeof(loginJson),
                              (const char *)sqlite3_column_text(stmt, 2)) == 0 ||
            repo_json_escapar(acaoJson, sizeof(acaoJson),
                              (const char *)sqlite3_column_text(stmt, 3)) == 0 ||
            repo_json_escapar(entidadeJson, sizeof(entidadeJson),
                              (const char *)sqlite3_column_text(stmt, 4)) == 0 ||
            repo_json_escapar(detalheJson, sizeof(detalheJson),
                              (const char *)sqlite3_column_text(stmt, 6)) == 0 ||
            repo_json_escapar(criadoJson, sizeof(criadoJson),
                              (const char *)sqlite3_column_text(stmt, 7)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"id\":%d,\"usuarioId\":%d,\"usuarioLogin\":%s,\"acao\":%s,"
                           "\"entidade\":%s,\"entidadeId\":%d,\"detalhe\":%s,\"criadoEm\":%s}",
                           primeiro ? "" : ",",
                           id, usuarioId, loginJson, acaoJson, entidadeJson, entidadeId,
                           detalheJson, criadoJson);

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

int auditoria_contar(void)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "SELECT COUNT(*) FROM auditoria;";
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
