#include "medico_repository.h"
#include "database.h"

#include <stdio.h>
#include <string.h>

/* Escreve em 'destino' a string 'origem' como literal JSON entre aspas,
 * escapando aspas e barras. Retorna 1 se coube, 0 se faltou espaco. */
static int escaparJson(char *destino, int tamanho, const char *origem)
{
    int pos = 0;
    int i;

    if (tamanho < 3)
    {
        return 0;
    }

    destino[pos++] = '"';

    for (i = 0; origem != NULL && origem[i] != '\0'; i++)
    {
        char c = origem[i];

        if (c == '"' || c == '\\')
        {
            if (pos + 2 > tamanho - 2)
            {
                return 0;
            }
            destino[pos++] = '\\';
            destino[pos++] = c;
        }
        else
        {
            if (pos + 1 > tamanho - 2)
            {
                return 0;
            }
            destino[pos++] = c;
        }
    }

    destino[pos++] = '"';
    destino[pos] = '\0';
    return 1;
}

/* Concatena 'texto' ao final de 'buffer', controlando o espaco usado.
 * Retorna 1 se coube, 0 caso contrario. */
static int anexarTexto(char *buffer, int tamanho, int *usado, const char *texto)
{
    int restante = tamanho - *usado;
    int escrito = snprintf(buffer + *usado, (size_t)restante, "%s", texto);

    if (escrito < 0 || escrito >= restante)
    {
        return 0;
    }

    *usado += escrito;
    return 1;
}

int medico_repo_criar(const char *nome,
                      const char *crm,
                      const char *especialidade,
                      int regiao_administrativa)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO medicos "
        "(nome, crm, especialidade, regiao_administrativa, ativo) "
        "VALUES (?, ?, ?, ?, 1);";

    if (nome == NULL || nome[0] == '\0')
    {
        return 0;
    }

    if (especialidade == NULL || especialidade[0] == '\0')
    {
        return 0;
    }

    if (crm == NULL)
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

    sqlite3_bind_text(stmt, 1, nome, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, crm, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, especialidade, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, regiao_administrativa);

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

int medico_repo_listar_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, nome, crm, especialidade, regiao_administrativa "
        "FROM medicos WHERE ativo = 1 ORDER BY id;";
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

    if (anexarTexto(buffer, tamanho, &usado, "[") == 0)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char nomeJson[256];
        char crmJson[64];
        char especialidadeJson[128];
        char objeto[700];
        int id = sqlite3_column_int(stmt, 0);
        const char *nome = (const char *)sqlite3_column_text(stmt, 1);
        const char *crm = (const char *)sqlite3_column_text(stmt, 2);
        const char *especialidade = (const char *)sqlite3_column_text(stmt, 3);
        int regiao = sqlite3_column_int(stmt, 4);
        int escrito;

        if (escaparJson(nomeJson, sizeof(nomeJson), nome) == 0 ||
            escaparJson(crmJson, sizeof(crmJson), crm) == 0 ||
            escaparJson(especialidadeJson, sizeof(especialidadeJson), especialidade) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"id\":%d,\"nome\":%s,\"crm\":%s,\"especialidade\":%s,"
            "\"regiaoAdministrativa\":%d}",
            primeiro ? "" : ",",
            id, nomeJson, crmJson, especialidadeJson, regiao);

        if (escrito < 0 || escrito >= (int)sizeof(objeto))
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        if (anexarTexto(buffer, tamanho, &usado, objeto) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        primeiro = 0;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);

    if (anexarTexto(buffer, tamanho, &usado, "]") == 0)
    {
        return 0;
    }

    return 1;
}

