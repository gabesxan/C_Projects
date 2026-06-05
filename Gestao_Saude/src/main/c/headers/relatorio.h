#ifndef RELATORIO_H
#define RELATORIO_H

#include "hospital.h"

void menuRelatorios(void);
int contarLeitosOcupados(void);
int contarLeitosLivres(void);
float calcularTaxaOcupacaoAla(int alaId);
int contarTriagensPorClassificacao(const char classificacao[]);

#endif
