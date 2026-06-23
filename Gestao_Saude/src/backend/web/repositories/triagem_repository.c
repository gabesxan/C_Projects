#include "triagem_repository.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>
#include <string.h>

static const char *texto_coluna(sqlite3_stmt *stmt, int col)
{
    const unsigned char *txt = sqlite3_column_text(stmt, col);
    return txt != NULL ? (const char *)txt : "";
}

static int tipo_por_especialidade(int especialidade_id)
{
    switch (especialidade_id)
    {
    case 2:
        return 2;
    case 3:
        return 3;
    case 4:
        return 4;
    case 5:
        return 5;
    default:
        return 1;
    }
}

int triagem_repo_criar_clinica(int paciente_id, int profissional_id,
                               int especialidade_principal_id, int pontuacao,
                               const char *classificacao, const char *itens,
                               const char *queixa, const char *observacoes,
                               const char *pressao, const char *temperatura,
                               const char *freq_cardiaca,
                               const char *saturacao)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO triagens "
        "(paciente_id, profissional_id, especialidade_principal_id, tipo_triagem, "
        "pontuacao, classificacao, prioridade, itens, queixa, observacoes, "
        "pressao, temperatura, freq_cardiaca, saturacao, status, ativo) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, 'EM_TRIAGEM', 1);";
    int tipo_triagem = tipo_por_especialidade(especialidade_principal_id);
    int novo_id;

    if (paciente_id <= 0 || especialidade_principal_id <= 0)
    {
        return 0;
    }

    if (classificacao == NULL || classificacao[0] == '\0')
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
    sqlite3_bind_int(stmt, 2, profissional_id);
    sqlite3_bind_int(stmt, 3, especialidade_principal_id);
    sqlite3_bind_int(stmt, 4, tipo_triagem);
    sqlite3_bind_int(stmt, 5, pontuacao);
    sqlite3_bind_text(stmt, 6, classificacao, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 7, pontuacao);
    sqlite3_bind_text(stmt, 8, itens != NULL ? itens : "", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 9, queixa != NULL ? queixa : "", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 10, observacoes != NULL ? observacoes : "", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 11, pressao != NULL ? pressao : "", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 12, temperatura != NULL ? temperatura : "", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 13, freq_cardiaca != NULL ? freq_cardiaca : "", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 14, saturacao != NULL ? saturacao : "", -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    sqlite3_finalize(stmt);
    novo_id = (int)sqlite3_last_insert_rowid(db);

    /* A triagem original e raiz de si mesma (raiz_id = id). */
    if (novo_id > 0)
    {
        sqlite3_stmt *upd = NULL;
        if (sqlite3_prepare_v2(db, "UPDATE triagens SET raiz_id = id WHERE id = ?;",
                               -1, &upd, NULL) == SQLITE_OK)
        {
            sqlite3_bind_int(upd, 1, novo_id);
            sqlite3_step(upd);
            sqlite3_finalize(upd);
        }
    }

    db_fechar(db);
    return novo_id;
}

int triagem_repo_criar_completa(int paciente_id, int tipo_triagem, int pontuacao,
                                const char *classificacao, const char *itens,
                                const char *queixa, const char *pressao,
                                const char *temperatura, const char *freq_cardiaca,
                                const char *saturacao)
{
    return triagem_repo_criar_clinica(paciente_id, 0, tipo_triagem, pontuacao,
                                      classificacao, itens, queixa, "",
                                      pressao, temperatura, freq_cardiaca,
                                      saturacao) > 0
               ? 1
               : 0;
}

/* Versao simples (sem sinais vitais/itens): mantida para compatibilidade. */
int triagem_repo_criar(int paciente_id, int tipo_triagem, int pontuacao,
                       const char *classificacao)
{
    return triagem_repo_criar_completa(paciente_id, tipo_triagem, pontuacao,
                                       classificacao, "", "", "", "", "", "");
}

