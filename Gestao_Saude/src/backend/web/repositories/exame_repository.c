#include "exame_repository.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>
#include <string.h>

static int statusAtual(int id, char *destino, int tam);

int exame_repo_criar(int paciente_id, int medico_id, int prontuario_id,
                     int tipo_exame, const char *data_solicitacao, int urgente)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO exames "
        "(paciente_id, medico_id, prontuario_id, tipo_exame, data_solicitacao, "
        "data_resultado, resultado, status, urgente, ativo) "
        "VALUES (?, ?, ?, ?, ?, '', '', 'SOLICITADO', ?, 1);";

    if (paciente_id <= 0 || medico_id <= 0 || prontuario_id <= 0 || tipo_exame <= 0)
    {
        return 0;
    }

    if (data_solicitacao == NULL || data_solicitacao[0] == '\0')
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_int(stmt, 1, paciente_id);
    sqlite3_bind_int(stmt, 2, medico_id);
    sqlite3_bind_int(stmt, 3, prontuario_id);
    sqlite3_bind_int(stmt, 4, tipo_exame);
    sqlite3_bind_text(stmt, 5, data_solicitacao, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, urgente);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    sqlite3_finalize(stmt);

    /* O exame original e raiz de si mesmo (raiz_id = id). */
    sqlite3_exec(db, "UPDATE exames SET raiz_id = id WHERE raiz_id = 0;",
                 NULL, NULL, NULL);

    db_fechar(db);
    return 1;
}

/* Recalcula se o exame deve ficar marcado como critico a partir dos resultados
 * estruturados fora da faixa de referencia ja gravados para ele. */
static int recalcularCriticoEstruturado(sqlite3 *db, int exame_id)
{
    sqlite3_stmt *stmt = NULL;
    int critico = 0;
    int ok = 0;

    if (sqlite3_prepare_v2(db,
            "SELECT COUNT(*) FROM exame_resultados_analitos "
            "WHERE exame_id = ? AND fora_referencia = 1;",
            -1, &stmt, NULL) != SQLITE_OK)
    {
        return 0;
    }
    sqlite3_bind_int(stmt, 1, exame_id);
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        critico = sqlite3_column_int(stmt, 0) > 0 ? 1 : 0;
    }
    sqlite3_finalize(stmt);

    if (sqlite3_prepare_v2(db,
            "UPDATE exames SET resultado_critico = ? "
            "WHERE id = ? AND ativo = 1;",
            -1, &stmt, NULL) != SQLITE_OK)
    {
        return 0;
    }
    sqlite3_bind_int(stmt, 1, critico);
    sqlite3_bind_int(stmt, 2, exame_id);
    ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok ? 1 : 0;
}

/* Retifica o resultado de um exame CONCLUIDO criando uma NOVA versao (vigente)
 * e preservando a anterior. Exige justificativa. 1 = ok, 0 = falha. */
