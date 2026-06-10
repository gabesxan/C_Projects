#ifndef SQLITE_DB_H
#define SQLITE_DB_H

#include "hospital.h"
#include <sqlite3.h>

const char *obterCaminhoBancoSQLite(void);
const char *obterCaminhoSchemaSQLite(void);
int definirCaminhoBancoSQLite(const char *caminho);
int abrirBancoSQLite(sqlite3 **db);
void fecharBancoSQLite(sqlite3 *db);
int executarSQLSQLite(const char *sql);
int reinicializarBancoSQLite(void);

#endif
