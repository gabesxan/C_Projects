#include "exame.h"
#include "triagem.h"

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

static int buscarExameAtivo(int exameId)
{
    for (int i = 0; i < totalExames; i++)
    {
        if (exames[i].id == exameId && exames[i].ativo == 1)
        {
            return i;
        }
    }

    return -1;
}

static int textoVazio(const char texto[])
{
    return texto[0] == '\0';
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

static int prontuarioCompativel(int prontuarioId, int pacienteId, int medicoId)
{
    int indiceProntuario = buscarProntuarioAtivo(prontuarioId);

    if (indiceProntuario == -1)
    {
        return 0;
    }

    return prontuarios[indiceProntuario].pacienteId == pacienteId &&
           prontuarios[indiceProntuario].medicoId == medicoId;
}

static int exameUrgentePorTriagem(int pacienteId)
{
    int indiceTriagem = triagemAtual(pacienteId);

    if (indiceTriagem == -1)
    {
        return 0;
    }

    return ehUrgente(triagens[indiceTriagem].classificacao);
}

void exibirNomeTipoExame(int tipoExame)
{
    switch (tipoExame)
    {
    case EXAME_HEMOGRAMA:
        printf("Hemograma");
        break;
    case EXAME_RAIO_X:
        printf("Raio-X");
        break;
    case EXAME_TOMOGRAFIA:
        printf("Tomografia");
        break;
    case EXAME_RESSONANCIA:
        printf("Ressonancia");
        break;
    case EXAME_ELETROCARDIOGRAMA:
        printf("Eletrocardiograma");
        break;
    case EXAME_URINA:
        printf("Urina");
        break;
    case EXAME_ULTRASSONOGRAFIA:
        printf("Ultrassonografia");
        break;
    default:
        printf("Desconhecido");
        break;
    }
}

static void exibirExame(const Exame *exame)
{
    printf("\nID: %d\n", exame->id);
    printf("Paciente: %s\n", nomePaciente(exame->pacienteId));
    printf("Medico: %s\n", nomeMedico(exame->medicoId));
    printf("Prontuario ID: %d\n", exame->prontuarioId);
    printf("Tipo: ");
    exibirNomeTipoExame(exame->tipoExame);
    printf("\nData da solicitacao: %s\n", exame->dataSolicitacao);
    printf("Data do resultado: %s\n", exame->dataResultado);
    printf("Resultado: %s\n", exame->resultado);
    printf("Status: %s\n", exame->status);
    printf("Urgente: %s\n", exame->urgente == 1 ? "Sim" : "Nao");
}

int escolherTipoExame(void)
{
    int tipoExame;

    printf("\nTipos de exame:\n");
    printf("1. Hemograma\n");
    printf("2. Raio-X\n");
    printf("3. Tomografia\n");
    printf("4. Ressonancia\n");
    printf("5. Eletrocardiograma\n");
    printf("6. Urina\n");
    printf("7. Ultrassonografia\n");
    printf("0. Cancelar\n");
    printf("Escolha uma opcao: ");

    if (lerInteiro(&tipoExame) == 0)
    {
        printf("\nTipo de exame invalido.\n");
        return 0;
    }

    if (tipoExame < 0 || tipoExame > EXAME_ULTRASSONOGRAFIA)
    {
        printf("\nTipo de exame invalido.\n");
        return 0;
    }

    return tipoExame;
}

int solicitarExame(int pacienteId, int medicoId, int prontuarioId,
                   int tipoExame, const char dataSolicitacao[])
{
    if (totalExames >= MAX_EXAMES)
    {
        return 0;
    }

    if (buscarPacienteAtivo(pacienteId) == -1 ||
        buscarMedicoAtivo(medicoId) == -1)
    {
        return 0;
    }

    if (prontuarioCompativel(prontuarioId, pacienteId, medicoId) == 0)
    {
        return 0;
    }

    if (tipoExame < EXAME_HEMOGRAMA || tipoExame > EXAME_ULTRASSONOGRAFIA)
    {
        return 0;
    }

    if (textoVazio(dataSolicitacao))
    {
        return 0;
    }

    exames[totalExames].id = totalExames + 1;
    exames[totalExames].pacienteId = pacienteId;
    exames[totalExames].medicoId = medicoId;
    exames[totalExames].prontuarioId = prontuarioId;
    exames[totalExames].tipoExame = tipoExame;
    strcpy(exames[totalExames].dataSolicitacao, dataSolicitacao);
    strcpy(exames[totalExames].dataResultado, "");
    strcpy(exames[totalExames].resultado, "");
    strcpy(exames[totalExames].status, "Solicitado");
    exames[totalExames].urgente = exameUrgentePorTriagem(pacienteId);
    exames[totalExames].ativo = 1;

    totalExames++;

    return 1;
}

int registrarResultadoExame(int exameId, const char dataResultado[],
                            const char resultado[])
{
    int indiceExame = buscarExameAtivo(exameId);

    if (indiceExame == -1)
    {
        return 0;
    }

    if (strcmp(exames[indiceExame].status, "Solicitado") != 0)
    {
        return 0;
    }

    if (textoVazio(dataResultado) || textoVazio(resultado))
    {
        return 0;
    }

    strcpy(exames[indiceExame].dataResultado, dataResultado);
    strcpy(exames[indiceExame].resultado, resultado);
    strcpy(exames[indiceExame].status, "Realizado");

    return 1;
}

int cancelarExame(int exameId)
{
    int indiceExame = buscarExameAtivo(exameId);

    if (indiceExame == -1)
    {
        return 0;
    }

    if (strcmp(exames[indiceExame].status, "Solicitado") != 0)
    {
        return 0;
    }

    strcpy(exames[indiceExame].status, "Cancelado");
    return 1;
}

int excluirExame(int exameId)
{
    int indiceExame = buscarExameAtivo(exameId);

    if (indiceExame == -1)
    {
        return 0;
    }

    exames[indiceExame].ativo = 0;
    return 1;
}

void listarExames(void)
{
    int encontrou = 0;

    for (int i = 0; i < totalExames; i++)
    {
        if (exames[i].ativo == 1)
        {
            exibirExame(&exames[i]);
            encontrou = 1;
        }
    }

    if (encontrou == 0)
    {
        printf("\nNenhum exame cadastrado.\n");
    }
}

void listarExamesPorPaciente(int pacienteId)
{
    int encontrou = 0;

    for (int i = 0; i < totalExames; i++)
    {
        if (exames[i].ativo == 1 && exames[i].pacienteId == pacienteId)
        {
            exibirExame(&exames[i]);
            encontrou = 1;
        }
    }

    if (encontrou == 0)
    {
        printf("\nNenhum exame encontrado para esse paciente.\n");
    }
}

void listarExamesPorMedico(int medicoId)
{
    int encontrou = 0;

    for (int i = 0; i < totalExames; i++)
    {
        if (exames[i].ativo == 1 && exames[i].medicoId == medicoId)
        {
            exibirExame(&exames[i]);
            encontrou = 1;
        }
    }

    if (encontrou == 0)
    {
        printf("\nNenhum exame encontrado para esse medico.\n");
    }
}

void listarExamesPorProntuario(int prontuarioId)
{
    int encontrou = 0;

    for (int i = 0; i < totalExames; i++)
    {
        if (exames[i].ativo == 1 && exames[i].prontuarioId == prontuarioId)
        {
            exibirExame(&exames[i]);
            encontrou = 1;
        }
    }

    if (encontrou == 0)
    {
        printf("\nNenhum exame encontrado para esse prontuario.\n");
    }
}

void listarExamesUrgentes(void)
{
    int encontrou = 0;

    for (int i = 0; i < totalExames; i++)
    {
        if (exames[i].ativo == 1 && exames[i].urgente == 1)
        {
            exibirExame(&exames[i]);
            encontrou = 1;
        }
    }

    if (encontrou == 0)
    {
        printf("\nNenhum exame urgente encontrado.\n");
    }
}

int contarExamesPorStatus(const char status[])
{
    int total = 0;

    for (int i = 0; i < totalExames; i++)
    {
        if (exames[i].ativo == 1 && strcmp(exames[i].status, status) == 0)
        {
            total++;
        }
    }

    return total;
}

int contarExamesPorTipo(int tipoExame)
{
    int total = 0;

    for (int i = 0; i < totalExames; i++)
    {
        if (exames[i].ativo == 1 && exames[i].tipoExame == tipoExame)
        {
            total++;
        }
    }

    return total;
}

int contarExamesUrgentes(void)
{
    int total = 0;

    for (int i = 0; i < totalExames; i++)
    {
        if (exames[i].ativo == 1 && exames[i].urgente == 1)
        {
            total++;
        }
    }

    return total;
}

void menuExames(void)
{
    int opcao;

    do
    {
        printf("\n=============================================\n");
        printf("MENU EXAMES\n");
        printf("=============================================\n");
        printf("1. Solicitar exame\n");
        printf("2. Registrar resultado\n");
        printf("3. Cancelar exame\n");
        printf("4. Excluir exame\n");
        printf("5. Listar todos os exames\n");
        printf("6. Listar exames por paciente\n");
        printf("7. Listar exames por medico\n");
        printf("8. Listar exames por prontuario\n");
        printf("9. Listar exames urgentes\n");
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
            int prontuarioId;
            int tipoExame;
            char dataSolicitacao[11];

            printf("\nID do paciente: ");
            if (lerInteiro(&pacienteId) == 0)
            {
                printf("\nPaciente invalido.\n");
                break;
            }

            printf("ID do medico: ");
            if (lerInteiro(&medicoId) == 0)
            {
                printf("\nMedico invalido.\n");
                break;
            }

            printf("ID do prontuario: ");
            if (lerInteiro(&prontuarioId) == 0)
            {
                printf("\nProntuario invalido.\n");
                break;
            }

            tipoExame = escolherTipoExame();
            if (tipoExame == 0)
            {
                break;
            }

            printf("Data da solicitacao (DD/MM/AAAA): ");
            scanf(" %[^\n]", dataSolicitacao);

            if (solicitarExame(pacienteId, medicoId, prontuarioId,
                               tipoExame, dataSolicitacao) == 1)
            {
                printf("\nExame solicitado com sucesso.\n");
            }
            else
            {
                printf("\nNao foi possivel solicitar o exame.\n");
            }

            break;
        }

        case 2:
        {
            int exameId;
            char dataResultado[11];
            char resultado[300];

            printf("\nID do exame: ");
            if (lerInteiro(&exameId) == 0)
            {
                printf("\nExame invalido.\n");
                break;
            }

            printf("Data do resultado (DD/MM/AAAA): ");
            scanf(" %[^\n]", dataResultado);

            printf("Resultado: ");
            scanf(" %[^\n]", resultado);

            if (registrarResultadoExame(exameId, dataResultado, resultado) == 1)
            {
                printf("\nResultado registrado com sucesso.\n");
            }
            else
            {
                printf("\nNao foi possivel registrar o resultado.\n");
            }

            break;
        }

        case 3:
        {
            int exameId;

            printf("\nID do exame: ");
            if (lerInteiro(&exameId) == 0)
            {
                printf("\nExame invalido.\n");
                break;
            }

            if (cancelarExame(exameId) == 1)
            {
                printf("\nExame cancelado com sucesso.\n");
            }
            else
            {
                printf("\nNao foi possivel cancelar o exame.\n");
            }

            break;
        }

        case 4:
        {
            int exameId;

            printf("\nID do exame: ");
            if (lerInteiro(&exameId) == 0)
            {
                printf("\nExame invalido.\n");
                break;
            }

            if (excluirExame(exameId) == 1)
            {
                printf("\nExame excluido com sucesso.\n");
            }
            else
            {
                printf("\nNao foi possivel excluir o exame.\n");
            }

            break;
        }

        case 5:
            listarExames();
            break;

        case 6:
        {
            int pacienteId;

            printf("\nID do paciente: ");
            if (lerInteiro(&pacienteId) == 0)
            {
                printf("\nPaciente invalido.\n");
                break;
            }

            listarExamesPorPaciente(pacienteId);
            break;
        }

        case 7:
        {
            int medicoId;

            printf("\nID do medico: ");
            if (lerInteiro(&medicoId) == 0)
            {
                printf("\nMedico invalido.\n");
                break;
            }

            listarExamesPorMedico(medicoId);
            break;
        }

        case 8:
        {
            int prontuarioId;

            printf("\nID do prontuario: ");
            if (lerInteiro(&prontuarioId) == 0)
            {
                printf("\nProntuario invalido.\n");
                break;
            }

            listarExamesPorProntuario(prontuarioId);
            break;
        }

        case 9:
            listarExamesUrgentes();
            break;

        case 0:
            printf("\nVoltando ao menu principal...\n");
            break;

        default:
            printf("\nOpcao invalida. Tente novamente.\n");
            break;
        }

    } while (opcao != 0);
}
