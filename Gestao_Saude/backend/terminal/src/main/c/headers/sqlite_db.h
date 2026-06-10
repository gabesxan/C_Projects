#ifndef SQLITE_DB_H
#define SQLITE_DB_H

#include "hospital.h"
#include <sqlite3.h>

const char *obterCaminhoBancoSQLite(void);
int abrirBancoSQLite(sqlite3 **db);
void fecharBancoSQLite(sqlite3 *db);
int executarSQLSQLite(const char *sql);

#endif
