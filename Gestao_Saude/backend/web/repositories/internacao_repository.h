#ifndef INTERNACAO_REPOSITORY_H
#define INTERNACAO_REPOSITORY_H

/*
 * Repository de internacoes do backend web do SIGEH-DF.
 * A tabela usa 'status' (nao 'ativo'); dar alta = status 'ALTA' + data_alta.
 * Usa database.h (SQLite) com prepared statements. Escrita: 1 = ok, 0 = falha.
 */

int internacao_repo_criar(int paciente_id, int ala_id, int leito_id,
                          const char *data_entrada);
int internacao_repo_listar_json(char *buffer, int tamanho);
int internacao_repo_dar_alta(int id, const char *data_alta);
int internacao_repo_contar_ativos(void);

#endif
