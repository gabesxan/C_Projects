#include "ala.h"

static void nomeTipoAla(int tipo)
{
    switch (tipo)
    {
    case ALA_INTERNACAO:
        printf("Internacao");
        break;

    case ALA_UTI:
        printf("UTI");
        break;

    case ALA_OBSERVACAO:
        printf("Observacao");
        break;

    case ALA_PEDIATRIA:
        printf("Pediatria");
        break;

    case ALA_CIRURGICA:
        printf("Cirurgica");
        break;
        
    default:
        printf("Nao informado");
        break;
    }
}

static int tipoAlaValido(int tipo)
{
    return tipo >= ALA_INTERNACAO && tipo <= ALA_CIRURGICA;
}

static void exibirOpcoesTipoAla(void)
{
    printf("\nTipos de ala:\n");
    printf("1. Internacao\n");
    printf("2. UTI\n");
    printf("3. Observacao\n");
    printf("4. Pediatria\n");
    printf("5. Cirurgica\n");
}

int cadastrarAla(const char nome[], int tipo, int totalLeitosAla)
{
    if (totalAlas >= MAX_ALAS)
    {
        return 0;
    }

    if (tipoAlaValido(tipo) == 0)
    {
        return 0;
    }

    alas[totalAlas].id = totalAlas + 1;
    strcpy(alas[totalAlas].nome, nome);
    alas[totalAlas].tipo = tipo;
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

int contarAlasPorTipo(int tipo)
{
    int total = 0;

    for (int i = 0; i < totalAlas; i++)
    {
        if (alas[i].ativo == 1 && alas[i].tipo == tipo)
        {
            total++;
        }
    }

    return total;
}

void listarAlasPorTipo(int tipo)
{
    int encontrou = 0;

    printf("\nAlas do tipo ");
    nomeTipoAla(tipo);
    printf(":\n");

    for (int i = 0; i < totalAlas; i++)
    {
        if (alas[i].ativo == 1 && alas[i].tipo == tipo)
        {
            printf("\nID: %d\n", alas[i].id);
            printf("Nome: %s\n", alas[i].nome);
            printf("Tipo: ");
            nomeTipoAla(alas[i].tipo);
            printf("\n");
            printf("Total de leitos: %d\n", alas[i].totalLeitos);
            printf("Leitos ocupados: %d\n", alas[i].leitosOcupados);
            encontrou = 1;
        }
    }

    if (encontrou == 0)
    {
        printf("\nNenhuma ala encontrada para esse tipo.\n");
    }
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
        printf("4. Consultar Alas por Tipo\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("---------------------------------------------\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &caso4);

        switch (caso4)
        {
        case 1:
        {
            char nome[100];
            int tipo;
            int totalLeitosAla;

            if (totalAlas >= MAX_ALAS)
            {
                printf("\nLimite de alas atingido.\n");
                break;
            }

            printf("\nNome da ala: ");
            scanf(" %[^\n]", nome);

            exibirOpcoesTipoAla();
            printf("Escolha o tipo da ala: ");
            scanf("%d", &tipo);

            if (tipoAlaValido(tipo) == 0)
            {
                printf("\nTipo de ala invalido.\n");
                break;
            }

            printf("Total de leitos: ");
            scanf("%d", &totalLeitosAla);

            if (cadastrarAla(nome, tipo, totalLeitosAla) == 1)
            {
                printf("\nAla cadastrada com sucesso! ID: %d\n", alas[totalAlas - 1].id);
            }
            else
            {
                printf("\nNao foi possivel cadastrar a ala.\n");
            }

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
                    printf("Tipo: ");
                    nomeTipoAla(alas[i].tipo);
                    printf("\n");
                    printf("Total de leitos: %d\n", alas[i].totalLeitos);
                    printf("Leitos ocupados: %d\n", alas[i].leitosOcupados);
                }
            }

            break;
        }

        case 3:
        {
            int idBusca;

            printf("\nDigite o ID da ala que deseja excluir: ");
            scanf("%d", &idBusca);

            if (excluirAla(idBusca) == 1)
            {
                printf("\nAla removida com sucesso.\n");
            }
            else
            {
                printf("\nAla nao encontrada ou ja inativa.\n");
            }

            break;
        }

        case 4:
        {
            int tipo;

            exibirOpcoesTipoAla();
            printf("Escolha o tipo que deseja consultar: ");
            scanf("%d", &tipo);

            if (tipoAlaValido(tipo) == 0)
            {
                printf("\nTipo de ala invalido.\n");
                break;
            }

            printf("\nTotal de alas desse tipo: %d\n", contarAlasPorTipo(tipo));
            listarAlasPorTipo(tipo);
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
