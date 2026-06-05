#include <assert.h>
#include <string.h>
#include "ala.h"

Ala alas[MAX_ALAS];
int totalAlas = 0;

int main(void)
{
    assert(cadastrarAla("Clinica Medica", "Internacao", 20) == 1);
    assert(totalAlas == 1);
    assert(alas[0].id == 1);
    assert(strcmp(alas[0].nome, "Clinica Medica") == 0);
    assert(strcmp(alas[0].tipo, "Internacao") == 0);
    assert(alas[0].totalLeitos == 20);
    assert(alas[0].leitosOcupados == 0);
    assert(alas[0].ativo == 1);

    assert(excluirAla(1) == 1);
    assert(alas[0].ativo == 0);
    assert(excluirAla(1) == 0);
    assert(excluirAla(99) == 0);

    return 0;
}
