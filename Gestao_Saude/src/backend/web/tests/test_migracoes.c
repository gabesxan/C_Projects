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

/* 1 se a tabela tem a coluna informada. */
static int coluna_existe(const char *tabela, const char *coluna)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    char sql[128];
    int existe = 0;

    if (db_abrir(&db) == 0) return 0;
    snprintf(sql, sizeof(sql), "PRAGMA table_info(%s);", tabela);
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const char *nome = (const char *)sqlite3_column_text(stmt, 1);
            if (nome != NULL && strcmp(nome, coluna) == 0) { existe = 1; break; }
        }
        sqlite3_finalize(stmt);
    }
    db_fechar(db);
    return existe;
}

/* 1 se existe uma tabela com o nome dado. */
static int tabela_existe(const char *nome)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int existe = 0;

    if (db_abrir(&db) == 0) return 0;
    if (sqlite3_prepare_v2(db,
            "SELECT 1 FROM sqlite_master WHERE type='table' AND name=?;",
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

    /* Schema completo: nasce ja na ultima versao, com indices e colunas novos. */
    assert(db_resetar_com_schema(SCHEMA) == 1);
    assert(versao() == 17);
    assert(indice_existe("idx_cobrancas_paciente") == 1);
    assert(indice_existe("idx_sessoes_expira") == 1);
    assert(coluna_existe("checkins", "rechamadas") == 1);
    assert(coluna_existe("checkins", "motivo") == 1);
    assert(coluna_existe("cobrancas", "lote_id") == 1);
    assert(tabela_existe("lotes") == 1);
    assert(coluna_existe("convenios", "cobertura_pct") == 1);
    assert(coluna_existe("cobrancas", "vencimento") == 1);
    assert(coluna_existe("cobrancas", "copart_centavos") == 1);
    assert(tabela_existe("analitos") == 1);
    assert(tabela_existe("painel_analitos") == 1);
    assert(indice_existe("idx_painel_tipo") == 1);
    assert(indice_existe("idx_painel_analito") == 1);
    assert(tabela_existe("exame_resultados_analitos") == 1);
    assert(indice_existe("idx_exame_resultados_exame") == 1);
    assert(indice_existe("idx_exame_resultados_exame_analito") == 1);
    assert(tabela_existe("especialidades_clinicas") == 1);
    assert(tabela_existe("problemas_clinicos") == 1);
    assert(tabela_existe("triagem_problemas") == 1);
    assert(coluna_existe("triagens", "profissional_id") == 1);
    assert(coluna_existe("triagens", "especialidade_principal_id") == 1);
    assert(coluna_existe("triagens", "prioridade") == 1);
    assert(indice_existe("idx_triagem_problemas_triagem") == 1);
    assert(tabela_existe("solicitacoes_paciente") == 1);
    assert(indice_existe("idx_solicitacoes_paciente") == 1);
    assert(indice_existe("idx_solicitacoes_status") == 1);
    assert(tabela_existe("medicamentos") == 1);
    assert(tabela_existe("estoque_itens") == 1);
    assert(tabela_existe("movimentacoes") == 1);
    assert(indice_existe("idx_estoque_itens_medicamento") == 1);
    assert(indice_existe("idx_estoque_itens_validade") == 1);
    assert(indice_existe("idx_movimentacoes_medicamento") == 1);
    assert(coluna_existe("medicamentos", "preco_centavos") == 1);
    assert(tabela_existe("vacinas") == 1);
    assert(indice_existe("idx_vacinas_nome_ativo") == 1);
    assert(coluna_existe("vacinas", "medicamento_id") == 1);
    assert(tabela_existe("aplicacoes_vacinas") == 1);
    assert(indice_existe("idx_aplicacoes_vacinas_paciente") == 1);
    assert(indice_existe("idx_aplicacoes_vacinas_vacina") == 1);
    assert(indice_existe("idx_aplicacoes_vacinas_data") == 1);
    assert(coluna_existe("agendamentos", "especialidade") == 1);
    assert(tabela_existe("anexos") == 1);
    assert(indice_existe("idx_anexos_entidade") == 1);
    assert(tabela_existe("consentimentos") == 1);
    assert(indice_existe("idx_consentimentos_paciente") == 1);
    assert(indice_existe("idx_consentimentos_status") == 1);
    assert(indice_existe("idx_consentimentos_finalidade") == 1);
    assert(coluna_existe("checkins", "medico_id") == 1);
    assert(tabela_existe("notificacoes") == 1);
    assert(indice_existe("idx_notificacoes_usuario") == 1);

    /* Simula um banco ANTIGO: volta para a v1, removendo artefatos v2..v17. */
    assert(db_executar("DROP INDEX IF EXISTS idx_notificacoes_usuario;") == 1);
    assert(db_executar("DROP TABLE notificacoes;") == 1);
    assert(db_executar("ALTER TABLE checkins DROP COLUMN medico_id;") == 1);
    assert(db_executar("DROP INDEX IF EXISTS idx_consentimentos_paciente;") == 1);
    assert(db_executar("DROP INDEX IF EXISTS idx_consentimentos_status;") == 1);
    assert(db_executar("DROP INDEX IF EXISTS idx_consentimentos_finalidade;") == 1);
    assert(db_executar("DROP TABLE consentimentos;") == 1);
    assert(db_executar("DROP INDEX IF EXISTS idx_anexos_entidade;") == 1);
    assert(db_executar("DROP TABLE anexos;") == 1);
    assert(db_executar("DROP INDEX IF EXISTS idx_cobrancas_paciente;") == 1);
    assert(db_executar("DROP INDEX IF EXISTS idx_sessoes_expira;") == 1);
    assert(db_executar("ALTER TABLE checkins DROP COLUMN rechamadas;") == 1);
    assert(db_executar("ALTER TABLE checkins DROP COLUMN motivo;") == 1);
    assert(db_executar("DROP INDEX IF EXISTS idx_cobrancas_lote;") == 1);
    assert(db_executar("ALTER TABLE cobrancas DROP COLUMN lote_id;") == 1);
    assert(db_executar("DROP TABLE lotes;") == 1);
    assert(db_executar("ALTER TABLE convenios DROP COLUMN cobertura_pct;") == 1);
    assert(db_executar("ALTER TABLE cobrancas DROP COLUMN vencimento;") == 1);
    assert(db_executar("ALTER TABLE cobrancas DROP COLUMN guia;") == 1);
    assert(db_executar("ALTER TABLE cobrancas DROP COLUMN guia_validade;") == 1);
    assert(db_executar("ALTER TABLE cobrancas DROP COLUMN coberto_centavos;") == 1);
    assert(db_executar("ALTER TABLE cobrancas DROP COLUMN copart_centavos;") == 1);
    assert(db_executar("DROP INDEX IF EXISTS idx_painel_tipo;") == 1);
    assert(db_executar("DROP INDEX IF EXISTS idx_painel_analito;") == 1);
    assert(db_executar("DROP INDEX IF EXISTS idx_analitos_codigo_ativo;") == 1);
    assert(db_executar("DROP INDEX IF EXISTS idx_exame_resultados_exame;") == 1);
    assert(db_executar("DROP INDEX IF EXISTS idx_exame_resultados_exame_analito;") == 1);
    assert(db_executar("DROP INDEX IF EXISTS idx_triagem_problemas_triagem;") == 1);
    assert(db_executar("DROP INDEX IF EXISTS idx_problemas_especialidade;") == 1);
    assert(db_executar("DROP INDEX IF EXISTS idx_solicitacoes_paciente;") == 1);
    assert(db_executar("DROP INDEX IF EXISTS idx_solicitacoes_status;") == 1);
    assert(db_executar("DROP INDEX IF EXISTS idx_estoque_itens_medicamento;") == 1);
    assert(db_executar("DROP INDEX IF EXISTS idx_estoque_itens_validade;") == 1);
    assert(db_executar("DROP INDEX IF EXISTS idx_movimentacoes_medicamento;") == 1);
    assert(db_executar("DROP INDEX IF EXISTS idx_vacinas_nome_ativo;") == 1);
    assert(db_executar("DROP INDEX IF EXISTS idx_aplicacoes_vacinas_paciente;") == 1);
    assert(db_executar("DROP INDEX IF EXISTS idx_aplicacoes_vacinas_vacina;") == 1);
    assert(db_executar("DROP INDEX IF EXISTS idx_aplicacoes_vacinas_data;") == 1);
    assert(db_executar("DROP TABLE aplicacoes_vacinas;") == 1);
    assert(db_executar("DROP TABLE vacinas;") == 1);
    assert(db_executar("ALTER TABLE agendamentos DROP COLUMN especialidade;") == 1);
    assert(db_executar("DROP TABLE movimentacoes;") == 1);
    assert(db_executar("DROP TABLE estoque_itens;") == 1);
    assert(db_executar("DROP TABLE medicamentos;") == 1);
    assert(db_executar("DROP TABLE solicitacoes_paciente;") == 1);
    assert(db_executar("DROP TABLE exame_resultados_analitos;") == 1);
    assert(db_executar("DROP TABLE painel_analitos;") == 1);
    assert(db_executar("DROP TABLE analitos;") == 1);
    assert(db_executar("DROP TABLE triagem_problemas;") == 1);
    assert(db_executar("DROP TABLE problemas_clinicos;") == 1);
    assert(db_executar("DROP TABLE especialidades_clinicas;") == 1);
    assert(db_executar("ALTER TABLE triagens DROP COLUMN profissional_id;") == 1);
    assert(db_executar("ALTER TABLE triagens DROP COLUMN especialidade_principal_id;") == 1);
    assert(db_executar("ALTER TABLE triagens DROP COLUMN prioridade;") == 1);
    assert(db_executar("ALTER TABLE triagens DROP COLUMN observacoes;") == 1);
    assert(db_executar("ALTER TABLE triagens DROP COLUMN status;") == 1);
    assert(db_executar("ALTER TABLE triagens DROP COLUMN data_hora;") == 1);
    assert(db_executar("PRAGMA user_version = 1;") == 1);
    /* Dado preexistente para provar que a migracao preserva os dados. */
    assert(db_executar("INSERT INTO convenios (nome, ativo) VALUES ('Antigo', 1);") == 1);
    assert(indice_existe("idx_cobrancas_paciente") == 0);
    assert(coluna_existe("checkins", "rechamadas") == 0);
    assert(coluna_existe("cobrancas", "lote_id") == 0);
    assert(tabela_existe("lotes") == 0);
    assert(coluna_existe("convenios", "cobertura_pct") == 0);
    assert(coluna_existe("cobrancas", "vencimento") == 0);
    assert(tabela_existe("analitos") == 0);
    assert(tabela_existe("painel_analitos") == 0);
    assert(tabela_existe("exame_resultados_analitos") == 0);
    assert(tabela_existe("especialidades_clinicas") == 0);
    assert(tabela_existe("solicitacoes_paciente") == 0);
    assert(tabela_existe("medicamentos") == 0);
    assert(tabela_existe("estoque_itens") == 0);
    assert(tabela_existe("movimentacoes") == 0);
    assert(tabela_existe("vacinas") == 0);
    assert(tabela_existe("aplicacoes_vacinas") == 0);
    assert(tabela_existe("anexos") == 0);
    assert(tabela_existe("consentimentos") == 0);
    assert(coluna_existe("agendamentos", "especialidade") == 0);
    assert(coluna_existe("triagens", "profissional_id") == 0);
    assert(tabela_existe("notificacoes") == 0);
    assert(coluna_existe("checkins", "medico_id") == 0);
    assert(versao() == 1);

    /* Migra: aplica v2..v17, sobe para a ultima versao e mantem os dados. */
    assert(db_migrar() == 1);
    assert(versao() == 17);
    assert(indice_existe("idx_cobrancas_paciente") == 1);
    assert(indice_existe("idx_sessoes_expira") == 1);
    assert(coluna_existe("checkins", "rechamadas") == 1);
    assert(coluna_existe("checkins", "motivo") == 1);
    assert(coluna_existe("cobrancas", "lote_id") == 1);
    assert(tabela_existe("lotes") == 1);
    assert(coluna_existe("convenios", "cobertura_pct") == 1);
    assert(coluna_existe("cobrancas", "vencimento") == 1);
    assert(coluna_existe("cobrancas", "copart_centavos") == 1);
    assert(tabela_existe("analitos") == 1);
    assert(tabela_existe("painel_analitos") == 1);
    assert(indice_existe("idx_painel_tipo") == 1);
    assert(indice_existe("idx_painel_analito") == 1);
    assert(tabela_existe("exame_resultados_analitos") == 1);
    assert(indice_existe("idx_exame_resultados_exame") == 1);
    assert(indice_existe("idx_exame_resultados_exame_analito") == 1);
    assert(tabela_existe("especialidades_clinicas") == 1);
    assert(tabela_existe("problemas_clinicos") == 1);
    assert(tabela_existe("triagem_problemas") == 1);
    assert(coluna_existe("triagens", "profissional_id") == 1);
    assert(coluna_existe("triagens", "especialidade_principal_id") == 1);
    assert(coluna_existe("triagens", "prioridade") == 1);
    assert(indice_existe("idx_triagem_problemas_triagem") == 1);
    assert(tabela_existe("solicitacoes_paciente") == 1);
    assert(indice_existe("idx_solicitacoes_paciente") == 1);
    assert(indice_existe("idx_solicitacoes_status") == 1);
    assert(tabela_existe("medicamentos") == 1);
    assert(tabela_existe("estoque_itens") == 1);
    assert(tabela_existe("movimentacoes") == 1);
    assert(indice_existe("idx_estoque_itens_medicamento") == 1);
    assert(indice_existe("idx_movimentacoes_medicamento") == 1);
    assert(coluna_existe("medicamentos", "preco_centavos") == 1);
    assert(tabela_existe("vacinas") == 1);
    assert(indice_existe("idx_vacinas_nome_ativo") == 1);
    assert(coluna_existe("vacinas", "medicamento_id") == 1);
    assert(tabela_existe("aplicacoes_vacinas") == 1);
    assert(indice_existe("idx_aplicacoes_vacinas_paciente") == 1);
    assert(indice_existe("idx_aplicacoes_vacinas_vacina") == 1);
    assert(indice_existe("idx_aplicacoes_vacinas_data") == 1);
    assert(coluna_existe("agendamentos", "especialidade") == 1);
    assert(tabela_existe("anexos") == 1);
    assert(indice_existe("idx_anexos_entidade") == 1);
    assert(tabela_existe("consentimentos") == 1);
    assert(indice_existe("idx_consentimentos_paciente") == 1);
    assert(indice_existe("idx_consentimentos_status") == 1);
    assert(indice_existe("idx_consentimentos_finalidade") == 1);
    assert(coluna_existe("checkins", "medico_id") == 1);
    assert(tabela_existe("notificacoes") == 1);
    assert(indice_existe("idx_notificacoes_usuario") == 1);
    assert(contar("SELECT COUNT(*) FROM especialidades_clinicas WHERE ativo = 1;") >= 7);
    assert(contar("SELECT COUNT(*) FROM convenios WHERE nome='Antigo';") == 1);

    /* Idempotente: rodar de novo num banco ja atualizado nao muda nada. */
    assert(db_migrar() == 1);
    assert(versao() == 17);
    assert(contar("SELECT COUNT(*) FROM convenios WHERE nome='Antigo';") == 1);

    printf("test_migracoes: OK\n");
    return 0;
}
