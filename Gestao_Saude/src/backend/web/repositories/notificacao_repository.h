#ifndef NOTIFICACAO_REPOSITORY_H
#define NOTIFICACAO_REPOSITORY_H

/*
 * Repository de notificacoes in-app do SIGEH-DF.
 *
 * Modelo: uma linha por destinatario. Notificacoes por papel sao materializadas
 * (fan-out) em uma linha para cada usuario ativo daquele papel, de modo que o
 * estado de leitura (lida) seja por usuario. Listar/contar/marcar sempre operam
 * sobre as linhas do proprio usuario autenticado.
 */

/* Cria uma notificacao para um usuario especifico. Retorna 1/0. */
int notificacao_criar_para_usuario(int usuario_id, const char *titulo,
                                   const char *mensagem, const char *tipo,
                                   const char *entidade, int entidade_id);

/* Cria uma notificacao para todos os usuarios ativos de um papel (fan-out).
 * Retorna 1 se inseriu ao menos uma linha (ou se nao havia destinatarios);
 * 0 em erro real. */
int notificacao_criar_para_papel(const char *papel, const char *titulo,
                                 const char *mensagem, const char *tipo,
                                 const char *entidade, int entidade_id);

/* Lista (JSON) as notificacoes do usuario, mais recentes primeiro (limite 100). */
int notificacao_listar_por_usuario_json(int usuario_id, char *buffer, int tamanho);

/* Conta as notificacoes nao lidas do usuario. -1 em erro. */
int notificacao_contar_nao_lidas(int usuario_id);

/* Marca uma notificacao do usuario como lida. Retorna 1 se marcou. */
int notificacao_marcar_lida(int id, int usuario_id);

/* Marca todas as notificacoes do usuario como lidas. Retorna 1/0. */
int notificacao_marcar_todas_lidas(int usuario_id);

#endif
