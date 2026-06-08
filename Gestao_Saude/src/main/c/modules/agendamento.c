#include "agendamento.h"
#include "prontuario.h"
#include "triagem.h"

static int buscarPaciente(int pacienteId)
{
    for (int i = 0; i < totalPacientes; i++)
    {
        if (pacientes[i].id == pacienteId && pacientes[i].ativo == 1)
        {
            return i;
        }
    }

    return -1;
}

static int mudarStatus(int id, const char status[])
{
    for (int i = 0; i < totalAgendamentos; i++)
    {
        if (agendamentos[i].id == id &&
            strcmp(agendamentos[i].status, "AGENDADO") == 0)
        {
            strcpy(agendamentos[i].status, status);
            return 1;
        }
    }

    return 0;
}

static int buscarAgendamentoPorId(int id)
{
    for (int i = 0; i < totalAgendamentos; i++)
    {
        if (agendamentos[i].id == id)
        {
            return i;
        }
    }

    return -1;
}

static void exibirAgendamento(const Agendamento *agendamento)
{
    printf("\nID: %d\n", agendamento->id);
    printf("Paciente ID: %d\n", agendamento->pacienteId);
    printf("Medico ID: %d\n", agendamento->medicoId);
    printf("Data: %s\n", agendamento->data);
    printf("Horario: %s\n", agendamento->horario);
    printf("Status: %s\n", agendamento->status);
}

int buscarAgenda(int medicoId, char data[], char horario[])
{
    for (int i = 0; i < totalAgendamentos; i++)
    {
        if (agendamentos[i].medicoId == medicoId &&
            strcmp(agendamentos[i].data, data) == 0 &&
            strcmp(agendamentos[i].horario, horario) == 0 &&
            strcmp(agendamentos[i].status, "AGENDADO") == 0)
        {
            return i;
        }
    }

    return -1;
}

int medicoOcupado(int medicoId, char data[], char horario[])
{
    return buscarAgenda(medicoId, data, horario) != -1;
}

static int nivelRegiao(int regiaoPaciente, int regiaoMedico)
{
    if (regiaoPaciente == regiaoMedico)
        return 0;

    if ((regiaoPaciente == 1 && regiaoMedico == 7) ||
        (regiaoPaciente == 7 && regiaoMedico == 1) ||
        (regiaoPaciente == 2 && regiaoMedico == 3) ||
        (regiaoPaciente == 3 && regiaoMedico == 2) ||
        (regiaoPaciente == 3 && regiaoMedico == 8) ||
        (regiaoPaciente == 8 && regiaoMedico == 3) ||
        (regiaoPaciente == 4 && regiaoMedico == 2) ||
        (regiaoPaciente == 2 && regiaoMedico == 4) ||
        (regiaoPaciente == 5 && regiaoMedico == 4) ||
        (regiaoPaciente == 4 && regiaoMedico == 5) ||
        (regiaoPaciente == 6 && regiaoMedico == 1) ||
        (regiaoPaciente == 1 && regiaoMedico == 6))
        return 1;

    return 2;
}

int cancelarAgendamento(int id)
{
    return mudarStatus(id, "CANCELADO");
}

int concluirAgendamento(int id)
{
    int indiceAgendamento = buscarAgendamentoPorId(id);

    if (indiceAgendamento == -1)
    {
        return 0;
    }

    if (strcmp(agendamentos[indiceAgendamento].status, "AGENDADO") != 0)
    {
        return 0;
    }

    strcpy(agendamentos[indiceAgendamento].status, "CONCLUIDO");

    if (criarProntuarioAutomatico(agendamentos[indiceAgendamento].pacienteId,
                                  agendamentos[indiceAgendamento].medicoId,
                                  agendamentos[indiceAgendamento].data) == 0)
    {
        strcpy(agendamentos[indiceAgendamento].status, "AGENDADO");
        return 0;
    }

    return 1;
}

static int criarAgendamento(int pacienteId, int medicoId, char data[], char horario[])
{
    if (totalAgendamentos >= MAX_AGENDAMENTOS)
    {
        return 0;
    }

    agendamentos[totalAgendamentos].id = totalAgendamentos + 1;
    agendamentos[totalAgendamentos].pacienteId = pacienteId;
    agendamentos[totalAgendamentos].medicoId = medicoId;
    strcpy(agendamentos[totalAgendamentos].data, data);
    strcpy(agendamentos[totalAgendamentos].horario, horario);
    strcpy(agendamentos[totalAgendamentos].status, "AGENDADO");

    totalAgendamentos++;

    return medicoId;
}

const char *obterEspecialidade(int tipoTriagem)
{
    switch (tipoTriagem)
    {
    case TRIAGEM_ORTOPEDIA:
        return "Ortopedia";
    case TRIAGEM_CARDIOLOGIA:
        return "Cardiologia";
    case TRIAGEM_PNEUMOLOGIA:
        return "Pneumologia";
    case TRIAGEM_PEDIATRIA:
        return "Pediatria";
    case TRIAGEM_GERAL:
        return "Clinico Geral";
    default:
        return "";
    }
}

