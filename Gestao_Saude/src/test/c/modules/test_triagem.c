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

    assert(nivelPrioridade("Emergencia") == 5);
    assert(nivelPrioridade("Muito prioritario") == 4);
    assert(nivelPrioridade("Prioritario") == 3);
    assert(nivelPrioridade("Comum") == 2);
    assert(nivelPrioridade("Orientacao basica") == 1);
    assert(nivelPrioridade("Invalida") == 0);

    assert(ehUrgente("Emergencia") == 1);
    assert(ehUrgente("Muito prioritario") == 1);
    assert(ehUrgente("Prioritario") == 0);
    assert(ehUrgente("Comum") == 0);
    assert(ehUrgente("Orientacao basica") == 0);

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

    totalTriagens = 0;
    triagens[0].id = 1;
    triagens[0].pacienteId = 2;
    triagens[0].tipoTriagem = TRIAGEM_GERAL;
    triagens[0].pontuacao = 2;
    strcpy(triagens[0].classificacao, "Comum");
    triagens[0].ativo = 1;

    triagens[1].id = 2;
    triagens[1].pacienteId = 2;
    triagens[1].tipoTriagem = TRIAGEM_CARDIOLOGIA;
    triagens[1].pontuacao = 9;
    strcpy(triagens[1].classificacao, "Emergencia");
    triagens[1].ativo = 1;

    triagens[2].id = 3;
    triagens[2].pacienteId = 3;
    triagens[2].tipoTriagem = TRIAGEM_GERAL;
    triagens[2].pontuacao = 1;
    strcpy(triagens[2].classificacao, "Comum");
    triagens[2].ativo = 0;
    totalTriagens = 3;

    assert(triagemAtual(2) == 1);
    assert(triagemAtual(3) == -1);
    assert(triagemAtual(99) == -1);

    return 0;
}
