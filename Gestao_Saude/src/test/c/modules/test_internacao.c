#include <assert.h>
#include <string.h>
#include "internacao.h"

Paciente pacientes[MAX_PACIENTES];
Ala alas[MAX_ALAS];
Leito leitos[MAX_LEITOS];
Internacao internacoes[MAX_INTERNACOES];

int totalPacientes = 0;
int totalAlas = 0;
int totalLeitos = 0;
int totalInternacoes = 0;

static void prepararDados(void)
{
    totalPacientes = 1;
    pacientes[0].id = 1;
    strcpy(pacientes[0].nome, "Maria Aparecida Santos");
    pacientes[0].idade = 62;
    pacientes[0].ativo = 1;

    totalAlas = 1;
    alas[0].id = 1;
    strcpy(alas[0].nome, "Clinica Medica");
    alas[0].tipo = ALA_INTERNACAO;
    alas[0].totalLeitos = 10;
    alas[0].leitosOcupados = 0;
    alas[0].ativo = 1;

    totalLeitos = 1;
    leitos[0].id = 1;
    leitos[0].alaId = 1;
    leitos[0].numero = 101;
    leitos[0].ocupado = 0;
    leitos[0].pacienteId = 0;

    totalInternacoes = 0;
}

int main(void)
{
    prepararDados();

    assert(internarPaciente(1, 1, "05/06/2026") == 1);
    assert(totalInternacoes == 1);
    assert(internacoes[0].id == 1);
    assert(internacoes[0].pacienteId == 1);
    assert(internacoes[0].alaId == 1);
    assert(internacoes[0].leitoId == 1);
    assert(strcmp(internacoes[0].status, "INTERNADO") == 0);
    assert(leitos[0].ocupado == 1);
    assert(leitos[0].pacienteId == 1);
    assert(alas[0].leitosOcupados == 1);

    assert(internarPaciente(1, 1, "05/06/2026") == 0);
    assert(internarPaciente(99, 1, "05/06/2026") == 0);
    assert(darAltaInternacao(1, "06/06/2026") == 1);
    assert(strcmp(internacoes[0].status, "ALTA") == 0);
    assert(strcmp(internacoes[0].dataAlta, "06/06/2026") == 0);
    assert(leitos[0].ocupado == 0);
    assert(leitos[0].pacienteId == 0);
    assert(alas[0].leitosOcupados == 0);
    assert(darAltaInternacao(1, "07/06/2026") == 0);

    return 0;
}
