#include "relatorio.h"

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

static int contarLeitos(int ocupado)
{
    int total = 0;

    for (int i = 0; i < totalLeitos; i++)
    {
        if (leitos[i].ativo == 1 && leitos[i].ocupado == ocupado)
        {
            total++;
        }
    }

    return total;
}

static int casoGrave(const char classificacao[])
{
    return strcmp(classificacao, "Emergencia") == 0 ||
           strcmp(classificacao, "Muito prioritario") == 0;
}

static void exibirContagemPorRegiao(const char titulo[], int (*contador)(int))
{
    printf("\n%s\n", titulo);

    for (int regiao = 1; regiao <= 8; regiao++)
    {
        printf("%s: %d\n", nomeRegiao(regiao), contador(regiao));
    }
}

int contarPacientesAtivos(void)
{
    int total = 0;

    for (int i = 0; i < totalPacientes; i++)
    {
        if (pacientes[i].ativo == 1)
        {
            total++;
        }
    }

    return total;
}

int contarMedicosAtivos(void)
{
    int total = 0;

    for (int i = 0; i < totalMedicos; i++)
    {
        if (medicos[i].ativo == 1)
        {
            total++;
        }
    }

    return total;
}

int contarProntuariosAtivos(void)
{
    int total = 0;

    for (int i = 0; i < totalProntuarios; i++)
    {
        if (prontuarios[i].ativo == 1)
        {
            total++;
        }
    }

    return total;
}

int contarLeitosOcupados(void)
{
    return contarLeitos(1);
}

int contarLivres(void)
{
    return contarLeitos(0);
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
        if (triagens[i].ativo == 1 &&
            strcmp(triagens[i].classificacao, classificacao) == 0)
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
    exibirContagemPorRegiao("Pacientes ativos por regiao:", contarPacRegiao);
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
    exibirContagemPorRegiao("Medicos ativos por regiao:", contarMedRegiao);
}

int contarCasosGravesRegiao(int regiaoAdministrativa)
{
    int total = 0;

    for (int i = 0; i < totalTriagens; i++)
    {
        if (triagens[i].ativo == 1 &&
            casoGrave(triagens[i].classificacao))
        {
            for (int j = 0; j < totalPacientes; j++)
            {
                if (pacientes[j].ativo == 1 &&
                    pacientes[j].id == triagens[i].pacienteId &&
                    pacientes[j].regiaoAdministrativa == regiaoAdministrativa)
                {
                    total++;
                    break;
                }
            }
        }
    }

    return total;
}

int regiaoMaisCasosGraves(void)
{
    int regiaoMaior = 0;
    int maiorTotal = 0;

    for (int regiao = 1; regiao <= 8; regiao++)
    {
        int total = contarCasosGravesRegiao(regiao);

        if (total > maiorTotal)
        {
            maiorTotal = total;
            regiaoMaior = regiao;
        }
    }

    return regiaoMaior;
}

static void exibirTotaisGerais(void)
{
    printf("\n=============================================\n");
    printf("RELATORIOS GERAIS\n");
    printf("=============================================\n");
    printf("Total de pacientes cadastrados: %d\n", totalPacientes);
    printf("Pacientes ativos: %d\n", contarPacientesAtivos());
    printf("Total de medicos cadastrados: %d\n", totalMedicos);
    printf("Medicos ativos: %d\n", contarMedicosAtivos());
    printf("Total de agendamentos cadastrados: %d\n", totalAgendamentos);
    printf("Total de alas cadastradas: %d\n", totalAlas);
    printf("Total de leitos cadastrados: %d\n", totalLeitos);
    printf("Total de internacoes cadastradas: %d\n", totalInternacoes);
    printf("Total de triagens cadastradas: %d\n", totalTriagens);
    printf("Prontuarios ativos: %d\n", contarProntuariosAtivos());
}

static void exibirTriagensClassificacao(void)
{
    printf("\n=============================================\n");
    printf("\nTriagens por classificacao:\n");
    printf("Emergencia: %d\n", contarTriagens("Emergencia"));
    printf("Muito prioritario: %d\n", contarTriagens("Muito prioritario"));
    printf("Prioritario: %d\n", contarTriagens("Prioritario"));
    printf("Comum: %d\n", contarTriagens("Comum"));
    printf("Orientacao basica: %d\n", contarTriagens("Orientacao basica"));
}

static void exibirResumoLeitos(void)
{
    printf("\n=============================================\n");
    printf("\nResumo de leitos:\n");
    printf("Leitos ocupados: %d\n", contarLeitosOcupados());
    printf("Leitos livres: %d\n", contarLivres());
}

static void exibirTaxaPorAla(void)
{
    printf("\n=============================================\n");
    printf("\nLeitos ocupados por ala:\n");
    printf("\n----------------------------------------------\n");

    for (int i = 0; i < totalAlas; i++)
    {
        if (alas[i].ativo == 1)
        {
            float taxa = taxaAla(alas[i].id);

            printf("\nAla: %s\n", alas[i].nome);
            printf("Ocupados: %d/%d\n", alas[i].leitosOcupados, alas[i].totalLeitos);
            printf("Taxa de ocupacao: %.2f%%\n", taxa);
        }
    }
}

static void exibirEspecialidadeDemandada(void)
{
    char especialidade[50];

    espDemandada(especialidade);
    printf("\nEspecialidade mais demandada: %s\n", especialidade);
}

static void exibirRegiaoMaisCasosGraves(void)
{
    int regiao = regiaoMaisCasosGraves();

    printf("\nRegiao com mais casos graves: ");

    if (regiao == 0)
    {
        printf("Nenhuma\n");
    }
    else
    {
        printf("%s", nomeRegiao(regiao));
        printf(" (%d casos)\n", contarCasosGravesRegiao(regiao));
    }
}

void menuRelatorios(void)
{
    exibirTotaisGerais();
    exibirTriagensClassificacao();
    exibirResumoLeitos();
    exibirTaxaPorAla();
    exibirEspecialidadeDemandada();
    relMedRegiao();
    relPacRegiao();
    exibirRegiaoMaisCasosGraves();
}
