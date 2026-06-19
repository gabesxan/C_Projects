#ifndef CHECKIN_REPOSITORY_H
#define CHECKIN_REPOSITORY_H

/*
 * Repository de check-in / recepcao do SIGEH-DF.
 * Confirma a chegada do paciente, gera senha de fila e o direciona para
 * TRIAGEM ou CONSULTA. A fila e consumida por triagem/atendimento.
 */

/* Confirma a chegada: cria um check-in para o paciente no destino ('TRIAGEM'
 * ou 'CONSULTA'), gerando a senha em 'senha_out'. Retorna 1 em sucesso. */
int checkin_repo_criar(int paciente_id, const char *destino,
                       char *senha_out, int senha_tam);

/* Lista (JSON) a fila atual (nao encerrados), em ordem de chegada, com o nome
 * do paciente. Retorna 1 em sucesso. */
int checkin_repo_listar_json(char *buffer, int tamanho);

/* Marca um check-in AGUARDANDO como EM_ATENDIMENTO (chamar senha). 1/0. */
int checkin_repo_chamar(int id);

/* Rechama a senha de um check-in EM_ATENDIMENTO (paciente ainda nao veio):
 * mantem o status e incrementa o contador de rechamadas. 1/0. */
int checkin_repo_rechamar(int id);

/* Marca como FALTOU (estava aguardando/chamado e nao compareceu). 1/0. */
int checkin_repo_faltar(int id);

/* Retorna um check-in FALTOU para a fila (AGUARDANDO). 1/0. */
int checkin_repo_retornar(int id);

/* Cancela um check-in (AGUARDANDO/EM_ATENDIMENTO) com motivo obrigatorio. 1/0. */
int checkin_repo_cancelar(int id, const char *motivo);

/* Encerra um check-in (atendido). 1/0. */
int checkin_repo_encerrar(int id);

/* Conta check-ins aguardando atendimento. -1 em erro. */
int checkin_repo_contar_aguardando(void);

#endif
