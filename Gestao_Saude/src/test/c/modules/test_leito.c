#include <assert.h>
#include <string.h>
#include "leito.h"

Ala alas[MAX_ALAS];
Leito leitos[MAX_LEITOS];

int totalAlas = 0;
int totalLeitos = 0;

static void prepararAlaAtiva(void)
{
    totalAlas = 1;

    alas[0].id = 1;
    strcpy(alas[0].nome, "Clinica Medica");
    strcpy(alas[0].tipo, "Internacao");
    alas[0].totalLeitos = 10;
    alas[0].leitosOcupados = 0;
    alas[0].ativo = 1;
}

int main(void)
{
    prepararAlaAtiva();

    assert(cadastrarLeito(1, 101) == 1);
    assert(totalLeitos == 1);
    assert(leitos[0].id == 1);
    assert(leitos[0].alaId == 1);
    assert(leitos[0].numero == 101);
    assert(leitos[0].ocupado == 0);
    assert(leitos[0].pacienteId == 0);
    assert(leitos[0].ativo == 1);

    assert(cadastrarLeito(99, 102) == 0);
    assert(totalLeitos == 1);

    alas[0].ativo = 0;
    assert(cadastrarLeito(1, 103) == 0);
    assert(totalLeitos == 1);

    alas[0].ativo = 1;
    assert(excluirLeito(1) == 1);
    assert(leitos[0].ativo == 0);
    assert(excluirLeito(1) == 0);
    assert(excluirLeito(99) == 0);

    assert(cadastrarLeito(1, 104) == 1);
    assert(totalLeitos == 2);
    leitos[1].ocupado = 1;
    leitos[1].pacienteId = 1;

    assert(excluirLeito(2) == 0);
    assert(leitos[1].ativo == 1);

    return 0;
}
