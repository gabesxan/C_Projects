#include "prontuario.h"

static int buscarPacienteAtivo(int pacienteId)
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

static int buscarMedicoAtivo(int medicoId)
{
    for (int i = 0; i < totalMedicos; i++)
    {
        if (medicos[i].id == medicoId && medicos[i].ativo == 1)
        {
            return i;
        }
    }

    return -1;
}

static const char *nomePaciente(int pacienteId)
{
    for (int i = 0; i < totalPacientes; i++)
    {
        if (pacientes[i].id == pacienteId && pacientes[i].ativo == 1)
        {
            return pacientes[i].nome;
        }
    }

    return "Paciente nao encontrado";
}

static const char *nomeMedico(int medicoId)
{
    for (int i = 0; i < totalMedicos; i++)
    {
        if (medicos[i].id == medicoId && medicos[i].ativo == 1)
        {
            return medicos[i].nome;
        }
    }

    return "Medico nao encontrado";
}

static const char *especialidadeMedico(int medicoId)
{
    for (int i = 0; i < totalMedicos; i++)
    {
        if (medicos[i].id == medicoId && medicos[i].ativo == 1)
        {
            return medicos[i].especialidade;
        }
    }

    return "Nao informada";
}

static void listarPacientesAtivos(void)
{
    printf("\nPacientes ativos:\n");

    for (int i = 0; i < totalPacientes; i++)
    {
        if (pacientes[i].ativo == 1)
        {
            printf("ID: %d | Nome: %s\n", pacientes[i].id, pacientes[i].nome);
        }
    }
}

static void listarMedicosAtivos(void)
{
    printf("\nMedicos ativos:\n");

    for (int i = 0; i < totalMedicos; i++)
    {
        if (medicos[i].ativo == 1)
        {
            printf("ID: %d | Nome: %s | Especialidade: %s\n",
                   medicos[i].id,
                   medicos[i].nome,
                   medicos[i].especialidade);
        }
    }
}

static int preencherEspecialidade(char especialidade[], int opcao)
{
    switch (opcao)
    {
    case 1:
        strcpy(especialidade, "Clinico Geral");
        return 1;
    case 2:
        strcpy(especialidade, "Ortopedia");
        return 1;
    case 3:
        strcpy(especialidade, "Cardiologia");
        return 1;
    case 4:
        strcpy(especialidade, "Pneumologia");
        return 1;
    case 5:
        strcpy(especialidade, "Pediatria");
        return 1;
    default:
        return 0;
    }
}

static void listarEspecialidades(void)
{
    printf("\nEspecialidades disponiveis:\n");
    printf("1. Clinico Geral\n");
    printf("2. Ortopedia\n");
    printf("3. Cardiologia\n");
    printf("4. Pneumologia\n");
    printf("5. Pediatria\n");
}

int registrarProntuario(int pacienteId, int medicoId, const char data[],
                        const char observacoes[], const char diagnostico[],
                        const char conduta[], int alertaImportante)
{
    if (totalProntuarios >= MAX_PRONTUARIOS)
    {
        return 0;
    }

    if (buscarPacienteAtivo(pacienteId) == -1)
    {
        return 0;
    }

    if (buscarMedicoAtivo(medicoId) == -1)
    {
        return 0;
    }

    prontuarios[totalProntuarios].id = totalProntuarios + 1;
    prontuarios[totalProntuarios].pacienteId = pacienteId;
    prontuarios[totalProntuarios].medicoId = medicoId;
    strcpy(prontuarios[totalProntuarios].data, data);
    strcpy(prontuarios[totalProntuarios].observacoes, observacoes);
    strcpy(prontuarios[totalProntuarios].diagnostico, diagnostico);
    strcpy(prontuarios[totalProntuarios].conduta, conduta);
    prontuarios[totalProntuarios].alertaImportante = alertaImportante;
    prontuarios[totalProntuarios].ativo = 1;

    totalProntuarios++;

    return 1;
}

void listarProntuarioPorPaciente(int pacienteId)
{
    int encontrou = 0;

    for (int i = 0; i < totalProntuarios; i++)
    {
        if (prontuarios[i].pacienteId == pacienteId && prontuarios[i].ativo == 1)
        {
            printf("\nID: %d\n", prontuarios[i].id);
            printf("Paciente: %s\n", nomePaciente(prontuarios[i].pacienteId));
            printf("Medico: %s\n", nomeMedico(prontuarios[i].medicoId));
            printf("Especialidade: %s\n", especialidadeMedico(prontuarios[i].medicoId));
            printf("Data: %s\n", prontuarios[i].data);
            printf("Observacoes: %s\n", prontuarios[i].observacoes);
            printf("Diagnostico: %s\n", prontuarios[i].diagnostico);
            printf("Conduta: %s\n", prontuarios[i].conduta);
            printf("Alerta importante: %d\n", prontuarios[i].alertaImportante);
            encontrou = 1;
        }
    }

    if (encontrou == 0)
    {
        printf("\nNenhum prontuario encontrado para esse paciente.\n");
    }
}

