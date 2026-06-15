#include "relatorio_service.h"
#include "paciente_repository.h"
#include "medico_repository.h"
#include "triagem_repository.h"
#include "agendamento_repository.h"
#include "prontuario_repository.h"
#include "exame_repository.h"
#include "repo_json.h"

#include <stdio.h>

int relatorio_service_indicadores_json(char *buffer, int tamanho)
{
    int pacientes;
    int medicos;
    int triagens;
    int agendamentos;
    int emergencia;
    int muitoPrioritario;
    int prioritario;
    int comum;
    int orientacaoBasica;
    int casosGraves;
    int escrito;

    if (buffer == NULL || tamanho <= 0)
    {
        return 0;
    }

    pacientes = paciente_repo_contar_ativos();
    medicos = medico_repo_contar_ativos();
    triagens = triagem_repo_contar_ativos();
    agendamentos = agendamento_repo_contar_ativos();
    emergencia = triagem_repo_contar_por_classificacao("Emergencia");
    muitoPrioritario = triagem_repo_contar_por_classificacao("Muito prioritario");
    prioritario = triagem_repo_contar_por_classificacao("Prioritario");
    comum = triagem_repo_contar_por_classificacao("Comum");
    orientacaoBasica = triagem_repo_contar_por_classificacao("Orientacao basica");

    if (pacientes < 0 || medicos < 0 || triagens < 0 || agendamentos < 0 ||
        emergencia < 0 || muitoPrioritario < 0 || prioritario < 0 ||
        comum < 0 || orientacaoBasica < 0)
    {
        return 0;
    }

    casosGraves = emergencia + muitoPrioritario;

    escrito = snprintf(buffer, (size_t)tamanho,
        "{\"pacientesAtivos\":%d,\"medicosAtivos\":%d,\"triagensAtivas\":%d,"
        "\"agendamentosAtivos\":%d,\"triagensPorClassificacao\":{"
        "\"emergencia\":%d,\"muitoPrioritario\":%d,\"prioritario\":%d,"
        "\"comum\":%d,\"orientacaoBasica\":%d},\"casosGraves\":%d}",
        pacientes, medicos, triagens, agendamentos,
        emergencia, muitoPrioritario, prioritario, comum, orientacaoBasica,
        casosGraves);

    if (escrito < 0 || escrito >= tamanho)
    {
        return 0;
    }

    return 1;
}

int relatorio_service_distribuicao_json(char *buffer, int tamanho)
{
    char porRegiao[2048];
    char porEspecialidade[2048];
    int escrito;

    if (buffer == NULL || tamanho <= 0)
    {
        return 0;
    }

    if (paciente_repo_distribuicao_por_regiao_json(
            porRegiao, sizeof(porRegiao)) != 1)
    {
        return 0;
    }

    if (medico_repo_distribuicao_por_especialidade_json(
            porEspecialidade, sizeof(porEspecialidade)) != 1)
    {
        return 0;
    }

    escrito = snprintf(buffer, (size_t)tamanho,
        "{\"pacientesPorRegiao\":%s,\"medicosPorEspecialidade\":%s}",
        porRegiao, porEspecialidade);

    if (escrito < 0 || escrito >= tamanho)
    {
        return 0;
    }

    return 1;
}

int relatorio_service_agendamentos_periodo_json(const char *inicio,
                                                const char *fim,
                                                char *buffer, int tamanho)
{
    char porDia[2048];
    char inicioJson[32];
    char fimJson[32];
    int total;
    int escrito;

    if (buffer == NULL || tamanho <= 0 ||
        inicio == NULL || inicio[0] == '\0' || fim == NULL || fim[0] == '\0')
    {
        return 0;
    }

    total = agendamento_repo_contar_por_periodo(inicio, fim);

    if (total < 0)
    {
        return 0;
    }

    if (agendamento_repo_distribuicao_por_dia_json(
            inicio, fim, porDia, sizeof(porDia)) != 1)
    {
        return 0;
    }

    if (repo_json_escapar(inicioJson, sizeof(inicioJson), inicio) == 0 ||
        repo_json_escapar(fimJson, sizeof(fimJson), fim) == 0)
    {
        return 0;
    }

    escrito = snprintf(buffer, (size_t)tamanho,
        "{\"inicio\":%s,\"fim\":%s,\"total\":%d,\"porDia\":%s}",
        inicioJson, fimJson, total, porDia);

    if (escrito < 0 || escrito >= tamanho)
    {
        return 0;
    }

    return 1;
}

int relatorio_service_resumo_medico_json(int medico_id, char *buffer, int tamanho)
{
    int pacientes;
    int agendamentos;
    int prontuarios;
    int exames;
    int escrito;

    if (buffer == NULL || tamanho <= 0 || medico_id <= 0)
    {
        return 0;
    }

    pacientes = paciente_repo_contar_por_medico(medico_id);
    agendamentos = agendamento_repo_contar_por_medico(medico_id);
    prontuarios = prontuario_repo_contar_por_medico(medico_id);
    exames = exame_repo_contar_por_medico(medico_id);

    if (pacientes < 0 || agendamentos < 0 || prontuarios < 0 || exames < 0)
    {
        return 0;
    }

    escrito = snprintf(buffer, (size_t)tamanho,
        "{\"medicoId\":%d,\"pacientes\":%d,\"agendamentos\":%d,"
        "\"prontuarios\":%d,\"exames\":%d}",
        medico_id, pacientes, agendamentos, prontuarios, exames);

    if (escrito < 0 || escrito >= tamanho)
    {
        return 0;
    }

    return 1;
}
