#include "agendamento.h"

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

        scanf("%d", &caso3);

        switch (caso3)
        {
        case 1:
        {
            int pacienteId;
            int medicoId;
            int pacienteEncontrado = 0;
            int medicoEncontrado = 0;
            int conflito = 0;
            char data[11];
            char horario[6];

            if (totalAgendamentos >= MAX_AGENDAMENTOS)
            {
                printf("\nLimite de agendamentos atingido.\n");
                break;
            }

            printf("\nID do paciente: ");
            scanf("%d", &pacienteId);

            for (int i = 0; i < totalPacientes; i++)
            {
                if (pacientes[i].id == pacienteId && pacientes[i].ativo == 1)
                {
                    pacienteEncontrado = 1;
                    break;
                }
            }

            if (pacienteEncontrado == 0)
            {
                printf("\nPaciente nao encontrado ou inativo.\n");
                break;
            }

            printf("ID do medico: ");
            scanf("%d", &medicoId);

            for (int i = 0; i < totalMedicos; i++)
            {
                if (medicos[i].id == medicoId && medicos[i].ativo == 1)
                {
                    medicoEncontrado = 1;
                    break;
                }
            }

            if (medicoEncontrado == 0)
            {
                printf("\nMedico nao encontrado ou inativo.\n");
                break;
            }

            printf("Data (DD/MM/AAAA): ");
            scanf(" %[^\n]", data);

            printf("Horario (HH:MM): ");
            scanf(" %[^\n]", horario);

            for (int i = 0; i < totalAgendamentos; i++)
            {
                if (agendamentos[i].medicoId == medicoId &&
                    strcmp(agendamentos[i].data, data) == 0 &&
                    strcmp(agendamentos[i].horario, horario) == 0 &&
                    strcmp(agendamentos[i].status, "CANCELADO") != 0)
                {
                    conflito = 1;
                    break;
                }
            }

            if (conflito == 1)
            {
                printf("\nEste medico ja possui agendamento nesse dia e horario.\n");
                break;
            }

            agendamentos[totalAgendamentos].id = totalAgendamentos + 1;
            agendamentos[totalAgendamentos].pacienteId = pacienteId;
            agendamentos[totalAgendamentos].medicoId = medicoId;
            strcpy(agendamentos[totalAgendamentos].data, data);
            strcpy(agendamentos[totalAgendamentos].horario, horario);
            strcpy(agendamentos[totalAgendamentos].status, "AGENDADO");

            totalAgendamentos++;

            printf("\nAgendamento criado com sucesso! ID: %d\n", agendamentos[totalAgendamentos - 1].id);
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
                printf("\nID: %d\n", agendamentos[i].id);
                printf("Paciente ID: %d\n", agendamentos[i].pacienteId);
                printf("Medico ID: %d\n", agendamentos[i].medicoId);
                printf("Data: %s\n", agendamentos[i].data);
                printf("Horario: %s\n", agendamentos[i].horario);
                printf("Status: %s\n", agendamentos[i].status);
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