int exame_repo_retificar_resultado(int id, const char *resultado, int critico,
                                   const char *justificativa)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sqlBusca =
        "SELECT paciente_id, medico_id, prontuario_id, tipo_exame, "
        "data_solicitacao, urgente, versao, raiz_id, status FROM exames "
        "WHERE id = ? AND ativo = 1 AND vigente = 1;";
    const char *sqlInsert =
        "INSERT INTO exames "
        "(paciente_id, medico_id, prontuario_id, tipo_exame, data_solicitacao, "
        "data_resultado, resultado, status, urgente, resultado_critico, "
        "versao, raiz_id, vigente, justificativa, ativo) "
        "VALUES (?, ?, ?, ?, ?, datetime('now'), ?, 'CONCLUIDO', ?, ?, ?, ?, 1, ?, 1);";
    int pac = 0, med = 0, pront = 0, tipo = 0, urgente = 0, versao = 0, raiz = 0;
    int novoId = 0;
    char dataSolic[32] = "";
    char status[32] = "";
    int ok = 0;

    if (resultado == NULL || resultado[0] == '\0' ||
        justificativa == NULL || justificativa[0] == '\0')
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sqlBusca, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, id);
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        pac = sqlite3_column_int(stmt, 0);
        med = sqlite3_column_int(stmt, 1);
        pront = sqlite3_column_int(stmt, 2);
        tipo = sqlite3_column_int(stmt, 3);
        snprintf(dataSolic, sizeof(dataSolic), "%s", (const char *)sqlite3_column_text(stmt, 4));
        urgente = sqlite3_column_int(stmt, 5);
        versao = sqlite3_column_int(stmt, 6);
        raiz = sqlite3_column_int(stmt, 7);
        snprintf(status, sizeof(status), "%s", (const char *)sqlite3_column_text(stmt, 8));
    }
    sqlite3_finalize(stmt);

    /* So retifica resultado de exame ja concluido. */
    if (pac == 0 || strcmp(status, "CONCLUIDO") != 0)
    {
        db_fechar(db);
        return 0;
    }

    if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    if (sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, NULL) != SQLITE_OK)
    {
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, pac);
    sqlite3_bind_int(stmt, 2, med);
    sqlite3_bind_int(stmt, 3, pront);
    sqlite3_bind_int(stmt, 4, tipo);
    sqlite3_bind_text(stmt, 5, dataSolic, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, resultado, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 7, urgente);
    sqlite3_bind_int(stmt, 8, critico ? 1 : 0);
    sqlite3_bind_int(stmt, 9, versao + 1);
    sqlite3_bind_int(stmt, 10, raiz);
    sqlite3_bind_text(stmt, 11, justificativa, -1, SQLITE_STATIC);
    ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (ok)
    {
        novoId = (int)sqlite3_last_insert_rowid(db);
    }
    sqlite3_finalize(stmt);

    if (ok)
    {
        sqlite3_stmt *copiar = NULL;
        if (sqlite3_prepare_v2(db,
                "INSERT INTO exame_resultados_analitos "
                "(exame_id, analito_id, valor_numerico, valor_texto, fora_referencia, observacao) "
                "SELECT ?, analito_id, valor_numerico, valor_texto, fora_referencia, observacao "
                "FROM exame_resultados_analitos WHERE exame_id = ?;",
                -1, &copiar, NULL) == SQLITE_OK)
        {
            sqlite3_bind_int(copiar, 1, novoId);
            sqlite3_bind_int(copiar, 2, id);
            ok = sqlite3_step(copiar) == SQLITE_DONE;
            sqlite3_finalize(copiar);
        }
        else
        {
            ok = 0;
        }
    }

    if (ok)
    {
        sqlite3_stmt *upd = NULL;
        if (sqlite3_prepare_v2(db, "UPDATE exames SET vigente = 0 WHERE id = ?;",
                               -1, &upd, NULL) == SQLITE_OK)
        {
            sqlite3_bind_int(upd, 1, id);
            ok = sqlite3_step(upd) == SQLITE_DONE && sqlite3_changes(db) > 0;
            sqlite3_finalize(upd);
        }
        else
        {
            ok = 0;
        }
    }

    sqlite3_exec(db, ok ? "COMMIT;" : "ROLLBACK;", NULL, NULL, NULL);
    db_fechar(db);
    return ok ? 1 : 0;
}

