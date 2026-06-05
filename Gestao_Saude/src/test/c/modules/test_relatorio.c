#include <assert.h>
#include <string.h>
#include "relatorio.h"

Medico medicos[MAX_MEDICOS];
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

    totalMedicos = 4;
    medicos[0].ativo = 1;
    medicos[0].regiaoAdministrativa = 1;
    medicos[1].ativo = 1;
    medicos[1].regiaoAdministrativa = 1;
    medicos[2].ativo = 1;
    medicos[2].regiaoAdministrativa = 4;
    medicos[3].ativo = 0;
    medicos[3].regiaoAdministrativa = 1;

    assert(contarLeitosOcupados() == 2);
    assert(contarLivres() == 2);
    assert(taxaAla(1) == 50.0f);
    assert(taxaAla(99) == 0.0f);
    assert(contarTriagens("Emergencia") == 2);
    assert(contarTriagens("Comum") == 1);
    assert(contarTriagens("Prioritario") == 0);
    assert(contarMedRegiao(1) == 2);
    assert(contarMedRegiao(4) == 1);
    assert(contarMedRegiao(8) == 0);

    return 0;
}
