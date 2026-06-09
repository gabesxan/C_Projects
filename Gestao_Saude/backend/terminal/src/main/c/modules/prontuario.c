#include "prontuario.h"
#include "agendamento.h"

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

static int textoVazio(const char texto[])
{
    return texto[0] == '\0';
}

static void exibirProntuario(const Prontuario *prontuario)
{
    printf("\nID: %d\n", prontuario->id);
    printf("Paciente: %s\n", nomePaciente(prontuario->pacienteId));
    printf("Medico: %s\n", nomeMedico(prontuario->medicoId));
    printf("Especialidade: %s\n", especialidadeMedico(prontuario->medicoId));
    printf("Data: %s\n", prontuario->data);
    printf("Observacoes: %s\n", prontuario->observacoes);
    printf("Diagnostico: %s\n", prontuario->diagnostico);
    printf("Conduta: %s\n", prontuario->conduta);
    printf("Alerta importante: %s\n", prontuario->alertaImportante == 1 ? "Sim" : "Nao");
}

static int buscarProntuarioAtivo(int prontuarioId)
{
    for (int i = 0; i < totalProntuarios; i++)
    {
        if (prontuarios[i].id == prontuarioId && prontuarios[i].ativo == 1)
        {
            return i;
        }
    }

    return -1;
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
    const char *especialidadeSistema = obterEspecialidade(opcao);

    if (strlen(especialidadeSistema) == 0)
    {
        return 0;
    }

    strcpy(especialidade, especialidadeSistema);
    return 1;
}

static void listarEspecialidades(void)
{
    printf("\nEspecialidades disponiveis:\n");

    for (int i = TRIAGEM_GERAL; i <= TRIAGEM_PEDIATRIA; i++)
    {
        printf("%d. %s\n", i, obterEspecialidade(i));
    }
}

int copiarProntuarios(Prontuario destino[], int maximo)
{
    int totalCopiados = 0;

    if (destino == NULL || maximo <= 0)
    {
        return 0;
    }

    for (int i = 0; i < totalProntuarios && totalCopiados < maximo; i++)
    {
        if (prontuarios[i].ativo == 1)
        {
            destino[totalCopiados] = prontuarios[i];
            totalCopiados++;
        }
    }

    return totalCopiados;
}

int copiarProntuariosPorPaciente(int pacienteId, Prontuario destino[], int maximo)
{
    int totalCopiados = 0;

    if (destino == NULL || maximo <= 0)
    {
        return 0;
    }

    for (int i = 0; i < totalProntuarios && totalCopiados < maximo; i++)
    {
        if (prontuarios[i].ativo == 1 && prontuarios[i].pacienteId == pacienteId)
        {
            destino[totalCopiados] = prontuarios[i];
            totalCopiados++;
        }
    }

    return totalCopiados;
}

int copiarProntuariosPorMedico(int medicoId, Prontuario destino[], int maximo)
{
    int totalCopiados = 0;

    if (destino == NULL || maximo <= 0)
    {
        return 0;
    }

    for (int i = 0; i < totalProntuarios && totalCopiados < maximo; i++)
    {
        if (prontuarios[i].ativo == 1 && prontuarios[i].medicoId == medicoId)
        {
            destino[totalCopiados] = prontuarios[i];
            totalCopiados++;
        }
    }

    return totalCopiados;
}

int copiarProntuariosPorEspecialidade(const char especialidade[], Prontuario destino[], int maximo)
{
    int totalCopiados = 0;

    if (especialidade == NULL || destino == NULL || maximo <= 0)
    {
        return 0;
    }

    for (int i = 0; i < totalProntuarios && totalCopiados < maximo; i++)
    {
        if (prontuarios[i].ativo == 1 &&
            strcmp(especialidadeMedico(prontuarios[i].medicoId), especialidade) == 0)
        {
            destino[totalCopiados] = prontuarios[i];
            totalCopiados++;
        }
    }

    return totalCopiados;
}

int criarProntuarioAutomatico(int pacienteId, int medicoId, const char data[])
{
    return registrarProntuario(pacienteId, medicoId, data,
                               "Prontuario base criado automaticamente pelo sistema.",
                               "Aguardando avaliacao medica.",
                               "Registrar evolucao do atendimento.",
                               0);
}