int exame_repo_listar_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT e.id, e.paciente_id, e.medico_id, e.prontuario_id, e.tipo_exame, "
        "e.data_solicitacao, e.data_resultado, e.resultado, e.status, e.urgente, "
        "COALESCE(p.nome, ''), COALESCE(m.nome, '') "
        "FROM exames e "
        "LEFT JOIN pacientes p ON p.id = e.paciente_id "
        "LEFT JOIN medicos m ON m.id = e.medico_id "
        "WHERE e.ativo = 1 AND e.vigente = 1 ORDER BY e.id;";
    int usado = 0;
    int primeiro = 1;

    if (buffer == NULL || tamanho <= 0)
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
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
        char dataSolicitacaoJson[32];
        char dataResultadoJson[32];
        char resultadoJson[640];
        char statusJson[48];
        char pacienteNomeJson[256];
        char medicoNomeJson[256];
        char objeto[1600];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int medicoId = sqlite3_column_int(stmt, 2);
        int prontuarioId = sqlite3_column_int(stmt, 3);
        int tipoExame = sqlite3_column_int(stmt, 4);
        const char *dataSolic = (const char *)sqlite3_column_text(stmt, 5);
        const char *dataResul = (const char *)sqlite3_column_text(stmt, 6);
        const char *resultado = (const char *)sqlite3_column_text(stmt, 7);
        const char *status = (const char *)sqlite3_column_text(stmt, 8);
        int urgente = sqlite3_column_int(stmt, 9);
        int escrito;

        if (repo_json_escapar(dataSolicitacaoJson, sizeof(dataSolicitacaoJson), dataSolic) == 0 ||
            repo_json_escapar(dataResultadoJson, sizeof(dataResultadoJson), dataResul) == 0 ||
            repo_json_escapar(resultadoJson, sizeof(resultadoJson), resultado) == 0 ||
            repo_json_escapar(statusJson, sizeof(statusJson), status) == 0 ||
            repo_json_escapar(pacienteNomeJson, sizeof(pacienteNomeJson), (const char *)sqlite3_column_text(stmt, 10)) == 0 ||
            repo_json_escapar(medicoNomeJson, sizeof(medicoNomeJson), (const char *)sqlite3_column_text(stmt, 11)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"id\":%d,\"pacienteId\":%d,\"medicoId\":%d,\"prontuarioId\":%d,"
                           "\"pacienteNome\":%s,\"medicoNome\":%s,"
                           "\"tipoExame\":%d,\"dataSolicitacao\":%s,\"dataResultado\":%s,"
                           "\"resultado\":%s,\"status\":%s,\"urgente\":%d}",
                           primeiro ? "" : ",",
                           id, pacienteId, medicoId, prontuarioId,
                           pacienteNomeJson, medicoNomeJson, tipoExame,
                           dataSolicitacaoJson, dataResultadoJson, resultadoJson, statusJson,
                           urgente);

        if (escrito < 0 || escrito >= (int)sizeof(objeto))
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        if (repo_json_anexar(buffer, tamanho, &usado, objeto) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        primeiro = 0;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);

    if (repo_json_anexar(buffer, tamanho, &usado, "]") == 0)
    {
        return 0;
    }

    return 1;
}

