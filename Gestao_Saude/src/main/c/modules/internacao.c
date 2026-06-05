#include "internacao.h"

int internarPaciente(int pacienteId, int leitoId, const char dataEntrada[])
{
    int pacienteEncontrado = 0;
    int indiceLeito = -1;
    int indiceAla = -1;

    if (totalInternacoes >= MAX_INTERNACOES)
    {
        return 0;
    }

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
        return 0;
    }

    for (int i = 0; i < totalLeitos; i++)
    {
        if (leitos[i].id == leitoId)
        {
            indiceLeito = i;
            break;
        }
    }

    if (indiceLeito == -1 || leitos[indiceLeito].ocupado == 1)
    {
        return 0;
    }

    for (int i = 0; i < totalAlas; i++)
    {
        if (alas[i].id == leitos[indiceLeito].alaId && alas[i].ativo == 1)
        {
            indiceAla = i;
            break;
        }
    }

    internacoes[totalInternacoes].id = totalInternacoes + 1;
    internacoes[totalInternacoes].pacienteId = pacienteId;
    internacoes[totalInternacoes].alaId = leitos[indiceLeito].alaId;
    internacoes[totalInternacoes].leitoId = leitoId;
    strcpy(internacoes[totalInternacoes].dataEntrada, dataEntrada);
    strcpy(internacoes[totalInternacoes].dataAlta, "00/00/0000");
    strcpy(internacoes[totalInternacoes].status, "INTERNADO");

    leitos[indiceLeito].ocupado = 1;
    leitos[indiceLeito].pacienteId = pacienteId;

    if (indiceAla != -1)
    {
        alas[indiceAla].leitosOcupados++;
    }

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

            for (int j = 0; j < totalLeitos; j++)
            {
                if (leitos[j].id == internacoes[i].leitoId)
                {
                    leitos[j].ocupado = 0;
                    leitos[j].pacienteId = 0;
                    break;
                }
            }

            for (int j = 0; j < totalAlas; j++)
            {
                if (alas[j].id == internacoes[i].alaId && alas[j].leitosOcupados > 0)
                {
                    alas[j].leitosOcupados--;
                    break;
                }
            }

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
        scanf("%d", &caso6);

        switch (caso6)
        {
        case 1:
        {
            int pacienteId, leitoId;
            int pacienteEncontrado = 0;
            int leitoEncontrado = 0;
            int indiceLeito = -1;
            int indiceAla = -1;

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

            for (int i = 0; i < totalAlas; i++)
            {
                if (alas[i].id == leitos[indiceLeito].alaId && alas[i].ativo == 1)
                {
                    indiceAla = i;
                    break;
                }
            }

            internacoes[totalInternacoes].id = totalInternacoes + 1;
            internacoes[totalInternacoes].pacienteId = pacienteId;
            internacoes[totalInternacoes].alaId = leitos[indiceLeito].alaId;
            internacoes[totalInternacoes].leitoId = leitoId;

            printf("Data de entrada (DD/MM/AAAA): ");
            scanf(" %[^\n]", internacoes[totalInternacoes].dataEntrada);

            strcpy(internacoes[totalInternacoes].dataAlta, "00/00/0000");
            strcpy(internacoes[totalInternacoes].status, "INTERNADO");

            leitos[indiceLeito].ocupado = 1;
            leitos[indiceLeito].pacienteId = pacienteId;

            if (indiceAla != -1)
            {
                alas[indiceAla].leitosOcupados++;
            }

            totalInternacoes++;

            printf("\nPaciente internado com sucesso! ID da internacao: %d\n", internacoes[totalInternacoes - 1].id);
            break;
        }

        case 2:
        {
            int idBusca;
            int encontrado = 0;

            printf("\nDigite o ID da internacao: ");
            scanf("%d", &idBusca);

            for (int i = 0; i < totalInternacoes; i++)
            {
                if (internacoes[i].id == idBusca && strcmp(internacoes[i].status, "INTERNADO") == 0)
                {
                    printf("Data de alta (DD/MM/AAAA): ");
                    scanf(" %[^\n]", internacoes[i].dataAlta);

                    strcpy(internacoes[i].status, "ALTA");

                    for (int j = 0; j < totalLeitos; j++)
                    {
                        if (leitos[j].id == internacoes[i].leitoId)
                        {
                            leitos[j].ocupado = 0;
                            leitos[j].pacienteId = 0;
                            break;
                        }
                    }

                    for (int j = 0; j < totalAlas; j++)
                    {
                        if (alas[j].id == internacoes[i].alaId && alas[j].leitosOcupados > 0)
                        {
                            alas[j].leitosOcupados--;
                            break;
                        }
                    }

                    encontrado = 1;
                    printf("\nAlta registrada com sucesso.\n");
                    break;
                }
            }

            if (encontrado == 0)
            {
                printf("\nInternacao nao encontrada ou ja finalizada.\n");
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
                printf("\nID: %d\n", internacoes[i].id);
                printf("Paciente ID: %d\n", internacoes[i].pacienteId);
                printf("Ala ID: %d\n", internacoes[i].alaId);
                printf("Leito ID: %d\n", internacoes[i].leitoId);
                printf("Entrada: %s\n", internacoes[i].dataEntrada);
                printf("Alta: %s\n", internacoes[i].dataAlta);
                printf("Status: %s\n", internacoes[i].status);
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
