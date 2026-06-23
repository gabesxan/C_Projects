#include "enfermagem_repository.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>
#include <string.h>

int administracao_criar(int prescricao_id, int usuario_id,
                        const char *usuario_login, const char *observacao)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int pacienteId = 0;
    int ok = 0;

    if (prescricao_id <= 0)
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    /* Deriva o paciente da prescricao ativa. */
    if (sqlite3_prepare_v2(db,
                           "SELECT paciente_id FROM prescricoes WHERE id = ? AND ativo = 1;",
                           -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, prescricao_id);
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        pacienteId = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (pacienteId == 0)
    {
        db_fechar(db);
        return 0;
    }

    if (sqlite3_prepare_v2(db,
                           "INSERT INTO administracoes "
                           "(prescricao_id, paciente_id, usuario_id, usuario_login, observacao) "
                           "VALUES (?, ?, ?, ?, ?);",
                           -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, prescricao_id);
    sqlite3_bind_int(stmt, 2, pacienteId);
    sqlite3_bind_int(stmt, 3, usuario_id);
    sqlite3_bind_text(stmt, 4, usuario_login != NULL ? usuario_login : "", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, observacao != NULL ? observacao : "", -1, SQLITE_STATIC);
    ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);

    db_fechar(db);
    return ok ? 1 : 0;
}

int administracao_listar_por_paciente_json(int paciente_id, char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT a.id, a.prescricao_id, a.usuario_login, a.observacao, "
        "a.criado_em, p.medicamento FROM administracoes a "
        "LEFT JOIN prescricoes p ON p.id = a.prescricao_id "
        "WHERE a.paciente_id = ? ORDER BY a.id DESC;";
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
        char loginJson[128];
        char obsJson[512];
        char criadoJson[64];
        char medJson[256];
        char objeto[1100];
        int id = sqlite3_column_int(stmt, 0);
        int prescricaoId = sqlite3_column_int(stmt, 1);
        int escrito;

        if (repo_json_escapar(loginJson, sizeof(loginJson), (const char *)sqlite3_column_text(stmt, 2)) == 0 ||
            repo_json_escapar(obsJson, sizeof(obsJson), (const char *)sqlite3_column_text(stmt, 3)) == 0 ||
            repo_json_escapar(criadoJson, sizeof(criadoJson), (const char *)sqlite3_column_text(stmt, 4)) == 0 ||
            repo_json_escapar(medJson, sizeof(medJson), (const char *)sqlite3_column_text(stmt, 5)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"id\":%d,\"prescricaoId\":%d,\"medicamento\":%s,\"por\":%s,"
                           "\"observacao\":%s,\"criadoEm\":%s}",
                           primeiro ? "" : ",",
                           id, prescricaoId, medJson, loginJson, obsJson, criadoJson);

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

int evolucao_criar(int paciente_id, const char *autor_login, const char *texto,
                   const char *pressao, const char *temperatura,
                   const char *freq_cardiaca, const char *saturacao)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO evolucoes_enfermagem "
        "(paciente_id, autor_login, texto, pressao, temperatura, freq_cardiaca, "
        "saturacao) VALUES (?, ?, ?, ?, ?, ?, ?);";
    int ok = 0;

    if (paciente_id <= 0 || texto == NULL || texto[0] == '\0')
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
    sqlite3_bind_text(stmt, 2, autor_login != NULL ? autor_login : "", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, texto, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, pressao != NULL ? pressao : "", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, temperatura != NULL ? temperatura : "", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, freq_cardiaca != NULL ? freq_cardiaca : "", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, saturacao != NULL ? saturacao : "", -1, SQLITE_STATIC);
    ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);

    db_fechar(db);
    return ok ? 1 : 0;
}

int evolucao_listar_por_paciente_json(int paciente_id, char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, autor_login, texto, pressao, temperatura, freq_cardiaca, "
        "saturacao, criado_em FROM evolucoes_enfermagem "
        "WHERE paciente_id = ? ORDER BY id DESC;";
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
        char autorJson[128];
        char textoJson[1024];
        char pressaoJson[32];
        char temperaturaJson[32];
        char freqJson[32];
        char saturacaoJson[32];
        char criadoJson[64];
        char objeto[1600];
        int id = sqlite3_column_int(stmt, 0);
        int escrito;

        if (repo_json_escapar(autorJson, sizeof(autorJson), (const char *)sqlite3_column_text(stmt, 1)) == 0 ||
            repo_json_escapar(textoJson, sizeof(textoJson), (const char *)sqlite3_column_text(stmt, 2)) == 0 ||
            repo_json_escapar(pressaoJson, sizeof(pressaoJson), (const char *)sqlite3_column_text(stmt, 3)) == 0 ||
            repo_json_escapar(temperaturaJson, sizeof(temperaturaJson), (const char *)sqlite3_column_text(stmt, 4)) == 0 ||
            repo_json_escapar(freqJson, sizeof(freqJson), (const char *)sqlite3_column_text(stmt, 5)) == 0 ||
            repo_json_escapar(saturacaoJson, sizeof(saturacaoJson), (const char *)sqlite3_column_text(stmt, 6)) == 0 ||
            repo_json_escapar(criadoJson, sizeof(criadoJson), (const char *)sqlite3_column_text(stmt, 7)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"id\":%d,\"autor\":%s,\"texto\":%s,\"pressao\":%s,"
                           "\"temperatura\":%s,\"freqCardiaca\":%s,\"saturacao\":%s,"
                           "\"criadoEm\":%s}",
                           primeiro ? "" : ",",
                           id, autorJson, textoJson, pressaoJson, temperaturaJson, freqJson,
                           saturacaoJson, criadoJson);

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