int exame_repo_listar_por_medico_json(int medico_id, char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT e.id, e.paciente_id, e.medico_id, e.prontuario_id, e.tipo_exame, "
        "e.data_solicitacao, e.data_resultado, e.resultado, e.status, e.urgente, "
        "COALESCE(p.nome, ''), COALESCE(m.nome, '') "
        "FROM exames e "
        "LEFT JOIN pacientes p ON p.id = e.paciente_id "
        "LEFT JOIN medicos m ON m.id = e.medico_id "
        "WHERE e.medico_id = ? AND e.ativo = 1 AND e.vigente = 1 ORDER BY e.id;";
    int usado = 0;
    int primeiro = 1;

    if (buffer == NULL || tamanho <= 0 || medico_id <= 0)
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_int(stmt, 1, medico_id);

    buffer[0] = '\0';

    if (repo_json_anexar(buffer, tamanho, &usado, "[") == 0)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char dataSolicitacaoJson[32];
        char dataResultadoJson[32];
        char resultadoJson[640];
        char statusJson[48];
        char pacienteNomeJson[256];
        char medicoNomeJson[256];
        char objeto[1600];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int medicoId = sqlite3_column_int(stmt, 2);
        int prontuarioId = sqlite3_column_int(stmt, 3);
        int tipoExame = sqlite3_column_int(stmt, 4);
        const char *dataSolic = (const char *)sqlite3_column_text(stmt, 5);
        const char *dataResul = (const char *)sqlite3_column_text(stmt, 6);
        const char *resultado = (const char *)sqlite3_column_text(stmt, 7);
        const char *status = (const char *)sqlite3_column_text(stmt, 8);
        int urgente = sqlite3_column_int(stmt, 9);
        int escrito;

        if (repo_json_escapar(dataSolicitacaoJson, sizeof(dataSolicitacaoJson), dataSolic) == 0 ||
            repo_json_escapar(dataResultadoJson, sizeof(dataResultadoJson), dataResul) == 0 ||
            repo_json_escapar(resultadoJson, sizeof(resultadoJson), resultado) == 0 ||
            repo_json_escapar(statusJson, sizeof(statusJson), status) == 0 ||
            repo_json_escapar(pacienteNomeJson, sizeof(pacienteNomeJson), (const char *)sqlite3_column_text(stmt, 10)) == 0 ||
            repo_json_escapar(medicoNomeJson, sizeof(medicoNomeJson), (const char *)sqlite3_column_text(stmt, 11)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"id\":%d,\"pacienteId\":%d,\"medicoId\":%d,\"prontuarioId\":%d,"
                           "\"pacienteNome\":%s,\"medicoNome\":%s,"
                           "\"tipoExame\":%d,\"dataSolicitacao\":%s,\"dataResultado\":%s,"
                           "\"resultado\":%s,\"status\":%s,\"urgente\":%d}",
                           primeiro ? "" : ",",
                           id, pacienteId, medicoId, prontuarioId,
                           pacienteNomeJson, medicoNomeJson, tipoExame,
                           dataSolicitacaoJson, dataResultadoJson, resultadoJson, statusJson,
                           urgente);

        if (escrito < 0 || escrito >= (int)sizeof(objeto))
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        if (repo_json_anexar(buffer, tamanho, &usado, objeto) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        primeiro = 0;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);

    if (repo_json_anexar(buffer, tamanho, &usado, "]") == 0)
    {
        return 0;
    }

    return 1;
}

int exame_repo_listar_por_paciente_json(int paciente_id, char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, paciente_id, medico_id, prontuario_id, tipo_exame, "
        "data_solicitacao, data_resultado, resultado, status, urgente "
        "FROM exames WHERE paciente_id = ? AND ativo = 1 AND vigente = 1 ORDER BY id;";
    int usado = 0;
    int primeiro = 1;

    if (buffer == NULL || tamanho <= 0 || paciente_id <= 0)
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_int(stmt, 1, paciente_id);

    buffer[0] = '\0';

    if (repo_json_anexar(buffer, tamanho, &usado, "[") == 0)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char dataSolicitacaoJson[32];
        char dataResultadoJson[32];
        char resultadoJson[640];
        char statusJson[48];
        char objeto[1024];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int medicoId = sqlite3_column_int(stmt, 2);
        int prontuarioId = sqlite3_column_int(stmt, 3);
        int tipoExame = sqlite3_column_int(stmt, 4);
        const char *dataSolic = (const char *)sqlite3_column_text(stmt, 5);
        const char *dataResul = (const char *)sqlite3_column_text(stmt, 6);
        const char *resultado = (const char *)sqlite3_column_text(stmt, 7);
        const char *status = (const char *)sqlite3_column_text(stmt, 8);
        int urgente = sqlite3_column_int(stmt, 9);
        int escrito;

        if (repo_json_escapar(dataSolicitacaoJson, sizeof(dataSolicitacaoJson), dataSolic) == 0 ||
            repo_json_escapar(dataResultadoJson, sizeof(dataResultadoJson), dataResul) == 0 ||
            repo_json_escapar(resultadoJson, sizeof(resultadoJson), resultado) == 0 ||
            repo_json_escapar(statusJson, sizeof(statusJson), status) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"id\":%d,\"pacienteId\":%d,\"medicoId\":%d,\"prontuarioId\":%d,"
                           "\"tipoExame\":%d,\"dataSolicitacao\":%s,\"dataResultado\":%s,"
                           "\"resultado\":%s,\"status\":%s,\"urgente\":%d}",
                           primeiro ? "" : ",",
                           id, pacienteId, medicoId, prontuarioId, tipoExame,
                           dataSolicitacaoJson, dataResultadoJson, resultadoJson, statusJson,
                           urgente);

        if (escrito < 0 || escrito >= (int)sizeof(objeto))
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        if (repo_json_anexar(buffer, tamanho, &usado, objeto) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        primeiro = 0;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);

    if (repo_json_anexar(buffer, tamanho, &usado, "]") == 0)
    {
        return 0;
    }

    return 1;
}

/* Posicao do status na linha do tempo do exame; -1 se desconhecido. */
static int ordemStatus(const char *s)
{
    if (s == NULL)
        return -1;
    if (strcmp(s, "SOLICITADO") == 0)
        return 0;
    if (strcmp(s, "AUTORIZADO") == 0)
        return 1;
    if (strcmp(s, "COLETADO") == 0)
        return 2;
    if (strcmp(s, "EM_ANALISE") == 0)
        return 3;
    if (strcmp(s, "CONCLUIDO") == 0)
        return 4;
    if (strcmp(s, "CANCELADO") == 0)
        return 5;
    return -1;
}

/* Le o status atual do exame ativo em 'destino'. 1 se encontrou. */
static int statusAtual(int id, char *destino, int tam)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "SELECT status FROM exames WHERE id = ? AND ativo = 1 AND vigente = 1;";
    int ok = 0;

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *st = (const char *)sqlite3_column_text(stmt, 0);
        snprintf(destino, (size_t)tam, "%s", st != NULL ? st : "");
        ok = 1;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return ok;
}

