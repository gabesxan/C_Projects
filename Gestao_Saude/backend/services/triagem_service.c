#include "triagem_service.h"
#include "triagem_repository.h"
#include "repo_json.h"

#include <stdio.h>
#include <string.h>

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
