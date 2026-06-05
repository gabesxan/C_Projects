#include "relatorio.h"

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

int contarLeitosOcupados(void)
{
    int total = 0;

    for (int i = 0; i < totalLeitos; i++)
    {
        if (leitos[i].ocupado == 1)
        {
            total++;
        }
    }

    return total;
}

int contarLivres(void)
{
    int total = 0;

    for (int i = 0; i < totalLeitos; i++)
    {
        if (leitos[i].ocupado == 0)
        {
            total++;
        }
    }

    return total;
}

float taxaAla(int alaId)
{
    for (int i = 0; i < totalAlas; i++)
    {
        if (alas[i].id == alaId && alas[i].ativo == 1 && alas[i].totalLeitos > 0)
        {
            return (alas[i].leitosOcupados * 100.0f) / alas[i].totalLeitos;
        }
    }

    return 0.0f;
}

int contarTriagens(const char classificacao[])
{
    int total = 0;

    for (int i = 0; i < totalTriagens; i++)
    {
        if (strcmp(triagens[i].classificacao, classificacao) == 0)
        {
            total++;
        }
    }

    return total;
}

int contarMedRegiao(int regiaoAdministrativa)
{
    int total = 0;

    for (int i = 0; i < totalMedicos; i++)
    {
        if (medicos[i].ativo == 1 && medicos[i].regiaoAdministrativa == regiaoAdministrativa)
        {
            total++;
        }
    }

    return total;
}

int contarPacRegiao(int regiaoAdministrativa)
{
    int total = 0;

    for (int i = 0; i < totalPacientes; i++)
    {
        if (pacientes[i].ativo == 1 && pacientes[i].regiaoAdministrativa == regiaoAdministrativa)
        {
            total++;
        }
    }

    return total;
}

void relPacRegiao(void)
{
    printf("\nPacientes ativos por regiao:\n");

    for (int regiao = 1; regiao <= 8; regiao++)
    {
        nomeRegiao(regiao);
        printf(": %d\n", contarPacRegiao(regiao));
    }
}

int contarEsp(const char especialidade[])
{
    int total = 0;

    for (int i = 0; i < totalAgendamentos; i++)
    {
        if (strcmp(agendamentos[i].status, "AGENDADO") == 0 ||
            strcmp(agendamentos[i].status, "CONCLUIDO") == 0)
        {
            for (int j = 0; j < totalMedicos; j++)
            {
                if (medicos[j].id == agendamentos[i].medicoId &&
                    medicos[j].ativo == 1 &&
                    strcmp(medicos[j].especialidade, especialidade) == 0)
                {
                    total++;
                    break;
                }
            }
        }
    }

    return total;
}

void espDemandada(char resultado[])
{
    int maior = 0;
    int encontrou = 0;

    strcpy(resultado, "Nenhuma");

    for (int i = 0; i < totalMedicos; i++)
    {
        int totalEspecialidade;

        if (medicos[i].ativo == 0)
        {
            continue;
        }

        totalEspecialidade = contarEsp(medicos[i].especialidade);

        if (totalEspecialidade > maior)
        {
            maior = totalEspecialidade;
            strcpy(resultado, medicos[i].especialidade);
            encontrou = 1;
        }
    }

    if (encontrou == 0)
    {
        strcpy(resultado, "Nenhuma");
    }
}
void relMedRegiao(void)
{
    printf("\nMedicos ativos por regiao:\n");

    for (int regiao = 1; regiao <= 8; regiao++)
    {
        nomeRegiao(regiao);
        printf(": %d\n", contarMedRegiao(regiao));
    }
}

void menuRelatorios(void)
{
    printf("\n=============================================\n");
    printf("RELATORIOS GERAIS\n");
    printf("=============================================\n");
    printf("Total de pacientes cadastrados: %d\n", totalPacientes);
    printf("Total de medicos cadastrados: %d\n", totalMedicos);
    printf("Total de agendamentos cadastrados: %d\n", totalAgendamentos);
    printf("Total de alas cadastradas: %d\n", totalAlas);
    printf("Total de leitos cadastrados: %d\n", totalLeitos);
    printf("Total de internacoes cadastradas: %d\n", totalInternacoes);
    printf("Total de triagens cadastradas: %d\n", totalTriagens);
    printf("\n=============================================\n");
    printf("\nTriagens por classificacao:\n");
    printf("Emergencia: %d\n", contarTriagens("Emergencia"));
    printf("Muito prioritario: %d\n", contarTriagens("Muito prioritario"));
    printf("Prioritario: %d\n", contarTriagens("Prioritario"));
    printf("Comum: %d\n", contarTriagens("Comum"));
    printf("Orientacao basica: %d\n", contarTriagens("Orientacao basica"));
    printf("\n=============================================\n");
    printf("\nResumo de leitos:\n");
    printf("Leitos ocupados: %d\n", contarLeitosOcupados());
    printf("Leitos livres: %d\n", contarLivres());
    printf("\n=============================================\n");
    printf("\nLeitos ocupados por ala:\n");
    printf("\n----------------------------------------------\n");

    for (int i = 0; i < totalAlas; i++)
    {
        if (alas[i].ativo == 1)
        {
            float taxa = 0;

            if (alas[i].totalLeitos > 0)
            {
                taxa = (alas[i].leitosOcupados * 100.0) / alas[i].totalLeitos;
            }

            printf("\nAla: %s\n", alas[i].nome);
            printf("Ocupados: %d/%d\n", alas[i].leitosOcupados, alas[i].totalLeitos);
            printf("Taxa de ocupacao: %.2f%%\n", taxa);
        }
    }
   
    {
        char especialidade[50];

        espDemandada(especialidade);
        printf("\nEspecialidade mais demandada: %s\n", especialidade);
    }
    relMedRegiao();
    relPacRegiao();
}
