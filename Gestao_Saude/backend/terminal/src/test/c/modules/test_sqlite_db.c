#include <assert.h>
#include <string.h>
#include "sqlite_db.h"

int main(void)
{
    sqlite3 *db = NULL;

    assert(definirCaminhoBancoSQLite("/tmp/sigeh_test_sqlite_db.db") == 1);
    assert(strcmp(obterCaminhoBancoSQLite(), "/tmp/sigeh_test_sqlite_db.db") == 0);
    assert(reinicializarBancoSQLite() == 1);
    assert(abrirBancoSQLite(&db) == 1);
    assert(db != NULL);

    fecharBancoSQLite(db);

    assert(abrirBancoSQLite(NULL) == 0);

    return 0;
}
