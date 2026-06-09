#ifndef MEDICO_H
#define MEDICO_H

#include "hospital.h"

void menuMedicos(void);
int cadastrarMedico(const char nome[], const char crm[], const char especialidade[], int regiaoAdministrativa);
int excluirMedico(int id);

#endif
