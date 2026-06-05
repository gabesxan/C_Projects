#ifndef AGENDAMENTO_H
#define AGENDAMENTO_H

#include "hospital.h"

void menuAgendamentos(void);
int verificarConflitoMedico(int medicoId, char data[], char horario[]);

#endif
