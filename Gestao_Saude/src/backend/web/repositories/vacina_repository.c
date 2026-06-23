#include "vacina_repository.h"
#include "paciente_repository.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>
#include <string.h>

int vacina_criar(const char *nome, const char *fabricante,
                 const char *doencas_alvo, int doses_previstas,
                 int intervalo_dias, int reforco_dias, int medicamento_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int ok = 0;
    const char *sql =
        "INSERT INTO vacinas "
        "(nome, fabricante, doencas_alvo, doses_previstas, intervalo_dias, reforco_dias, medicamento_id, ativo) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, 1);";

    if (nome == NULL || nome[0] == '\0' || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (doses_previstas <= 0) doses_previstas = 1;
    if (intervalo_dias < 0) intervalo_dias = 0;
    if (reforco_dias < 0) reforco_dias = 0;
    if (medicamento_id < 0) medicamento_id = 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, nome, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, fabricante != NULL ? fabricante : "", -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, doencas_alvo != NULL ? doencas_alvo : "", -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, doses_previstas);
        sqlite3_bind_int(stmt, 5, intervalo_dias);
        sqlite3_bind_int(stmt, 6, reforco_dias);
        if (medicamento_id > 0)
        {
            sqlite3_bind_int(stmt, 7, medicamento_id);
        }
        else
        {
            sqlite3_bind_null(stmt, 7);
        }
        ok = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
    }

    db_fechar(db);
    return ok ? 1 : 0;
}

int vacina_listar_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, nome, fabricante, doencas_alvo, doses_previstas, intervalo_dias, reforco_dias, medicamento_id "
        "FROM vacinas WHERE ativo = 1 ORDER BY nome;";
    int usado = 0;
    int primeiro = 1;

    if (buffer == NULL || tamanho <= 0 || db_abrir(&db) == 0)
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
        char nomeJson[256];
        char fabricanteJson[160];
        char doencasJson[256];
        char objeto[900];
        int id = sqlite3_column_int(stmt, 0);
        int dosesPrevistas = sqlite3_column_int(stmt, 4);
        int intervaloDias = sqlite3_column_int(stmt, 5);
        int reforcoDias = sqlite3_column_int(stmt, 6);
        int medicamentoId = sqlite3_column_int(stmt, 7);
        int escrito;

        if (repo_json_escapar(nomeJson, sizeof(nomeJson), (const char *)sqlite3_column_text(stmt, 1)) == 0 ||
            repo_json_escapar(fabricanteJson, sizeof(fabricanteJson), (const char *)sqlite3_column_text(stmt, 2)) == 0 ||
            repo_json_escapar(doencasJson, sizeof(doencasJson), (const char *)sqlite3_column_text(stmt, 3)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"id\":%d,\"nome\":%s,\"fabricante\":%s,\"doencasAlvo\":%s,"
            "\"dosesPrevistas\":%d,\"intervaloDias\":%d,\"reforcoDias\":%d,"
            "\"medicamentoId\":%d}",
            primeiro ? "" : ",",
            id, nomeJson, fabricanteJson, doencasJson,
            dosesPrevistas, intervaloDias, reforcoDias, medicamentoId);

        if (escrito < 0 || escrito >= (int)sizeof(objeto) ||
            repo_json_anexar(buffer, tamanho, &usado, objeto) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }
        primeiro = 0;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return repo_json_anexar(buffer, tamanho, &usado, "]");
}

int vacina_desativar(int id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int alteradas = 0;

    if (id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db,
            "UPDATE vacinas SET ativo = 0 WHERE id = ? AND ativo = 1;",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, id);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        alteradas = sqlite3_changes(db);
    }

    db_fechar(db);
    return alteradas > 0 ? 1 : 0;
}

int vacina_contar_ativas(void)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int total = -1;

    if (db_abrir(&db) == 0)
    {
        return -1;
    }

    if (sqlite3_prepare_v2(db,
            "SELECT COUNT(*) FROM vacinas WHERE ativo = 1;",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            total = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    db_fechar(db);
    return total;
}

int vacina_ativa(int id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int ativo = 0;

    if (id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db,
            "SELECT 1 FROM vacinas WHERE id = ? AND ativo = 1;",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, id);
        ativo = sqlite3_step(stmt) == SQLITE_ROW;
        sqlite3_finalize(stmt);
    }

    db_fechar(db);
    return ativo;
}

