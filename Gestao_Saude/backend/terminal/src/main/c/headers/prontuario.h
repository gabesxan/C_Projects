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
int salvarProntuarioNoBanco(const Prontuario *prontuario);
int carregarProntuariosDoBanco(Prontuario destino[], int maximo);
int copiarProntuarios(Prontuario destino[], int maximo);
int copiarProntuariosPorPaciente(int pacienteId, Prontuario destino[], int maximo);
int copiarProntuariosPorMedico(int medicoId, Prontuario destino[], int maximo);
int copiarProntuariosPorEspecialidade(const char especialidade[], Prontuario destino[], int maximo);
void listarProntuarioPorPaciente(int pacienteId);
void listarProntuarioPorMedico(int medicoId);
void listarProntuarioPorEspecialidade(const char especialidade[]);
int contarProntuariosPorPaciente(int pacienteId);
int contarProntuariosPorMedico(int medicoId);
int contarProntuariosPorEspecialidade(const char especialidade[]);
#endif