int complementarProntuario(int prontuarioId, const char observacoes[],
                           const char diagnostico[], const char conduta[],
                           int alertaImportante)
{
    int indiceProntuario = buscarProntuarioAtivo(prontuarioId);

    if (indiceProntuario == -1)
    {
        return 0;
    }

    if (textoVazio(observacoes) || textoVazio(diagnostico) || textoVazio(conduta))
    {
        return 0;
    }

    strcpy(prontuarios[indiceProntuario].observacoes, observacoes);
    strcpy(prontuarios[indiceProntuario].diagnostico, diagnostico);
    strcpy(prontuarios[indiceProntuario].conduta, conduta);
    prontuarios[indiceProntuario].alertaImportante = alertaImportante;

    return 1;
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

    if (textoVazio(data) || textoVazio(observacoes) ||
        textoVazio(diagnostico) || textoVazio(conduta))
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

int contarProntuariosPorPaciente(int pacienteId)
{
    int total = 0;

    for (int i = 0; i < totalProntuarios; i++)
    {
        if (prontuarios[i].pacienteId == pacienteId && prontuarios[i].ativo == 1)
        {
            total++;
        }
    }
    return total;
}

int contarProntuariosPorMedico(int medicoId)
{
    int total = 0;

    for (int i = 0; i < totalProntuarios; i++)
    {
        if (prontuarios[i].medicoId == medicoId && prontuarios[i].ativo == 1)
        {
            total++;
        }
    }

    return total;
}

int contarProntuariosPorEspecialidade(const char especialidade[])
{
    int total = 0;

    if (especialidade == NULL)
    {
        return 0;
    }

    for (int i = 0; i < totalProntuarios; i++)
    {
        if (prontuarios[i].ativo == 1 &&
            strcmp(especialidadeMedico(prontuarios[i].medicoId), especialidade) == 0)
        {
            total++;
        }
    }

    return total;
}

void listarProntuarioPorPaciente(int pacienteId)
{
    Prontuario lista[MAX_PRONTUARIOS];
    int totalCopiados = copiarProntuariosPorPaciente(pacienteId, lista, MAX_PRONTUARIOS);

    if (totalCopiados == 0)
    {
        printf("\nNenhum prontuario encontrado para esse paciente.\n");
        return;
    }

    for (int i = 0; i < totalCopiados; i++)
    {
        exibirProntuario(&lista[i]);
    }
}

void listarProntuarioPorMedico(int medicoId)
{
    Prontuario lista[MAX_PRONTUARIOS];
    int totalCopiados = copiarProntuariosPorMedico(medicoId, lista, MAX_PRONTUARIOS);

    if (totalCopiados == 0)
    {
        printf("\nNenhum prontuario encontrado para esse medico.\n");
        return;
    }

    for (int i = 0; i < totalCopiados; i++)
    {
        exibirProntuario(&lista[i]);
    }
}

void listarProntuarioPorEspecialidade(const char especialidade[])
{
    Prontuario lista[MAX_PRONTUARIOS];
    int totalCopiados = copiarProntuariosPorEspecialidade(especialidade, lista, MAX_PRONTUARIOS);

    if (totalCopiados == 0)
    {
        printf("\nNenhum prontuario encontrado para essa especialidade.\n");
        return;
    }

    for (int i = 0; i < totalCopiados; i++)
    {
        exibirProntuario(&lista[i]);
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
        printf("1. Criar prontuario automatico\n");
        printf("2. Registrar prontuario de urgencia\n");
        printf("3. Complementar prontuario existente\n");
        printf("4. Listar prontuarios por paciente\n");
        printf("5. Listar prontuarios por medico\n");
        printf("6. Listar prontuarios por especialidade\n");
        printf("0. Voltar ao menu principal\n");
        printf("---------------------------------------------\n");
        printf("Escolha uma opcao: ");
        if (lerInteiro(&opcao) == 0)
        {
            printf("\nOpcao invalida. Tente novamente.\n");
            continue;
        }

        switch (opcao)
        {
        case 1:
        {
            int pacienteId;
            int medicoId;
            char data[11];

            listarPacientesAtivos();
            printf("\nID do paciente: ");
            if (lerInteiro(&pacienteId) == 0)
            {
                printf("\nPaciente invalido.\n");
                break;
            }

            if (buscarPacienteAtivo(pacienteId) == -1)
            {
                printf("\nPaciente nao encontrado ou inativo.\n");
                break;
            }

            listarMedicosAtivos();
            printf("\nID do medico: ");
            if (lerInteiro(&medicoId) == 0)
            {
                printf("\nMedico invalido.\n");
                break;
            }

            if (buscarMedicoAtivo(medicoId) == -1)
            {
                printf("\nMedico nao encontrado ou inativo.\n");
                break;
            }

            printf("Data (DD/MM/AAAA): ");
            scanf(" %[^\n]", data);

            if (criarProntuarioAutomatico(pacienteId, medicoId, data) == 1)
            {
                printf("\nProntuario automatico criado com sucesso.\n");
            }
            else
            {
                printf("\nNao foi possivel criar o prontuario automatico.\n");
            }

            break;
        }

        case 2:
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
            if (lerInteiro(&pacienteId) == 0)
            {
                printf("\nPaciente invalido.\n");
                break;
            }

            if (buscarPacienteAtivo(pacienteId) == -1)
            {
                printf("\nPaciente nao encontrado ou inativo.\n");
                break;
            }

            listarMedicosAtivos();
            printf("\nID do medico: ");
            if (lerInteiro(&medicoId) == 0)
            {
                printf("\nMedico invalido.\n");
                break;
            }

            if (buscarMedicoAtivo(medicoId) == -1)
            {
                printf("\nMedico nao encontrado ou inativo.\n");
                break;
            }

            printf("Data (DD/MM/AAAA): ");
            scanf(" %[^\n]", data);

            printf("Observacoes: ");
            scanf(" %[^\n]", observacoes);

            printf("Diagnostico: ");
            scanf(" %[^\n]", diagnostico);

            printf("Conduta: ");
            scanf(" %[^\n]", conduta);

            printf("Alerta importante? (1-Sim / 0-Nao): ");
            if (lerInteiro(&alertaImportante) == 0)
            {
                printf("\nAlerta invalido.\n");
                break;
            }

            if (registrarProntuario(pacienteId, medicoId, data, observacoes,
                                    diagnostico, conduta, alertaImportante) == 1)
            {
                printf("\nProntuario de urgencia registrado com sucesso.\n");
            }
            else
            {
                printf("\nNao foi possivel registrar o prontuario.\n");
            }

            break;
        }

        case 3:
        {
            int prontuarioId;
            int alertaImportante;
            char observacoes[300];
            char diagnostico[200];
            char conduta[200];

            if (totalProntuarios == 0)
            {
                printf("\nNenhum prontuario cadastrado.\n");
                break;
            }

            printf("\nProntuarios ativos:\n");
            for (int i = 0; i < totalProntuarios; i++)
            {
                if (prontuarios[i].ativo == 1)
                {
                    printf("ID: %d | Paciente: %s | Medico: %s | Data: %s\n",
                           prontuarios[i].id,
                           nomePaciente(prontuarios[i].pacienteId),
                           nomeMedico(prontuarios[i].medicoId),
                           prontuarios[i].data);
                }
            }

            printf("\nDigite o ID do prontuario: ");
            if (lerInteiro(&prontuarioId) == 0)
            {
                printf("\nProntuario invalido.\n");
                break;
            }

            if (buscarProntuarioAtivo(prontuarioId) == -1)
            {
                printf("\nProntuario nao encontrado ou inativo.\n");
                break;
            }

            printf("Novas observacoes: ");
            scanf(" %[^\n]", observacoes);

            printf("Novo diagnostico: ");
            scanf(" %[^\n]", diagnostico);

            printf("Nova conduta: ");
            scanf(" %[^\n]", conduta);

            printf("Alerta importante? (1-Sim / 0-Nao): ");
            if (lerInteiro(&alertaImportante) == 0)
            {
                printf("\nAlerta invalido.\n");
                break;
            }

            if (complementarProntuario(prontuarioId, observacoes, diagnostico,
                                       conduta, alertaImportante) == 1)
            {
                printf("\nProntuario complementado com sucesso.\n");
            }
            else
            {
                printf("\nNao foi possivel complementar o prontuario.\n");
            }

            break;
        }

        case 4:
        {
            int pacienteId;

            listarPacientesAtivos();
            printf("\nDigite o ID do paciente: ");
            if (lerInteiro(&pacienteId) == 0)
            {
                printf("\nPaciente invalido.\n");
                break;
            }

            listarProntuarioPorPaciente(pacienteId);
            break;
        }

        case 5:
        {
            int medicoId;

            listarMedicosAtivos();
            printf("\nDigite o ID do medico: ");
            if (lerInteiro(&medicoId) == 0)
            {
                printf("\nMedico invalido.\n");
                break;
            }

            listarProntuarioPorMedico(medicoId);
            break;
        }

        case 6:
        {
            int opcaoEspecialidade;
            char especialidade[50];

            listarEspecialidades();
            printf("\nEscolha a especialidade: ");
            if (lerInteiro(&opcaoEspecialidade) == 0)
            {
                printf("\nEspecialidade invalida.\n");
                break;
            }

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