int buscarMedicoRegiao(const char especialidade[], int regiaoAdministrativa, char data[], char horario[])
{
    for (int i = 0; i < totalMedicos; i++)
    {
        if (medicos[i].ativo == 1 &&
            strcmp(medicos[i].especialidade, especialidade) == 0 &&
            medicos[i].regiaoAdministrativa == regiaoAdministrativa &&
            medicoOcupado(medicos[i].id, data, horario) == 0)
        {
            return medicos[i].id;
        }
    }

    return 0;
}

int buscarMedico(const char especialidade[], char data[], char horario[])
{
    for (int i = 0; i < totalMedicos; i++)
    {
        if (medicos[i].ativo == 1 &&
            strcmp(medicos[i].especialidade, especialidade) == 0 &&
            medicoOcupado(medicos[i].id, data, horario) == 0)
        {
            return medicos[i].id;
        }
    }

    return 0;
}

int trocaHorario(int pacienteNovo, int pacienteAtual)
{
    int indiceNova = triagemAtual(pacienteNovo);
    int indiceAtual = triagemAtual(pacienteAtual);
    int prioridadeNova;
    int prioridadeAtual;

    if (indiceNova == -1 || indiceAtual == -1)
    {
        return 0;
    }

    if (ehUrgente(triagens[indiceNova].classificacao) == 0)
    {
        return 0;
    }

    prioridadeNova = nivelPrioridade(triagens[indiceNova].classificacao);
    prioridadeAtual = nivelPrioridade(triagens[indiceAtual].classificacao);

    return prioridadeNova > prioridadeAtual;
}

int agendarMedico(int medicoId, int pacienteId, char data[], char horario[])
{
    int agendamentoAtual = buscarAgenda(medicoId, data, horario);
    int pacienteAtualId;

    if (agendamentoAtual == -1)
    {
        return criarAgendamento(pacienteId, medicoId, data, horario);
    }

    pacienteAtualId = agendamentos[agendamentoAtual].pacienteId;

    if (trocaHorario(pacienteId, pacienteAtualId) == 0)
    {
        return 0;
    }

    strcpy(agendamentos[agendamentoAtual].status, "REMANEJADO");

    return criarAgendamento(pacienteId, medicoId, data, horario);
}

static int remanejarRegiao(const char especialidade[], int regiaoAdministrativa, int pacienteId, char data[], char horario[])
{
    int medicoSelecionado;

    for (int i = 0; i < totalMedicos; i++)
    {
        if (medicos[i].ativo == 1 &&
            strcmp(medicos[i].especialidade, especialidade) == 0 &&
            medicos[i].regiaoAdministrativa == regiaoAdministrativa &&
            medicoOcupado(medicos[i].id, data, horario) == 1)
        {
            medicoSelecionado = agendarMedico(medicos[i].id, pacienteId, data, horario);

            if (medicoSelecionado != 0)
            {
                return medicoSelecionado;
            }
        }
    }

    return 0;
}

static int buscarProxima(const char especialidade[], int regiaoAdministrativa, char data[], char horario[])
{
    int melhorMedico = 0;
    int melhorPrioridade = 99;

    for (int i = 0; i < totalMedicos; i++)
    {
        if (medicos[i].ativo == 1 &&
            strcmp(medicos[i].especialidade, especialidade) == 0 &&
            medicos[i].regiaoAdministrativa != regiaoAdministrativa &&
            medicoOcupado(medicos[i].id, data, horario) == 0)
        {
            int prioridade = nivelRegiao(regiaoAdministrativa, medicos[i].regiaoAdministrativa);

            if (prioridade < melhorPrioridade)
            {
                melhorPrioridade = prioridade;
                melhorMedico = medicos[i].id;
            }
        }
    }

    return melhorMedico;
}

static int remanejarExterno(const char especialidade[], int regiaoAdministrativa, int pacienteId, char data[], char horario[])
{
    int medicoSelecionado;

    for (int i = 0; i < totalMedicos; i++)
    {
        if (medicos[i].ativo == 1 &&
            strcmp(medicos[i].especialidade, especialidade) == 0 &&
            medicos[i].regiaoAdministrativa != regiaoAdministrativa &&
            medicoOcupado(medicos[i].id, data, horario) == 1)
        {
            medicoSelecionado = agendarMedico(medicos[i].id, pacienteId, data, horario);

            if (medicoSelecionado != 0)
            {
                return medicoSelecionado;
            }
        }
    }

    return 0;
}

