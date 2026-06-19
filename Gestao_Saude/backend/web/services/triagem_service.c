#include "triagem_service.h"
#include "triagem_repository.h"
#include "paciente_repository.h"
#include "medico_repository.h"
#include "prontuario_repository.h"
#include "exame_repository.h"
#include "agendamento_repository.h"
#include "regiao_df.h"
#include "repo_json.h"
#include "database.h"

#include <stdio.h>
#include <string.h>

#define SVC_MAX_MEDICOS 64

/*
 * Tipos de triagem conforme o schema (espelham as constantes da V1, mas
 * declaradas aqui para nao acoplar o backend web ao model do terminal).
 */
#define SVC_TRIAGEM_GERAL 1
#define SVC_TRIAGEM_ORTOPEDIA 2
#define SVC_TRIAGEM_CARDIOLOGIA 3
#define SVC_TRIAGEM_PNEUMOLOGIA 4
#define SVC_TRIAGEM_PEDIATRIA 5

/* Fonte unica do mapa tipo de triagem -> especialidade provavel. */
static const char *especialidade_provavel(int tipo_triagem)
{
    switch (tipo_triagem)
    {
    case SVC_TRIAGEM_ORTOPEDIA:
        return "Ortopedia";
    case SVC_TRIAGEM_CARDIOLOGIA:
        return "Cardiologia";
    case SVC_TRIAGEM_PNEUMOLOGIA:
        return "Pneumologia";
    case SVC_TRIAGEM_PEDIATRIA:
        return "Pediatria";
    case SVC_TRIAGEM_GERAL:
        return "Clinico Geral";
    default:
        return "Indefinida";
    }
}

/* Fonte unica do mapa classificacao -> nivel de prioridade.
 * Reconhece as cores do protocolo de Manchester (usadas pela triagem por
 * checklist) e tambem os nomes legados, para compatibilidade. */
static int prioridade_de(const char *classificacao)
{
    if (strcmp(classificacao, "Vermelho") == 0 ||
        strcmp(classificacao, "Emergencia") == 0)
    {
        return 5;
    }
    if (strcmp(classificacao, "Laranja") == 0 ||
        strcmp(classificacao, "Muito prioritario") == 0)
    {
        return 4;
    }
    if (strcmp(classificacao, "Amarelo") == 0 ||
        strcmp(classificacao, "Prioritario") == 0)
    {
        return 3;
    }
    if (strcmp(classificacao, "Verde") == 0 ||
        strcmp(classificacao, "Comum") == 0)
    {
        return 2;
    }
    if (strcmp(classificacao, "Azul") == 0 ||
        strcmp(classificacao, "Orientacao basica") == 0)
    {
        return 1;
    }

    return 0;
}

/* -------------------------------------------------------------------------
 * Checklist de triagem (protocolo de Manchester simplificado).
 * O usuario apenas MARCA os sinais/sintomas presentes; o sistema deriva a
 * classificacao de risco pela regra "o discriminador mais grave vence".
 * Nada de pontuacao digitada manualmente.
 * ------------------------------------------------------------------------- */
typedef struct
{
    const char *chave;
    const char *rotulo;
    int nivel; /* 5=Vermelho .. 1=Azul */
} ItemTriagem;

static const ItemTriagem CHECKLIST[] = {
    {"inconsciente", "Inconsciente / nao responde", 5},
    {"parada_respiratoria", "Parada respiratoria / engasgo grave", 5},
    {"dor_toracica", "Dor no peito intensa", 5},
    {"hemorragia", "Hemorragia intensa", 5},
    {"convulsao", "Convulsao em curso", 5},
    {"falta_ar", "Falta de ar", 4},
    {"dor_intensa", "Dor intensa (8-10)", 4},
    {"confusao", "Confusao mental / desmaio", 4},
    {"febre_alta", "Febre alta (>= 39,5 C)", 4},
    {"dor_moderada", "Dor moderada (4-7)", 3},
    {"vomito", "Vomito ou diarreia persistente", 3},
    {"febre", "Febre (38-39 C)", 3},
    {"sintomas_leves", "Sintomas leves (resfriado, dor leve)", 2},
    {"administrativo", "Receita / atestado / reavaliacao", 1},
};
static const int CHECKLIST_N = (int)(sizeof(CHECKLIST) / sizeof(CHECKLIST[0]));

