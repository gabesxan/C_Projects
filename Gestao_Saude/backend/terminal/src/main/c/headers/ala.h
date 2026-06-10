#ifndef ALA_H
#define ALA_H

#include "hospital.h"

void menuAlas(void);
int cadastrarAla(const char nome[], int tipo, int totalLeitos);
int excluirAla(int id);
int contarAlasPorTipo(int tipo);
void listarAlasPorTipo(int tipo);

#endif
 
