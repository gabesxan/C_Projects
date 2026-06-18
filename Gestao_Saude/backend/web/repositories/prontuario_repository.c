#include "prontuario_repository.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>
#include <string.h>

int prontuario_repo_criar(int paciente_id, int medico_id, const char *data,
                          const char *observacoes, const char *diagnostico,
                          const char *conduta, int alerta_importante)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO prontuarios "
        "(paciente_id, medico_id, data, observacoes, diagnostico, conduta, "
        "alerta_importante, ativo) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, 1);";

    if (paciente_id <= 0 || medico_id <= 0)
    {
        return 0;
    }

    if (data == NULL || data[0] == '\0')
    {
        return 0;
    }

    if (observacoes == NULL || diagnostico == NULL || conduta == NULL)
    {
        return 0;
    }

    /* Nao finaliza atendimento sem conduta (regra clinica). */
    if (conduta[0] == '\0')
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
    sqlite3_bind_text(stmt, 3, data, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, observacoes, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, diagnostico, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, conduta, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 7, alerta_importante);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    sqlite3_finalize(stmt);

    /* A versao original e raiz de si mesma (raiz_id = id). */
    {
        sqlite3_int64 novo = sqlite3_last_insert_rowid(db);
        sqlite3_exec(db, "UPDATE prontuarios SET raiz_id = id WHERE raiz_id = 0;",
                     NULL, NULL, NULL);
        (void)novo;
    }

    db_fechar(db);
    return 1;
}