/* Nome (cor Manchester) da classificacao a partir do nivel 1-5. */
static const char *classificacao_de_nivel(int nivel)
{
    switch (nivel)
    {
    case 5:
        return "Vermelho";
    case 4:
        return "Laranja";
    case 3:
        return "Amarelo";
    case 2:
        return "Verde";
    default:
        return "Azul";
    }
}

int triagem_service_checklist_json(char *buffer, int tamanho)
{
    int usado = 0;
    int i;

    if (buffer == NULL || tamanho <= 0)
    {
        return 0;
    }

    buffer[0] = '\0';
    if (repo_json_anexar(buffer, tamanho, &usado, "[") == 0)
    {
        return 0;
    }

    for (i = 0; i < CHECKLIST_N; i++)
    {
        char objeto[256];
        int escrito = snprintf(objeto, sizeof(objeto),
            "%s{\"chave\":\"%s\",\"rotulo\":\"%s\",\"nivel\":%d,"
            "\"classificacao\":\"%s\"}",
            i == 0 ? "" : ",",
            CHECKLIST[i].chave, CHECKLIST[i].rotulo, CHECKLIST[i].nivel,
            classificacao_de_nivel(CHECKLIST[i].nivel));

        if (escrito < 0 || escrito >= (int)sizeof(objeto) ||
            repo_json_anexar(buffer, tamanho, &usado, objeto) == 0)
        {
            return 0;
        }
    }

    return repo_json_anexar(buffer, tamanho, &usado, "]");
}

int triagem_service_classificar(const char *itens, char *classificacao,
                                int classificacao_tam, int *nivel_out)
{
    char copia[512];
    char *token;
    char *saveptr = NULL;
    int maxNivel = 0;

    if (classificacao == NULL || classificacao_tam <= 0)
    {
        return 0;
    }

    /* Sem itens marcados: caso nao urgente (Azul). */
    snprintf(copia, sizeof(copia), "%s", itens != NULL ? itens : "");

    token = strtok_r(copia, ",", &saveptr);
    while (token != NULL)
    {
        int i;
        for (i = 0; i < CHECKLIST_N; i++)
        {
            if (strcmp(token, CHECKLIST[i].chave) == 0)
            {
                if (CHECKLIST[i].nivel > maxNivel)
                {
                    maxNivel = CHECKLIST[i].nivel;
                }
                break;
            }
        }
        token = strtok_r(NULL, ",", &saveptr);
    }

    if (maxNivel == 0)
    {
        maxNivel = 1; /* nenhum item reconhecido -> Azul */
    }

    snprintf(classificacao, (size_t)classificacao_tam, "%s",
             classificacao_de_nivel(maxNivel));

    if (nivel_out != NULL)
    {
        *nivel_out = maxNivel;
    }

    return 1;
}

/* Fonte unica do mapa tipo de triagem -> exames iniciais sugeridos (JSON). */
static const char *exames_sugeridos(int tipo_triagem)
{
    switch (tipo_triagem)
    {
    case SVC_TRIAGEM_ORTOPEDIA:
        return "[\"Raio-X\"]";
    case SVC_TRIAGEM_CARDIOLOGIA:
        return "[\"Eletrocardiograma\",\"Hemograma\"]";
    case SVC_TRIAGEM_PNEUMOLOGIA:
        return "[\"Raio-X\",\"Hemograma\"]";
    case SVC_TRIAGEM_PEDIATRIA:
        return "[\"Hemograma\"]";
    case SVC_TRIAGEM_GERAL:
        return "[\"Hemograma\",\"Urina\"]";
    default:
        return "[]";
    }
}

