#ifndef LEITO_REPOSITORY_H
#define LEITO_REPOSITORY_H

/*
 * Repository de leitos do backend web do SIGEH-DF.
 * Usa database.h (SQLite) com prepared statements. Escrita: 1 = ok, 0 = falha.
 */

int leito_repo_criar(int ala_id, int numero);
int leito_repo_listar_json(char *buffer, int tamanho);
int leito_repo_desativar(int id);
int leito_repo_contar_ativos(void);

/* Le o status atual do leito ativo em 'destino'. 1 se encontrou, 0 senao. */
int leito_repo_status(int id, char *destino, int tamanho);

/* Ocupa um leito DISPONIVEL com o paciente (usado pela internacao). 1/0. */
int leito_repo_ocupar(int leito_id, int paciente_id);

/* Libera o leito para HIGIENIZACAO (apos alta/transferencia). 1/0. */
int leito_repo_liberar(int leito_id, const char *responsavel);

/* Altera manualmente o status do leito (exceto OCUPADO) e registra historico.
 * Status validos: DISPONIVEL, HIGIENIZACAO, MANUTENCAO, BLOQUEADO. 1/0. */
int leito_repo_registrar_status(int id, const char *novo_status,
                                const char *responsavel);

/* Escreve (JSON) a ocupacao: total/ocupados/disponiveis/... e taxaOcupacao. */
int leito_repo_ocupacao_json(char *buffer, int tamanho);

#endif
