#include "leito.h"

int cadastrarLeito(int alaId, int numero)
{
    int alaEncontrada = 0;

    if (totalLeitos >= MAX_LEITOS)
    {
        return 0;
    }

    for (int i = 0; i < totalAlas; i++)
    {
        if (alas[i].id == alaId && alas[i].ativo == 1)
        {
            alaEncontrada = 1;
            break;
        }
    }

    if (alaEncontrada == 0)
    {
        return 0;
    }

    leitos[totalLeitos].id = totalLeitos + 1;
    leitos[totalLeitos].alaId = alaId;
    leitos[totalLeitos].numero = numero;
    leitos[totalLeitos].ocupado = 0;
    leitos[totalLeitos].pacienteId = 0;

    totalLeitos++;

    return 1;
}

void menuLeitos(void)
{
    int caso5;

    do
    {
        printf("\n=============================================\n");
        printf("MENU GERENCIAMENTO DE LEITOS\n");
        printf("=============================================\n");
        printf("1. Cadastrar Leito\n");
        printf("2. Listar Leitos\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("---------------------------------------------\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &caso5);

        switch (caso5)
        {
        case 1:
        {
            int alaId;
            int numero;

            printf("\nID da ala: ");
            scanf("%d", &alaId);

            printf("Numero do leito: ");
            scanf("%d", &numero);

            if (cadastrarLeito(alaId, numero) == 0)
            {
                printf("\nNao foi possivel cadastrar o leito. Verifique se a ala existe, esta ativa ou se o limite foi atingido.\n");
                break;
            }

            printf("\nLeito cadastrado com sucesso! ID: %d\n", leitos[totalLeitos - 1].id);
            break;
        }

        case 2:
        {
            if (totalLeitos == 0)
            {
                printf("\nNenhum leito cadastrado.\n");
                break;
            }

            printf("\nLista de Leitos:\n");

            for (int i = 0; i < totalLeitos; i++)
            {
                printf("\nID: %d\n", leitos[i].id);
                printf("Ala ID: %d\n", leitos[i].alaId);
                printf("Numero: %d\n", leitos[i].numero);
                printf("Ocupado: %s\n", leitos[i].ocupado == 1 ? "Sim" : "Nao");
                printf("Paciente ID: %d\n", leitos[i].pacienteId);
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

    } while (caso5 != 0);
}
