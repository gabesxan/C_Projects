#ifndef ALA_H
#define ALA_H

#include "hospital.h"

void menuAlas(void);
int cadastrarAla(const char nome[], const char tipo[], int totalLeitos);
int excluirAla(int id);

#endif
