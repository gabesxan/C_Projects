#include "internacao_repository.h"
#include "leito_repository.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>
#include <string.h>

int internacao_repo_criar(int paciente_id, int ala_id, int leito_id,
                          const char *data_entrada, const char *responsavel)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO internacoes "
        "(paciente_id, ala_id, leito_id, data_entrada, data_alta, status, "
        "responsavel) VALUES (?, ?, ?, ?, '', 'INTERNADO', ?);";
    int ok;

    if (paciente_id <= 0 || ala_id <= 0 || leito_id <= 0)
    {
        return 0;
    }

    if (data_entrada == NULL || data_entrada[0] == '\0')
    {
        return 0;
    }

    /* Ocupa o leito de forma atomica: so internou se o leito estava DISPONIVEL
     * (garante 1 paciente por leito). Em falha de insercao, libera de volta. */
    if (leito_repo_ocupar(leito_id, paciente_id) == 0)
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        leito_repo_liberar(leito_id, "rollback");
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        leito_repo_liberar(leito_id, "rollback");
        return 0;
    }

    sqlite3_bind_int(stmt, 1, paciente_id);
    sqlite3_bind_int(stmt, 2, ala_id);
    sqlite3_bind_int(stmt, 3, leito_id);
    sqlite3_bind_text(stmt, 4, data_entrada, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, responsavel != NULL ? responsavel : "",
                      -1, SQLITE_STATIC);

    ok = sqlite3_step(stmt) == SQLITE_DONE;

    sqlite3_finalize(stmt);
    db_fechar(db);

    if (ok == 0)
    {
        leito_repo_liberar(leito_id, "rollback");
        return 0;
    }

    return 1;
}

int internacao_repo_listar_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, paciente_id, ala_id, leito_id, data_entrada, data_alta, "
        "status, responsavel FROM internacoes ORDER BY id;";
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
        char dataEntradaJson[32];
        char dataAltaJson[32];
        char statusJson[48];
        char responsavelJson[256];
        char objeto[640];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int alaId = sqlite3_column_int(stmt, 2);
        int leitoId = sqlite3_column_int(stmt, 3);
        const char *dataEntrada = (const char *)sqlite3_column_text(stmt, 4);
        const char *dataAlta = (const char *)sqlite3_column_text(stmt, 5);
        const char *status = (const char *)sqlite3_column_text(stmt, 6);
        const char *responsavel = (const char *)sqlite3_column_text(stmt, 7);
        int escrito;

        if (repo_json_escapar(dataEntradaJson, sizeof(dataEntradaJson), dataEntrada) == 0 ||
            repo_json_escapar(dataAltaJson, sizeof(dataAltaJson), dataAlta) == 0 ||
            repo_json_escapar(statusJson, sizeof(statusJson), status) == 0 ||
            repo_json_escapar(responsavelJson, sizeof(responsavelJson), responsavel) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"id\":%d,\"pacienteId\":%d,\"alaId\":%d,\"leitoId\":%d,"
                           "\"dataEntrada\":%s,\"dataAlta\":%s,\"status\":%s,"
                           "\"responsavel\":%s}",
                           primeiro ? "" : ",",
                           id, pacienteId, alaId, leitoId, dataEntradaJson, dataAltaJson,
                           statusJson, responsavelJson);

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

/* Le o leito_id de uma internacao com o status informado. 0 se nao houver. */
static int leitoDaInternacao(int id, const char *statusEsperado)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT leito_id FROM internacoes WHERE id = ? AND status = ?;";
    int leito = 0;

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
    sqlite3_bind_text(stmt, 2, statusEsperado, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        leito = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return leito;
}

