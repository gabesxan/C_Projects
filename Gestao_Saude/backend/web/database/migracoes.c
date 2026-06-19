/*
 * Migracoes de schema do SIGEH-DF.
 *
 * A versao corrente do banco fica em 'PRAGMA user_version'. Um banco recem
 * criado pelo schema (schema_v3.sql) ja nasce na ultima versao. Bancos mais
 * antigos sao atualizados aplicando, em ordem, apenas as migracoes com versao
 * maior que a atual — sem perder dados. Cada migracao roda numa transacao e so
 * avanca a versao se concluir.
 *
 * Para evoluir o schema: adicione um item ao final de MIGRACOES com a proxima
 * versao e o SQL idempotente do delta, e atualize 'PRAGMA user_version' no fim
 * de schema_v3.sql para o mesmo numero (LATEST_VERSION).
 */

#include "database.h"

#include <sqlite3.h>
#include <stdio.h>

#define LATEST_VERSION 7

typedef struct
{
    int versao;
    const char *descricao;
    const char *sql; /* NULL = marco de baseline (criado pelo schema). */
} Migracao;

static const Migracao MIGRACOES[] = {
    {1, "baseline (schema_v3)", NULL},
    {2, "indices de cobrancas e expiracao de sessoes",
     "CREATE INDEX IF NOT EXISTS idx_cobrancas_paciente ON cobrancas(paciente_id);"
     "CREATE INDEX IF NOT EXISTS idx_sessoes_expira ON sessoes(expira_em);"},
    {3, "fila de recepcao: rechamadas e motivo de cancelamento",
     "ALTER TABLE checkins ADD COLUMN rechamadas INTEGER NOT NULL DEFAULT 0;"
     "ALTER TABLE checkins ADD COLUMN motivo TEXT NOT NULL DEFAULT '';"},
    {4, "faturamento em lote: tabela lotes e vinculo da cobranca",
     "CREATE TABLE IF NOT EXISTS lotes ("
     "  id INTEGER PRIMARY KEY,"
     "  convenio_id INTEGER NOT NULL,"
     "  status TEXT NOT NULL DEFAULT 'ABERTO',"
     "  criado_em TEXT NOT NULL DEFAULT (datetime('now')),"
     "  fechado_em TEXT NOT NULL DEFAULT '');"
     "ALTER TABLE cobrancas ADD COLUMN lote_id INTEGER NOT NULL DEFAULT 0;"
     "CREATE INDEX IF NOT EXISTS idx_cobrancas_lote ON cobrancas(lote_id);"},
    {5, "financeiro: cobertura do convenio, vencimento, guia e coparticipacao",
     "ALTER TABLE convenios ADD COLUMN cobertura_pct INTEGER NOT NULL DEFAULT 100;"
     "ALTER TABLE cobrancas ADD COLUMN vencimento TEXT NOT NULL DEFAULT '';"
     "ALTER TABLE cobrancas ADD COLUMN guia TEXT NOT NULL DEFAULT '';"
     "ALTER TABLE cobrancas ADD COLUMN guia_validade TEXT NOT NULL DEFAULT '';"
     "ALTER TABLE cobrancas ADD COLUMN coberto_centavos INTEGER NOT NULL DEFAULT 0;"
     "ALTER TABLE cobrancas ADD COLUMN copart_centavos INTEGER NOT NULL DEFAULT 0;"},
    {6, "laboratorio: catalogo de analitos e composicao dos paineis",
     "CREATE TABLE IF NOT EXISTS analitos ("
     "  id INTEGER PRIMARY KEY,"
     "  codigo TEXT NOT NULL,"
     "  nome TEXT NOT NULL,"
     "  unidade TEXT NOT NULL DEFAULT '',"
     "  valor_ref_min REAL NOT NULL DEFAULT 0,"
     "  valor_ref_max REAL NOT NULL DEFAULT 0,"
     "  metodo TEXT NOT NULL DEFAULT '',"
     "  ativo INTEGER NOT NULL DEFAULT 1);"
     "CREATE TABLE IF NOT EXISTS painel_analitos ("
     "  id INTEGER PRIMARY KEY,"
     "  tipo_exame INTEGER NOT NULL,"
     "  analito_id INTEGER NOT NULL,"
     "  ordem INTEGER NOT NULL DEFAULT 0,"
     "  FOREIGN KEY (analito_id) REFERENCES analitos(id));"
     "CREATE INDEX IF NOT EXISTS idx_painel_tipo ON painel_analitos(tipo_exame);"
     "CREATE INDEX IF NOT EXISTS idx_painel_analito ON painel_analitos(analito_id);"
     "CREATE UNIQUE INDEX IF NOT EXISTS idx_analitos_codigo_ativo ON analitos(codigo) WHERE ativo = 1;"},
    {7, "laboratorio: resultados estruturados por analito",
     "CREATE TABLE IF NOT EXISTS exame_resultados_analitos ("
     "  id INTEGER PRIMARY KEY,"
     "  exame_id INTEGER NOT NULL,"
     "  analito_id INTEGER NOT NULL,"
     "  valor_numerico REAL NOT NULL,"
     "  valor_texto TEXT NOT NULL DEFAULT '',"
     "  fora_referencia INTEGER NOT NULL DEFAULT 0,"
     "  observacao TEXT NOT NULL DEFAULT '',"
     "  FOREIGN KEY (exame_id) REFERENCES exames(id),"
     "  FOREIGN KEY (analito_id) REFERENCES analitos(id));"
     "CREATE INDEX IF NOT EXISTS idx_exame_resultados_exame ON exame_resultados_analitos(exame_id);"
     "CREATE UNIQUE INDEX IF NOT EXISTS idx_exame_resultados_exame_analito "
     "ON exame_resultados_analitos(exame_id, analito_id);"},
};

