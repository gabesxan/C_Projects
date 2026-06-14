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

#endif
