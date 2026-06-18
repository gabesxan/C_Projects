#ifndef SESSAO_REPOSITORY_H
#define SESSAO_REPOSITORY_H

/*
 * Repository de sessoes do SIGEH-DF.
 * Cada login bem-sucedido cria um token opaco (aleatorio) com validade. O
 * cliente passa a se autenticar por 'Authorization: Bearer <token>', de modo
 * que a senha nao trafega a cada requisicao.
 */

/* Cria uma sessao para o usuario, com validade de 'validade_horas' horas.
 * Escreve o token (hex) em token_out (precisa de >= 65 bytes). Antes, remove
 * as sessoes ja expiradas. Retorna 1 em sucesso, 0 caso contrario. */
int sessao_repo_criar(int usuario_id, int validade_horas,
                      char *token_out, int token_tam);

/* Valida um token ainda vigente cujo usuario continua ativo. Em sucesso
 * preenche papel, vinculos, id e login (ponteiros podem ser NULL) e retorna 1.
 * Token inexistente/expirado ou usuario inativo -> 0. */
int sessao_repo_validar(const char *token,
                        char *papel, int papel_tam,
                        int *paciente_id, int *medico_id, int *usuario_id,
                        char *login_out, int login_tam);

/* Encerra (remove) a sessao do token informado. Retorna 1 se removeu. */
int sessao_repo_remover(const char *token);

#endif
