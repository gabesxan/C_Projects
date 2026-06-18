#include "database.h"

#include <sqlite3.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *BANCO_TESTE = "build/test_sigeh_repository.db";
static const char *SCHEMA = "../data/schema_v3.sql";

/* Le PRAGMA user_version do banco de teste. */
static int versao(void)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int v = -1;

    if (db_abrir(&db) == 0) return -1;
    if (sqlite3_prepare_v2(db, "PRAGMA user_version;", -1, &stmt, NULL) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW) v = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
    }
    db_fechar(db);
    return v;
}

/* 1 se existe um indice com o nome dado. */
static int indice_existe(const char *nome)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int existe = 0;

    if (db_abrir(&db) == 0) return 0;
    if (sqlite3_prepare_v2(db,
            "SELECT 1 FROM sqlite_master WHERE type='index' AND name=?;",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, nome, -1, SQLITE_STATIC);
        existe = sqlite3_step(stmt) == SQLITE_ROW;
        sqlite3_finalize(stmt);
    }
    db_fechar(db);
    return existe;
}

/* Conta linhas de uma consulta COUNT(*). */
static int contar(const char *sql)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int n = -1;

    if (db_abrir(&db) == 0) return -1;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW) n = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
    }
    db_fechar(db);
    return n;
}

int main(void)
{
    assert(db_definir_caminho(BANCO_TESTE) == 1);

    /* Banco vazio (sem schema base): db_migrar e um no-op seguro. */
    assert(db_executar("DROP TABLE IF EXISTS usuarios;") == 1);
    assert(db_migrar() == 1);

    /* Schema completo: nasce ja na ultima versao, com os indices novos. */
    assert(db_resetar_com_schema(SCHEMA) == 1);
    assert(versao() == 2);
    assert(indice_existe("idx_cobrancas_paciente") == 1);
    assert(indice_existe("idx_sessoes_expira") == 1);

    /* Simula um banco ANTIGO: volta para a v1 e remove os indices da v2. */
    assert(db_executar("DROP INDEX IF EXISTS idx_cobrancas_paciente;") == 1);
    assert(db_executar("DROP INDEX IF EXISTS idx_sessoes_expira;") == 1);
    assert(db_executar("PRAGMA user_version = 1;") == 1);
    /* Dado preexistente para provar que a migracao preserva os dados. */
    assert(db_executar("INSERT INTO convenios (nome, ativo) VALUES ('Antigo', 1);") == 1);
    assert(indice_existe("idx_cobrancas_paciente") == 0);
    assert(versao() == 1);

    /* Migra: cria os indices, sobe para a v2 e mantem os dados. */
    assert(db_migrar() == 1);
    assert(versao() == 2);
    assert(indice_existe("idx_cobrancas_paciente") == 1);
    assert(indice_existe("idx_sessoes_expira") == 1);
    assert(contar("SELECT COUNT(*) FROM convenios WHERE nome='Antigo';") == 1);

    /* Idempotente: rodar de novo num banco ja atualizado nao muda nada. */
    assert(db_migrar() == 1);
    assert(versao() == 2);
    assert(contar("SELECT COUNT(*) FROM convenios WHERE nome='Antigo';") == 1);

    printf("test_migracoes: OK\n");
    return 0;
}
