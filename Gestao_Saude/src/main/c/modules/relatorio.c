#include "relatorio.h"

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

int contarLeitosLivres(void)
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

float calcularTaxaOcupacaoAla(int alaId)
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

int contarTriagensPorClassificacao(const char classificacao[])
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

    printf("\nLeitos ocupados por ala:\n");

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
}
