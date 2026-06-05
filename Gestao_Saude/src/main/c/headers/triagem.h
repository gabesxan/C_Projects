#ifndef TRIAGEM_H
#define TRIAGEM_H

#include "hospital.h"

void menuTriagem(void);
void classificarTriagem(int pontuacao, char classificacao[]);
int excluirTriagem(int id);

int selecionarTipoTriagem(void);
int submenuTriagemGeral(int idadePaciente);
int submenuTriagemOrtopedia(int idadePaciente);
int submenuTriagemCardiologia(int idadePaciente);
int submenuTriagemPneumologia(int idadePaciente);
int submenuTriagemPediatria(int idadePaciente);
void exibirNomeTipoTriagem(int tipoTriagem);

#endif