int triagem_service_listar_por_especialidade_json(const char *especialidade,
                                                  char *buffer, int tamanho)
{
    int tipos[5];
    int n = 0;
    int tipo;

    if (buffer == NULL || tamanho <= 0 || especialidade == NULL)
    {
        return 0;
    }

    /* Seleciona os tipos de triagem cuja especialidade provavel bate com a
     * especialidade do medico (mesma fonte usada na sugestao de medicos). */
    for (tipo = SVC_TRIAGEM_GERAL; tipo <= SVC_TRIAGEM_PEDIATRIA; tipo++)
    {
        if (strcmp(especialidade_provavel(tipo), especialidade) == 0)
        {
            tipos[n++] = tipo;
        }
    }

    return triagem_repo_listar_por_tipos_json(tipos, n, buffer, tamanho);
}

int triagem_service_avaliar_json(int paciente_id, char *buffer, int tamanho)
{
    int tipo = 0;
    char classificacao[64];
    char classificacaoJson[96];
    char especialidadeJson[64];
    const char *especialidade;
    int prioridade;
    int escrito;

    if (buffer == NULL || tamanho <= 0 || paciente_id <= 0)
    {
        return 0;
    }

    if (triagem_repo_ultima_por_paciente(paciente_id, &tipo, NULL,
                                         classificacao, sizeof(classificacao)) == 0)
    {
        return 0;
    }

    especialidade = especialidade_provavel(tipo);
    prioridade = prioridade_de(classificacao);

    if (repo_json_escapar(classificacaoJson, sizeof(classificacaoJson), classificacao) == 0 ||
        repo_json_escapar(especialidadeJson, sizeof(especialidadeJson), especialidade) == 0)
    {
        return 0;
    }

    escrito = snprintf(buffer, (size_t)tamanho,
                       "{\"pacienteId\":%d,\"classificacao\":%s,\"prioridade\":%d,"
                       "\"especialidadeProvavel\":%s}",
                       paciente_id, classificacaoJson, prioridade, especialidadeJson);

    if (escrito < 0 || escrito >= tamanho)
    {
        return 0;
    }

    return 1;
}

int triagem_service_sugerir_medicos_json(int paciente_id, char *buffer, int tamanho)
{
    int tipo = 0;
    int regiao;
    char classificacao[64];
    char especialidadeJson[64];
    char medicosJson[4096];
    const char *especialidade;
    int escrito;

    if (buffer == NULL || tamanho <= 0 || paciente_id <= 0)
    {
        return 0;
    }

    if (triagem_repo_ultima_por_paciente(paciente_id, &tipo, NULL,
                                         classificacao, sizeof(classificacao)) == 0)
    {
        return 0;
    }

    regiao = paciente_repo_regiao(paciente_id);

    if (regiao < 0)
    {
        return 0;
    }

    especialidade = especialidade_provavel(tipo);

    if (medico_repo_listar_por_especialidade_regiao_json(
            especialidade, regiao, medicosJson, sizeof(medicosJson)) == 0)
    {
        return 0;
    }

    if (repo_json_escapar(especialidadeJson, sizeof(especialidadeJson), especialidade) == 0)
    {
        return 0;
    }

    escrito = snprintf(buffer, (size_t)tamanho,
                       "{\"pacienteId\":%d,\"especialidadeProvavel\":%s,\"regiao\":%d,"
                       "\"medicosSugeridos\":%s}",
                       paciente_id, especialidadeJson, regiao, medicosJson);

    if (escrito < 0 || escrito >= tamanho)
    {
        return 0;
    }

    return 1;
}

int triagem_service_historico_json(int paciente_id, char *buffer, int tamanho)
{
    char prontuariosJson[8192];
    char examesJson[8192];
    int escrito;

    if (buffer == NULL || tamanho <= 0 || paciente_id <= 0)
    {
        return 0;
    }

    if (prontuario_repo_listar_por_paciente_json(paciente_id, prontuariosJson,
                                                 sizeof(prontuariosJson)) == 0)
    {
        return 0;
    }

    if (exame_repo_listar_por_paciente_json(paciente_id, examesJson,
                                            sizeof(examesJson)) == 0)
    {
        return 0;
    }

    escrito = snprintf(buffer, (size_t)tamanho,
                       "{\"pacienteId\":%d,\"prontuarios\":%s,\"exames\":%s}",
                       paciente_id, prontuariosJson, examesJson);

    if (escrito < 0 || escrito >= tamanho)
    {
        return 0;
    }

    return 1;
}

