#include "financeiro_repository.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>
#include <string.h>

/* ----------------------------------------------------------------------- */
/* Convenios                                                                */
/* ----------------------------------------------------------------------- */

int convenio_criar(const char *nome)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "INSERT INTO convenios (nome, ativo) VALUES (?, 1);";
    int ok = 0;

    if (nome == NULL || nome[0] == '\0')
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

    sqlite3_bind_text(stmt, 1, nome, -1, SQLITE_STATIC);
    ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    db_fechar(db);
    return ok ? 1 : 0;
}

int convenio_listar_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, nome FROM convenios WHERE ativo = 1 ORDER BY nome;";
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
        char nomeJson[256];
        char objeto[320];
        int id = sqlite3_column_int(stmt, 0);
        int escrito;

        if (repo_json_escapar(nomeJson, sizeof(nomeJson),
                              (const char *)sqlite3_column_text(stmt, 1)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"id\":%d,\"nome\":%s}",
                           primeiro ? "" : ",", id, nomeJson);

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

int convenio_desativar(int id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "UPDATE convenios SET ativo = 0 WHERE id = ? AND ativo = 1;";
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
    ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;
    sqlite3_finalize(stmt);
    db_fechar(db);
    return ok ? 1 : 0;
}

int convenio_contar_ativos(void)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "SELECT COUNT(*) FROM convenios WHERE ativo = 1;";
    int total = -1;

    if (db_abrir(&db) == 0)
    {
        return -1;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK &&
        sqlite3_step(stmt) == SQLITE_ROW)
    {
        total = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return total;
}

/* ----------------------------------------------------------------------- */
/* Cobrancas                                                                */
/* ----------------------------------------------------------------------- */

static int status_cobranca_valido(const char *s)
{
    return s != NULL && (
        strcmp(s, "PENDENTE") == 0 || strcmp(s, "AUTORIZADA") == 0 ||
        strcmp(s, "PAGA") == 0 || strcmp(s, "GLOSADA") == 0 ||
        strcmp(s, "CANCELADA") == 0);
}

static int status_terminal(const char *s)
{
    return strcmp(s, "PAGA") == 0 || strcmp(s, "GLOSADA") == 0 ||
           strcmp(s, "CANCELADA") == 0;
}

int cobranca_criar(int paciente_id, int convenio_id, const char *forma,
                   const char *origem, const char *descricao,
                   int valor_centavos)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO cobrancas "
        "(paciente_id, convenio_id, forma, origem, descricao, valor_centavos, "
        "status) VALUES (?, ?, ?, ?, ?, ?, 'PENDENTE');";
    int ok = 0;

    if (paciente_id <= 0 || valor_centavos <= 0 || forma == NULL)
    {
        return 0;
    }

    if (strcmp(forma, "PARTICULAR") != 0 && strcmp(forma, "CONVENIO") != 0)
    {
        return 0;
    }

    /* Cobranca por convenio exige um convenio. */
    if (strcmp(forma, "CONVENIO") == 0 && convenio_id <= 0)
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
    sqlite3_bind_int(stmt, 2, strcmp(forma, "CONVENIO") == 0 ? convenio_id : 0);
    sqlite3_bind_text(stmt, 3, forma, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, origem != NULL ? origem : "", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, descricao != NULL ? descricao : "", -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, valor_centavos);
    ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    db_fechar(db);
    return ok ? 1 : 0;
}

int cobranca_atualizar_status(int id, const char *novo_status, const char *motivo)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE cobrancas SET status = ?, motivo = ? WHERE id = ?;";
    char atual[32] = "";
    int exigeMotivo;
    int ok = 0;

    if (status_cobranca_valido(novo_status) == 0)
    {
        return 0;
    }

    /* GLOSADA/CANCELADA exigem justificativa. */
    exigeMotivo = (strcmp(novo_status, "GLOSADA") == 0 ||
                   strcmp(novo_status, "CANCELADA") == 0);
    if (exigeMotivo && (motivo == NULL || motivo[0] == '\0'))
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db,
            "SELECT status FROM cobrancas WHERE id = ?;", -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, id);
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        snprintf(atual, sizeof(atual), "%s", (const char *)sqlite3_column_text(stmt, 0));
    }
    sqlite3_finalize(stmt);

    /* Nao mexe em cobranca ja encerrada. */
    if (atual[0] == '\0' || status_terminal(atual))
    {
        db_fechar(db);
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_text(stmt, 1, novo_status, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, motivo != NULL ? motivo : "", -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, id);
    ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;
    sqlite3_finalize(stmt);
    db_fechar(db);
    return ok ? 1 : 0;
}

