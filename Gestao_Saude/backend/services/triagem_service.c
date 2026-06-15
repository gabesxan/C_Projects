#include "triagem_service.h"
#include "triagem_repository.h"
#include "paciente_repository.h"
#include "medico_repository.h"
#include "prontuario_repository.h"
#include "exame_repository.h"
#include "agendamento_repository.h"
#include "repo_json.h"

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

/* Fonte unica do mapa classificacao -> nivel de prioridade. */
static int prioridade_de(const char *classificacao)
{
    if (strcmp(classificacao, "Emergencia") == 0)
    {
        return 5;
    }
    if (strcmp(classificacao, "Muito prioritario") == 0)
    {
        return 4;
    }
    if (strcmp(classificacao, "Prioritario") == 0)
    {
        return 3;
    }
    if (strcmp(classificacao, "Comum") == 0)
    {
        return 2;
    }
    if (strcmp(classificacao, "Orientacao basica") == 0)
    {
        return 1;
    }

    return 0;
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

    if (buffer == NULL || tamanho <= 0 || paciente_id <= 0)
    {
        return 0;
    }

    if (data == NULL || data[0] == '\0' || horario == NULL || horario[0] == '\0')
    {
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

    especialidade = especialidade_provavel(tipo);
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
            "{\"agendado\":false,\"motivo\":\"sem medico disponivel\"}");
        return 0;
    }

    if (agendamento_repo_criar(paciente_id, escolhido, data, horario) == 0)
    {
        snprintf(buffer, (size_t)tamanho,
            "{\"agendado\":false,\"motivo\":\"falha ao gravar agendamento\"}");
        return 0;
    }

    if (repo_json_escapar(dataJson, sizeof(dataJson), data) == 0 ||
        repo_json_escapar(horarioJson, sizeof(horarioJson), horario) == 0)
    {
        return 0;
    }

    escrito = snprintf(buffer, (size_t)tamanho,
        "{\"agendado\":true,\"pacienteId\":%d,\"medicoId\":%d,"
        "\"data\":%s,\"horario\":%s}",
        paciente_id, escolhido, dataJson, horarioJson);

    if (escrito < 0 || escrito >= tamanho)
    {
        return 0;
    }

    return 1;
}
