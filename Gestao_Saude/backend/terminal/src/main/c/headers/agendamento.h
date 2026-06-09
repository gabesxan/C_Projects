#ifndef AGENDAMENTO_H
#define AGENDAMENTO_H

#include "hospital.h"

void menuAgendamentos(void);
int buscarAgenda(int medicoId, char data[], char horario[]);
int medicoOcupado(int medicoId, char data[], char horario[]);
int cancelarAgendamento(int id);
int concluirAgendamento(int id);
int criarAgendamentoTriagem(int pacienteId, char data[], char horario[], int *agendamentoId, int *medicoId);
int trocaHorario(int pacienteNovo, int pacienteAtual);
int agendarMedico(int medicoId, int pacienteId, char data[], char horario[]);
const char *obterEspecialidade(int tipoTriagem);
int buscarMedicoRegiao(const char especialidade[], int regiaoAdministrativa, char data[], char horario[]);
int buscarMedico(const char especialidade[], char data[], char horario[]);
int agendarTriagem(int pacienteId, char data[], char horario[]);

#endif