/* Serializa as linhas de uma consulta de cobrancas (com bind opcional). */
static int cobrancas_para_json(const char *sql, int filtro, char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
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

    if (filtro > 0)
    {
        sqlite3_bind_int(stmt, 1, filtro);
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
        char formaJson[24];
        char origemJson[128];
        char descricaoJson[256];
        char statusJson[24];
        char motivoJson[256];
        char criadoJson[64];
        char objeto[1100];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int convenioId = sqlite3_column_int(stmt, 2);
        int valor = sqlite3_column_int(stmt, 8);
        int escrito;

        if (repo_json_escapar(formaJson, sizeof(formaJson), (const char *)sqlite3_column_text(stmt, 3)) == 0 ||
            repo_json_escapar(origemJson, sizeof(origemJson), (const char *)sqlite3_column_text(stmt, 4)) == 0 ||
            repo_json_escapar(descricaoJson, sizeof(descricaoJson), (const char *)sqlite3_column_text(stmt, 5)) == 0 ||
            repo_json_escapar(statusJson, sizeof(statusJson), (const char *)sqlite3_column_text(stmt, 6)) == 0 ||
            repo_json_escapar(motivoJson, sizeof(motivoJson), (const char *)sqlite3_column_text(stmt, 7)) == 0 ||
            repo_json_escapar(criadoJson, sizeof(criadoJson), (const char *)sqlite3_column_text(stmt, 9)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"id\":%d,\"pacienteId\":%d,\"convenioId\":%d,\"forma\":%s,"
            "\"origem\":%s,\"descricao\":%s,\"status\":%s,\"motivo\":%s,"
            "\"valorCentavos\":%d,\"criadoEm\":%s}",
            primeiro ? "" : ",",
            id, pacienteId, convenioId, formaJson, origemJson, descricaoJson,
            statusJson, motivoJson, valor, criadoJson);

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

/* Colunas/ordem usadas pela serializacao de cobrancas. */
#define COBRANCA_COLS \
    "id, paciente_id, convenio_id, forma, origem, descricao, status, motivo, " \
    "valor_centavos, criado_em"

int cobranca_listar_json(char *buffer, int tamanho)
{
    return cobrancas_para_json(
        "SELECT " COBRANCA_COLS " FROM cobrancas ORDER BY id DESC;", 0,
        buffer, tamanho);
}

int cobranca_listar_por_paciente_json(int paciente_id, char *buffer, int tamanho)
{
    if (paciente_id <= 0)
    {
        return 0;
    }
    return cobrancas_para_json(
        "SELECT " COBRANCA_COLS " FROM cobrancas WHERE paciente_id = ? "
        "ORDER BY id DESC;", paciente_id, buffer, tamanho);
}

int cobranca_contar_pendentes(void)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT COUNT(*) FROM cobrancas WHERE status IN ('PENDENTE','AUTORIZADA');";
    int total = -1;

    if (db_abrir(&db) == 0)
    {
        return -1;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK &&
        sqlite3_step(stmt) == SQLITE_ROW)
    {
        total = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return total;
}

int cobranca_demonstrativo_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT status, COUNT(*), COALESCE(SUM(valor_centavos),0) "
        "FROM cobrancas GROUP BY status ORDER BY status;";
    char porStatus[1024];
    int usadoPS = 0;
    int primeiro = 1;
    int recebido = 0, pendente = 0, glosado = 0;
    int escrito;

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

    porStatus[0] = '\0';
    repo_json_anexar(porStatus, sizeof(porStatus), &usadoPS, "[");

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char statusJson[24];
        char objeto[160];
        const char *status = (const char *)sqlite3_column_text(stmt, 0);
        int qtd = sqlite3_column_int(stmt, 1);
        int soma = sqlite3_column_int(stmt, 2);

        if (status != NULL)
        {
            if (strcmp(status, "PAGA") == 0) recebido += soma;
            else if (strcmp(status, "PENDENTE") == 0 || strcmp(status, "AUTORIZADA") == 0) pendente += soma;
            else if (strcmp(status, "GLOSADA") == 0) glosado += soma;
        }

        if (repo_json_escapar(statusJson, sizeof(statusJson), status) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        snprintf(objeto, sizeof(objeto),
                 "%s{\"status\":%s,\"quantidade\":%d,\"valorCentavos\":%d}",
                 primeiro ? "" : ",", statusJson, qtd, soma);
        repo_json_anexar(porStatus, sizeof(porStatus), &usadoPS, objeto);
        primeiro = 0;
    }
    sqlite3_finalize(stmt);
    db_fechar(db);

    repo_json_anexar(porStatus, sizeof(porStatus), &usadoPS, "]");

    escrito = snprintf(buffer, (size_t)tamanho,
        "{\"recebidoCentavos\":%d,\"pendenteCentavos\":%d,\"glosadoCentavos\":%d,"
        "\"porStatus\":%s}",
        recebido, pendente, glosado, porStatus);

    return (escrito > 0 && escrito < tamanho) ? 1 : 0;
}
