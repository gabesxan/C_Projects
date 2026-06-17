#include "triagem_repository.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>
#include <string.h>

int triagem_repo_criar_completa(int paciente_id, int tipo_triagem, int pontuacao,
                                const char *classificacao, const char *queixa,
                                const char *pressao, const char *temperatura,
                                const char *freq_cardiaca, const char *saturacao)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO triagens "
        "(paciente_id, tipo_triagem, pontuacao, classificacao, queixa, "
        "pressao, temperatura, freq_cardiaca, saturacao, ativo) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, 1);";

    if (paciente_id <= 0 || tipo_triagem <= 0)
    {
        return 0;
    }

    if (classificacao == NULL || classificacao[0] == '\0')
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
    sqlite3_bind_int(stmt, 2, tipo_triagem);
    sqlite3_bind_int(stmt, 3, pontuacao);
    sqlite3_bind_text(stmt, 4, classificacao, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, queixa != NULL ? queixa : "", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, pressao != NULL ? pressao : "", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, temperatura != NULL ? temperatura : "", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 8, freq_cardiaca != NULL ? freq_cardiaca : "", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 9, saturacao != NULL ? saturacao : "", -1, SQLITE_STATIC);

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

/* Versao simples (sem sinais vitais): mantida para compatibilidade. */
int triagem_repo_criar(int paciente_id, int tipo_triagem, int pontuacao,
                       const char *classificacao)
{
    return triagem_repo_criar_completa(paciente_id, tipo_triagem, pontuacao,
                                       classificacao, "", "", "", "", "");
}

int triagem_repo_distribuicao_por_classificacao_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT classificacao, COUNT(*) FROM triagens WHERE ativo = 1 "
        "GROUP BY classificacao ORDER BY classificacao;";
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
        char classificacaoJson[128];
        char objeto[192];
        int total = sqlite3_column_int(stmt, 1);
        int escrito;

        if (repo_json_escapar(classificacaoJson, sizeof(classificacaoJson),
                              (const char *)sqlite3_column_text(stmt, 0)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"classificacao\":%s,\"total\":%d}",
                           primeiro ? "" : ",", classificacaoJson, total);

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

int triagem_repo_listar_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, paciente_id, tipo_triagem, pontuacao, classificacao "
        "FROM triagens WHERE ativo = 1 ORDER BY id;";
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
        char classificacaoJson[128];
        char objeto[384];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int tipo = sqlite3_column_int(stmt, 2);
        int pontuacao = sqlite3_column_int(stmt, 3);
        const char *classificacao = (const char *)sqlite3_column_text(stmt, 4);
        int escrito;

        if (repo_json_escapar(classificacaoJson, sizeof(classificacaoJson),
                              classificacao) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"id\":%d,\"pacienteId\":%d,\"tipoTriagem\":%d,"
                           "\"pontuacao\":%d,\"classificacao\":%s}",
                           primeiro ? "" : ",",
                           id, pacienteId, tipo, pontuacao, classificacaoJson);

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

int triagem_repo_listar_por_tipos_json(const int *tipos, int n,
                                       char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    char placeholders[64];
    char sql[256];
    int usado = 0;
    int primeiro = 1;
    int i;
    int pos = 0;

    if (buffer == NULL || tamanho <= 0 || tipos == NULL)
    {
        return 0;
    }

    /* Sem tipos correspondentes: lista vazia, mas resposta valida. */
    if (n <= 0)
    {
        buffer[0] = '\0';
        return repo_json_anexar(buffer, tamanho, &usado, "[]");
    }

    /* Monta "?,?,..." com n marcadores para o IN, limitando a capacidade. */
    if (n > 32)
    {
        n = 32;
    }
    for (i = 0; i < n; i++)
    {
        int escrito = snprintf(placeholders + pos, sizeof(placeholders) - pos,
                               i == 0 ? "?" : ",?");
        if (escrito < 0 || escrito >= (int)sizeof(placeholders) - pos)
        {
            return 0;
        }
        pos += escrito;
    }

    if (snprintf(sql, sizeof(sql),
                 "SELECT id, paciente_id, tipo_triagem, pontuacao, classificacao "
                 "FROM triagens WHERE ativo = 1 AND tipo_triagem IN (%s) "
                 "ORDER BY id;",
                 placeholders) >= (int)sizeof(sql))
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

    for (i = 0; i < n; i++)
    {
        sqlite3_bind_int(stmt, i + 1, tipos[i]);
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
        char classificacaoJson[128];
        char objeto[384];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int tipo = sqlite3_column_int(stmt, 2);
        int pontuacao = sqlite3_column_int(stmt, 3);
        const char *classificacao = (const char *)sqlite3_column_text(stmt, 4);
        int escrito;

        if (repo_json_escapar(classificacaoJson, sizeof(classificacaoJson),
                              classificacao) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"id\":%d,\"pacienteId\":%d,\"tipoTriagem\":%d,"
                           "\"pontuacao\":%d,\"classificacao\":%s}",
                           primeiro ? "" : ",",
                           id, pacienteId, tipo, pontuacao, classificacaoJson);

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

int triagem_repo_desativar(int id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "UPDATE triagens SET ativo = 0 WHERE id = ? AND ativo = 1;";
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

int triagem_repo_ultima_por_paciente(int paciente_id, int *tipo_triagem,
                                     int *pontuacao, char *classificacao,
                                     int classificacao_tam)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT tipo_triagem, pontuacao, classificacao "
        "FROM triagens WHERE paciente_id = ? AND ativo = 1 "
        "ORDER BY id DESC LIMIT 1;";
    int encontrou = 0;

    if (paciente_id <= 0)
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

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (tipo_triagem != NULL)
        {
            *tipo_triagem = sqlite3_column_int(stmt, 0);
        }

        if (pontuacao != NULL)
        {
            *pontuacao = sqlite3_column_int(stmt, 1);
        }

        if (classificacao != NULL && classificacao_tam > 0)
        {
            const char *texto = (const char *)sqlite3_column_text(stmt, 2);

            if (texto != NULL)
            {
                strncpy(classificacao, texto, (size_t)classificacao_tam - 1);
                classificacao[classificacao_tam - 1] = '\0';
            }
            else
            {
                classificacao[0] = '\0';
            }
        }

        encontrou = 1;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return encontrou;
}

int triagem_repo_contar_por_classificacao(const char *classificacao)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT COUNT(*) FROM triagens "
        "WHERE ativo = 1 AND classificacao = ?;";
    int total = -1;

    if (classificacao == NULL)
    {
        return -1;
    }

    if (db_abrir(&db) == 0)
    {
        return -1;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return -1;
    }

    sqlite3_bind_text(stmt, 1, classificacao, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        total = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return total;
}

int triagem_repo_contar_ativos(void)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "SELECT COUNT(*) FROM triagens WHERE ativo = 1;";
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