int triagem_repo_reclassificar(int id, const char *classificacao, int nivel,
                               const char *itens, const char *justificativa)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sqlBusca =
        "SELECT paciente_id, tipo_triagem, queixa, pressao, temperatura, "
        "freq_cardiaca, saturacao, versao, raiz_id, profissional_id, "
        "especialidade_principal_id, observacoes, status FROM triagens "
        "WHERE id = ? AND ativo = 1 AND vigente = 1;";
    const char *sqlInsert =
        "INSERT INTO triagens "
        "(paciente_id, profissional_id, especialidade_principal_id, tipo_triagem, "
        "pontuacao, classificacao, prioridade, itens, justificativa, queixa, "
        "observacoes, pressao, temperatura, freq_cardiaca, saturacao, status, "
        "versao, raiz_id, vigente, ativo) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, 1, 1);";
    int pacienteId = 0, tipo = 0, versao = 0, raiz = 0;
    int profissionalId = 0, especialidadeId = 0;
    char queixa[256] = "", pressao[32] = "", temperatura[32] = "";
    char freq[32] = "", saturacao[32] = "";
    char observacoes[256] = "", status[32] = "EM_TRIAGEM";
    int ok = 0;

    /* Mudanca de classificacao exige justificativa. */
    if (classificacao == NULL || classificacao[0] == '\0' ||
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
        tipo = sqlite3_column_int(stmt, 1);
        snprintf(queixa, sizeof(queixa), "%s", (const char *)sqlite3_column_text(stmt, 2));
        snprintf(pressao, sizeof(pressao), "%s", (const char *)sqlite3_column_text(stmt, 3));
        snprintf(temperatura, sizeof(temperatura), "%s", (const char *)sqlite3_column_text(stmt, 4));
        snprintf(freq, sizeof(freq), "%s", (const char *)sqlite3_column_text(stmt, 5));
        snprintf(saturacao, sizeof(saturacao), "%s", (const char *)sqlite3_column_text(stmt, 6));
        versao = sqlite3_column_int(stmt, 7);
        raiz = sqlite3_column_int(stmt, 8);
        profissionalId = sqlite3_column_int(stmt, 9);
        especialidadeId = sqlite3_column_int(stmt, 10);
        snprintf(observacoes, sizeof(observacoes), "%s", texto_coluna(stmt, 11));
        snprintf(status, sizeof(status), "%s", texto_coluna(stmt, 12));
    }
    sqlite3_finalize(stmt);

    if (pacienteId == 0)
    {
        db_fechar(db);
        return 0;
    }

    /* Nova versao vigente com a nova classificacao. */
    if (sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    if (especialidadeId <= 0)
    {
        especialidadeId = tipo;
    }
    sqlite3_bind_int(stmt, 1, pacienteId);
    sqlite3_bind_int(stmt, 2, profissionalId);
    sqlite3_bind_int(stmt, 3, especialidadeId);
    sqlite3_bind_int(stmt, 4, tipo);
    sqlite3_bind_int(stmt, 5, nivel);
    sqlite3_bind_text(stmt, 6, classificacao, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 7, nivel);
    sqlite3_bind_text(stmt, 8, itens != NULL ? itens : "", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 9, justificativa, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 10, queixa, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 11, observacoes, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 12, pressao, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 13, temperatura, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 14, freq, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 15, saturacao, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 16, status, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 17, versao + 1);
    sqlite3_bind_int(stmt, 18, raiz);
    ok = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);

    /* Supera a versao anterior (preservada). */
    if (ok)
    {
        sqlite3_stmt *upd = NULL;
        if (sqlite3_prepare_v2(db, "UPDATE triagens SET vigente = 0 WHERE id = ?;",
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

int triagem_repo_distribuicao_por_classificacao_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT classificacao, COUNT(*) FROM triagens WHERE ativo = 1 AND vigente = 1 "
        "GROUP BY classificacao ORDER BY classificacao;";
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
        char classificacaoJson[128];
        char objeto[192];
        int total = sqlite3_column_int(stmt, 1);
        int escrito;

        if (repo_json_escapar(classificacaoJson, sizeof(classificacaoJson),
                              (const char *)sqlite3_column_text(stmt, 0)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"classificacao\":%s,\"total\":%d}",
                           primeiro ? "" : ",", classificacaoJson, total);

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

int triagem_repo_listar_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, paciente_id, tipo_triagem, pontuacao, classificacao, queixa, pressao, temperatura, freq_cardiaca, saturacao, itens "
        "FROM triagens WHERE ativo = 1 AND vigente = 1 ORDER BY pontuacao DESC, id;";
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
        char classificacaoJson[128];
        char queixaJson[256];
        char pressaoJson[32];
        char temperaturaJson[32];
        char freqJson[32];
        char saturacaoJson[32];
        char itensJson[512];
        char objeto[1280];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int tipo = sqlite3_column_int(stmt, 2);
        int pontuacao = sqlite3_column_int(stmt, 3);
        const char *classificacao = (const char *)sqlite3_column_text(stmt, 4);
        int escrito;

        if (repo_json_escapar(classificacaoJson, sizeof(classificacaoJson), classificacao) == 0 ||
            repo_json_escapar(queixaJson, sizeof(queixaJson), (const char *)sqlite3_column_text(stmt, 5)) == 0 ||
            repo_json_escapar(pressaoJson, sizeof(pressaoJson), (const char *)sqlite3_column_text(stmt, 6)) == 0 ||
            repo_json_escapar(temperaturaJson, sizeof(temperaturaJson), (const char *)sqlite3_column_text(stmt, 7)) == 0 ||
            repo_json_escapar(freqJson, sizeof(freqJson), (const char *)sqlite3_column_text(stmt, 8)) == 0 ||
            repo_json_escapar(saturacaoJson, sizeof(saturacaoJson), (const char *)sqlite3_column_text(stmt, 9)) == 0 ||
            repo_json_escapar(itensJson, sizeof(itensJson), (const char *)sqlite3_column_text(stmt, 10)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"id\":%d,\"pacienteId\":%d,\"tipoTriagem\":%d,"
                           "\"pontuacao\":%d,\"classificacao\":%s,\"queixa\":%s,"
                           "\"pressao\":%s,\"temperatura\":%s,\"freqCardiaca\":%s,"
                           "\"saturacao\":%s,\"itens\":%s}",
                           primeiro ? "" : ",",
                           id, pacienteId, tipo, pontuacao, classificacaoJson, queixaJson,
                           pressaoJson, temperaturaJson, freqJson, saturacaoJson, itensJson);

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

int triagem_repo_listar_por_tipos_json(const int *tipos, int n,
                                       char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    char placeholders[64];
    char sql[512];
    int usado = 0;
    int primeiro = 1;
    int i;
    int pos = 0;

    if (buffer == NULL || tamanho <= 0 || tipos == NULL)
    {
        return 0;
    }

    /* Sem tipos correspondentes: lista vazia, mas resposta valida. */
    if (n <= 0)
    {
        buffer[0] = '\0';
        return repo_json_anexar(buffer, tamanho, &usado, "[]");
    }

    /* Monta "?,?,..." com n marcadores para o IN, limitando a capacidade. */
    if (n > 32)
    {
        n = 32;
    }
    for (i = 0; i < n; i++)
    {
        int escrito = snprintf(placeholders + pos, sizeof(placeholders) - pos,
                               i == 0 ? "?" : ",?");
        if (escrito < 0 || escrito >= (int)sizeof(placeholders) - pos)
        {
            return 0;
        }
        pos += escrito;
    }

    if (snprintf(sql, sizeof(sql),
                 "SELECT id, paciente_id, tipo_triagem, pontuacao, classificacao, queixa, pressao, temperatura, freq_cardiaca, saturacao, itens "
                 "FROM triagens WHERE ativo = 1 AND vigente = 1 AND tipo_triagem IN (%s) "
                 "ORDER BY pontuacao DESC, id;",
                 placeholders) >= (int)sizeof(sql))
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

    for (i = 0; i < n; i++)
    {
        sqlite3_bind_int(stmt, i + 1, tipos[i]);
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
        char classificacaoJson[128];
        char queixaJson[256];
        char pressaoJson[32];
        char temperaturaJson[32];
        char freqJson[32];
        char saturacaoJson[32];
        char itensJson[512];
        char objeto[1280];
        int id = sqlite3_column_int(stmt, 0);
        int pacienteId = sqlite3_column_int(stmt, 1);
        int tipo = sqlite3_column_int(stmt, 2);
        int pontuacao = sqlite3_column_int(stmt, 3);
        const char *classificacao = (const char *)sqlite3_column_text(stmt, 4);
        int escrito;

        if (repo_json_escapar(classificacaoJson, sizeof(classificacaoJson), classificacao) == 0 ||
            repo_json_escapar(queixaJson, sizeof(queixaJson), (const char *)sqlite3_column_text(stmt, 5)) == 0 ||
            repo_json_escapar(pressaoJson, sizeof(pressaoJson), (const char *)sqlite3_column_text(stmt, 6)) == 0 ||
            repo_json_escapar(temperaturaJson, sizeof(temperaturaJson), (const char *)sqlite3_column_text(stmt, 7)) == 0 ||
            repo_json_escapar(freqJson, sizeof(freqJson), (const char *)sqlite3_column_text(stmt, 8)) == 0 ||
            repo_json_escapar(saturacaoJson, sizeof(saturacaoJson), (const char *)sqlite3_column_text(stmt, 9)) == 0 ||
            repo_json_escapar(itensJson, sizeof(itensJson), (const char *)sqlite3_column_text(stmt, 10)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"id\":%d,\"pacienteId\":%d,\"tipoTriagem\":%d,"
                           "\"pontuacao\":%d,\"classificacao\":%s,\"queixa\":%s,"
                           "\"pressao\":%s,\"temperatura\":%s,\"freqCardiaca\":%s,"
                           "\"saturacao\":%s,\"itens\":%s}",
                           primeiro ? "" : ",",
                           id, pacienteId, tipo, pontuacao, classificacaoJson, queixaJson,
                           pressaoJson, temperaturaJson, freqJson, saturacaoJson, itensJson);

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

int triagem_repo_detalhar_json(int id, char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    sqlite3_stmt *prob = NULL;
    const char *sql =
        "SELECT t.id, t.paciente_id, t.profissional_id, t.especialidade_principal_id, "
        "COALESCE(e.nome, ''), t.tipo_triagem, t.pontuacao, t.classificacao, "
        "t.prioridade, t.queixa, t.observacoes, t.status, t.data_hora "
        "FROM triagens t LEFT JOIN especialidades_clinicas e "
        "ON e.id = t.especialidade_principal_id "
        "WHERE t.id = ? AND t.ativo = 1 AND t.vigente = 1;";
    const char *sqlProb =
        "SELECT tp.problema_id, tp.especialidade_id, e.nome, p.nome, "
        "p.peso_risco, p.exame_sugerido, tp.principal, tp.observacao "
        "FROM triagem_problemas tp "
        "JOIN problemas_clinicos p ON p.id = tp.problema_id "
        "JOIN especialidades_clinicas e ON e.id = tp.especialidade_id "
        "WHERE tp.triagem_id = ? AND tp.ativo = 1 ORDER BY tp.principal DESC, tp.id;";
    char espJson[128], classeJson[96], queixaJson[512], obsJson[512];
    char statusJson[64], dataJson[64];
    int usado = 0;
    int primeiro = 1;
    int escrito;

    if (buffer == NULL || tamanho <= 0 || id <= 0)
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
    sqlite3_bind_int(stmt, 1, id);
    if (sqlite3_step(stmt) != SQLITE_ROW)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    if (repo_json_escapar(espJson, sizeof(espJson), texto_coluna(stmt, 4)) == 0 ||
        repo_json_escapar(classeJson, sizeof(classeJson), texto_coluna(stmt, 7)) == 0 ||
        repo_json_escapar(queixaJson, sizeof(queixaJson), texto_coluna(stmt, 9)) == 0 ||
        repo_json_escapar(obsJson, sizeof(obsJson), texto_coluna(stmt, 10)) == 0 ||
        repo_json_escapar(statusJson, sizeof(statusJson), texto_coluna(stmt, 11)) == 0 ||
        repo_json_escapar(dataJson, sizeof(dataJson), texto_coluna(stmt, 12)) == 0)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    buffer[0] = '\0';
    escrito = snprintf(buffer, (size_t)tamanho,
                       "{\"id\":%d,\"pacienteId\":%d,\"profissionalId\":%d,"
                       "\"especialidadePrincipalId\":%d,"
                       "\"especialidadePrincipal\":%s,\"tipoTriagem\":%d,"
                       "\"pontuacao\":%d,\"classificacao\":%s,"
                       "\"prioridade\":%d,\"queixa\":%s,\"observacoes\":%s,"
                       "\"status\":%s,\"dataHora\":%s,\"problemas\":[",
                       sqlite3_column_int(stmt, 0), sqlite3_column_int(stmt, 1),
                       sqlite3_column_int(stmt, 2), sqlite3_column_int(stmt, 3),
                       espJson, sqlite3_column_int(stmt, 5),
                       sqlite3_column_int(stmt, 6), classeJson,
                       sqlite3_column_int(stmt, 8), queixaJson, obsJson,
                       statusJson, dataJson);
    sqlite3_finalize(stmt);
    if (escrito < 0 || escrito >= tamanho)
    {
        db_fechar(db);
        return 0;
    }
    usado = escrito;

    if (sqlite3_prepare_v2(db, sqlProb, -1, &prob, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(prob, 1, id);
    while (sqlite3_step(prob) == SQLITE_ROW)
    {
        char espProbJson[128], probJson[256], exameJson[256], observacaoJson[256];
        char objeto[1024];
        if (repo_json_escapar(espProbJson, sizeof(espProbJson), texto_coluna(prob, 2)) == 0 ||
            repo_json_escapar(probJson, sizeof(probJson), texto_coluna(prob, 3)) == 0 ||
            repo_json_escapar(exameJson, sizeof(exameJson), texto_coluna(prob, 5)) == 0 ||
            repo_json_escapar(observacaoJson, sizeof(observacaoJson), texto_coluna(prob, 7)) == 0)
        {
            sqlite3_finalize(prob);
            db_fechar(db);
            return 0;
        }
        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"problemaId\":%d,\"especialidadeId\":%d,"
                           "\"especialidade\":%s,\"nome\":%s,\"pesoRisco\":%d,"
                           "\"exameSugerido\":%s,\"principal\":%s,"
                           "\"observacao\":%s}",
                           primeiro ? "" : ",",
                           sqlite3_column_int(prob, 0), sqlite3_column_int(prob, 1),
                           espProbJson, probJson, sqlite3_column_int(prob, 4),
                           exameJson, sqlite3_column_int(prob, 6) ? "true" : "false",
                           observacaoJson);
        if (escrito < 0 || escrito >= (int)sizeof(objeto) ||
            repo_json_anexar(buffer, tamanho, &usado, objeto) == 0)
        {
            sqlite3_finalize(prob);
            db_fechar(db);
            return 0;
        }
        primeiro = 0;
    }
    sqlite3_finalize(prob);
    db_fechar(db);

    return repo_json_anexar(buffer, tamanho, &usado, "]}");
}

int triagem_repo_paciente_id(int id, int *paciente_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int ok = 0;

    if (paciente_id == NULL || id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }
    if (sqlite3_prepare_v2(db,
                           "SELECT paciente_id FROM triagens WHERE id = ? AND ativo = 1 AND vigente = 1;",
                           -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, id);
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            *paciente_id = sqlite3_column_int(stmt, 0);
            ok = 1;
        }
        sqlite3_finalize(stmt);
    }
    db_fechar(db);
    return ok;
}

int triagem_repo_profissional_id(int id, int *profissional_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int ok = 0;

    if (profissional_id == NULL || id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }
    if (sqlite3_prepare_v2(db,
                           "SELECT profissional_id FROM triagens WHERE id = ? AND ativo = 1 AND vigente = 1;",
                           -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, id);
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            *profissional_id = sqlite3_column_int(stmt, 0);
            ok = 1;
        }
        sqlite3_finalize(stmt);
    }
    db_fechar(db);
    return ok;
}

int triagem_repo_especialidades_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int usado = 0;
    int primeiro = 1;

    if (buffer == NULL || tamanho <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }
    if (sqlite3_prepare_v2(db,
                           "SELECT id, nome FROM especialidades_clinicas WHERE ativo = 1 ORDER BY id;",
                           -1, &stmt, NULL) != SQLITE_OK)
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
        char nomeJson[128], objeto[192];
        int escrito;
        if (repo_json_escapar(nomeJson, sizeof(nomeJson), texto_coluna(stmt, 1)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }
        escrito = snprintf(objeto, sizeof(objeto), "%s{\"id\":%d,\"nome\":%s}",
                           primeiro ? "" : ",", sqlite3_column_int(stmt, 0), nomeJson);
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

int triagem_repo_problemas_por_especialidade_json(int especialidade_id,
                                                  char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int usado = 0;
    int primeiro = 1;

    if (buffer == NULL || tamanho <= 0 || especialidade_id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }
    if (sqlite3_prepare_v2(db,
                           "SELECT id, especialidade_id, nome, peso_risco, exame_sugerido_id, exame_sugerido "
                           "FROM problemas_clinicos WHERE ativo = 1 AND especialidade_id = ? ORDER BY peso_risco DESC, nome;",
                           -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, especialidade_id);
    buffer[0] = '\0';
    if (repo_json_anexar(buffer, tamanho, &usado, "[") == 0)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char nomeJson[256], exameJson[256], objeto[768];
        int escrito;
        if (repo_json_escapar(nomeJson, sizeof(nomeJson), texto_coluna(stmt, 2)) == 0 ||
            repo_json_escapar(exameJson, sizeof(exameJson), texto_coluna(stmt, 5)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }
        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"id\":%d,\"especialidadeId\":%d,\"nome\":%s,"
                           "\"pesoRisco\":%d,\"exameSugeridoId\":%d,"
                           "\"exameSugerido\":%s}",
                           primeiro ? "" : ",", sqlite3_column_int(stmt, 0),
                           sqlite3_column_int(stmt, 1), nomeJson,
                           sqlite3_column_int(stmt, 3), sqlite3_column_int(stmt, 4),
                           exameJson);
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

int triagem_repo_adicionar_problema(int triagem_id, int problema_id,
                                    int principal, const char *observacao)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    sqlite3_stmt *ins = NULL;
    int especialidade = 0;
    int ok = 0;

    if (triagem_id <= 0 || problema_id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }
    if (sqlite3_prepare_v2(db,
                           "SELECT especialidade_id FROM problemas_clinicos WHERE id = ? AND ativo = 1;",
                           -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, problema_id);
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        especialidade = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    if (especialidade == 0)
    {
        db_fechar(db);
        return 0;
    }
    if (principal)
    {
        sqlite3_stmt *upd = NULL;
        if (sqlite3_prepare_v2(db,
                               "UPDATE triagem_problemas SET principal = 0 WHERE triagem_id = ? AND ativo = 1;",
                               -1, &upd, NULL) == SQLITE_OK)
        {
            sqlite3_bind_int(upd, 1, triagem_id);
            sqlite3_step(upd);
            sqlite3_finalize(upd);
        }
    }
    if (sqlite3_prepare_v2(db,
                           "INSERT INTO triagem_problemas "
                           "(triagem_id, problema_id, especialidade_id, principal, observacao, ativo) "
                           "VALUES (?, ?, ?, ?, ?, 1);",
                           -1, &ins, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(ins, 1, triagem_id);
        sqlite3_bind_int(ins, 2, problema_id);
        sqlite3_bind_int(ins, 3, especialidade);
        sqlite3_bind_int(ins, 4, principal ? 1 : 0);
        sqlite3_bind_text(ins, 5, observacao != NULL ? observacao : "", -1, SQLITE_STATIC);
        ok = sqlite3_step(ins) == SQLITE_DONE;
        sqlite3_finalize(ins);
    }
    db_fechar(db);
    return ok ? 1 : 0;
}

int triagem_repo_remover_problema(int triagem_id, int problema_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int ok = 0;

    if (triagem_id <= 0 || problema_id <= 0 || db_abrir(&db) == 0)
    {
        return 0;
    }
    if (sqlite3_prepare_v2(db,
                           "UPDATE triagem_problemas SET ativo = 0 "
                           "WHERE triagem_id = ? AND problema_id = ? AND ativo = 1;",
                           -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, triagem_id);
        sqlite3_bind_int(stmt, 2, problema_id);
        ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;
        sqlite3_finalize(stmt);
    }
    db_fechar(db);
    return ok ? 1 : 0;
}

int triagem_repo_atualizar_resultado(int id, int especialidade_id, int prioridade,
                                     const char *classificacao)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int ok = 0;
    int tipo = tipo_por_especialidade(especialidade_id);

    if (id <= 0 || especialidade_id <= 0 || classificacao == NULL ||
        classificacao[0] == '\0' || db_abrir(&db) == 0)
    {
        return 0;
    }
    if (sqlite3_prepare_v2(db,
                           "UPDATE triagens SET especialidade_principal_id = ?, tipo_triagem = ?, "
                           "pontuacao = ?, prioridade = ?, classificacao = ? "
                           "WHERE id = ? AND ativo = 1 AND vigente = 1;",
                           -1, &stmt, NULL) == SQLITE_OK)
    {
        sqlite3_bind_int(stmt, 1, especialidade_id);
        sqlite3_bind_int(stmt, 2, tipo);
        sqlite3_bind_int(stmt, 3, prioridade);
        sqlite3_bind_int(stmt, 4, prioridade);
        sqlite3_bind_text(stmt, 5, classificacao, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 6, id);
        ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;
        sqlite3_finalize(stmt);
    }
    db_fechar(db);
    return ok ? 1 : 0;
}

int triagem_repo_desativar(int id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "UPDATE triagens SET ativo = 0 WHERE id = ? AND ativo = 1;";
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

int triagem_repo_ultima_por_paciente(int paciente_id, int *tipo_triagem,
                                     int *pontuacao, char *classificacao,
                                     int classificacao_tam)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT tipo_triagem, pontuacao, classificacao "
        "FROM triagens WHERE paciente_id = ? AND ativo = 1 AND vigente = 1 "
        "ORDER BY id DESC LIMIT 1;";
    int encontrou = 0;

    if (paciente_id <= 0)
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

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        if (tipo_triagem != NULL)
        {
            *tipo_triagem = sqlite3_column_int(stmt, 0);
        }

        if (pontuacao != NULL)
        {
            *pontuacao = sqlite3_column_int(stmt, 1);
        }

        if (classificacao != NULL && classificacao_tam > 0)
        {
            const char *texto = (const char *)sqlite3_column_text(stmt, 2);

            if (texto != NULL)
            {
                strncpy(classificacao, texto, (size_t)classificacao_tam - 1);
                classificacao[classificacao_tam - 1] = '\0';
            }
            else
            {
                classificacao[0] = '\0';
            }
        }

        encontrou = 1;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return encontrou;
}

int triagem_repo_contar_por_classificacao(const char *classificacao)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT COUNT(*) FROM triagens "
        "WHERE ativo = 1 AND vigente = 1 AND classificacao = ?;";
    int total = -1;

    if (classificacao == NULL)
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

    sqlite3_bind_text(stmt, 1, classificacao, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        total = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return total;
}

int triagem_repo_contar_ativos(void)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "SELECT COUNT(*) FROM triagens WHERE ativo = 1 AND vigente = 1;";
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