int exame_repo_atualizar_status(int id, const char *novo_status)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "UPDATE exames SET status = ? WHERE id = ? AND ativo = 1 AND vigente = 1;";
    char atual[32];
    int ok = 0;

    if (statusAtual(id, atual, sizeof(atual)) == 0)
    {
        return 0;
    }

    /* So avanca um passo na linha do tempo, ate EM_ANALISE. A conclusao se da
     * pelo registro de resultado; o cancelamento por rota propria. */
    if (ordemStatus(novo_status) < 1 || ordemStatus(novo_status) > 3 ||
        ordemStatus(novo_status) != ordemStatus(atual) + 1)
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_text(stmt, 1, novo_status, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, id);
    ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;

    sqlite3_finalize(stmt);
    db_fechar(db);
    return ok ? 1 : 0;
}

int exame_repo_registrar_resultado(int id, const char *resultado, int critico)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE exames SET resultado = ?, resultado_critico = ?, "
        "status = 'CONCLUIDO', data_resultado = datetime('now') "
        "WHERE id = ? AND ativo = 1 AND vigente = 1;";
    char atual[32];
    int criticoFinal = critico ? 1 : 0;
    int ok = 0;

    if (resultado == NULL || resultado[0] == '\0')
    {
        return 0;
    }

    /* Resultado so apos a coleta (COLETADO ou EM_ANALISE). */
    if (statusAtual(id, atual, sizeof(atual)) == 0 ||
        (strcmp(atual, "COLETADO") != 0 && strcmp(atual, "EM_ANALISE") != 0))
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db,
            "SELECT COUNT(*) FROM exame_resultados_analitos "
            "WHERE exame_id = ? AND fora_referencia = 1;",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, id);
        if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_int(stmt, 0) > 0)
        {
            criticoFinal = 1;
        }
        sqlite3_finalize(stmt);
        stmt = NULL;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_text(stmt, 1, resultado, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, criticoFinal);
    sqlite3_bind_int(stmt, 3, id);
    ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;

    sqlite3_finalize(stmt);
    db_fechar(db);
    return ok ? 1 : 0;
}

int exame_repo_registrar_resultado_analito(int exame_id, int analito_id,
                                           double valor_numerico,
                                           const char *valor_texto,
                                           const char *observacao)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    char atual[32];
    double refMin = 0;
    double refMax = 0;
    int foraReferencia = 0;
    int ok = 0;

    if (exame_id <= 0 || analito_id <= 0)
    {
        return 0;
    }

    if (statusAtual(exame_id, atual, sizeof(atual)) == 0 ||
        (strcmp(atual, "COLETADO") != 0 && strcmp(atual, "EM_ANALISE") != 0))
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db,
            "SELECT a.valor_ref_min, a.valor_ref_max "
            "FROM exames e "
            "JOIN painel_analitos pa ON pa.tipo_exame = e.tipo_exame "
            "JOIN analitos a ON a.id = pa.analito_id "
            "WHERE e.id = ? AND e.ativo = 1 AND e.vigente = 1 "
            "AND a.id = ? AND a.ativo = 1;",
            -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, exame_id);
    sqlite3_bind_int(stmt, 2, analito_id);
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        refMin = sqlite3_column_double(stmt, 0);
        refMax = sqlite3_column_double(stmt, 1);
        if ((refMin != 0 || refMax != 0) &&
            (valor_numerico < refMin || valor_numerico > refMax))
        {
            foraReferencia = 1;
        }
        ok = 1;
    }
    sqlite3_finalize(stmt);

    if (ok == 0)
    {
        db_fechar(db);
        return 0;
    }

    if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    if (sqlite3_prepare_v2(db,
            "INSERT INTO exame_resultados_analitos "
            "(exame_id, analito_id, valor_numerico, valor_texto, fora_referencia, observacao) "
            "VALUES (?, ?, ?, ?, ?, ?) "
            "ON CONFLICT(exame_id, analito_id) DO UPDATE SET "
            "valor_numerico = excluded.valor_numerico, "
            "valor_texto = excluded.valor_texto, "
            "fora_referencia = excluded.fora_referencia, "
            "observacao = excluded.observacao;",
            -1, &stmt, NULL) != SQLITE_OK)
    {
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, exame_id);
    sqlite3_bind_int(stmt, 2, analito_id);
    sqlite3_bind_double(stmt, 3, valor_numerico);
    sqlite3_bind_text(stmt, 4, valor_texto != NULL ? valor_texto : "", -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, foraReferencia);
    sqlite3_bind_text(stmt, 6, observacao != NULL ? observacao : "", -1, SQLITE_STATIC);
    ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);

    if (ok)
    {
        ok = recalcularCriticoEstruturado(db, exame_id) == 1;
    }

    sqlite3_exec(db, ok ? "COMMIT;" : "ROLLBACK;", NULL, NULL, NULL);
    db_fechar(db);
    return ok ? 1 : 0;
}

