#ifndef AGENDAMENTO_H
#define AGENDAMENTO_H

#include "hospital.h"

void menuAgendamentos(void);
int conflitoMedico(int medicoId, char data[], char horario[]);
int cancelarAgendamento(int id);
int concluirAgendamento(int id);

const char *especialidadeTriagem(int tipoTriagem);
int buscarMedicoRegiao(const char especialidade[], int regiaoAdministrativa, char data[], char horario[]);
int buscarMedico(const char especialidade[], char data[], char horario[]);
int agendarTriagem(int pacienteId, char data[], char horario[]);

#endif
