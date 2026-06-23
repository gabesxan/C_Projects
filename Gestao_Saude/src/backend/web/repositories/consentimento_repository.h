#ifndef CONSENTIMENTO_REPOSITORY_H
#define CONSENTIMENTO_REPOSITORY_H

/*
 * Repository de consentimentos LGPD (SIGEH-DF): registro do consentimento de um
 * paciente para uma finalidade especifica. O historico e IMUTAVEL: a revogacao
 * nao apaga a linha; apenas muda o status para REVOGADO, preenche
 * revogado_em/motivo_revogacao e desliga o flag 'ativo'. Estado logico via
 * status/ativo, nunca delete fisico.
 *
 * Status possiveis: CONCEDIDO, REVOGADO.
 */

/* Cria um consentimento (status CONCEDIDO, concedido_em = agora, ativo = 1).
 * Preenche '*novo_id' com o id gerado quando informado. Retorna 1/0. */
int consentimento_criar(int paciente_id, const char *finalidade,
                        const char *versao_termo, int *novo_id);

/* Lista (JSON) os consentimentos de um paciente, do mais novo ao mais antigo,
 * incluindo os revogados (historico completo). Retorna 1/0. */
int consentimento_listar_por_paciente_json(int paciente_id, char *buffer,
                                           int tamanho);

/* Busca um consentimento por id, devolvendo o paciente dono e o status atual
 * (para checagens de autorizacao e de transicao). Retorna 1 se encontrou. */
int consentimento_buscar(int id, int *paciente_id, char *status, int tstatus);

/* Revoga um consentimento CONCEDIDO: muda o status para REVOGADO, grava
 * revogado_em (agora) e o motivo, e desliga 'ativo'. Preserva a linha original.
 * Retorna 1 se uma linha CONCEDIDA foi revogada; 0 se nao existe ou ja estava
 * revogada (transicao invalida). */
int consentimento_revogar(int id, const char *motivo);

#endif
