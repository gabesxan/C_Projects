#ifndef TRIAGEM_H
#define TRIAGEM_H

#include "hospital.h"

void menuTriagem(void);
void classificarTriagem(int pontuacao, char classificacao[]);
int nivelPrioridade(const char classificacao[]);
int ehUrgente(const char classificacao[]);
int triagemAtual(int pacienteId);
int excluirTriagem(int id);

int escolherTriagem(void);
int triagemGeral(int idadePaciente);
int triagemOrtopedia(int idadePaciente);
int triagemCardiologia(int idadePaciente);
int triagemPneumologia(int idadePaciente);
int triagemPediatria(int idadePaciente);
void exibirTipo(int tipoTriagem);

#endif
