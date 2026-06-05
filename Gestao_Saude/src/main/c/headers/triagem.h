#ifndef TRIAGEM_H
#define TRIAGEM_H

#include "hospital.h"

void menuTriagem(void);
int calcularPontuacaoTriagem(int febre, int faltaAr, int dorIntensa, int pressaoAlta, int idadePaciente);
void classificarTriagem(int pontuacao, char classificacao[]);

#endif
