#include "ala.h"

int cadastrarAla(const char nome[], const char tipo[], int totalLeitosAla)
{
    if (totalAlas >= MAX_ALAS)
    {
        return 0;
    }

    alas[totalAlas].id = totalAlas + 1;
    strcpy(alas[totalAlas].nome, nome);
    strcpy(alas[totalAlas].tipo, tipo);
    alas[totalAlas].totalLeitos = totalLeitosAla;
    alas[totalAlas].leitosOcupados = 0;
    alas[totalAlas].ativo = 1;

    totalAlas++;

    return 1;
}

int excluirAla(int id)
{
    for (int i = 0; i < totalAlas; i++)
    {
        if (alas[i].id == id && alas[i].ativo == 1)
        {
            alas[i].ativo = 0;
            return 1;
        }
    }

    return 0;
}

void menuAlas(void)
{
    int caso4;

    do
    {
        printf("\n=============================================\n");
        printf("MENU GERENCIAMENTO DE ALAS\n");
        printf("=============================================\n");
        printf("1. Cadastrar Ala\n");
        printf("2. Listar Alas\n");
        printf("3. Excluir Ala\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("---------------------------------------------\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &caso4);

        switch (caso4)
        {
        case 1:
        {
            if (totalAlas >= MAX_ALAS)
            {
                printf("\nLimite de alas atingido.\n");
                break;
            }

            alas[totalAlas].id = totalAlas + 1;

            printf("\nNome da ala: ");
            scanf(" %[^\n]", alas[totalAlas].nome);

            printf("Tipo da ala: ");
            scanf(" %[^\n]", alas[totalAlas].tipo);

            printf("Total de leitos: ");
            scanf("%d", &alas[totalAlas].totalLeitos);

            alas[totalAlas].leitosOcupados = 0;
            alas[totalAlas].ativo = 1;
            totalAlas++;

            printf("\nAla cadastrada com sucesso! ID: %d\n", alas[totalAlas - 1].id);
            break;
        }

        case 2:
        {
            if (totalAlas == 0)
            {
                printf("\nNenhuma ala cadastrada.\n");
                break;
            }

            printf("\nLista de Alas:\n");

            for (int i = 0; i < totalAlas; i++)
            {
                if (alas[i].ativo == 1)
                {
                    printf("\nID: %d\n", alas[i].id);
                    printf("Nome: %s\n", alas[i].nome);
                    printf("Tipo: %s\n", alas[i].tipo);
                    printf("Total de leitos: %d\n", alas[i].totalLeitos);
                    printf("Leitos ocupados: %d\n", alas[i].leitosOcupados);
                }
            }

            break;
        }

        case 3:
        {
            int idBusca;
            int encontrado = 0;

            printf("\nDigite o ID da ala que deseja excluir: ");
            scanf("%d", &idBusca);

            for (int i = 0; i < totalAlas; i++)
            {
                if (alas[i].id == idBusca && alas[i].ativo == 1)
                {
                    alas[i].ativo = 0;
                    encontrado = 1;
                    printf("\nAla removida com sucesso.\n");
                    break;
                }
            }

            if (encontrado == 0)
            {
                printf("\nAla nao encontrada ou ja inativa.\n");
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

    } while (caso4 != 0);
}
