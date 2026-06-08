#include "internacao.h"

static int buscarPaciente(int pacienteId)
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

static int buscarLeito(int leitoId)
{
    for (int i = 0; i < totalLeitos; i++)
    {
        if (leitos[i].id == leitoId)
        {
            return i;
        }
    }

    return -1;
}

static int buscarAlaAtiva(int alaId)
{
    for (int i = 0; i < totalAlas; i++)
    {
        if (alas[i].id == alaId && alas[i].ativo == 1)
        {
            return i;
        }
    }

    return -1;
}

static void ocuparLeito(int indiceLeito, int indiceAla, int pacienteId)
{
    leitos[indiceLeito].ocupado = 1;
    leitos[indiceLeito].pacienteId = pacienteId;

    if (indiceAla != -1)
    {
        alas[indiceAla].leitosOcupados++;
    }
}

static void liberarLeito(int leitoId, int alaId)
{
    int indiceLeito = buscarLeito(leitoId);

    if (indiceLeito != -1)
    {
        leitos[indiceLeito].ocupado = 0;
        leitos[indiceLeito].pacienteId = 0;
    }

    for (int i = 0; i < totalAlas; i++)
    {
        if (alas[i].id == alaId && alas[i].leitosOcupados > 0)
        {
            alas[i].leitosOcupados--;
            break;
        }
    }
}

static void exibirInternacao(const Internacao *internacao)
{
    printf("\nID: %d\n", internacao->id);
    printf("Paciente ID: %d\n", internacao->pacienteId);
    printf("Ala ID: %d\n", internacao->alaId);
    printf("Leito ID: %d\n", internacao->leitoId);
    printf("Entrada: %s\n", internacao->dataEntrada);
    printf("Alta: %s\n", internacao->dataAlta);
    printf("Status: %s\n", internacao->status);
}

int internarPaciente(int pacienteId, int leitoId, const char dataEntrada[])
{
    int indicePaciente;
    int indiceLeito;
    int indiceAla;

    if (totalInternacoes >= MAX_INTERNACOES)
    {
        return 0;
    }

    indicePaciente = buscarPaciente(pacienteId);

    if (indicePaciente == -1)
    {
        return 0;
    }

    indiceLeito = buscarLeito(leitoId);

    if (indiceLeito == -1 || leitos[indiceLeito].ocupado == 1)
    {
        return 0;
    }

    indiceAla = buscarAlaAtiva(leitos[indiceLeito].alaId);

    internacoes[totalInternacoes].id = totalInternacoes + 1;
    internacoes[totalInternacoes].pacienteId = pacienteId;
    internacoes[totalInternacoes].alaId = leitos[indiceLeito].alaId;
    internacoes[totalInternacoes].leitoId = leitoId;
    strcpy(internacoes[totalInternacoes].dataEntrada, dataEntrada);
    strcpy(internacoes[totalInternacoes].dataAlta, "00/00/0000");
    strcpy(internacoes[totalInternacoes].status, "INTERNADO");

    ocuparLeito(indiceLeito, indiceAla, pacienteId);

    totalInternacoes++;

    return 1;
}

int darAltaInternacao(int internacaoId, const char dataAlta[])
{
    for (int i = 0; i < totalInternacoes; i++)
    {
        if (internacoes[i].id == internacaoId && strcmp(internacoes[i].status, "INTERNADO") == 0)
        {
            strcpy(internacoes[i].dataAlta, dataAlta);
            strcpy(internacoes[i].status, "ALTA");
            liberarLeito(internacoes[i].leitoId, internacoes[i].alaId);

            return 1;
        }
    }

    return 0;
}

void menuInternacoes(void)
{
    int caso6;

    do
    {
        printf("\n=============================================\n");
        printf("MENU GERENCIAMENTO DE INTERNACOES\n");
        printf("=============================================\n");
        printf("1. Internar Paciente\n");
        printf("2. Dar Alta\n");
        printf("3. Listar Internacoes\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("---------------------------------------------\n");
        printf("Escolha uma opcao: ");
        if (lerInteiro(&caso6) == 0)
        {
            printf("\nOpcao invalida. Tente novamente.\n");
            continue;
        }

        switch (caso6)
        {
        case 1:
        {
            int pacienteId;
            int leitoId;
            int pacienteEncontrado = 0;
            int leitoEncontrado = 0;
            int indiceLeito = -1;
            char dataEntrada[11];

            if (totalInternacoes >= MAX_INTERNACOES)
            {
                printf("\nLimite de internacoes atingido.\n");
                break;
            }

            printf("\nID do paciente: ");
            scanf("%d", &pacienteId);

            for (int i = 0; i < totalPacientes; i++)
            {
                if (pacientes[i].id == pacienteId && pacientes[i].ativo == 1)
                {
                    pacienteEncontrado = 1;
                    break;
                }
            }

            if (pacienteEncontrado == 0)
            {
                printf("\nPaciente nao encontrado ou inativo.\n");
                break;
            }

            printf("ID do leito: ");
            scanf("%d", &leitoId);

            for (int i = 0; i < totalLeitos; i++)
            {
                if (leitos[i].id == leitoId)
                {
                    leitoEncontrado = 1;
                    indiceLeito = i;
                    break;
                }
            }

            if (leitoEncontrado == 0 || leitos[indiceLeito].ocupado == 1)
            {
                printf("\nLeito nao encontrado ou ja ocupado.\n");
                break;
            }

            printf("Data de entrada (DD/MM/AAAA): ");
            scanf(" %[^\n]", dataEntrada);

            if (internarPaciente(pacienteId, leitoId, dataEntrada) == 0)
            {
                printf("\nNao foi possivel internar o paciente.\n");
                break;
            }

            printf("\nPaciente internado com sucesso! ID da internacao: %d\n", internacoes[totalInternacoes - 1].id);
            break;
        }

        case 2:
        {
            int idBusca;
            int encontrado = 0;
            char dataAlta[11];

            printf("\nDigite o ID da internacao: ");
            scanf("%d", &idBusca);

            for (int i = 0; i < totalInternacoes; i++)
            {
                if (internacoes[i].id == idBusca && strcmp(internacoes[i].status, "INTERNADO") == 0)
                {
                    encontrado = 1;
                    break;
                }
            }

            if (encontrado == 0)
            {
                printf("\nInternacao nao encontrada ou ja finalizada.\n");
                break;
            }

            printf("Data de alta (DD/MM/AAAA): ");
            scanf(" %[^\n]", dataAlta);

            if (darAltaInternacao(idBusca, dataAlta) == 1)
            {
                printf("\nAlta registrada com sucesso.\n");
            }

            break;
        }

        case 3:
        {
            if (totalInternacoes == 0)
            {
                printf("\nNenhuma internacao cadastrada.\n");
                break;
            }

            printf("\nLista de Internacoes:\n");

            for (int i = 0; i < totalInternacoes; i++)
            {
                exibirInternacao(&internacoes[i]);
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

    } while (caso6 != 0);
}