void listarProntuarioPorMedico(int medicoId)
{
    int encontrou = 0;

    for (int i = 0; i < totalProntuarios; i++)
    {
        if (prontuarios[i].medicoId == medicoId && prontuarios[i].ativo == 1)
        {
            printf("\nID: %d\n", prontuarios[i].id);
            printf("Paciente: %s\n", nomePaciente(prontuarios[i].pacienteId));
            printf("Medico: %s\n", nomeMedico(prontuarios[i].medicoId));
            printf("Especialidade: %s\n", especialidadeMedico(prontuarios[i].medicoId));
            printf("Data: %s\n", prontuarios[i].data);
            printf("Observacoes: %s\n", prontuarios[i].observacoes);
            printf("Diagnostico: %s\n", prontuarios[i].diagnostico);
            printf("Conduta: %s\n", prontuarios[i].conduta);
            printf("Alerta importante: %d\n", prontuarios[i].alertaImportante);
            encontrou = 1;
        }
    }

    if (encontrou == 0)
    {
        printf("\nNenhum prontuario encontrado para esse medico.\n");
    }
}

void listarProntuarioPorEspecialidade(const char especialidade[])
{
    int encontrou = 0;

    for (int i = 0; i < totalProntuarios; i++)
    {
        if (prontuarios[i].ativo == 1 &&
            strcmp(especialidadeMedico(prontuarios[i].medicoId), especialidade) == 0)
        {
            printf("\nID: %d\n", prontuarios[i].id);
            printf("Paciente: %s\n", nomePaciente(prontuarios[i].pacienteId));
            printf("Medico: %s\n", nomeMedico(prontuarios[i].medicoId));
            printf("Especialidade: %s\n", especialidadeMedico(prontuarios[i].medicoId));
            printf("Data: %s\n", prontuarios[i].data);
            printf("Observacoes: %s\n", prontuarios[i].observacoes);
            printf("Diagnostico: %s\n", prontuarios[i].diagnostico);
            printf("Conduta: %s\n", prontuarios[i].conduta);
            printf("Alerta importante: %d\n", prontuarios[i].alertaImportante);
            encontrou = 1;
        }
    }

    if (encontrou == 0)
    {
        printf("\nNenhum prontuario encontrado para essa especialidade.\n");
    }
}

void menuProntuarios(void)
{
    int opcao;

    do
    {
        printf("\n=============================================\n");
        printf("MENU PRONTUARIOS\n");
        printf("=============================================\n");
        printf("1. Registrar prontuario\n");
        printf("2. Listar prontuarios por paciente\n");
        printf("3. Listar prontuarios por medico\n");
        printf("4. Listar prontuarios por especialidade\n");
        printf("0. Voltar ao menu principal\n");
        printf("---------------------------------------------\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &opcao);

        switch (opcao)
        {
        case 1:
        {
            int pacienteId;
            int medicoId;
            int alertaImportante;
            char data[11];
            char observacoes[300];
            char diagnostico[200];
            char conduta[200];

            listarPacientesAtivos();
            printf("\nID do paciente: ");
            scanf("%d", &pacienteId);

            listarMedicosAtivos();
            printf("\nID do medico: ");
            scanf("%d", &medicoId);

            printf("Data (DD/MM/AAAA): ");
            scanf(" %[^\n]", data);

            printf("Observacoes: ");
            scanf(" %[^\n]", observacoes);

            printf("Diagnostico: ");
            scanf(" %[^\n]", diagnostico);

            printf("Conduta: ");
            scanf(" %[^\n]", conduta);

            printf("Alerta importante? (1-Sim / 0-Nao): ");
            scanf("%d", &alertaImportante);

            if (registrarProntuario(pacienteId, medicoId, data, observacoes,
                                    diagnostico, conduta, alertaImportante) == 1)
            {
                printf("\nProntuario registrado com sucesso.\n");
            }
            else
            {
                printf("\nNao foi possivel registrar o prontuario.\n");
            }

            break;
        }

        case 2:
        {
            int pacienteId;

            listarPacientesAtivos();
            printf("\nDigite o ID do paciente: ");
            scanf("%d", &pacienteId);

            listarProntuarioPorPaciente(pacienteId);
            break;
        }

        case 3:
        {
            int medicoId;

            listarMedicosAtivos();
            printf("\nDigite o ID do medico: ");
            scanf("%d", &medicoId);

            listarProntuarioPorMedico(medicoId);
            break;
        }

        case 4:
        {
            int opcaoEspecialidade;
            char especialidade[50];

            listarEspecialidades();
            printf("\nEscolha a especialidade: ");
            scanf("%d", &opcaoEspecialidade);

            if (preencherEspecialidade(especialidade, opcaoEspecialidade) == 0)
            {
                printf("\nEspecialidade invalida.\n");
                break;
            }

            listarProntuarioPorEspecialidade(especialidade);
            break;
        }

        case 0:
            printf("\nVoltando ao menu principal...\n");
            break;

        default:
            printf("\nOpcao invalida. Tente novamente.\n");
            break;
        }

    } while (opcao != 0);
}
