#ifndef LEITO_H
#define LEITO_H

#include "hospital.h"

void menuLeitos(void);
int cadastrarLeito(int alaId, int numero);
int excluirLeito(int id);
int salvarLeitoNoBanco(const Leito *leito);
int carregarLeitosDoBanco(Leito destino[], int maximo);

#endif
