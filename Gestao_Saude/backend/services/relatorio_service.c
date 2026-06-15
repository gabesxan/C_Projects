#include "relatorio_service.h"
#include "paciente_repository.h"
#include "medico_repository.h"
#include "triagem_repository.h"
#include "agendamento_repository.h"

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
