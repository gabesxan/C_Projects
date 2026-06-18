#include "exame_repository.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>
#include <string.h>

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

    if (sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, NULL) != SQLITE_OK)
    {
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
    sqlite3_finalize(stmt);

    if (ok)
    {
        sqlite3_stmt *upd = NULL;
        if (sqlite3_prepare_v2(db, "UPDATE exames SET vigente = 0 WHERE id = ?;",
                               -1, &upd, NULL) == SQLITE_OK)
        {
            sqlite3_bind_int(upd, 1, id);
            sqlite3_step(upd);
            sqlite3_finalize(upd);
        }
    }

    db_fechar(db);
    return ok ? 1 : 0;
}

int exame_repo_listar_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, paciente_id, medico_id, prontuario_id, tipo_exame, "
        "data_solicitacao, data_resultado, resultado, status, urgente "
        "FROM exames WHERE ativo = 1 AND vigente = 1 ORDER BY id;";
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

int exame_repo_listar_por_medico_json(int medico_id, char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, paciente_id, medico_id, prontuario_id, tipo_exame, "
        "data_solicitacao, data_resultado, resultado, status, urgente "
        "FROM exames WHERE medico_id = ? AND ativo = 1 AND vigente = 1 ORDER BY id;";
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

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_text(stmt, 1, resultado, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, critico ? 1 : 0);
    sqlite3_bind_int(stmt, 3, id);
    ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;

    sqlite3_finalize(stmt);
    db_fechar(db);
    return ok ? 1 : 0;
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