int prontuario_repo_retificar(int id, const char *data, const char *observacoes,
                              const char *diagnostico, const char *conduta,
                              int alerta_importante, const char *justificativa)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sqlBusca =
        "SELECT paciente_id, medico_id, versao, raiz_id FROM prontuarios "
        "WHERE id = ? AND vigente = 1;";
    const char *sqlInsert =
        "INSERT INTO prontuarios "
        "(paciente_id, medico_id, data, observacoes, diagnostico, conduta, "
        "alerta_importante, versao, raiz_id, vigente, justificativa, ativo) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, 1, ?, 1);";
    int pacienteId = 0, medicoId = 0, versao = 0, raiz = 0;
    int ok = 0;

    if (data == NULL || data[0] == '\0' || diagnostico == NULL ||
        conduta == NULL || conduta[0] == '\0' ||
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
        pacienteId = sqlite3_column_int(stmt, 0);
        medicoId = sqlite3_column_int(stmt, 1);
        versao = sqlite3_column_int(stmt, 2);
        raiz = sqlite3_column_int(stmt, 3);
    }
    sqlite3_finalize(stmt);

    if (pacienteId == 0)
    {
        db_fechar(db); /* id inexistente ou ja superado */
        return 0;
    }

    /* Insere a nova versao (vigente). */
    if (sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, pacienteId);
    sqlite3_bind_int(stmt, 2, medicoId);
    sqlite3_bind_text(stmt, 3, data, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, observacoes != NULL ? observacoes : "", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, diagnostico, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, conduta, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 7, alerta_importante);
    sqlite3_bind_int(stmt, 8, versao + 1);
    sqlite3_bind_int(stmt, 9, raiz);
    sqlite3_bind_text(stmt, 10, justificativa, -1, SQLITE_STATIC);
    ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);

    /* Marca a versao anterior como superada (preservada, nunca apagada). */
    if (ok)
    {
        sqlite3_stmt *upd = NULL;
        if (sqlite3_prepare_v2(db,
                "UPDATE prontuarios SET vigente = 0 WHERE id = ?;",
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

int prontuario_repo_listar_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, paciente_id, medico_id, data, observacoes, diagnostico, "
        "conduta, alerta_importante, versao, vigente, justificativa "
        "FROM prontuarios WHERE vigente = 1 ORDER BY id;";
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
        char dataJson[32];
        char observacoesJson[640];
        char diagnosticoJson[448];
        char condutaJson[448];
        char justificativaJson[448];
        char objeto[2304];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int medicoId = sqlite3_column_int(stmt, 2);
        const char *data = (const char *)sqlite3_column_text(stmt, 3);
        const char *observacoes = (const char *)sqlite3_column_text(stmt, 4);
        const char *diagnostico = (const char *)sqlite3_column_text(stmt, 5);
        const char *conduta = (const char *)sqlite3_column_text(stmt, 6);
        int alerta = sqlite3_column_int(stmt, 7);
        int versao = sqlite3_column_int(stmt, 8);
        int vigente = sqlite3_column_int(stmt, 9);
        const char *justificativa = (const char *)sqlite3_column_text(stmt, 10);
        int escrito;

        if (repo_json_escapar(dataJson, sizeof(dataJson), data) == 0 ||
            repo_json_escapar(observacoesJson, sizeof(observacoesJson), observacoes) == 0 ||
            repo_json_escapar(diagnosticoJson, sizeof(diagnosticoJson), diagnostico) == 0 ||
            repo_json_escapar(condutaJson, sizeof(condutaJson), conduta) == 0 ||
            repo_json_escapar(justificativaJson, sizeof(justificativaJson), justificativa) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"id\":%d,\"pacienteId\":%d,\"medicoId\":%d,\"data\":%s,"
                           "\"observacoes\":%s,\"diagnostico\":%s,\"conduta\":%s,"
                           "\"alertaImportante\":%d,\"versao\":%d,\"vigente\":%d,"
                           "\"justificativa\":%s}",
                           primeiro ? "" : ",",
                           id, pacienteId, medicoId, dataJson, observacoesJson,
                           diagnosticoJson, condutaJson, alerta, versao, vigente,
                           justificativaJson);

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

int prontuario_repo_listar_por_medico_json(int medico_id, char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, paciente_id, medico_id, data, observacoes, diagnostico, "
        "conduta, alerta_importante, versao, vigente, justificativa "
        "FROM prontuarios WHERE medico_id = ? AND vigente = 1 ORDER BY id;";
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
        char dataJson[32];
        char observacoesJson[640];
        char diagnosticoJson[448];
        char condutaJson[448];
        char justificativaJson[448];
        char objeto[2304];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int medicoId = sqlite3_column_int(stmt, 2);
        const char *data = (const char *)sqlite3_column_text(stmt, 3);
        const char *observacoes = (const char *)sqlite3_column_text(stmt, 4);
        const char *diagnostico = (const char *)sqlite3_column_text(stmt, 5);
        const char *conduta = (const char *)sqlite3_column_text(stmt, 6);
        int alerta = sqlite3_column_int(stmt, 7);
        int versao = sqlite3_column_int(stmt, 8);
        int vigente = sqlite3_column_int(stmt, 9);
        const char *justificativa = (const char *)sqlite3_column_text(stmt, 10);
        int escrito;

        if (repo_json_escapar(dataJson, sizeof(dataJson), data) == 0 ||
            repo_json_escapar(observacoesJson, sizeof(observacoesJson), observacoes) == 0 ||
            repo_json_escapar(diagnosticoJson, sizeof(diagnosticoJson), diagnostico) == 0 ||
            repo_json_escapar(condutaJson, sizeof(condutaJson), conduta) == 0 ||
            repo_json_escapar(justificativaJson, sizeof(justificativaJson), justificativa) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"id\":%d,\"pacienteId\":%d,\"medicoId\":%d,\"data\":%s,"
                           "\"observacoes\":%s,\"diagnostico\":%s,\"conduta\":%s,"
                           "\"alertaImportante\":%d,\"versao\":%d,\"vigente\":%d,"
                           "\"justificativa\":%s}",
                           primeiro ? "" : ",",
                           id, pacienteId, medicoId, dataJson, observacoesJson,
                           diagnosticoJson, condutaJson, alerta, versao, vigente,
                           justificativaJson);

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

int prontuario_repo_listar_por_paciente_json(int paciente_id, char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, paciente_id, medico_id, data, observacoes, diagnostico, "
        "conduta, alerta_importante, versao, vigente, justificativa "
        "FROM prontuarios WHERE paciente_id = ? ORDER BY raiz_id, versao;";
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
        char dataJson[32];
        char observacoesJson[640];
        char diagnosticoJson[448];
        char condutaJson[448];
        char justificativaJson[448];
        char objeto[2304];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int medicoId = sqlite3_column_int(stmt, 2);
        const char *data = (const char *)sqlite3_column_text(stmt, 3);
        const char *observacoes = (const char *)sqlite3_column_text(stmt, 4);
        const char *diagnostico = (const char *)sqlite3_column_text(stmt, 5);
        const char *conduta = (const char *)sqlite3_column_text(stmt, 6);
        int alerta = sqlite3_column_int(stmt, 7);
        int versao = sqlite3_column_int(stmt, 8);
        int vigente = sqlite3_column_int(stmt, 9);
        const char *justificativa = (const char *)sqlite3_column_text(stmt, 10);
        int escrito;

        if (repo_json_escapar(dataJson, sizeof(dataJson), data) == 0 ||
            repo_json_escapar(observacoesJson, sizeof(observacoesJson), observacoes) == 0 ||
            repo_json_escapar(diagnosticoJson, sizeof(diagnosticoJson), diagnostico) == 0 ||
            repo_json_escapar(condutaJson, sizeof(condutaJson), conduta) == 0 ||
            repo_json_escapar(justificativaJson, sizeof(justificativaJson), justificativa) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"id\":%d,\"pacienteId\":%d,\"medicoId\":%d,\"data\":%s,"
                           "\"observacoes\":%s,\"diagnostico\":%s,\"conduta\":%s,"
                           "\"alertaImportante\":%d,\"versao\":%d,\"vigente\":%d,"
                           "\"justificativa\":%s}",
                           primeiro ? "" : ",",
                           id, pacienteId, medicoId, dataJson, observacoesJson,
                           diagnosticoJson, condutaJson, alerta, versao, vigente,
                           justificativaJson);

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

int prontuario_repo_contar_por_medico(int medico_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT COUNT(*) FROM prontuarios WHERE medico_id = ? AND vigente = 1;";
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

int prontuario_repo_contar_ativos(void)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "SELECT COUNT(*) FROM prontuarios WHERE vigente = 1;";
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
