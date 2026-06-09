#ifndef PRONTUARIO_H
#define PRONTUARIO_H

#include "hospital.h"

void menuProntuarios(void);
int criarProntuarioAutomatico(int pacienteId, int medicoId, const char data[]);
int complementarProntuario(int prontuarioId, const char observacoes[],
                           const char diagnostico[], const char conduta[],
                           int alertaImportante);
int registrarProntuario(int pacienteId, int medicoId, const char data[],
                        const char observacoes[], const char diagnostico[],
                        const char conduta[], int alertaImportante);
void listarProntuarioPorPaciente(int pacienteId);
void listarProntuarioPorMedico(int medicoId);
void listarProntuarioPorEspecialidade(const char especialidade[]);
int contarProntuariosPorPaciente(int pacienteId);
int contarProntuariosPorMedico(int medicoId);
int contarProntuariosPorEspecialidade(const char especialidade[]);
#endif