int vacina_aplicar(int paciente_id, int vacina_id, int dose_numero,
                   const char *lote, const char *validade,
                   int aplicador_usuario_id, const char *aplicador_login,
                   const char *observacao, int *aplicacao_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int medicamento_id = 0;
    int item_id = 0;
    int saldo = 0;
    int novo_id = 0;
    char motivo[160];

    if (aplicacao_id != NULL) *aplicacao_id = 0;
    if (paciente_id <= 0 || vacina_id <= 0 ||
        lote == NULL || lote[0] == '\0' ||
        validade == NULL || validade[0] == '\0' ||
        paciente_repo_regiao(paciente_id) < 0 ||
        db_abrir(&db) == 0)
    {
        return 0;
    }

    if (dose_numero <= 0) dose_numero = 1;
    if (aplicador_login == NULL) aplicador_login = "";
    if (observacao == NULL) observacao = "";

    if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    if (sqlite3_prepare_v2(db,
            "SELECT medicamento_id FROM vacinas WHERE id = ? AND ativo = 1;",
            -1, &stmt, NULL) != SQLITE_OK)
    {
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, vacina_id);
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        medicamento_id = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (medicamento_id <= 0)
    {
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        db_fechar(db);
        return 0;
    }

    if (sqlite3_prepare_v2(db,
            "SELECT id, quantidade FROM estoque_itens "
            "WHERE medicamento_id = ? AND lote = ? AND validade = ? AND quantidade >= 1 "
            "ORDER BY id LIMIT 1;",
            -1, &stmt, NULL) != SQLITE_OK)
    {
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, medicamento_id);
    sqlite3_bind_text(stmt, 2, lote, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, validade, -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        item_id = sqlite3_column_int(stmt, 0);
        saldo = sqlite3_column_int(stmt, 1);
    }
    sqlite3_finalize(stmt);

    if (item_id <= 0 || saldo <= 0)
    {
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        db_fechar(db);
        return 0;
    }

    if (sqlite3_prepare_v2(db,
            "UPDATE estoque_itens SET quantidade = quantidade - 1 WHERE id = ?;",
            -1, &stmt, NULL) != SQLITE_OK)
    {
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, item_id);
    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        db_fechar(db);
        return 0;
    }
    sqlite3_finalize(stmt);

    snprintf(motivo, sizeof(motivo), "Aplicacao de vacina #%d no paciente #%d",
             vacina_id, paciente_id);
    if (sqlite3_prepare_v2(db,
            "INSERT INTO movimentacoes "
            "(medicamento_id, estoque_item_id, tipo, quantidade, motivo, usuario_id, usuario_login) "
            "VALUES (?, ?, 'SAIDA', 1, ?, ?, ?);",
            -1, &stmt, NULL) != SQLITE_OK)
    {
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, medicamento_id);
    sqlite3_bind_int(stmt, 2, item_id);
    sqlite3_bind_text(stmt, 3, motivo, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, aplicador_usuario_id);
    sqlite3_bind_text(stmt, 5, aplicador_login, -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        db_fechar(db);
        return 0;
    }
    sqlite3_finalize(stmt);

    if (sqlite3_prepare_v2(db,
            "INSERT INTO aplicacoes_vacinas "
            "(paciente_id, vacina_id, medicamento_id, dose_numero, lote, validade, "
            "aplicador_usuario_id, aplicador_login, observacao) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);",
            -1, &stmt, NULL) != SQLITE_OK)
    {
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, paciente_id);
    sqlite3_bind_int(stmt, 2, vacina_id);
    sqlite3_bind_int(stmt, 3, medicamento_id);
    sqlite3_bind_int(stmt, 4, dose_numero);
    sqlite3_bind_text(stmt, 5, lote, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, validade, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 7, aplicador_usuario_id);
    sqlite3_bind_text(stmt, 8, aplicador_login, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 9, observacao, -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        db_fechar(db);
        return 0;
    }
    sqlite3_finalize(stmt);
    novo_id = (int)sqlite3_last_insert_rowid(db);

    if (sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL) != SQLITE_OK)
    {
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        db_fechar(db);
        return 0;
    }

    db_fechar(db);
    if (aplicacao_id != NULL) *aplicacao_id = novo_id;
    return 1;
}

