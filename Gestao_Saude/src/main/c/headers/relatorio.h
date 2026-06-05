#ifndef RELATORIO_H
#define RELATORIO_H

#include "hospital.h"

void menuRelatorios(void);
int contarLeitosOcupados(void);
int contarLivres(void);
float taxaAla(int alaId);
int contarTriagens(const char classificacao[]);
int contarMedRegiao(int regiaoAdministrativa);
void relMedRegiao(void);

#endif