int exame_repo_retificar_resultado_analito(int exame_id, int analito_id,
                                           double valor_numerico,
                                           const char *valor_texto,
                                           const char *observacao,
                                           const char *justificativa)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int pac = 0, med = 0, pront = 0, tipo = 0, urgente = 0, critico = 0;
    int versao = 0, raiz = 0, novoId = 0;
    double refMin = 0;
    double refMax = 0;
    int foraReferencia = 0;
    char dataSolic[32] = "";
    char resultado[512] = "";
    char status[32] = "";
    int ok = 0;

    if (exame_id <= 0 || analito_id <= 0 ||
        justificativa == NULL || justificativa[0] == '\0')
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db,
            "SELECT paciente_id, medico_id, prontuario_id, tipo_exame, "
            "data_solicitacao, resultado, status, urgente, resultado_critico, "
            "versao, raiz_id "
            "FROM exames WHERE id = ? AND ativo = 1 AND vigente = 1;",
            -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, exame_id);
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        pac = sqlite3_column_int(stmt, 0);
        med = sqlite3_column_int(stmt, 1);
        pront = sqlite3_column_int(stmt, 2);
        tipo = sqlite3_column_int(stmt, 3);
        snprintf(dataSolic, sizeof(dataSolic), "%s", (const char *)sqlite3_column_text(stmt, 4));
        snprintf(resultado, sizeof(resultado), "%s", (const char *)sqlite3_column_text(stmt, 5));
        snprintf(status, sizeof(status), "%s", (const char *)sqlite3_column_text(stmt, 6));
        urgente = sqlite3_column_int(stmt, 7);
        critico = sqlite3_column_int(stmt, 8);
        versao = sqlite3_column_int(stmt, 9);
        raiz = sqlite3_column_int(stmt, 10);
    }
    sqlite3_finalize(stmt);

    if (pac == 0 || strcmp(status, "CONCLUIDO") != 0)
    {
        db_fechar(db);
        return 0;
    }

    if (sqlite3_prepare_v2(db,
            "SELECT a.valor_ref_min, a.valor_ref_max "
            "FROM painel_analitos pa "
            "JOIN analitos a ON a.id = pa.analito_id "
            "WHERE pa.tipo_exame = ? AND a.id = ? AND a.ativo = 1;",
            -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, tipo);
    sqlite3_bind_int(stmt, 2, analito_id);
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        refMin = sqlite3_column_double(stmt, 0);
        refMax = sqlite3_column_double(stmt, 1);
        if ((refMin != 0 || refMax != 0) &&
            (valor_numerico < refMin || valor_numerico > refMax))
        {
            foraReferencia = 1;
        }
        ok = 1;
    }
    sqlite3_finalize(stmt);

    if (ok == 0)
    {
        db_fechar(db);
        return 0;
    }

    if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    if (sqlite3_prepare_v2(db,
            "INSERT INTO exames "
            "(paciente_id, medico_id, prontuario_id, tipo_exame, data_solicitacao, "
            "data_resultado, resultado, status, urgente, resultado_critico, "
            "versao, raiz_id, vigente, justificativa, ativo) "
            "VALUES (?, ?, ?, ?, ?, datetime('now'), ?, 'CONCLUIDO', ?, ?, ?, ?, 1, ?, 1);",
            -1, &stmt, NULL) != SQLITE_OK)
    {
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, pac);
    sqlite3_bind_int(stmt, 2, med);
    sqlite3_bind_int(stmt, 3, pront);
    sqlite3_bind_int(stmt, 4, tipo);
    sqlite3_bind_text(stmt, 5, dataSolic, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, resultado, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 7, urgente);
    sqlite3_bind_int(stmt, 8, critico);
    sqlite3_bind_int(stmt, 9, versao + 1);
    sqlite3_bind_int(stmt, 10, raiz);
    sqlite3_bind_text(stmt, 11, justificativa, -1, SQLITE_STATIC);
    ok = sqlite3_step(stmt) == SQLITE_DONE;
    if (ok)
    {
        novoId = (int)sqlite3_last_insert_rowid(db);
    }
    sqlite3_finalize(stmt);

    if (ok && sqlite3_prepare_v2(db,
            "INSERT INTO exame_resultados_analitos "
            "(exame_id, analito_id, valor_numerico, valor_texto, fora_referencia, observacao) "
            "SELECT ?, analito_id, valor_numerico, valor_texto, fora_referencia, observacao "
            "FROM exame_resultados_analitos WHERE exame_id = ? AND analito_id != ?;",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, novoId);
        sqlite3_bind_int(stmt, 2, exame_id);
        sqlite3_bind_int(stmt, 3, analito_id);
        ok = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
    }
    else
    {
        ok = 0;
    }

    if (ok && sqlite3_prepare_v2(db,
            "INSERT INTO exame_resultados_analitos "
            "(exame_id, analito_id, valor_numerico, valor_texto, fora_referencia, observacao) "
            "VALUES (?, ?, ?, ?, ?, ?);",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, novoId);
        sqlite3_bind_int(stmt, 2, analito_id);
        sqlite3_bind_double(stmt, 3, valor_numerico);
        sqlite3_bind_text(stmt, 4, valor_texto != NULL ? valor_texto : "", -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 5, foraReferencia);
        sqlite3_bind_text(stmt, 6, observacao != NULL ? observacao : "", -1, SQLITE_STATIC);
        ok = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
    }
    else
    {
        ok = 0;
    }

    if (ok && sqlite3_prepare_v2(db,
            "UPDATE exames SET vigente = 0 WHERE id = ?;",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, exame_id);
        ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;
        sqlite3_finalize(stmt);
    }
    else
    {
        ok = 0;
    }

    if (ok)
    {
        ok = recalcularCriticoEstruturado(db, novoId) == 1;
    }

    sqlite3_exec(db, ok ? "COMMIT;" : "ROLLBACK;", NULL, NULL, NULL);
    db_fechar(db);
    return ok ? 1 : 0;
}