/* Le a versao atual do schema (PRAGMA user_version). */
static int versao_atual(sqlite3 *db)
{
    sqlite3_stmt *stmt = NULL;
    int versao = 0;

    if (sqlite3_prepare_v2(db, "PRAGMA user_version;", -1, &stmt, NULL) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            versao = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    return versao;
}

/* 1 se o schema base ja existe (banco semeado); 0 num arquivo vazio/novo. */
static int baseline_existe(sqlite3 *db)
{
    sqlite3_stmt *stmt = NULL;
    int existe = 0;

    if (sqlite3_prepare_v2(db,
            "SELECT 1 FROM sqlite_master WHERE type='table' AND name='usuarios';",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        existe = sqlite3_step(stmt) == SQLITE_ROW;
        sqlite3_finalize(stmt);
    }

    return existe;
}

int db_migrar(void)
{
    sqlite3 *db = NULL;
    int atual;
    int i;
    int total = (int)(sizeof(MIGRACOES) / sizeof(MIGRACOES[0]));

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    /* Banco ainda nao semeado: nada a migrar (o schema cria a base). */
    if (baseline_existe(db) == 0)
    {
        db_fechar(db);
        return 1;
    }

    atual = versao_atual(db);

    for (i = 0; i < total; i++)
    {
        const Migracao *m = &MIGRACOES[i];
        char pragma[64];

        if (m->versao <= atual)
        {
            continue;
        }

        if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
        {
            db_fechar(db);
            return 0;
        }

        if (m->sql != NULL &&
            sqlite3_exec(db, m->sql, NULL, NULL, NULL) != SQLITE_OK)
        {
            sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
            db_fechar(db);
            return 0;
        }

        /* user_version nao aceita bind; o numero vem do array (confiavel). */
        snprintf(pragma, sizeof(pragma), "PRAGMA user_version = %d;", m->versao);
        if (sqlite3_exec(db, pragma, NULL, NULL, NULL) != SQLITE_OK)
        {
            sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
            db_fechar(db);
            return 0;
        }

        sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
        atual = m->versao;
    }

    db_fechar(db);
    return 1;
}