int internacao_repo_dar_alta(int id, const char *data_alta, const char *resumo,
                             const char *diagnostico, const char *orientacoes)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE internacoes SET status = 'ALTA', data_alta = ?, "
        "resumo_alta = ?, diagnostico_final = ?, orientacoes = ? "
        "WHERE id = ? AND status = 'INTERNADO';";
    int leito;
    int ok;

    if (data_alta == NULL || data_alta[0] == '\0')
    {
        return 0;
    }

    /* Alta deve conter resumo clinico e diagnostico final. */
    if (resumo == NULL || resumo[0] == '\0' ||
        diagnostico == NULL || diagnostico[0] == '\0')
    {
        return 0;
    }

    leito = leitoDaInternacao(id, "INTERNADO");
    if (leito == 0)
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

    sqlite3_bind_text(stmt, 1, data_alta, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, resumo, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, diagnostico, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, orientacoes != NULL ? orientacoes : "",
                      -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, id);

    ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;

    sqlite3_finalize(stmt);
    db_fechar(db);

    /* Alta encerra a internacao e o leito vai para HIGIENIZACAO. */
    if (ok)
    {
        leito_repo_liberar(leito, "alta");
        return 1;
    }

    return 0;
}

int internacao_repo_transferir(int id, int novo_leito_id, const char *data,
                               const char *responsavel)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int leitoOrigem;
    int novaAla = 0;
    int ok;

    if (novo_leito_id <= 0 || data == NULL || data[0] == '\0')
    {
        return 0;
    }

    leitoOrigem = leitoDaInternacao(id, "INTERNADO");
    if (leitoOrigem == 0 || leitoOrigem == novo_leito_id)
    {
        return 0;
    }

    /* Descobre a ala do leito destino. */
    if (db_abrir(&db) == 0)
    {
        return 0;
    }
    if (sqlite3_prepare_v2(db,
                           "SELECT ala_id FROM leitos WHERE id = ? AND ativo = 1;",
                           -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, novo_leito_id);
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            novaAla = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    db_fechar(db);

    if (novaAla == 0)
    {
        return 0;
    }

    /* Ocupa o destino (so se DISPONIVEL); depois libera a origem. */
    leitoOrigem = leitoDaInternacao(id, "INTERNADO");
    {
        int pacienteId = 0;
        if (db_abrir(&db) == 0)
            return 0;
        if (sqlite3_prepare_v2(db,
                               "SELECT paciente_id FROM internacoes WHERE id = ?;",
                               -1, &stmt, NULL) == SQLITE_OK)
        {
            sqlite3_bind_int(stmt, 1, id);
            if (sqlite3_step(stmt) == SQLITE_ROW)
            {
                pacienteId = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }
        db_fechar(db);

        if (leito_repo_ocupar(novo_leito_id, pacienteId) == 0)
        {
            return 0;
        }
    }

    leito_repo_liberar(leitoOrigem, responsavel);

    /* Atualiza a internacao e registra a transferencia. */
    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db,
                           "UPDATE internacoes SET leito_id = ?, ala_id = ? WHERE id = ?;",
                           -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, novo_leito_id);
    sqlite3_bind_int(stmt, 2, novaAla);
    sqlite3_bind_int(stmt, 3, id);
    ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);

    if (ok && sqlite3_prepare_v2(db,
                                 "INSERT INTO transferencias "
                                 "(internacao_id, leito_origem, leito_destino, data, responsavel) "
                                 "VALUES (?, ?, ?, ?, ?);",
                                 -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, id);
        sqlite3_bind_int(stmt, 2, leitoOrigem);
        sqlite3_bind_int(stmt, 3, novo_leito_id);
        sqlite3_bind_text(stmt, 4, data, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, responsavel != NULL ? responsavel : "",
                          -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    db_fechar(db);
    return ok ? 1 : 0;
}

int internacao_repo_distribuicao_por_status_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT status, COUNT(*) FROM internacoes "
        "GROUP BY status ORDER BY status;";
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
        char statusJson[48];
        char objeto[128];
        int total = sqlite3_column_int(stmt, 1);
        int escrito;

        if (repo_json_escapar(statusJson, sizeof(statusJson),
                              (const char *)sqlite3_column_text(stmt, 0)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"status\":%s,\"total\":%d}",
                           primeiro ? "" : ",", statusJson, total);

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

int internacao_repo_contar_ativos(void)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT COUNT(*) FROM internacoes WHERE status = 'INTERNADO';";
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
