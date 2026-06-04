#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PACIENTES 100
#define MAX_MEDICOS 50
#define MAX_AGENDAMENTOS 200
#define MAX_ALAS 20
#define MAX_LEITOS 300
#define MAX_INTERNACOES 200
#define MAX_TRIAGENS 300

typedef struct
{
    int id;
    char nome[100];
    char cpf[15];
    int idade;
    char telefone[20];
    char sexo;
    int ativo;
} Paciente;

typedef struct
{
    int id;
    char nome[100];
    char crm[20];
    char especialidade[50];
    int ativo;
} Medico;

typedef struct
{
    int id;
    int pacienteId;
    int medicoId;
    char data[11];
    char horario[6];
    char status[20];
} Agendamento;

typedef struct
{
    int id;
    char nome[100];
    char tipo[50];
    int totalLeitos;
    int leitosOcupados;
    int ativo;
} Ala;

typedef struct
{
    int id;
    int alaId;
    int numero;
    int ocupado;
    int pacienteId;
} Leito;

typedef struct
{
    int id;
    int pacienteId;
    int alaId;
    int leitoId;
    char dataEntrada[11];
    char dataAlta[11];
    char status[20];
} Internacao;

typedef struct
{
    int id;
    int pacienteId;
    int febre;
    int faltaAr;
    int dorIntensa;
    int pressaoAlta;
    int pontuacao;
    char classificacao[30];
} Triagem;

Paciente pacientes[MAX_PACIENTES];
Medico medicos[MAX_MEDICOS];
Agendamento agendamentos[MAX_AGENDAMENTOS];
Ala alas[MAX_ALAS];
Leito leitos[MAX_LEITOS];
Internacao internacoes[MAX_INTERNACOES];
Triagem triagens[MAX_TRIAGENS];

int totalPacientes = 0;
int totalMedicos = 0;
int totalAgendamentos = 0;
int totalAlas = 0;
int totalLeitos = 0;
int totalInternacoes = 0;
int totalTriagens = 0;

int main(void)
{
    int opcao;

    do
    {
        printf("\n=============================================\n");
        printf("SIGEH-DF - Sistema de Gestao Hospitalar do DF\n");
        printf("=============================================\n");
        printf("1. Gerenciar Pacientes\n");
        printf("2. Gerenciar Medicos\n");
        printf("3. Gerenciar Agendamentos\n");
        printf("4. Gerenciar Alas\n");
        printf("5. Gerenciar Leitos\n");
        printf("6. Gerenciar Internacoes\n");
        printf("7. Gerenciar Triagem\n");
        printf("8. Gerar Relatorios\n");
        printf("0. Sair\n");
        printf("---------------------------------------------\n");
        printf("Escolha uma opcao: ");

        scanf("%d", &opcao);

        switch (opcao)
        {
        case 1:
        {
            int caso1;

            printf("\nVoce escolheu: Gerenciar Pacientes\n");

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
                        printf("\nLimite de pacientes atingido. Nao e possivel cadastrar mais.\n");
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

                            printf("\nO que deseja editar?\n");
                            printf("1. Nome\n");
                            printf("2. CPF\n");
                            printf("3. Idade\n");
                            printf("4. Telefone\n");
                            printf("5. Sexo\n");
                            printf("0. Cancelar\n");
                            printf("Escolha uma opcao: ");
                            scanf("%d", &opcaoEditar);

                            switch (opcaoEditar)
                            {
                            case 1:
                                printf("Novo nome: ");
                                scanf(" %[^\n]", pacientes[i].nome);
                                printf("\nNome atualizado com sucesso.\n");
                                break;

                            case 2:
                                printf("Novo CPF: ");
                                scanf(" %[^\n]", pacientes[i].cpf);
                                printf("\nCPF atualizado com sucesso.\n");
                                break;

                            case 3:
                                printf("Nova idade: ");
                                scanf("%d", &pacientes[i].idade);
                                printf("\nIdade atualizada com sucesso.\n");
                                break;

                            case 4:
                                printf("Novo telefone: ");
                                scanf(" %[^\n]", pacientes[i].telefone);
                                printf("\nTelefone atualizado com sucesso.\n");
                                break;

                            case 5:
                                printf("Novo sexo (M/F/O): ");
                                scanf(" %c", &pacientes[i].sexo);
                                printf("\nSexo atualizado com sucesso.\n");
                                break;

                            case 0:
                                printf("\nEdicao cancelada.\n");
                                break;

                            default:
                                printf("\nOpcao invalida.\n");
                                break;
                            }

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

                    for (int i = 0; i < totalPacientes; i++)
                    {
                        if (pacientes[i].id == idBusca && pacientes[i].ativo == 1)
                        {
                            pacientes[i].ativo = 0;
                            encontrado = 1;

                            printf("\nPaciente removido com sucesso.\n");
                            break;
                        }
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

            break;
        }

        case 2:
            printf("\nVoce escolheu: Gerenciar Medicos\n");
            printf("Modulo de medicos sera implementado depois.\n");
            break;

        case 3:
            printf("\nVoce escolheu: Gerenciar Agendamentos\n");
            printf("Modulo de agendamentos sera implementado depois.\n");
            break;

        case 4:
            printf("\nVoce escolheu: Gerenciar Alas\n");
            printf("Modulo de alas sera implementado depois.\n");
            break;

        case 5:
            printf("\nVoce escolheu: Gerenciar Leitos\n");
            printf("Modulo de leitos sera implementado depois.\n");
            break;

        case 6:
            printf("\nVoce escolheu: Gerenciar Internacoes\n");
            printf("Modulo de internacoes sera implementado depois.\n");
            break;

        case 7:
            printf("\nVoce escolheu: Gerenciar Triagem\n");
            printf("Modulo de triagem sera implementado depois.\n");
            break;

        case 8:
            printf("\nVoce escolheu: Gerar Relatorios\n");
            printf("Modulo de relatorios sera implementado depois.\n");
            break;

        case 0:
            printf("\nEncerrando o SIGEH-DF...\n");
            break;

        default:
            printf("\nOpcao invalida. Tente novamente.\n");
            break;
        }

    } while (opcao != 0);

    return 0;
}
