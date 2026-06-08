#ifndef RELATORIO_H
#define RELATORIO_H

#include "hospital.h"

void menuRelatorios(void);
int contarPacientesAtivos(void);
int contarMedicosAtivos(void);
int contarProntuariosAtivos(void);
int contarLeitosOcupados(void);
int contarLivres(void);
float taxaAla(int alaId);
int contarTriagens(const char classificacao[]);
int contarMedRegiao(int regiaoAdministrativa);
void relMedRegiao(void);
int contarPacRegiao(int regiaoAdministrativa);
void relPacRegiao(void);
int contarEsp(const char especialidade[]);
void espDemandada(char resultado[]);
int contarCasosGravesRegiao(int regiaoAdministrativa);
int regiaoMaisCasosGraves(void);

#endif
