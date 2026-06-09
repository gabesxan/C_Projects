#include "paciente.h"

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

static const char *nomeRegiao(int regiaoAdministrativa)
{
    switch (regiaoAdministrativa)
    {
    case 1:
        return "Plano Piloto";
    case 2:
        return "Ceilandia";
    case 3:
        return "Taguatinga";
    case 4:
        return "Samambaia";
    case 5:
        return "Gama";
    case 6:
        return "Sobradinho";
    case 7:
        return "Guara";
    case 8:
        return "Aguas Claras";
    default:
        return "Nao informada";
    }
}

static int buscarPaciente(int id)
{
    for (int i = 0; i < totalPacientes; i++)
    {
        if (pacientes[i].id == id && pacientes[i].ativo == 1)
        {
            return i;
        }
    }

    return -1;
}

static void exibirPaciente(const Paciente *paciente)
{
    printf("\nID: %d\n", paciente->id);
    printf("Nome: %s\n", paciente->nome);
    printf("CPF: %s\n", paciente->cpf);
    printf("Idade: %d\n", paciente->idade);
    printf("Telefone: %s\n", paciente->telefone);
    printf("Sexo: %c\n", paciente->sexo);
    printf("Regiao administrativa: %s\n", nomeRegiao(paciente->regiaoAdministrativa));
}

int cadastrarPaciente(const char nome[], const char cpf[], int idade, const char telefone[], char sexo, int regiaoAdministrativa)
{
    if (totalPacientes >= MAX_PACIENTES)
    {
        return 0;
    }

    if (regiaoAdministrativa < 1 || regiaoAdministrativa > 8)
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

        if (lerInteiro(&caso1) == 0)
        {
            printf("\nOpcao invalida. Tente novamente.\n");
            continue;
        }

        switch (caso1)
        {
        case 1:
        {
            char nome[100];
            char cpf[15];
            int idade;
            char telefone[20];
            char sexo;
            int regiaoAdministrativa;

            if (totalPacientes >= MAX_PACIENTES)
            {
                printf("\nLimite de pacientes atingido.\n");
                break;
            }

            printf("\nNome: ");
            scanf(" %[^\n]", nome);

            printf("CPF: ");
            scanf(" %[^\n]", cpf);

            printf("Idade: ");
            scanf("%d", &idade);

            printf("Telefone: ");
            scanf(" %[^\n]", telefone);

            printf("Sexo (M/F/O): ");
            scanf(" %c", &sexo);

            printf("Regiao administrativa do DF:\n");
            listarRegioes();
            printf("Escolha: ");
            scanf("%d", &regiaoAdministrativa);

            if (cadastrarPaciente(nome, cpf, idade, telefone, sexo, regiaoAdministrativa) == 0)
            {
                printf("\nNao foi possivel cadastrar o paciente.\n");
                break;
            }

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
                    exibirPaciente(&pacientes[i]);
                }
            }

            break;
        }

        case 3:
        {
            int idBusca;
            int opcaoEditar;
            int indicePaciente;

            printf("\nDigite o ID do paciente que deseja editar: ");
            scanf("%d", &idBusca);

            indicePaciente = buscarPaciente(idBusca);

            if (indicePaciente == -1)
            {
                printf("\nPaciente nao encontrado ou inativo.\n");
                break;
            }

            printf("\nPaciente encontrado:\n");
            exibirPaciente(&pacientes[indicePaciente]);

            printf("\nO que deseja editar?\n");
            printf("1. Nome\n");
            printf("2. CPF\n");
            printf("3. Idade\n");
            printf("4. Telefone\n");
            printf("5. Sexo\n");
            printf("6. Regiao administrativa\n");
            printf("0. Cancelar\n");
            printf("Escolha uma opcao: ");
            if (lerInteiro(&opcaoEditar) == 0)
            {
                printf("\nOpcao invalida. Tente novamente.\n");
                break;
            }

            switch (opcaoEditar)
            {
            case 1:
                printf("Novo nome: ");
                scanf(" %[^\n]", pacientes[indicePaciente].nome);
                break;

            case 2:
                printf("Novo CPF: ");
                scanf(" %[^\n]", pacientes[indicePaciente].cpf);
                break;

            case 3:
                printf("Nova idade: ");
                scanf("%d", &pacientes[indicePaciente].idade);
                break;

            case 4:
                printf("Novo telefone: ");
                scanf(" %[^\n]", pacientes[indicePaciente].telefone);
                break;

            case 5:
                printf("Novo sexo (M/F/O): ");
                scanf(" %c", &pacientes[indicePaciente].sexo);
                break;

            case 6:
                printf("Nova regiao administrativa do DF:\n");
                listarRegioes();
                printf("Escolha: ");
                scanf("%d", &pacientes[indicePaciente].regiaoAdministrativa);
                break;

            case 0:
                printf("\nEdicao cancelada.\n");
                break;

            default:
                printf("\nOpcao invalida.\n");
                break;
            }

            if (opcaoEditar != 0)
            {
                printf("\nPaciente atualizado com sucesso.\n");
            }

            break;
        }

        case 4:
        {
            int idBusca;

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