/* Lista os resultados estruturados de um exame. 'paciente_id' > 0 restringe ao
 * dono do exame (portal do paciente); <= 0 nao filtra (uso interno/equipe). */
static int listar_resultados_analito(int exame_id, int paciente_id,
                                     char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int usado = 0;
    int primeiro = 1;
    int existe;

    if (exame_id <= 0 || buffer == NULL || tamanho <= 0)
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    /* Confirma que o exame existe (e, no portal, que e do paciente). */
    if (paciente_id > 0)
    {
        if (sqlite3_prepare_v2(db,
                "SELECT 1 FROM exames WHERE id = ? AND paciente_id = ? AND ativo = 1;",
                -1, &stmt, NULL) != SQLITE_OK)
        {
            db_fechar(db);
            return 0;
        }
        sqlite3_bind_int(stmt, 1, exame_id);
        sqlite3_bind_int(stmt, 2, paciente_id);
    }
    else if (sqlite3_prepare_v2(db,
            "SELECT 1 FROM exames WHERE id = ? AND ativo = 1;",
            -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    else
    {
        sqlite3_bind_int(stmt, 1, exame_id);
    }
    existe = sqlite3_step(stmt) == SQLITE_ROW;
    sqlite3_finalize(stmt);
    if (existe == 0)
    {
        db_fechar(db);
        return 0;
    }

    if (sqlite3_prepare_v2(db,
            "SELECT a.id, a.codigo, a.nome, a.unidade, a.valor_ref_min, a.valor_ref_max, "
            "er.valor_numerico, er.valor_texto, er.fora_referencia, er.observacao, "
            "COALESCE(pa.ordem, 0) "
            "FROM exame_resultados_analitos er "
            "JOIN analitos a ON a.id = er.analito_id "
            "JOIN exames e ON e.id = er.exame_id "
            "LEFT JOIN painel_analitos pa ON pa.tipo_exame = e.tipo_exame AND pa.analito_id = a.id "
            "WHERE er.exame_id = ? "
            "ORDER BY COALESCE(pa.ordem, 0), a.codigo;",
            -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, exame_id);

    buffer[0] = '\0';
    if (repo_json_anexar(buffer, tamanho, &usado, "[") == 0)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char codigoJson[64];
        char nomeJson[128];
        char unidadeJson[48];
        char valorTextoJson[64];
        char observacaoJson[160];
        char objeto[640];
        int analitoId = sqlite3_column_int(stmt, 0);
        double refMin = sqlite3_column_double(stmt, 4);
        double refMax = sqlite3_column_double(stmt, 5);
        double valor = sqlite3_column_double(stmt, 6);
        int fora = sqlite3_column_int(stmt, 8);
        int ordem = sqlite3_column_int(stmt, 10);
        int escrito;

        if (repo_json_escapar(codigoJson, sizeof(codigoJson), (const char *)sqlite3_column_text(stmt, 1)) == 0 ||
            repo_json_escapar(nomeJson, sizeof(nomeJson), (const char *)sqlite3_column_text(stmt, 2)) == 0 ||
            repo_json_escapar(unidadeJson, sizeof(unidadeJson), (const char *)sqlite3_column_text(stmt, 3)) == 0 ||
            repo_json_escapar(valorTextoJson, sizeof(valorTextoJson), (const char *)sqlite3_column_text(stmt, 7)) == 0 ||
            repo_json_escapar(observacaoJson, sizeof(observacaoJson), (const char *)sqlite3_column_text(stmt, 9)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"analitoId\":%d,\"codigo\":%s,\"nome\":%s,\"unidade\":%s,"
                           "\"refMin\":%g,\"refMax\":%g,\"valor\":%g,\"valorTexto\":%s,"
                           "\"foraReferencia\":%d,\"observacao\":%s,\"ordem\":%d}",
                           primeiro ? "" : ",",
                           analitoId, codigoJson, nomeJson, unidadeJson,
                           refMin, refMax, valor, valorTextoJson, fora, observacaoJson, ordem);

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

int exame_repo_listar_resultados_analito_json(int exame_id, char *buffer, int tamanho)
{
    return listar_resultados_analito(exame_id, 0, buffer, tamanho);
}

int exame_repo_listar_resultados_analito_do_paciente_json(int exame_id,
                                                          int paciente_id,
                                                          char *buffer,
                                                          int tamanho)
{
    if (paciente_id <= 0)
    {
        return 0;
    }
    return listar_resultados_analito(exame_id, paciente_id, buffer, tamanho);
}

int exame_repo_cancelar(int id, const char *motivo)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE exames SET status = 'CANCELADO', motivo_cancelamento = ?, "
        "ativo = 0 WHERE id = ? AND ativo = 1 AND vigente = 1 AND status != 'CONCLUIDO';";
    int ok = 0;

    /* Cancelamento exige motivo; exame concluido nao se cancela. */
    if (motivo == NULL || motivo[0] == '\0')
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_text(stmt, 1, motivo, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, id);
    ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;

    sqlite3_finalize(stmt);
    db_fechar(db);
    return ok ? 1 : 0;
}

int exame_repo_desativar(int id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "UPDATE exames SET ativo = 0 WHERE id = ? AND ativo = 1 AND vigente = 1;";
    int alteradas;

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    sqlite3_finalize(stmt);
    alteradas = sqlite3_changes(db);
    db_fechar(db);

    return alteradas > 0 ? 1 : 0;
}

int exame_repo_contar_por_medico(int medico_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT COUNT(*) FROM exames WHERE medico_id = ? AND ativo = 1 AND vigente = 1;";
    int total = -1;

    if (medico_id <= 0)
    {
        return -1;
    }

    if (db_abrir(&db) == 0)
    {
        return -1;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return -1;
    }

    sqlite3_bind_int(stmt, 1, medico_id);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        total = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return total;
}

int exame_repo_contar_ativos(void)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "SELECT COUNT(*) FROM exames WHERE ativo = 1 AND vigente = 1;";
    int total = -1;

    if (db_abrir(&db) == 0)
    {
        return -1;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return -1;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        total = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return total;
}
