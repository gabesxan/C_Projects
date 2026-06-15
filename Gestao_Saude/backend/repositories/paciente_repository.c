#include "paciente_repository.h"
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

int paciente_repo_criar(const char *nome,
                        const char *cpf,
                        int idade,
                        const char *telefone,
                        const char *sexo,
                        int regiao_administrativa)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO pacientes "
        "(nome, cpf, idade, telefone, sexo, regiao_administrativa, ativo) "
        "VALUES (?, ?, ?, ?, ?, ?, 1);";

    if (nome == NULL || nome[0] == '\0')
    {
        return 0;
    }

    if (idade < 0)
    {
        return 0;
    }

    if (cpf == NULL || telefone == NULL || sexo == NULL)
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
    sqlite3_bind_text(stmt, 2, cpf, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, idade);
    sqlite3_bind_text(stmt, 4, telefone, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, sexo, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, regiao_administrativa);

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

int paciente_repo_listar_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, nome, cpf, idade, telefone, sexo, regiao_administrativa "
        "FROM pacientes WHERE ativo = 1 ORDER BY id;";
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
        char cpfJson[64];
        char telefoneJson[64];
        char sexoJson[16];
        char objeto[700];
        int id = sqlite3_column_int(stmt, 0);
        const char *nome = (const char *)sqlite3_column_text(stmt, 1);
        const char *cpf = (const char *)sqlite3_column_text(stmt, 2);
        int idade = sqlite3_column_int(stmt, 3);
        const char *telefone = (const char *)sqlite3_column_text(stmt, 4);
        const char *sexo = (const char *)sqlite3_column_text(stmt, 5);
        int regiao = sqlite3_column_int(stmt, 6);
        int escrito;

        if (escaparJson(nomeJson, sizeof(nomeJson), nome) == 0 ||
            escaparJson(cpfJson, sizeof(cpfJson), cpf) == 0 ||
            escaparJson(telefoneJson, sizeof(telefoneJson), telefone) == 0 ||
            escaparJson(sexoJson, sizeof(sexoJson), sexo) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"id\":%d,\"nome\":%s,\"cpf\":%s,\"idade\":%d,"
            "\"telefone\":%s,\"sexo\":%s,\"regiaoAdministrativa\":%d}",
            primeiro ? "" : ",",
            id, nomeJson, cpfJson, idade, telefoneJson, sexoJson, regiao);

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

int paciente_repo_desativar(int id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE pacientes SET ativo = 0 WHERE id = ? AND ativo = 1;";
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

int paciente_repo_regiao(int id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT regiao_administrativa FROM pacientes "
        "WHERE id = ? AND ativo = 1;";
    int regiao = -1;

    if (id <= 0)
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

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        regiao = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return regiao;
}

int paciente_repo_contar_ativos(void)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "SELECT COUNT(*) FROM pacientes WHERE ativo = 1;";
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
