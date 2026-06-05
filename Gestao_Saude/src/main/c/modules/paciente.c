#include "paciente.h"

int cadastrarPaciente(const char nome[], const char cpf[], int idade, const char telefone[], char sexo, int regiaoAdministrativa)
{
    if (totalPacientes >= MAX_PACIENTES)
    {
        return 0;
    }

    pacientes[totalPacientes].id = totalPacientes + 1;
    strcpy(pacientes[totalPacientes].nome, nome);
    strcpy(pacientes[totalPacientes].cpf, cpf);
    pacientes[totalPacientes].idade = idade;
    strcpy(pacientes[totalPacientes].telefone, telefone);
    pacientes[totalPacientes].sexo = sexo;
    pacientes[totalPacientes].regiaoAdministrativa = regiaoAdministrativa;
    pacientes[totalPacientes].ativo = 1;

    totalPacientes++;

    return 1;
}

int excluirPaciente(int id)
{
    for (int i = 0; i < totalPacientes; i++)
    {
        if (pacientes[i].id == id && pacientes[i].ativo == 1)
        {
            pacientes[i].ativo = 0;
            return 1;
        }
    }

    return 0;
}
void menuPacientes(void)
{
    int caso1;

    do
    {
        printf("\n=============================================\n");
        printf("MENU GERENCIAMENTO DE PACIENTES\n");
        printf("=============================================\n");
        printf("1. Cadastrar Paciente\n");
        printf("2. Listar Pacientes\n");
        printf("3. Editar Paciente\n");
        printf("4. Excluir Paciente\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("---------------------------------------------\n");
        printf("Escolha uma opcao: ");

        scanf("%d", &caso1);

        switch (caso1)
        {
        case 1:
        {
            if (totalPacientes >= MAX_PACIENTES)
            {
                printf("\nLimite de pacientes atingido.\n");
                break;
            }

            pacientes[totalPacientes].id = totalPacientes + 1;

            printf("\nNome: ");
            scanf(" %[^\n]", pacientes[totalPacientes].nome);

            printf("CPF: ");
            scanf(" %[^\n]", pacientes[totalPacientes].cpf);

            printf("Idade: ");
            scanf("%d", &pacientes[totalPacientes].idade);

            printf("Telefone: ");
            scanf(" %[^\n]", pacientes[totalPacientes].telefone);

            printf("Sexo (M/F/O): ");
            scanf(" %c", &pacientes[totalPacientes].sexo);

            printf("Regiao administrativa do DF:\n");
            printf("1. Plano Piloto\n");
            printf("2. Ceilandia\n");
            printf("3. Taguatinga\n");
            printf("4. Samambaia\n");
            printf("5. Gama\n");
            printf("6. Sobradinho\n");
            printf("7. Guara\n");
            printf("8. Aguas Claras\n");
            printf("Escolha: ");
            scanf("%d", &pacientes[totalPacientes].regiaoAdministrativa);

            pacientes[totalPacientes].ativo = 1;
            totalPacientes++;

            printf("\nPaciente cadastrado com sucesso! ID: %d\n", pacientes[totalPacientes - 1].id);
            break;
        }

        case 2:
        {
            if (totalPacientes == 0)
            {
                printf("\nNenhum paciente cadastrado.\n");
                break;
            }

            printf("\nLista de Pacientes:\n");

            for (int i = 0; i < totalPacientes; i++)
            {
                if (pacientes[i].ativo == 1)
                {
                    printf("\nID: %d\n", pacientes[i].id);
                    printf("Nome: %s\n", pacientes[i].nome);
                    printf("CPF: %s\n", pacientes[i].cpf);
                    printf("Idade: %d\n", pacientes[i].idade);
                    printf("Telefone: %s\n", pacientes[i].telefone);
                    printf("Sexo: %c\n", pacientes[i].sexo);
                    printf("Regiao administrativa: ");

                    switch (pacientes[i].regiaoAdministrativa)
                    {
                    case 1:
                        printf("Plano Piloto\n");
                        break;
                    case 2:
                        printf("Ceilandia\n");
                        break;
                    case 3:
                        printf("Taguatinga\n");
                        break;
                    case 4:
                        printf("Samambaia\n");
                        break;
                    case 5:
                        printf("Gama\n");
                        break;
                    case 6:
                        printf("Sobradinho\n");
                        break;
                    case 7:
                        printf("Guara\n");
                        break;
                    case 8:
                        printf("Aguas Claras\n");
                        break;
                    default:
                        printf("Nao informada\n");
                        break;
                    }
                }
            }

            break;
        }

        case 3:
        {
            int idBusca;
            int encontrado = 0;
            int opcaoEditar;

            printf("\nDigite o ID do paciente que deseja editar: ");
            scanf("%d", &idBusca);

            for (int i = 0; i < totalPacientes; i++)
            {
                if (pacientes[i].id == idBusca && pacientes[i].ativo == 1)
                {
                    encontrado = 1;

                    printf("\nPaciente encontrado:\n");
                    printf("ID: %d\n", pacientes[i].id);
                    printf("Nome: %s\n", pacientes[i].nome);
                    printf("CPF: %s\n", pacientes[i].cpf);
                    printf("Idade: %d\n", pacientes[i].idade);
                    printf("Telefone: %s\n", pacientes[i].telefone);
                    printf("Sexo: %c\n", pacientes[i].sexo);
                    printf("Regiao administrativa: ");

                    switch (pacientes[i].regiaoAdministrativa)
                    {
                    case 1:
                        printf("Plano Piloto\n");
                        break;
                    case 2:
                        printf("Ceilandia\n");
                        break;
                    case 3:
                        printf("Taguatinga\n");
                        break;
                    case 4:
                        printf("Samambaia\n");
                        break;
                    case 5:
                        printf("Gama\n");
                        break;
                    case 6:
                        printf("Sobradinho\n");
                        break;
                    case 7:
                        printf("Guara\n");
                        break;
                    case 8:
                        printf("Aguas Claras\n");
                        break;
                    default:
                        printf("Nao informada\n");
                        break;
                    }

                    printf("\nO que deseja editar?\n");
                    printf("1. Nome\n");
                    printf("2. CPF\n");
                    printf("3. Idade\n");
                    printf("4. Telefone\n");
                    printf("5. Sexo\n");
                    printf("6. Regiao administrativa\n");
                    printf("0. Cancelar\n");
                    printf("Escolha uma opcao: ");
                    scanf("%d", &opcaoEditar);

                    switch (opcaoEditar)
                    {
                    case 1:
                        printf("Novo nome: ");
                        scanf(" %[^\n]", pacientes[i].nome);
                        break;

                    case 2:
                        printf("Novo CPF: ");
                        scanf(" %[^\n]", pacientes[i].cpf);
                        break;

                    case 3:
                        printf("Nova idade: ");
                        scanf("%d", &pacientes[i].idade);
                        break;

                    case 4:
                        printf("Novo telefone: ");
                        scanf(" %[^\n]", pacientes[i].telefone);
                        break;

                    case 5:
                        printf("Novo sexo (M/F/O): ");
                        scanf(" %c", &pacientes[i].sexo);
                        break;

                    case 6:
                        printf("Nova regiao administrativa do DF:\n");
                        printf("1. Plano Piloto\n");
                        printf("2. Ceilandia\n");
                        printf("3. Taguatinga\n");
                        printf("4. Samambaia\n");
                        printf("5. Gama\n");
                        printf("6. Sobradinho\n");
                        printf("7. Guara\n");
                        printf("8. Aguas Claras\n");
                        printf("Escolha: ");
                        scanf("%d", &pacientes[i].regiaoAdministrativa);
                        break;

                    case 0:
                        printf("\nEdicao cancelada.\n");
                        break;

                    default:
                        printf("\nOpcao invalida.\n");
                        break;
                    }

                    printf("\nPaciente atualizado com sucesso.\n");
                    break;
                }
            }

            if (encontrado == 0)
            {
                printf("\nPaciente nao encontrado ou inativo.\n");
            }

            break;
        }

        case 4:
        {
            int idBusca;
            int encontrado = 0;

            printf("\nDigite o ID do paciente que deseja excluir: ");
            scanf("%d", &idBusca);

            if (excluirPaciente(idBusca) == 1)
            {
                printf("\nPaciente removido com sucesso.\n");
            }
            else
            {
                printf("\nPaciente nao encontrado ou ja inativo.\n");
            }

            if (encontrado == 0)
            {
                printf("\nPaciente nao encontrado ou ja inativo.\n");
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

    } while (caso1 != 0);
}
