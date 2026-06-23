#ifndef ALA_REPOSITORY_H
#define ALA_REPOSITORY_H

/*
 * Repository de alas do backend web do SIGEH-DF.
 * Usa database.h (SQLite) com prepared statements. Escrita: 1 = ok, 0 = falha.
 */

int ala_repo_criar(const char *nome, int tipo, int total_leitos);
int ala_repo_listar_json(char *buffer, int tamanho);
int ala_repo_desativar(int id);
int ala_repo_contar_ativos(void);

#endif
