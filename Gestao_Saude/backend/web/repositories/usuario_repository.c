#include "usuario_repository.h"
#include "senha_util.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>
#include <string.h>

static int papel_valido(const char *papel)
{
    return strcmp(papel, "ADMIN") == 0 ||
           strcmp(papel, "CADASTRO") == 0 ||
           strcmp(papel, "MEDICO") == 0 ||
           strcmp(papel, "ENFERMAGEM") == 0 ||
           strcmp(papel, "PACIENTE") == 0;
}

int usuario_repo_criar(const char *nome, const char *login, const char *senha,
                       const char *papel, int paciente_id, int medico_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO usuarios "
        "(nome, login, senha_hash, salt, papel, paciente_id, medico_id, ativo) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, 1);";
    char salt[17];
    char hash[65];

    if (nome == NULL || nome[0] == '\0' ||
        login == NULL || login[0] == '\0' ||
        senha == NULL || senha[0] == '\0' ||
        papel == NULL || papel_valido(papel) == 0)
    {
        return 0;
    }

    if (senha_gerar_salt(salt, sizeof(salt)) == 0)
    {
        return 0;
    }

    if (senha_hash(salt, senha, hash, sizeof(hash)) == 0)
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
    sqlite3_bind_text(stmt, 2, login, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, hash, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, salt, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, papel, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, paciente_id);
    sqlite3_bind_int(stmt, 7, medico_id);

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

int usuario_repo_autenticar(const char *login, const char *senha,
                            char *papel, int papel_tam,
                            int *paciente_id, int *medico_id,
                            int *usuario_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT senha_hash, salt, papel, paciente_id, medico_id, id "
        "FROM usuarios WHERE login = ? AND ativo = 1;";
    int autenticado = 0;

    if (login == NULL || senha == NULL)
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

    sqlite3_bind_text(stmt, 1, login, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *hashArmazenado = (const char *)sqlite3_column_text(stmt, 0);
        const char *salt = (const char *)sqlite3_column_text(stmt, 1);
        const char *papelArmazenado = (const char *)sqlite3_column_text(stmt, 2);
        char hashCalculado[65];

        if (salt != NULL && hashArmazenado != NULL &&
            senha_hash(salt, senha, hashCalculado, sizeof(hashCalculado)) == 1 &&
            strcmp(hashCalculado, hashArmazenado) == 0)
        {
            if (papel != NULL && papel_tam > 0 && papelArmazenado != NULL)
            {
                strncpy(papel, papelArmazenado, (size_t)papel_tam - 1);
                papel[papel_tam - 1] = '\0';
            }

            if (paciente_id != NULL)
            {
                *paciente_id = sqlite3_column_int(stmt, 3);
            }

            if (medico_id != NULL)
            {
                *medico_id = sqlite3_column_int(stmt, 4);
            }

            if (usuario_id != NULL)
            {
                *usuario_id = sqlite3_column_int(stmt, 5);
            }

            autenticado = 1;
        }
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return autenticado;
}

int usuario_repo_listar_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, nome, login, papel, paciente_id, medico_id, ativo, criado_em "
        "FROM usuarios ORDER BY id;";
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
        char nomeJson[128];
        char loginJson[128];
        char papelJson[32];
        char criadoJson[64];
        char objeto[512];
        int id = sqlite3_column_int(stmt, 0);
        const char *nome = (const char *)sqlite3_column_text(stmt, 1);
        const char *login = (const char *)sqlite3_column_text(stmt, 2);
        const char *papel = (const char *)sqlite3_column_text(stmt, 3);
        int pacienteId = sqlite3_column_int(stmt, 4);
        int medicoId = sqlite3_column_int(stmt, 5);
        int ativo = sqlite3_column_int(stmt, 6);
        const char *criado = (const char *)sqlite3_column_text(stmt, 7);
        int escrito;

        if (repo_json_escapar(nomeJson, sizeof(nomeJson), nome) == 0 ||
            repo_json_escapar(loginJson, sizeof(loginJson), login) == 0 ||
            repo_json_escapar(papelJson, sizeof(papelJson), papel) == 0 ||
            repo_json_escapar(criadoJson, sizeof(criadoJson), criado) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"id\":%d,\"nome\":%s,\"login\":%s,\"papel\":%s,"
            "\"pacienteId\":%d,\"medicoId\":%d,\"ativo\":%d,\"criadoEm\":%s}",
            primeiro ? "" : ",",
            id, nomeJson, loginJson, papelJson, pacienteId, medicoId,
            ativo, criadoJson);

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

int usuario_repo_desativar(int id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "UPDATE usuarios SET ativo = 0 WHERE id = ? AND ativo = 1;";
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

int usuario_repo_reativar(int id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "UPDATE usuarios SET ativo = 1 WHERE id = ? AND ativo = 0;";
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

int usuario_repo_contar_ativos(void)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "SELECT COUNT(*) FROM usuarios WHERE ativo = 1;";
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
