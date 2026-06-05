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

    classificarTriagem(0, classificacao);
    assert(strcmp(classificacao, "Orientacao basica") == 0);

    classificarTriagem(2, classificacao);
    assert(strcmp(classificacao, "Comum") == 0);

    classificarTriagem(4, classificacao);
    assert(strcmp(classificacao, "Prioritario") == 0);

    classificarTriagem(6, classificacao);
    assert(strcmp(classificacao, "Muito prioritario") == 0);

    classificarTriagem(10, classificacao);
    assert(strcmp(classificacao, "Emergencia") == 0);

    totalTriagens = 1;
    triagens[0].id = 1;
    triagens[0].pacienteId = 1;
    triagens[0].tipoTriagem = TRIAGEM_ORTOPEDIA;
    triagens[0].pontuacao = 9;
    strcpy(triagens[0].classificacao, "Emergencia");
    triagens[0].ativo = 1;

    assert(triagens[0].ativo == 1);
    assert(triagens[0].tipoTriagem == TRIAGEM_ORTOPEDIA);
    assert(excluirTriagem(1) == 1);
    assert(triagens[0].ativo == 0);
    assert(excluirTriagem(1) == 0);
    assert(excluirTriagem(99) == 0);

    return 0;
}
