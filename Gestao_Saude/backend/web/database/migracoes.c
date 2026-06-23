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

#define LATEST_VERSION 14

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
    {8, "triagem clinica guiada por especialidade e problemas",
     "ALTER TABLE triagens ADD COLUMN profissional_id INTEGER NOT NULL DEFAULT 0;"
     "ALTER TABLE triagens ADD COLUMN especialidade_principal_id INTEGER NOT NULL DEFAULT 0;"
     "ALTER TABLE triagens ADD COLUMN prioridade INTEGER NOT NULL DEFAULT 0;"
     "ALTER TABLE triagens ADD COLUMN observacoes TEXT NOT NULL DEFAULT '';"
     "ALTER TABLE triagens ADD COLUMN status TEXT NOT NULL DEFAULT 'EM_TRIAGEM';"
     "ALTER TABLE triagens ADD COLUMN data_hora TEXT NOT NULL DEFAULT '';"
     "UPDATE triagens SET especialidade_principal_id = tipo_triagem WHERE especialidade_principal_id = 0;"
     "UPDATE triagens SET prioridade = pontuacao WHERE prioridade = 0;"
     "UPDATE triagens SET data_hora = datetime('now') WHERE data_hora = '';"
     "CREATE TABLE IF NOT EXISTS especialidades_clinicas ("
     "  id INTEGER PRIMARY KEY,"
     "  nome TEXT NOT NULL UNIQUE,"
     "  ativo INTEGER NOT NULL DEFAULT 1);"
     "CREATE TABLE IF NOT EXISTS problemas_clinicos ("
     "  id INTEGER PRIMARY KEY,"
     "  especialidade_id INTEGER NOT NULL,"
     "  nome TEXT NOT NULL,"
     "  peso_risco INTEGER NOT NULL DEFAULT 1,"
     "  exame_sugerido_id INTEGER NOT NULL DEFAULT 0,"
     "  exame_sugerido TEXT NOT NULL DEFAULT '',"
     "  ativo INTEGER NOT NULL DEFAULT 1,"
     "  FOREIGN KEY (especialidade_id) REFERENCES especialidades_clinicas(id));"
     "CREATE TABLE IF NOT EXISTS triagem_problemas ("
     "  id INTEGER PRIMARY KEY,"
     "  triagem_id INTEGER NOT NULL,"
     "  problema_id INTEGER NOT NULL,"
     "  especialidade_id INTEGER NOT NULL,"
     "  principal INTEGER NOT NULL DEFAULT 0,"
     "  observacao TEXT NOT NULL DEFAULT '',"
     "  ativo INTEGER NOT NULL DEFAULT 1,"
     "  criado_em TEXT NOT NULL DEFAULT (datetime('now')),"
     "  FOREIGN KEY (triagem_id) REFERENCES triagens(id),"
     "  FOREIGN KEY (problema_id) REFERENCES problemas_clinicos(id),"
     "  FOREIGN KEY (especialidade_id) REFERENCES especialidades_clinicas(id));"
     "INSERT OR IGNORE INTO especialidades_clinicas (id, nome, ativo) VALUES"
     "  (1, 'Clinico Geral', 1),"
     "  (2, 'Ortopedia', 1),"
     "  (3, 'Cardiologia', 1),"
     "  (4, 'Pneumologia', 1),"
     "  (5, 'Pediatria', 1),"
     "  (6, 'Neurologia', 1),"
     "  (7, 'Gastroenterologia', 1);"
     "INSERT OR IGNORE INTO problemas_clinicos (id, especialidade_id, nome, peso_risco, exame_sugerido_id, exame_sugerido, ativo) VALUES"
     "  (1, 3, 'dor no peito', 5, 2, 'Eletrocardiograma', 1),"
     "  (2, 3, 'falta de ar', 4, 1, 'Hemograma', 1),"
     "  (3, 3, 'palpitacao', 3, 2, 'Eletrocardiograma', 1),"
     "  (4, 3, 'pressao alta', 3, 1, 'Hemograma', 1),"
     "  (5, 3, 'desmaio', 4, 2, 'Eletrocardiograma', 1),"
     "  (6, 3, 'dor irradiando para braco', 5, 2, 'Eletrocardiograma', 1),"
     "  (7, 2, 'dor no joelho', 2, 3, 'Raio-X', 1),"
     "  (8, 2, 'fratura suspeita', 4, 3, 'Raio-X', 1),"
     "  (9, 2, 'dor lombar', 2, 0, '', 1),"
     "  (10, 2, 'torcao', 3, 3, 'Raio-X', 1),"
     "  (11, 2, 'limitacao de movimento', 3, 3, 'Raio-X', 1),"
     "  (12, 4, 'tosse persistente', 2, 1, 'Hemograma', 1),"
     "  (13, 4, 'falta de ar', 4, 4, 'Raio-X de torax', 1),"
     "  (14, 4, 'chiado no peito', 3, 4, 'Raio-X de torax', 1),"
     "  (15, 4, 'dor ao respirar', 4, 4, 'Raio-X de torax', 1),"
     "  (16, 4, 'suspeita de pneumonia', 4, 4, 'Raio-X de torax', 1),"
     "  (17, 5, 'febre infantil', 3, 1, 'Hemograma', 1),"
     "  (18, 5, 'vomito', 3, 1, 'Hemograma', 1),"
     "  (19, 5, 'dor abdominal', 3, 0, '', 1),"
     "  (20, 5, 'choro persistente', 3, 0, '', 1),"
     "  (21, 5, 'dificuldade respiratoria', 4, 4, 'Raio-X de torax', 1),"
     "  (22, 1, 'febre', 3, 1, 'Hemograma', 1),"
     "  (23, 1, 'mal-estar', 1, 0, '', 1),"
     "  (24, 1, 'dor de cabeca', 2, 0, '', 1),"
     "  (25, 1, 'tontura', 2, 1, 'Hemograma', 1),"
     "  (26, 1, 'dor abdominal', 3, 0, '', 1),"
     "  (27, 1, 'nausea', 2, 0, '', 1),"
     "  (28, 6, 'confusao mental', 4, 0, '', 1),"
     "  (29, 6, 'convulsao', 5, 0, '', 1),"
     "  (30, 6, 'fraqueza em um lado do corpo', 5, 0, '', 1),"
     "  (31, 7, 'dor abdominal intensa', 4, 1, 'Hemograma', 1),"
     "  (32, 7, 'vomitos persistentes', 3, 1, 'Hemograma', 1),"
     "  (33, 7, 'sangramento digestivo', 5, 1, 'Hemograma', 1);"
     "CREATE INDEX IF NOT EXISTS idx_triagem_problemas_triagem ON triagem_problemas(triagem_id);"
     "CREATE INDEX IF NOT EXISTS idx_problemas_especialidade ON problemas_clinicos(especialidade_id);"},
    {9, "solicitacoes do paciente para ajuda e consulta comum",
     "CREATE TABLE IF NOT EXISTS solicitacoes_paciente ("
     "  id INTEGER PRIMARY KEY,"
     "  paciente_id INTEGER NOT NULL,"
     "  tipo TEXT NOT NULL,"
     "  mensagem TEXT NOT NULL DEFAULT '',"
     "  status TEXT NOT NULL DEFAULT 'ABERTA',"
     "  criado_em TEXT NOT NULL DEFAULT (datetime('now')),"
     "  atualizado_em TEXT NOT NULL DEFAULT '',"
     "  FOREIGN KEY (paciente_id) REFERENCES pacientes(id));"
     "CREATE INDEX IF NOT EXISTS idx_solicitacoes_paciente ON solicitacoes_paciente(paciente_id);"
     "CREATE INDEX IF NOT EXISTS idx_solicitacoes_status ON solicitacoes_paciente(status);"},
    {10, "farmacia/estoque: catalogo de medicamentos, lotes e movimentacoes",
     "CREATE TABLE IF NOT EXISTS medicamentos ("
     "  id INTEGER PRIMARY KEY,"
     "  nome TEXT NOT NULL,"
     "  apresentacao TEXT NOT NULL DEFAULT '',"
     "  unidade TEXT NOT NULL DEFAULT '',"
     "  estoque_minimo INTEGER NOT NULL DEFAULT 0,"
     "  ativo INTEGER NOT NULL DEFAULT 1,"
     "  criado_em TEXT NOT NULL DEFAULT (datetime('now')));"
     "CREATE TABLE IF NOT EXISTS estoque_itens ("
     "  id INTEGER PRIMARY KEY,"
     "  medicamento_id INTEGER NOT NULL,"
     "  lote TEXT NOT NULL DEFAULT '',"
     "  validade TEXT NOT NULL DEFAULT '',"
     "  quantidade INTEGER NOT NULL DEFAULT 0,"
     "  localizacao TEXT NOT NULL DEFAULT '',"
     "  criado_em TEXT NOT NULL DEFAULT (datetime('now')),"
     "  FOREIGN KEY (medicamento_id) REFERENCES medicamentos(id));"
     "CREATE TABLE IF NOT EXISTS movimentacoes ("
     "  id INTEGER PRIMARY KEY,"
     "  medicamento_id INTEGER NOT NULL,"
     "  estoque_item_id INTEGER,"
     "  tipo TEXT NOT NULL,"
     "  quantidade INTEGER NOT NULL,"
     "  motivo TEXT NOT NULL DEFAULT '',"
     "  prescricao_id INTEGER,"
     "  usuario_id INTEGER NOT NULL DEFAULT 0,"
     "  usuario_login TEXT NOT NULL DEFAULT '',"
     "  criado_em TEXT NOT NULL DEFAULT (datetime('now')),"
     "  FOREIGN KEY (medicamento_id) REFERENCES medicamentos(id),"
     "  FOREIGN KEY (estoque_item_id) REFERENCES estoque_itens(id));"
     "CREATE INDEX IF NOT EXISTS idx_estoque_itens_medicamento ON estoque_itens(medicamento_id);"
     "CREATE INDEX IF NOT EXISTS idx_estoque_itens_validade ON estoque_itens(validade);"
     "CREATE INDEX IF NOT EXISTS idx_movimentacoes_medicamento ON movimentacoes(medicamento_id);"},
    {11, "farmacia: preco unitario do medicamento (vinculo com o financeiro)",
     "ALTER TABLE medicamentos ADD COLUMN preco_centavos INTEGER NOT NULL DEFAULT 0;"},
    {12, "vacinacao: catalogo de vacinas e esquema de doses",
     "CREATE TABLE IF NOT EXISTS vacinas ("
     "  id INTEGER PRIMARY KEY,"
     "  nome TEXT NOT NULL,"
     "  fabricante TEXT NOT NULL DEFAULT '',"
     "  doencas_alvo TEXT NOT NULL DEFAULT '',"
     "  doses_previstas INTEGER NOT NULL DEFAULT 1,"
     "  intervalo_dias INTEGER NOT NULL DEFAULT 0,"
     "  reforco_dias INTEGER NOT NULL DEFAULT 0,"
     "  ativo INTEGER NOT NULL DEFAULT 1,"
     "  criado_em TEXT NOT NULL DEFAULT (datetime('now')));"
     "CREATE UNIQUE INDEX IF NOT EXISTS idx_vacinas_nome_ativo ON vacinas(nome) WHERE ativo = 1;"},
    {13, "agendamentos: especialidade solicitada pelo paciente",
     "ALTER TABLE agendamentos ADD COLUMN especialidade TEXT NOT NULL DEFAULT '';"
     "UPDATE agendamentos SET especialidade = ("
     "  SELECT COALESCE(m.especialidade, '') FROM medicos m WHERE m.id = agendamentos.medico_id"
     ") WHERE especialidade = '';"},
    {14, "vacinacao: aplicacoes e vinculo com estoque",
     "ALTER TABLE vacinas ADD COLUMN medicamento_id INTEGER;"
     "CREATE TABLE IF NOT EXISTS aplicacoes_vacinas ("
     "  id INTEGER PRIMARY KEY,"
     "  paciente_id INTEGER NOT NULL,"
     "  vacina_id INTEGER NOT NULL,"
     "  medicamento_id INTEGER NOT NULL,"
     "  dose_numero INTEGER NOT NULL DEFAULT 1,"
     "  lote TEXT NOT NULL DEFAULT '',"
     "  validade TEXT NOT NULL DEFAULT '',"
     "  aplicador_usuario_id INTEGER NOT NULL DEFAULT 0,"
     "  aplicador_login TEXT NOT NULL DEFAULT '',"
     "  observacao TEXT NOT NULL DEFAULT '',"
     "  aplicada_em TEXT NOT NULL DEFAULT (datetime('now')),"
     "  FOREIGN KEY (paciente_id) REFERENCES pacientes(id),"
     "  FOREIGN KEY (vacina_id) REFERENCES vacinas(id),"
     "  FOREIGN KEY (medicamento_id) REFERENCES medicamentos(id));"
     "CREATE INDEX IF NOT EXISTS idx_aplicacoes_vacinas_paciente ON aplicacoes_vacinas(paciente_id);"
     "CREATE INDEX IF NOT EXISTS idx_aplicacoes_vacinas_vacina ON aplicacoes_vacinas(vacina_id);"
     "CREATE INDEX IF NOT EXISTS idx_aplicacoes_vacinas_data ON aplicacoes_vacinas(aplicada_em);"},
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