int medico_repo_listar_por_especialidade_regiao_json(const char *especialidade,
                                                     int regiao,
                                                     char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, nome, crm, especialidade, regiao_administrativa "
        "FROM medicos "
        "WHERE ativo = 1 AND especialidade = ? AND regiao_administrativa = ? "
        "ORDER BY id;";
    int usado = 0;
    int primeiro = 1;

    if (buffer == NULL || tamanho <= 0 || especialidade == NULL)
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

    sqlite3_bind_text(stmt, 1, especialidade, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, regiao);

    buffer[0] = '\0';

    if (anexarTexto(buffer, tamanho, &usado, "[") == 0)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char nomeJson[256];
        char crmJson[64];
        char especialidadeJson[128];
        char objeto[700];
        int id = sqlite3_column_int(stmt, 0);
        const char *nome = (const char *)sqlite3_column_text(stmt, 1);
        const char *crm = (const char *)sqlite3_column_text(stmt, 2);
        const char *esp = (const char *)sqlite3_column_text(stmt, 3);
        int regiaoMedico = sqlite3_column_int(stmt, 4);
        int escrito;

        if (escaparJson(nomeJson, sizeof(nomeJson), nome) == 0 ||
            escaparJson(crmJson, sizeof(crmJson), crm) == 0 ||
            escaparJson(especialidadeJson, sizeof(especialidadeJson), esp) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"id\":%d,\"nome\":%s,\"crm\":%s,\"especialidade\":%s,"
            "\"regiaoAdministrativa\":%d}",
            primeiro ? "" : ",",
            id, nomeJson, crmJson, especialidadeJson, regiaoMedico);

        if (escrito < 0 || escrito >= (int)sizeof(objeto))
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        if (anexarTexto(buffer, tamanho, &usado, objeto) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        primeiro = 0;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);

    if (anexarTexto(buffer, tamanho, &usado, "]") == 0)
    {
        return 0;
    }

    return 1;
}

int medico_repo_ids_por_especialidade_regiao(const char *especialidade,
                                             int regiao, int *ids, int max)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id FROM medicos "
        "WHERE ativo = 1 AND especialidade = ? AND regiao_administrativa = ? "
        "ORDER BY id;";
    int total = 0;

    if (ids == NULL || max <= 0 || especialidade == NULL)
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

    sqlite3_bind_text(stmt, 1, especialidade, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, regiao);

    while (sqlite3_step(stmt) == SQLITE_ROW && total < max)
    {
        ids[total] = sqlite3_column_int(stmt, 0);
        total++;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return total;
}

int medico_repo_distribuicao_por_especialidade_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT especialidade, COUNT(*) FROM medicos "
        "WHERE ativo = 1 GROUP BY especialidade ORDER BY especialidade;";
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

    if (anexarTexto(buffer, tamanho, &usado, "[") == 0)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char especialidadeJson[128];
        char objeto[200];
        const char *especialidade = (const char *)sqlite3_column_text(stmt, 0);
        int total = sqlite3_column_int(stmt, 1);
        int escrito;

        if (escaparJson(especialidadeJson, sizeof(especialidadeJson),
                        especialidade) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"especialidade\":%s,\"total\":%d}",
            primeiro ? "" : ",", especialidadeJson, total);

        if (escrito < 0 || escrito >= (int)sizeof(objeto))
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        if (anexarTexto(buffer, tamanho, &usado, objeto) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        primeiro = 0;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);

    if (anexarTexto(buffer, tamanho, &usado, "]") == 0)
    {
        return 0;
    }

    return 1;
}

int medico_repo_desativar(int id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE medicos SET ativo = 0 WHERE id = ? AND ativo = 1;";
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

int medico_repo_contar_ativos(void)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "SELECT COUNT(*) FROM medicos WHERE ativo = 1;";
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

int medico_repo_ativo(int id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "SELECT 1 FROM medicos WHERE id = ? AND ativo = 1;";
    int ativo = 0;

    if (id <= 0)
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
        ativo = 1;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return ativo;
}

int medico_repo_especialidade(int id, char *destino, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT especialidade FROM medicos WHERE id = ? AND ativo = 1;";
    int encontrado = 0;

    if (destino == NULL || tamanho <= 0 || id <= 0)
    {
        return 0;
    }

    destino[0] = '\0';

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
        const char *especialidade = (const char *)sqlite3_column_text(stmt, 0);
        int escrito = snprintf(destino, (size_t)tamanho, "%s",
                               especialidade != NULL ? especialidade : "");

        if (escrito >= 0 && escrito < tamanho)
        {
            encontrado = 1;
        }
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return encontrado;
}
