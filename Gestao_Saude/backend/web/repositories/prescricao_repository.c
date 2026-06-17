#include "prescricao_repository.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Copia 's' em minusculas para 'out'. */
static void emMinusculas(const char *s, char *out, int tam)
{
    int i = 0;
    for (; s != NULL && s[i] != '\0' && i < tam - 1; i++)
    {
        out[i] = (char)tolower((unsigned char)s[i]);
    }
    out[i] = '\0';
}

/* 1 se o paciente tem o medicamento entre suas alergias (comparacao por
 * substring, sem diferenciar maiusculas). 0 caso contrario. */
static int pacienteTemAlergia(int paciente_id, const char *medicamento)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "SELECT alergias FROM pacientes WHERE id = ?;";
    char alergiasLower[512];
    char medLower[128];
    int conflito = 0;

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

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *alergias = (const char *)sqlite3_column_text(stmt, 0);

        if (alergias != NULL && alergias[0] != '\0')
        {
            emMinusculas(alergias, alergiasLower, sizeof(alergiasLower));
            emMinusculas(medicamento, medLower, sizeof(medLower));
            if (medLower[0] != '\0' && strstr(alergiasLower, medLower) != NULL)
            {
                conflito = 1;
            }
        }
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return conflito;
}

int prescricao_repo_criar(int paciente_id, int medico_id,
                          const char *medicamento, const char *dosagem,
                          const char *frequencia, const char *via,
                          const char *duracao, const char *observacoes)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO prescricoes "
        "(paciente_id, medico_id, medicamento, dosagem, frequencia, via, "
        "duracao, observacoes, ativo) VALUES (?, ?, ?, ?, ?, ?, ?, ?, 1);";

    if (paciente_id <= 0 || medico_id <= 0)
    {
        return 0;
    }

    if (medicamento == NULL || medicamento[0] == '\0' ||
        dosagem == NULL || frequencia == NULL || observacoes == NULL)
    {
        return 0;
    }

    /* Verifica alergia do paciente ao medicamento antes de confirmar. */
    if (pacienteTemAlergia(paciente_id, medicamento))
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
    sqlite3_bind_text(stmt, 3, medicamento, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, dosagem, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, frequencia, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, via != NULL ? via : "", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, duracao != NULL ? duracao : "", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 8, observacoes, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return 1;
}

/* Listagem em JSON com filtro opcional por coluna ('paciente_id'/'medico_id',
 * nomes internos fixos — sem entrada do usuario). filtroCol NULL = todas. */
static int listar_filtrado_json(const char *filtroCol, int filtroVal,
                                char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    char sql[256];
    int usado = 0;
    int primeiro = 1;

    if (buffer == NULL || tamanho <= 0)
    {
        return 0;
    }

    if (filtroCol != NULL)
    {
        snprintf(sql, sizeof(sql),
                 "SELECT id, paciente_id, medico_id, medicamento, dosagem, "
                 "frequencia, observacoes, via, duracao FROM prescricoes "
                 "WHERE ativo = 1 AND %s = ? ORDER BY id;",
                 filtroCol);
    }
    else
    {
        snprintf(sql, sizeof(sql),
                 "SELECT id, paciente_id, medico_id, medicamento, dosagem, "
                 "frequencia, observacoes, via, duracao FROM prescricoes "
                 "WHERE ativo = 1 ORDER BY id;");
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

    if (filtroCol != NULL)
    {
        sqlite3_bind_int(stmt, 1, filtroVal);
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
        char medicamentoJson[256];
        char dosagemJson[96];
        char frequenciaJson[96];
        char observacoesJson[640];
        char viaJson[96];
        char duracaoJson[96];
        char objeto[1440];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int medicoId = sqlite3_column_int(stmt, 2);
        const char *medicamento = (const char *)sqlite3_column_text(stmt, 3);
        const char *dosagem = (const char *)sqlite3_column_text(stmt, 4);
        const char *frequencia = (const char *)sqlite3_column_text(stmt, 5);
        const char *observacoes = (const char *)sqlite3_column_text(stmt, 6);
        const char *via = (const char *)sqlite3_column_text(stmt, 7);
        const char *duracao = (const char *)sqlite3_column_text(stmt, 8);
        int escrito;

        if (repo_json_escapar(medicamentoJson, sizeof(medicamentoJson), medicamento) == 0 ||
            repo_json_escapar(dosagemJson, sizeof(dosagemJson), dosagem) == 0 ||
            repo_json_escapar(frequenciaJson, sizeof(frequenciaJson), frequencia) == 0 ||
            repo_json_escapar(observacoesJson, sizeof(observacoesJson), observacoes) == 0 ||
            repo_json_escapar(viaJson, sizeof(viaJson), via) == 0 ||
            repo_json_escapar(duracaoJson, sizeof(duracaoJson), duracao) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"id\":%d,\"pacienteId\":%d,\"medicoId\":%d,\"medicamento\":%s,"
                           "\"dosagem\":%s,\"frequencia\":%s,\"via\":%s,\"duracao\":%s,"
                           "\"observacoes\":%s}",
                           primeiro ? "" : ",",
                           id, pacienteId, medicoId, medicamentoJson, dosagemJson,
                           frequenciaJson, viaJson, duracaoJson, observacoesJson);

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

int prescricao_repo_listar_json(char *buffer, int tamanho)
{
    return listar_filtrado_json(NULL, 0, buffer, tamanho);
}

int prescricao_repo_listar_por_paciente_json(int paciente_id, char *buffer, int tamanho)
{
    if (paciente_id <= 0)
    {
        return 0;
    }
    return listar_filtrado_json("paciente_id", paciente_id, buffer, tamanho);
}

int prescricao_repo_listar_por_medico_json(int medico_id, char *buffer, int tamanho)
{
    if (medico_id <= 0)
    {
        return 0;
    }
    return listar_filtrado_json("medico_id", medico_id, buffer, tamanho);
}

int prescricao_repo_desativar(int id, const char *motivo)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE prescricoes SET ativo = 0, motivo_suspensao = ? "
        "WHERE id = ? AND ativo = 1;";
    int alteradas;

    /* Suspensao/cancelamento exige justificativa. */
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

int prescricao_repo_contar_ativos(void)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "SELECT COUNT(*) FROM prescricoes WHERE ativo = 1;";
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