int triagem_service_sugerir_exames_json(int paciente_id, char *buffer, int tamanho)
{
    int tipo = 0;
    int escrito;

    if (buffer == NULL || tamanho <= 0 || paciente_id <= 0)
    {
        return 0;
    }

    if (triagem_repo_ultima_por_paciente(paciente_id, &tipo, NULL, NULL, 0) == 0)
    {
        return 0;
    }

    escrito = snprintf(buffer, (size_t)tamanho,
                       "{\"pacienteId\":%d,\"tipoTriagem\":%d,\"examesSugeridos\":%s}",
                       paciente_id, tipo, exames_sugeridos(tipo));

    if (escrito < 0 || escrito >= tamanho)
    {
        return 0;
    }

    return 1;
}

int triagem_service_recalcular_triagem(int triagem_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int especialidade = 0;
    int prioridade = 1;
    int totalProblemas = 0;
    int existe = 0;
    char classificacao[32];

    if (triagem_id <= 0)
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db,
            "SELECT COUNT(*) FROM triagens WHERE id = ? AND ativo = 1 AND vigente = 1;",
            -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, triagem_id);
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        existe = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    if (existe == 0)
    {
        db_fechar(db);
        return 0;
    }

    if (sqlite3_prepare_v2(db,
            "SELECT COUNT(*) FROM triagem_problemas WHERE triagem_id = ? AND ativo = 1;",
            -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, triagem_id);
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        totalProblemas = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    if (totalProblemas == 0)
    {
        db_fechar(db);
        return 1;
    }

    if (sqlite3_prepare_v2(db,
            "SELECT tp.especialidade_id, SUM(p.peso_risco) AS total, "
            "MAX(p.peso_risco) AS max_peso "
            "FROM triagem_problemas tp "
            "JOIN problemas_clinicos p ON p.id = tp.problema_id "
            "WHERE tp.triagem_id = ? AND tp.ativo = 1 AND p.ativo = 1 "
            "GROUP BY tp.especialidade_id "
            "ORDER BY total DESC, max_peso DESC, tp.especialidade_id LIMIT 1;",
            -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, triagem_id);
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        especialidade = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (sqlite3_prepare_v2(db,
            "SELECT COALESCE(MAX(p.peso_risco), 1) "
            "FROM triagem_problemas tp "
            "JOIN problemas_clinicos p ON p.id = tp.problema_id "
            "WHERE tp.triagem_id = ? AND tp.ativo = 1 AND p.ativo = 1;",
            -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, triagem_id);
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        prioridade = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (especialidade == 0)
    {
        if (sqlite3_prepare_v2(db,
                "SELECT especialidade_principal_id FROM triagens "
                "WHERE id = ? AND ativo = 1 AND vigente = 1;",
                -1, &stmt, NULL) != SQLITE_OK)
        {
            db_fechar(db);
            return 0;
        }
        sqlite3_bind_int(stmt, 1, triagem_id);
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            especialidade = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    db_fechar(db);

    if (especialidade <= 0)
    {
        return 0;
    }

    snprintf(classificacao, sizeof(classificacao), "%s",
             classificacao_de_nivel(prioridade));
    return triagem_repo_atualizar_resultado(triagem_id, especialidade,
                                            prioridade, classificacao);
}

int triagem_service_avaliar_triagem_json(int triagem_id, char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int pacienteId = 0;
    int prioridade = 0;
    int totalProblemas = 0;
    char classificacao[64] = "";
    char especialidade[128] = "";
    char classificacaoJson[96];
    char especialidadeJson[160];
    char justificativaJson[512];
    char proximosJson[512];
    char justificativa[384];
    const char *proximos;
    int escrito;

    if (buffer == NULL || tamanho <= 0 || triagem_id <= 0)
    {
        return 0;
    }

    if (triagem_service_recalcular_triagem(triagem_id) == 0)
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }
    if (sqlite3_prepare_v2(db,
            "SELECT t.paciente_id, t.classificacao, t.prioridade, e.nome, "
            "(SELECT COUNT(*) FROM triagem_problemas tp "
            " WHERE tp.triagem_id = t.id AND tp.ativo = 1) "
            "FROM triagens t LEFT JOIN especialidades_clinicas e "
            "ON e.id = t.especialidade_principal_id "
            "WHERE t.id = ? AND t.ativo = 1 AND t.vigente = 1;",
            -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, triagem_id);
    if (sqlite3_step(stmt) != SQLITE_ROW)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }
    pacienteId = sqlite3_column_int(stmt, 0);
    snprintf(classificacao, sizeof(classificacao), "%s", (const char *)sqlite3_column_text(stmt, 1));
    prioridade = sqlite3_column_int(stmt, 2);
    snprintf(especialidade, sizeof(especialidade), "%s", (const char *)sqlite3_column_text(stmt, 3));
    totalProblemas = sqlite3_column_int(stmt, 4);
    sqlite3_finalize(stmt);
    db_fechar(db);

    if (prioridade >= 5)
    {
        proximos = "Atender agora, monitorar sinais vitais e solicitar exames urgentes conforme decisao profissional.";
    }
    else if (prioridade >= 4)
    {
        proximos = "Priorizar atendimento, manter observacao e confirmar necessidade de exame inicial.";
    }
    else if (prioridade >= 3)
    {
        proximos = "Agendar ou encaminhar conforme disponibilidade e registrar conduta no prontuario.";
    }
    else
    {
        proximos = "Orientar paciente, agendar consulta comum ou registrar atendimento se houver indicacao.";
    }

    snprintf(justificativa, sizeof(justificativa),
             "Classificacao calculada a partir de %d problema(s) clinico(s) selecionado(s), usando o maior peso de risco e a especialidade com maior carga clinica.",
             totalProblemas);

    if (repo_json_escapar(classificacaoJson, sizeof(classificacaoJson), classificacao) == 0 ||
        repo_json_escapar(especialidadeJson, sizeof(especialidadeJson), especialidade) == 0 ||
        repo_json_escapar(justificativaJson, sizeof(justificativaJson), justificativa) == 0 ||
        repo_json_escapar(proximosJson, sizeof(proximosJson), proximos) == 0)
    {
        return 0;
    }

    escrito = snprintf(buffer, (size_t)tamanho,
                       "{\"triagemId\":%d,\"pacienteId\":%d,"
                       "\"classificacao\":%s,\"prioridade\":%d,"
                       "\"especialidadeProvavel\":%s,\"justificativa\":%s,"
                       "\"proximosPassos\":%s}",
                       triagem_id, pacienteId, classificacaoJson, prioridade,
                       especialidadeJson, justificativaJson, proximosJson);

    return (escrito > 0 && escrito < tamanho) ? 1 : 0;
}

int triagem_service_sugerir_exames_triagem_json(int triagem_id, char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int usado = 0;
    int primeiro = 1;
    int pacienteId = 0;

    if (buffer == NULL || tamanho <= 0 || triagem_id <= 0)
    {
        return 0;
    }
    triagem_repo_paciente_id(triagem_id, &pacienteId);

    if (db_abrir(&db) == 0)
    {
        return 0;
    }
    if (sqlite3_prepare_v2(db,
            "SELECT DISTINCT p.exame_sugerido_id, p.exame_sugerido "
            "FROM triagem_problemas tp "
            "JOIN problemas_clinicos p ON p.id = tp.problema_id "
            "WHERE tp.triagem_id = ? AND tp.ativo = 1 AND p.ativo = 1 "
            "AND p.exame_sugerido <> '' ORDER BY p.exame_sugerido;",
            -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }
    sqlite3_bind_int(stmt, 1, triagem_id);

    buffer[0] = '\0';
    if (repo_json_anexar(buffer, tamanho, &usado, "{\"triagemId\":") == 0)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }
    {
        char cab[96];
        int escrito = snprintf(cab, sizeof(cab), "%d,\"pacienteId\":%d,\"examesSugeridos\":[",
                               triagem_id, pacienteId);
        if (escrito < 0 || escrito >= (int)sizeof(cab) ||
            repo_json_anexar(buffer, tamanho, &usado, cab) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char exameJson[256];
        char objeto[384];
        int escrito;

        if (repo_json_escapar(exameJson, sizeof(exameJson),
                              (const char *)sqlite3_column_text(stmt, 1)) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }
        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"exameSugeridoId\":%d,\"nome\":%s}",
                           primeiro ? "" : ",", sqlite3_column_int(stmt, 0),
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
    return repo_json_anexar(buffer, tamanho, &usado, "]}");
}

/* Procura um medico livre da 'especialidade' na 'regiao' no slot informado.
 * Retorna o id do medico, ou 0 se nenhum disponivel. */
static int medico_livre_na_regiao(const char *especialidade, int regiao,
                                  const char *data, const char *horario)
{
    int ids[SVC_MAX_MEDICOS];
    int total = medico_repo_ids_por_especialidade_regiao(especialidade, regiao,
                                                         ids, SVC_MAX_MEDICOS);
    int i;

    for (i = 0; i < total; i++)
    {
        if (agendamento_repo_medico_ocupado(ids[i], data, horario) == 0)
        {
            return ids[i];
        }
    }

    return 0;
}

/* Procura um medico livre da especialidade comecando pela 'regiao' de origem e,
 * se nao houver, varrendo as RAs do DF em ordem de proximidade. Preenche
 * *regiao_destino com a RA escolhida. Retorna o id do medico, ou 0. */
static int medico_livre_mais_proximo(const char *especialidade, int regiao,
                                     const char *data, const char *horario,
                                     int *regiao_destino)
{
    int regioes[64];
    int n = regiao_df_ordenar_por_distancia(regiao, regioes, 64);
    int i;
    int medico;

    /* regiao_df ja coloca a propria origem em primeiro (distancia 0). */
    for (i = 0; i < n; i++)
    {
        medico = medico_livre_na_regiao(especialidade, regioes[i], data, horario);
        if (medico != 0)
        {
            if (regiao_destino != NULL)
            {
                *regiao_destino = regioes[i];
            }
            return medico;
        }
    }

    /* Origem nao esta na tabela de RAs: tenta ao menos a propria regiao. */
    medico = medico_livre_na_regiao(especialidade, regiao, data, horario);
    if (medico != 0 && regiao_destino != NULL)
    {
        *regiao_destino = regiao;
    }
    return medico;
}

/* Prioridade (1-5) da triagem ativa do paciente; 0 se nao houver. */
static int prioridade_do_paciente(int paciente_id)
{
    int nivel = 0;
    if (triagem_repo_ultima_por_paciente(paciente_id, NULL, &nivel, NULL, 0) == 0)
    {
        return 0;
    }
    return nivel;
}

/*
 * Agendamento inteligente:
 *  1) tenta um medico livre da especialidade na RA do paciente;
 *  2) sem vaga, tenta PREEMPCAO: desloca, no mesmo slot, o paciente de MENOR
 *     prioridade que seja inferior a do novo paciente (um "muito emergente"
 *     toma o lugar de um "emergente");
 *  3) o paciente deslocado e reacomodado com um medico na sua RA ou, se nao
 *     houver, na RA mais proxima do DF (distancias reais).
 */
int triagem_service_agendar_json(int paciente_id, const char *data,
                                 const char *horario, char *buffer, int tamanho)
{
    int tipo = 0;
    int regiao;
    int ids[SVC_MAX_MEDICOS];
    int total;
    int i;
    int escolhido = 0;
    char dataJson[32];
    char horarioJson[24];
    const char *especialidade;
    int escrito;
    int nivelP;
    int bumpAg = 0;
    int bumpPac = 0;
    int bumpMed = 0;
    int menor;

    if (buffer == NULL || tamanho <= 0 || paciente_id <= 0)
    {
        return 0;
    }

    if (data == NULL || data[0] == '\0' || horario == NULL || horario[0] == '\0')
    {
        return 0;
    }

    if (agendamento_repo_horario_valido(horario) == 0)
    {
        snprintf(buffer, (size_t)tamanho,
                 "{\"agendado\":false,\"motivo\":\"horario fora da grade ou do expediente\"}");
        return 0;
    }

    if (triagem_repo_ultima_por_paciente(paciente_id, &tipo, NULL, NULL, 0) == 0)
    {
        return 0;
    }

    regiao = paciente_repo_regiao(paciente_id);

    if (regiao < 0)
    {
        return 0;
    }

    nivelP = prioridade_do_paciente(paciente_id);
    especialidade = especialidade_provavel(tipo);
    total = medico_repo_ids_por_especialidade_regiao(especialidade, regiao,
                                                     ids, SVC_MAX_MEDICOS);

    if (repo_json_escapar(dataJson, sizeof(dataJson), data) == 0 ||
        repo_json_escapar(horarioJson, sizeof(horarioJson), horario) == 0)
    {
        return 0;
    }

    /* 1) Vaga livre direta. */
    for (i = 0; i < total; i++)
    {
        if (agendamento_repo_medico_ocupado(ids[i], data, horario) == 0)
        {
            escolhido = ids[i];
            break;
        }
    }

    if (escolhido != 0)
    {
        if (agendamento_repo_criar(paciente_id, escolhido, data, horario) == 0)
        {
            snprintf(buffer, (size_t)tamanho,
                     "{\"agendado\":false,\"motivo\":\"falha ao gravar agendamento\"}");
            return 0;
        }

        escrito = snprintf(buffer, (size_t)tamanho,
                           "{\"agendado\":true,\"pacienteId\":%d,\"medicoId\":%d,"
                           "\"data\":%s,\"horario\":%s,\"preempcao\":false}",
                           paciente_id, escolhido, dataJson, horarioJson);
        return (escrito > 0 && escrito < tamanho) ? 1 : 0;
    }

    /* 2) Sem vaga: tenta preempcao pelo ocupante de MENOR prioridade que ainda
     *    seja inferior a do novo paciente. */
    menor = nivelP;
    for (i = 0; i < total; i++)
    {
        int agId = 0;
        int ocupante = 0;
        if (agendamento_repo_buscar_no_slot(ids[i], data, horario, &agId,
                                            &ocupante))
        {
            int nivelOcc = prioridade_do_paciente(ocupante);
            if (nivelOcc < menor)
            {
                menor = nivelOcc;
                bumpAg = agId;
                bumpPac = ocupante;
                bumpMed = ids[i];
            }
        }
    }

    if (bumpAg == 0)
    {
        snprintf(buffer, (size_t)tamanho,
                 "{\"agendado\":false,\"motivo\":\"sem vaga e sem paciente de "
                 "menor prioridade para realocar\"}");
        return 0;
    }

    /* Desloca o ocupante e coloca o paciente prioritario no slot. */
    agendamento_repo_cancelar(bumpAg, "Realocado por prioridade superior");
    if (agendamento_repo_criar(paciente_id, bumpMed, data, horario) == 0)
    {
        snprintf(buffer, (size_t)tamanho,
                 "{\"agendado\":false,\"motivo\":\"falha ao gravar agendamento\"}");
        return 0;
    }

    /* 3) Reacomoda o paciente deslocado: medico urgente na sua RA ou na mais
     *    proxima do DF. */
    {
        int tipoB = 0;
        int regiaoB;
        int destinoMed = 0;
        int destinoRegiao = 0;
        const char *espB;
        char nomeRegiao[64];

        regiaoB = paciente_repo_regiao(bumpPac);
        triagem_repo_ultima_por_paciente(bumpPac, &tipoB, NULL, NULL, 0);
        espB = especialidade_provavel(tipoB);

        destinoMed = medico_livre_mais_proximo(espB, regiaoB, data, horario,
                                               &destinoRegiao);
        if (destinoMed != 0)
        {
            agendamento_repo_criar(bumpPac, destinoMed, data, horario);
        }

        snprintf(nomeRegiao, sizeof(nomeRegiao), "%s",
                 regiao_df_nome(destinoRegiao));

        escrito = snprintf(buffer, (size_t)tamanho,
                           "{\"agendado\":true,\"pacienteId\":%d,\"medicoId\":%d,"
                           "\"data\":%s,\"horario\":%s,\"preempcao\":true,"
                           "\"realocado\":{\"pacienteId\":%d,\"medicoId\":%d,"
                           "\"regiao\":%d,\"regiaoNome\":\"%s\",\"reagendado\":%s}}",
                           paciente_id, bumpMed, dataJson, horarioJson,
                           bumpPac, destinoMed, destinoRegiao, nomeRegiao,
                           destinoMed != 0 ? "true" : "false");
        return (escrito > 0 && escrito < tamanho) ? 1 : 0;
    }
}

int triagem_service_encaminhar_json(int paciente_id, const char *especialidade,
                                    const char *data, const char *horario,
                                    char *buffer, int tamanho)
{
    int regiao;
    int ids[SVC_MAX_MEDICOS];
    int total;
    int i;
    int escolhido = 0;
    char especialidadeJson[64];
    char dataJson[32];
    char horarioJson[24];
    int escrito;

    if (buffer == NULL || tamanho <= 0 || paciente_id <= 0)
    {
        return 0;
    }

    if (especialidade == NULL || especialidade[0] == '\0' ||
        data == NULL || data[0] == '\0' ||
        horario == NULL || horario[0] == '\0')
    {
        return 0;
    }

    if (agendamento_repo_horario_valido(horario) == 0)
    {
        snprintf(buffer, (size_t)tamanho,
                 "{\"encaminhado\":false,\"motivo\":\"horario fora da grade ou do expediente\"}");
        return 0;
    }

    regiao = paciente_repo_regiao(paciente_id);

    if (regiao < 0)
    {
        return 0;
    }

    total = medico_repo_ids_por_especialidade_regiao(especialidade, regiao,
                                                     ids, SVC_MAX_MEDICOS);

    for (i = 0; i < total; i++)
    {
        if (agendamento_repo_medico_ocupado(ids[i], data, horario) == 0)
        {
            escolhido = ids[i];
            break;
        }
    }

    if (escolhido == 0)
    {
        snprintf(buffer, (size_t)tamanho,
                 "{\"encaminhado\":false,\"motivo\":\"sem medico disponivel\"}");
        return 0;
    }

    if (agendamento_repo_criar(paciente_id, escolhido, data, horario) == 0)
    {
        snprintf(buffer, (size_t)tamanho,
                 "{\"encaminhado\":false,\"motivo\":\"falha ao gravar agendamento\"}");
        return 0;
    }

    if (repo_json_escapar(especialidadeJson, sizeof(especialidadeJson), especialidade) == 0 ||
        repo_json_escapar(dataJson, sizeof(dataJson), data) == 0 ||
        repo_json_escapar(horarioJson, sizeof(horarioJson), horario) == 0)
    {
        return 0;
    }

    escrito = snprintf(buffer, (size_t)tamanho,
                       "{\"encaminhado\":true,\"pacienteId\":%d,\"especialidade\":%s,"
                       "\"medicoId\":%d,\"data\":%s,\"horario\":%s}",
                       paciente_id, especialidadeJson, escolhido, dataJson, horarioJson);

    if (escrito < 0 || escrito >= tamanho)
    {
        return 0;
    }

    return 1;
}
