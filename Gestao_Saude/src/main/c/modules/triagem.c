#include "triagem.h"

int calcularPontuacaoTriagem(int febre, int faltaAr, int dorIntensa, int pressaoAlta, int idadePaciente)
{
    int pontuacao = 0;

    if (faltaAr == 1)
        pontuacao += 5;
    if (dorIntensa == 1)
        pontuacao += 4;
    if (febre == 1)
        pontuacao += 2;
    if (pressaoAlta == 1)
        pontuacao += 2;
    if (idadePaciente >= 60 || idadePaciente <= 5)
        pontuacao += 2;

    return pontuacao;
}

void classificarTriagem(int pontuacao, char classificacao[])
{
    if (pontuacao >= 8)
        strcpy(classificacao, "Emergencia");
    else if (pontuacao >= 5)
        strcpy(classificacao, "Muito prioritario");
    else if (pontuacao >= 3)
        strcpy(classificacao, "Prioritario");
    else if (pontuacao >= 1)
        strcpy(classificacao, "Comum");
    else
        strcpy(classificacao, "Orientacao basica");
}
void menuTriagem(void)
{
    int caso7;

    do
    {
        printf("\n=============================================\n");
        printf("MENU GERENCIAMENTO DE TRIAGEM\n");
        printf("=============================================\n");
        printf("1. Realizar Triagem\n");
        printf("2. Listar Triagens\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("---------------------------------------------\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &caso7);

        switch (caso7)
        {
        case 1:
        {
            int pacienteId;
            int idadePaciente = 0;
            int pacienteEncontrado = 0;

            if (totalTriagens >= MAX_TRIAGENS)
            {
                printf("\nLimite de triagens atingido.\n");
                break;
            }

            printf("\nID do paciente: ");
            scanf("%d", &pacienteId);

            for (int i = 0; i < totalPacientes; i++)
            {
                if (pacientes[i].id == pacienteId && pacientes[i].ativo == 1)
                {
                    pacienteEncontrado = 1;
                    idadePaciente = pacientes[i].idade;
                    break;
                }
            }

            if (pacienteEncontrado == 0)
            {
                printf("\nPaciente nao encontrado ou inativo.\n");
                break;
            }

            triagens[totalTriagens].id = totalTriagens + 1;
            triagens[totalTriagens].pacienteId = pacienteId;

            printf("Febre? (1-Sim / 0-Nao): ");
            scanf("%d", &triagens[totalTriagens].febre);

            printf("Falta de ar? (1-Sim / 0-Nao): ");
            scanf("%d", &triagens[totalTriagens].faltaAr);

            printf("Dor intensa? (1-Sim / 0-Nao): ");
            scanf("%d", &triagens[totalTriagens].dorIntensa);

            printf("Pressao alta? (1-Sim / 0-Nao): ");
            scanf("%d", &triagens[totalTriagens].pressaoAlta);

            triagens[totalTriagens].pontuacao = calcularPontuacaoTriagem(
                triagens[totalTriagens].febre,
                triagens[totalTriagens].faltaAr,
                triagens[totalTriagens].dorIntensa,
                triagens[totalTriagens].pressaoAlta,
                idadePaciente);

            classificarTriagem(
                triagens[totalTriagens].pontuacao,
                triagens[totalTriagens].classificacao);
                
            printf("\nTriagem registrada com sucesso.\n");
            printf("Pontuacao: %d\n", triagens[totalTriagens].pontuacao);
            printf("Classificacao: %s\n", triagens[totalTriagens].classificacao);

            totalTriagens++;
            break;
        }

        case 2:
        {
            if (totalTriagens == 0)
            {
                printf("\nNenhuma triagem cadastrada.\n");
                break;
            }

            printf("\nLista de Triagens:\n");

            for (int i = 0; i < totalTriagens; i++)
            {
                printf("\nID: %d\n", triagens[i].id);
                printf("Paciente ID: %d\n", triagens[i].pacienteId);
                printf("Pontuacao: %d\n", triagens[i].pontuacao);
                printf("Classificacao: %s\n", triagens[i].classificacao);
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

    } while (caso7 != 0);
}
