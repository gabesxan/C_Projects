#include "medico.h"

static void listarRegioes(void)
{
    printf("1. Plano Piloto\n");
    printf("2. Ceilandia\n");
    printf("3. Taguatinga\n");
    printf("4. Samambaia\n");
    printf("5. Gama\n");
    printf("6. Sobradinho\n");
    printf("7. Guara\n");
    printf("8. Aguas Claras\n");
}

static void nomeRegiao(int regiaoAdministrativa)
{
    switch (regiaoAdministrativa)
    {
    case 1:
        printf("Plano Piloto");
        break;
    case 2:
        printf("Ceilandia");
        break;
    case 3:
        printf("Taguatinga");
        break;
    case 4:
        printf("Samambaia");
        break;
    case 5:
        printf("Gama");
        break;
    case 6:
        printf("Sobradinho");
        break;
    case 7:
        printf("Guara");
        break;
    case 8:
        printf("Aguas Claras");
        break;
    default:
        printf("Nao informada");
        break;
    }
}

int cadastrarMedico(const char nome[], const char crm[], const char especialidade[], int regiaoAdministrativa)
{
    if (totalMedicos >= MAX_MEDICOS)
    {
        return 0;
    }

    medicos[totalMedicos].id = totalMedicos + 1;
    strcpy(medicos[totalMedicos].nome, nome);
    strcpy(medicos[totalMedicos].crm, crm);
    strcpy(medicos[totalMedicos].especialidade, especialidade);
    medicos[totalMedicos].regiaoAdministrativa = regiaoAdministrativa;
    medicos[totalMedicos].ativo = 1;

    totalMedicos++;

    return 1;
}

int excluirMedico(int id)
{
    for (int i = 0; i < totalMedicos; i++)
    {
        if (medicos[i].id == id && medicos[i].ativo == 1)
        {
            medicos[i].ativo = 0;
            return 1;
        }
    }

    return 0;
}

void menuMedicos(void)
{
    int caso2;

    do
    {
        printf("\n=============================================\n");
        printf("MENU GERENCIAMENTO DE MEDICOS\n");
        printf("=============================================\n");
        printf("1. Cadastrar Medico\n");
        printf("2. Listar Medicos\n");
        printf("3. Editar Medico\n");
        printf("4. Excluir Medico\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("---------------------------------------------\n");
        printf("Escolha uma opcao: ");

        scanf("%d", &caso2);

        switch (caso2)
        {
        case 1:
        {
            char nome[100];
            char crm[20];
            char especialidade[50];
            int regiaoAdministrativa;

            if (totalMedicos >= MAX_MEDICOS)
            {
                printf("\nLimite de medicos atingido.\n");
                break;
            }

            printf("\nNome do medico, sem Dr/Dr(a): ");
            scanf(" %[^\n]", nome);

            printf("Numero do CRM: ");
            scanf(" %[^\n]", crm);

            printf("Especialidade: ");
            scanf(" %[^\n]", especialidade);

            printf("Regiao administrativa do DF:\n");
            listarRegioes();
            printf("Escolha: ");
            scanf("%d", &regiaoAdministrativa);

            if (cadastrarMedico(nome, crm, especialidade, regiaoAdministrativa) == 0)
            {
                printf("\nNao foi possivel cadastrar o medico.\n");
                break;
            }

            printf("\nMedico cadastrado com sucesso! ID: %d\n", medicos[totalMedicos - 1].id);
            break;
        }

        case 2:
        {
            if (totalMedicos == 0)
            {
                printf("\nNenhum medico cadastrado.\n");
                break;
            }

            printf("\nLista de Medicos:\n");

            for (int i = 0; i < totalMedicos; i++)
            {
                if (medicos[i].ativo == 1)
                {
                    printf("\nID: %d\n", medicos[i].id);
                    printf("Nome: Dr(a). %s\n", medicos[i].nome);
                    printf("CRM: CRM%s\n", medicos[i].crm);
                    printf("Especialidade: %s\n", medicos[i].especialidade);
                    printf("Regiao administrativa: ");
                    nomeRegiao(medicos[i].regiaoAdministrativa);
                    printf("\n");
                }
            }

            break;
        }

        case 3:
        {
            int idBusca;
            int encontrado = 0;
            int opcaoEditar;

            printf("\nDigite o ID do medico que deseja editar: ");
            scanf("%d", &idBusca);

            for (int i = 0; i < totalMedicos; i++)
            {
                if (medicos[i].id == idBusca && medicos[i].ativo == 1)
                {
                    encontrado = 1;

                    printf("\nMedico encontrado:\n");
                    printf("ID: %d\n", medicos[i].id);
                    printf("Nome: Dr(a). %s\n", medicos[i].nome);
                    printf("CRM: CRM%s\n", medicos[i].crm);
                    printf("Especialidade: %s\n", medicos[i].especialidade);
                    printf("Regiao administrativa: ");
                    nomeRegiao(medicos[i].regiaoAdministrativa);
                    printf("\n");

                    printf("\nO que deseja editar?\n");
                    printf("1. Nome\n");
                    printf("2. CRM\n");
                    printf("3. Especialidade\n");
                    printf("4. Regiao administrativa\n");
                    printf("0. Cancelar\n");
                    printf("Escolha uma opcao: ");
                    scanf("%d", &opcaoEditar);

                    switch (opcaoEditar)
                    {
                    case 1:
                        printf("Novo nome do medico, sem Dr/Dr(a): ");
                        scanf(" %[^\n]", medicos[i].nome);
                        break;

                    case 2:
                        printf("Novo numero do CRM: ");
                        scanf(" %[^\n]", medicos[i].crm);
                        break;

                    case 3:
                        printf("Nova especialidade: ");
                        scanf(" %[^\n]", medicos[i].especialidade);
                        break;

                    case 4:
                        printf("Nova regiao administrativa do DF:\n");
                        listarRegioes();
                        printf("Escolha: ");
                        scanf("%d", &medicos[i].regiaoAdministrativa);
                        break;

                    case 0:
                        printf("\nEdicao cancelada.\n");
                        break;

                    default:
                        printf("\nOpcao invalida.\n");
                        break;
                    }

                    printf("\nMedico atualizado com sucesso.\n");
                    break;
                }
            }

            if (encontrado == 0)
            {
                printf("\nMedico nao encontrado ou inativo.\n");
            }

            break;
        }

        case 4:
        {
            int idBusca;

            printf("\nDigite o ID do medico que deseja excluir: ");
            scanf("%d", &idBusca);

            if (excluirMedico(idBusca) == 1)
            {
                printf("\nMedico removido com sucesso.\n");
            }
            else
            {
                printf("\nMedico nao encontrado ou ja inativo.\n");
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

    } while (caso2 != 0);
}
