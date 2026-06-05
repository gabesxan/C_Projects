#ifndef PACIENTE_H
#define PACIENTE_H

#include "hospital.h"

void menuPacientes(void);
int cadastrarPaciente(const char nome[], const char cpf[], int idade, const char telefone[], char sexo, int regiaoAdministrativa);
int excluirPaciente(int id);
#endif
