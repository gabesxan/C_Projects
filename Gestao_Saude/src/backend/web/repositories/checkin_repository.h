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

/* Numero maximo de rechamadas antes de marcar o paciente como FALTOU. */
#define CHECKIN_MAX_RECHAMADAS 3

/* Marca um check-in AGUARDANDO como EM_ATENDIMENTO (chamar senha). 1/0. */
int checkin_repo_chamar(int id);

/* Um medico assume o proximo da fila: marca o check-in AGUARDANDO como
 * EM_ATENDIMENTO e grava o medico responsavel. So altera quem esta AGUARDANDO.
 * Retorna 1 se assumiu; 0 caso contrario. */
int checkin_repo_assumir(int id, int medico_id);

/* Rechama a senha de um check-in EM_ATENDIMENTO (paciente ainda nao veio):
 * incrementa o contador de rechamadas. Ao atingir CHECKIN_MAX_RECHAMADAS, o
 * check-in e automaticamente marcado como FALTOU. Retorna:
 *   2 = limite atingido (marcado FALTOU); 1 = rechamado; 0 = nao aplicavel. */
int checkin_repo_rechamar(int id);

/* Lista (JSON) a fila de CONSULTA aguardando atendimento (o que o medico pode
 * assumir), ordenada por prioridade e chegada. Retorna 1/0. */
int checkin_repo_fila_consulta_json(char *buffer, int tamanho);

/* Lista (JSON) os check-ins EM_ATENDIMENTO assumidos por um medico (quem ele
 * deve atender agora). Retorna 1/0. */
int checkin_repo_atendimentos_medico_json(int medico_id, char *buffer, int tamanho);

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

/* Tempo maximo de espera (minutos) tolerado para uma classificacao de risco
 * (Protocolo de Manchester). -1 quando nao ha SLA por risco (sem triagem). */
int checkin_sla_minutos(const char *classificacao);

#endif
