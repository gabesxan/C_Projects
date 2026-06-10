#include <assert.h>
#include <string.h>
#include "sqlite_db.h"

int main(void)
{
    sqlite3 *db = NULL;

    assert(strcmp(obterCaminhoBancoSQLite(), "../data/sigeh_v2.db") == 0);
    assert(abrirBancoSQLite(&db) == 1);
    assert(db != NULL);

    fecharBancoSQLite(db);

    assert(abrirBancoSQLite(NULL) == 0);

    return 0;
}
