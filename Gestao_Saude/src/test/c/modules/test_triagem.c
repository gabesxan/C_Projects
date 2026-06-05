#include <assert.h>
#include <string.h>
#include "triagem.h"

Paciente pacientes[MAX_PACIENTES];
Triagem triagens[MAX_TRIAGENS];

int totalPacientes = 0;
int totalTriagens = 0;

int main(void)
{
    char classificacao[30];

    assert(calcularPontuacaoTriagem(0, 0, 0, 0, 30) == 0);
    classificarTriagem(0, classificacao);
    assert(strcmp(classificacao, "Orientacao basica") == 0);

    assert(calcularPontuacaoTriagem(1, 0, 0, 0, 30) == 2);
    classificarTriagem(2, classificacao);
    assert(strcmp(classificacao, "Comum") == 0);

    assert(calcularPontuacaoTriagem(1, 0, 1, 0, 30) == 6);
    classificarTriagem(6, classificacao);
    assert(strcmp(classificacao, "Muito prioritario") == 0);

    assert(calcularPontuacaoTriagem(1, 1, 1, 1, 65) == 15);
    classificarTriagem(15, classificacao);
    assert(strcmp(classificacao, "Emergencia") == 0);

    assert(calcularPontuacaoTriagem(0, 0, 0, 0, 4) == 2);
    classificarTriagem(3, classificacao);
    assert(strcmp(classificacao, "Prioritario") == 0);

    return 0;
}
