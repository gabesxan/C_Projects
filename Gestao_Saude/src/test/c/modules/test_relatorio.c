#include <assert.h>
#include <string.h>
#include "relatorio.h"

Ala alas[MAX_ALAS];
Leito leitos[MAX_LEITOS];
Triagem triagens[MAX_TRIAGENS];

int totalAlas = 0;
int totalLeitos = 0;
int totalTriagens = 0;

int totalPacientes = 0;
int totalMedicos = 0;
int totalAgendamentos = 0;
int totalInternacoes = 0;

int main(void)
{
    totalAlas = 1;
    alas[0].id = 1;
    strcpy(alas[0].nome, "Clinica Medica");
    strcpy(alas[0].tipo, "Internacao");
    alas[0].totalLeitos = 4;
    alas[0].leitosOcupados = 2;
    alas[0].ativo = 1;

    totalLeitos = 4;
    leitos[0].ocupado = 1;
    leitos[1].ocupado = 1;
    leitos[2].ocupado = 0;
    leitos[3].ocupado = 0;

    totalTriagens = 3;
    strcpy(triagens[0].classificacao, "Emergencia");
    strcpy(triagens[1].classificacao, "Emergencia");
    strcpy(triagens[2].classificacao, "Comum");

    assert(contarLeitosOcupados() == 2);
    assert(contarLeitosLivres() == 2);
    assert(calcularTaxaOcupacaoAla(1) == 50.0f);
    assert(calcularTaxaOcupacaoAla(99) == 0.0f);
    assert(contarTriagensPorClassificacao("Emergencia") == 2);
    assert(contarTriagensPorClassificacao("Comum") == 1);
    assert(contarTriagensPorClassificacao("Prioritario") == 0);

    return 0;
}
