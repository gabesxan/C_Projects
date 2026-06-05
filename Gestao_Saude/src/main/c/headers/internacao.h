#ifndef INTERNACAO_H
#define INTERNACAO_H

#include "hospital.h"

void menuInternacoes(void);
int internarPaciente(int pacienteId, int leitoId, const char dataEntrada[]);
int darAltaInternacao(int internacaoId, const char dataAlta[]);

#endif