static int vacina_aplicacoes_listar_filtrado_json(int paciente_id,
                                                  char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql_todos =
        "SELECT a.id, a.paciente_id, COALESCE(p.nome, ''), a.vacina_id, "
        "COALESCE(v.nome, ''), a.medicamento_id, a.dose_numero, a.lote, "
        "a.validade, a.aplicador_login, a.observacao, a.aplicada_em "
        "FROM aplicacoes_vacinas a "
        "LEFT JOIN pacientes p ON p.id = a.paciente_id "
        "LEFT JOIN vacinas v ON v.id = a.vacina_id "
        "ORDER BY a.id DESC;";
    const char *sql_paciente =
        "SELECT a.id, a.paciente_id, COALESCE(p.nome, ''), a.vacina_id, "
        "COALESCE(v.nome, ''), a.medicamento_id, a.dose_numero, a.lote, "
        "a.validade, a.aplicador_login, a.observacao, a.aplicada_em "
        "FROM aplicacoes_vacinas a "
        "LEFT JOIN pacientes p ON p.id = a.paciente_id "
        "LEFT JOIN vacinas v ON v.id = a.vacina_id "
        "WHERE a.paciente_id = ? "
        "ORDER BY a.id DESC;";
    int usado = 0;
    int primeiro = 1;

    if (buffer == NULL || tamanho <= 0 || paciente_id < 0 || db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, paciente_id > 0 ? sql_paciente : sql_todos,
                           -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    if (paciente_id > 0)
    {
        sqlite3_bind_int(stmt, 1, paciente_id);
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
        char pacienteJson[256];
        char vacinaJson[256];
        char loteJson[160];
        char validadeJson[80];
        char loginJson[128];
        char observacaoJson[256];
        char aplicadaJson[80];
        char objeto[1400];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int vacinaId = sqlite3_column_int(stmt, 3);
        int medicamentoId = sqlite3_column_int(stmt, 5);
        int doseNumero = sqlite3_column_int(stmt, 6);
        int escrito;

        if (repo_json_escapar(pacienteJson, sizeof(pacienteJson), (const char *)sqlite3_column_text(stmt, 2)) == 0 ||
            repo_json_escapar(vacinaJson, sizeof(vacinaJson), (const char *)sqlite3_column_text(stmt, 4)) == 0 ||
            repo_json_escapar(loteJson, sizeof(loteJson), (const char *)sqlite3_column_text(stmt, 7)) == 0 ||
            repo_json_escapar(validadeJson, sizeof(validadeJson), (const char *)sqlite3_column_text(stmt, 8)) == 0 ||
            repo_json_escapar(loginJson, sizeof(loginJson), (const char *)sqlite3_column_text(stmt, 9)) == 0 ||
            repo_json_escapar(observacaoJson, sizeof(observacaoJson), (const char *)sqlite3_column_text(stmt, 10)) == 0 ||
            repo_json_escapar(aplicadaJson, sizeof(aplicadaJson), (const char *)sqlite3_column_text(stmt, 11)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"id\":%d,\"pacienteId\":%d,\"pacienteNome\":%s,"
            "\"vacinaId\":%d,\"vacinaNome\":%s,\"medicamentoId\":%d,"
            "\"doseNumero\":%d,\"lote\":%s,\"validade\":%s,"
            "\"aplicadorLogin\":%s,\"observacao\":%s,\"aplicadaEm\":%s}",
            primeiro ? "" : ",",
            id, pacienteId, pacienteJson,
            vacinaId, vacinaJson, medicamentoId,
            doseNumero, loteJson, validadeJson,
            loginJson, observacaoJson, aplicadaJson);

        if (escrito < 0 || escrito >= (int)sizeof(objeto) ||
            repo_json_anexar(buffer, tamanho, &usado, objeto) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }
        primeiro = 0;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return repo_json_anexar(buffer, tamanho, &usado, "]");
}

int vacina_aplicacoes_listar_json(char *buffer, int tamanho)
{
    return vacina_aplicacoes_listar_filtrado_json(0, buffer, tamanho);
}

int vacina_aplicacoes_listar_por_paciente_json(int paciente_id,
                                               char *buffer, int tamanho)
{
    return vacina_aplicacoes_listar_filtrado_json(paciente_id, buffer, tamanho);
}
