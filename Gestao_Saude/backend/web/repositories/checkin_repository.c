#include "checkin_repository.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>
#include <string.h>

static int destino_valido(const char *destino)
{
    return destino != NULL &&
           (strcmp(destino, "TRIAGEM") == 0 || strcmp(destino, "CONSULTA") == 0);
}

int checkin_repo_criar(int paciente_id, const char *destino,
                       char *senha_out, int senha_tam)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO checkins (paciente_id, senha, destino, status) "
        "VALUES (?, ?, ?, 'AGUARDANDO');";
    char senha[16];
    int total = 0;
    int ok = 0;

    if (paciente_id <= 0 || destino_valido(destino) == 0 ||
        senha_out == NULL || senha_tam < 8)
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    /* Senha sequencial por destino: T### (triagem) ou C### (consulta). */
    if (sqlite3_prepare_v2(db,
            "SELECT COUNT(*) FROM checkins WHERE destino = ?;",
            -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, destino, -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            total = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    snprintf(senha, sizeof(senha), "%c%03d",
             destino[0] == 'T' ? 'T' : 'C', total + 1);

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_int(stmt, 1, paciente_id);
    sqlite3_bind_text(stmt, 2, senha, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, destino, -1, SQLITE_STATIC);
    ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    db_fechar(db);

    if (ok)
    {
        snprintf(senha_out, (size_t)senha_tam, "%s", senha);
        return 1;
    }
    return 0;
}

int checkin_repo_listar_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    /* Fila ordenada por PRIORIDADE da triagem vigente (urgentes no topo) e,
     * em empate, por ordem de chegada. Pacientes sem triagem tem prioridade 0. */
    const char *sql =
        "SELECT c.id, c.paciente_id, p.nome, c.senha, c.destino, c.status, "
        "c.criado_em, COALESCE(("
        "  SELECT t.pontuacao FROM triagens t "
        "  WHERE t.paciente_id = c.paciente_id AND t.ativo = 1 AND t.vigente = 1 "
        "  ORDER BY t.id DESC LIMIT 1), 0) AS prioridade, "
        "c.rechamadas, c.motivo, "
        "COALESCE(("
        "  SELECT t.classificacao FROM triagens t "
        "  WHERE t.paciente_id = c.paciente_id AND t.ativo = 1 AND t.vigente = 1 "
        "  ORDER BY t.id DESC LIMIT 1), '') AS classificacao, "
        "CAST((julianday('now') - julianday(c.criado_em)) * 1440 AS INTEGER) AS espera_min "
        "FROM checkins c "
        "LEFT JOIN pacientes p ON p.id = c.paciente_id "
        "WHERE c.status NOT IN ('ENCERRADO', 'CANCELADO') "
        "ORDER BY prioridade DESC, c.id ASC;";
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
        char senhaJson[24];
        char destinoJson[24];
        char statusJson[32];
        char criadoJson[64];
        char motivoJson[256];
        char classifJson[32];
        char objeto[1100];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int prioridade = sqlite3_column_int(stmt, 7);
        int rechamadas = sqlite3_column_int(stmt, 8);
        const char *classificacao = (const char *)sqlite3_column_text(stmt, 10);
        int esperaMin = sqlite3_column_int(stmt, 11);
        int slaMin = checkin_sla_minutos(classificacao);
        /* Estouro de SLA so faz sentido enquanto o paciente aguarda. */
        int estouro = (slaMin >= 0 &&
                       strcmp((const char *)sqlite3_column_text(stmt, 5), "AGUARDANDO") == 0 &&
                       esperaMin > slaMin);
        int escrito;

        if (repo_json_escapar(nomeJson, sizeof(nomeJson), (const char *)sqlite3_column_text(stmt, 2)) == 0 ||
            repo_json_escapar(senhaJson, sizeof(senhaJson), (const char *)sqlite3_column_text(stmt, 3)) == 0 ||
            repo_json_escapar(destinoJson, sizeof(destinoJson), (const char *)sqlite3_column_text(stmt, 4)) == 0 ||
            repo_json_escapar(statusJson, sizeof(statusJson), (const char *)sqlite3_column_text(stmt, 5)) == 0 ||
            repo_json_escapar(criadoJson, sizeof(criadoJson), (const char *)sqlite3_column_text(stmt, 6)) == 0 ||
            repo_json_escapar(motivoJson, sizeof(motivoJson), (const char *)sqlite3_column_text(stmt, 9)) == 0 ||
            repo_json_escapar(classifJson, sizeof(classifJson), classificacao) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"id\":%d,\"pacienteId\":%d,\"pacienteNome\":%s,\"senha\":%s,"
            "\"destino\":%s,\"status\":%s,\"prioridade\":%d,\"rechamadas\":%d,"
            "\"motivo\":%s,\"classificacao\":%s,\"esperaMinutos\":%d,"
            "\"slaMinutos\":%d,\"estouroSla\":%s,\"criadoEm\":%s}",
            primeiro ? "" : ",",
            id, pacienteId, nomeJson, senhaJson, destinoJson, statusJson,
            prioridade, rechamadas, motivoJson, classifJson, esperaMin,
            slaMin, estouro ? "true" : "false", criadoJson);

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

/* Helper de transicao de status com guarda na origem. */
static int mudar_status(int id, const char *de, const char *para)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE checkins SET status = ? WHERE id = ? AND status = ?;";
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

    sqlite3_bind_text(stmt, 1, para, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, id);
    sqlite3_bind_text(stmt, 3, de, -1, SQLITE_STATIC);
    ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;

    sqlite3_finalize(stmt);
    db_fechar(db);
    return ok ? 1 : 0;
}

