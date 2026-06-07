#ifndef PRONTUARIO_H
#define PRONTUARIO_H

#include "hospital.h"

void menuProntuarios(void);
int registrarProntuario(int pacienteId, int medicoId, const char data[],
                        const char observacoes[], const char diagnostico[],
                        const char conduta[], int alertaImportante);
void listarProntuarioPorPaciente(int pacienteId);
void listarProntuarioPorMedico(int medicoId);
void listarProntuarioPorEspecialidade(const char especialidade[]);

#endif
