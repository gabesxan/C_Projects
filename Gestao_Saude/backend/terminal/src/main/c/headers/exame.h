#ifndef EXAME_H
#define EXAME_H

#include "hospital.h"

void menuExames(void);
int solicitarExame(int pacienteId, int medicoId, int prontuarioId,
                   int tipoExame, const char dataSolicitacao[]);
int registrarResultadoExame(int exameId, const char dataResultado[],
                            const char resultado[]);
int cancelarExame(int exameId);
int excluirExame(int exameId);
int salvarExameNoBanco(const Exame *exame);
int carregarExamesDoBanco(Exame destino[], int maximo);
int copiarExames(Exame destino[], int maximo);
int copiarExamesPorPaciente(int pacienteId, Exame destino[], int maximo);
int copiarExamesPorMedico(int medicoId, Exame destino[], int maximo);
int copiarExamesPorProntuario(int prontuarioId, Exame destino[], int maximo);
int copiarExamesUrgentes(Exame destino[], int maximo);
void listarExames(void);
void listarExamesPorPaciente(int pacienteId);
void listarExamesPorMedico(int medicoId);
void listarExamesPorProntuario(int prontuarioId);
void listarExamesUrgentes(void);
int escolherTipoExame(void);
void exibirNomeTipoExame(int tipoExame);
int contarExamesPorStatus(const char status[]);
int contarExamesPorTipo(int tipoExame);
int contarExamesUrgentes(void);

#endif