int checkin_repo_chamar(int id)
{
    return mudar_status(id, "AGUARDANDO", "EM_ATENDIMENTO");
}

int checkin_repo_rechamar(int id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE checkins SET rechamadas = rechamadas + 1 "
        "WHERE id = ? AND status = 'EM_ATENDIMENTO';";
    int ok = 0;

    if (db_abrir(&db) == 0)
    {
        return 0;
    }
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, id);
        ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;
        sqlite3_finalize(stmt);
    }
    db_fechar(db);
    return ok ? 1 : 0;
}

int checkin_repo_faltar(int id)
{
    /* Falta a partir de chamado ou ainda aguardando. */
    if (mudar_status(id, "EM_ATENDIMENTO", "FALTOU") == 1)
    {
        return 1;
    }
    return mudar_status(id, "AGUARDANDO", "FALTOU");
}

int checkin_repo_retornar(int id)
{
    return mudar_status(id, "FALTOU", "AGUARDANDO");
}

int checkin_repo_cancelar(int id, const char *motivo)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE checkins SET status = 'CANCELADO', motivo = ? "
        "WHERE id = ? AND status IN ('AGUARDANDO', 'EM_ATENDIMENTO');";
    int ok = 0;

    if (motivo == NULL || motivo[0] == '\0')
    {
        return 0;
    }
    if (db_abrir(&db) == 0)
    {
        return 0;
    }
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, motivo, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, id);
        ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;
        sqlite3_finalize(stmt);
    }
    db_fechar(db);
    return ok ? 1 : 0;
}

int checkin_repo_encerrar(int id)
{
    /* Encerra a partir de aguardando ou em atendimento. */
    if (mudar_status(id, "EM_ATENDIMENTO", "ENCERRADO") == 1)
    {
        return 1;
    }
    return mudar_status(id, "AGUARDANDO", "ENCERRADO");
}

int checkin_repo_contar_aguardando(void)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "SELECT COUNT(*) FROM checkins WHERE status = 'AGUARDANDO';";
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

int checkin_sla_minutos(const char *classificacao)
{
    if (classificacao == NULL)
    {
        return -1;
    }
    /* Tempos-alvo do Protocolo de Manchester (espera maxima por nivel). */
    if (strcmp(classificacao, "Vermelho") == 0) return 0;
    if (strcmp(classificacao, "Laranja") == 0) return 10;
    if (strcmp(classificacao, "Amarelo") == 0) return 60;
    if (strcmp(classificacao, "Verde") == 0) return 120;
    if (strcmp(classificacao, "Azul") == 0) return 240;
    return -1; /* sem triagem vigente: sem SLA por risco */
}