int agendarTriagem(int pacienteId, char data[], char horario[])
{
    int regiaoPaciente = 0;
    int triagemId;
    int medicoId = 0;
    const char *especialidade;

    if (totalAgendamentos >= MAX_AGENDAMENTOS)
    {
        return 0;
    }

    for (int i = 0; i < totalPacientes; i++)
    {
        if (pacientes[i].id == pacienteId && pacientes[i].ativo == 1)
        {
            regiaoPaciente = pacientes[i].regiaoAdministrativa;
            break;
        }
    }

    if (regiaoPaciente == 0)
    {
        return 0;
    }

    triagemId = triagemAtual(pacienteId);

    if (triagemId == -1)
    {
        return 0;
    }

    especialidade = obterEspecialidade(triagens[triagemId].tipoTriagem);

    if (strlen(especialidade) == 0)
    {
        return 0;
    }

    medicoId = buscarMedicoRegiao(especialidade, regiaoPaciente, data, horario);

    if (medicoId != 0)
    {
        return agendarMedico(medicoId, pacienteId, data, horario);
    }

    medicoId = remanejarRegiao(especialidade, regiaoPaciente, pacienteId, data, horario);

    if (medicoId != 0)
    {
        return medicoId;
    }

    medicoId = buscarProxima(especialidade, regiaoPaciente, data, horario);

    if (medicoId != 0)
    {
        return agendarMedico(medicoId, pacienteId, data, horario);
    }

    return remanejarExterno(especialidade, regiaoPaciente, pacienteId, data, horario);
}

void menuAgendamentos(void)
{
    int caso3;

    do
    {
        printf("\n=============================================\n");
        printf("MENU GERENCIAMENTO DE AGENDAMENTOS\n");
        printf("=============================================\n");
        printf("1. Criar Agendamento\n");
        printf("2. Listar Agendamentos\n");
        printf("3. Cancelar Agendamento\n");
        printf("4. Concluir Agendamento\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("---------------------------------------------\n");
        printf("Escolha uma opcao: ");

        if (lerInteiro(&caso3) == 0)
        {
            printf("\nOpcao invalida. Tente novamente.\n");
            continue;
        }

        switch (caso3)
        {
        case 1:
        {
            int pacienteId;
            int indicePaciente;
            int medicoSelecionado;
            char data[11];
            char horario[6];

            if (totalAgendamentos >= MAX_AGENDAMENTOS)
            {
                printf("\nLimite de agendamentos atingido.\n");
                break;
            }

            printf("\nID do paciente: ");
            scanf("%d", &pacienteId);

            indicePaciente = buscarPaciente(pacienteId);

            if (indicePaciente == -1)
            {
                printf("\nPaciente nao encontrado ou inativo.\n");
                break;
            }

            printf("Data (DD/MM/AAAA): ");
            scanf(" %[^\n]", data);

            printf("Horario (HH:MM): ");
            scanf(" %[^\n]", horario);

            medicoSelecionado = agendarTriagem(pacienteId, data, horario);

            if (medicoSelecionado == 0)
            {
                printf("\nNao foi possivel criar o agendamento.\n");
                printf("Verifique se o paciente possui triagem ativa e se ha medico disponivel.\n");
                break;
            }

            printf("\nAgendamento criado com sucesso! ID: %d\n", agendamentos[totalAgendamentos - 1].id);
            printf("Medico selecionado automaticamente. ID: %d\n", medicoSelecionado);
            break;
        }

        case 2:
        {
            if (totalAgendamentos == 0)
            {
                printf("\nNenhum agendamento cadastrado.\n");
                break;
            }

            printf("\nLista de Agendamentos:\n");

            for (int i = 0; i < totalAgendamentos; i++)
            {
                exibirAgendamento(&agendamentos[i]);
            }

            break;
        }

        case 3:
        {
            int idBusca;
            int encontrado = 0;

            printf("\nDigite o ID do agendamento que deseja cancelar: ");
            scanf("%d", &idBusca);

            for (int i = 0; i < totalAgendamentos; i++)
            {
                if (agendamentos[i].id == idBusca)
                {
                    strcpy(agendamentos[i].status, "CANCELADO");
                    encontrado = 1;
                    printf("\nAgendamento cancelado com sucesso.\n");
                    break;
                }
            }

            if (encontrado == 0)
            {
                printf("\nAgendamento nao encontrado.\n");
            }

            break;
        }

        case 4:
        {
            int idBusca;
            int encontrado = 0;

            printf("\nDigite o ID do agendamento que deseja concluir: ");
            scanf("%d", &idBusca);

            for (int i = 0; i < totalAgendamentos; i++)
            {
                if (agendamentos[i].id == idBusca)
                {
                    strcpy(agendamentos[i].status, "CONCLUIDO");
                    encontrado = 1;
                    printf("\nAgendamento concluido com sucesso.\n");
                    break;
                }
            }

            if (encontrado == 0)
            {
                printf("\nAgendamento nao encontrado.\n");
            }

            break;
        }

        case 0:
            printf("\nVoltando ao menu principal...\n");
            break;

        default:
            printf("\nOpcao invalida. Tente novamente.\n");
            break;
        }

    } while (caso3 != 0);
}
