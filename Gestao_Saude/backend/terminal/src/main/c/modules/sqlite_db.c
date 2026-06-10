#include "sqlite_db.h"

const char *obterCaminhoBancoSQLite(void)
{
    return "../data/sigeh_v2.db";
}

int abrirBancoSQLite(sqlite3 **db)
{
    if (db == NULL)
    {
        return 0;
    }

    if (sqlite3_open(obterCaminhoBancoSQLite(), db) != SQLITE_OK)
    {
        if (*db != NULL)
        {
            sqlite3_close(*db);
            *db = NULL;
        }

        return 0;
    }

    return 1;
}

void fecharBancoSQLite(sqlite3 *db)
{
    if (db != NULL)
    {
        sqlite3_close(db);
    }
}

int executarSQLSQLite(const char *sql)
{
    sqlite3 *db = NULL;
    char *mensagemErro = NULL;
    int resultado;

    if (sql == NULL)
    {
        return 0;
    }

    if (abrirBancoSQLite(&db) == 0)
    {
        return 0;
    }

    resultado = sqlite3_exec(db, sql, NULL, NULL, &mensagemErro);

    if (mensagemErro != NULL)
    {
        sqlite3_free(mensagemErro);
    }

    fecharBancoSQLite(db);

    return resultado == SQLITE_OK;
}
