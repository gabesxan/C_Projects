#ifndef TRIAGEM_REPOSITORY_H
#define TRIAGEM_REPOSITORY_H

/*
 * Repository de triagens do backend web do SIGEH-DF.
 * Usa database.h (SQLite) com prepared statements. Escrita: 1 = ok, 0 = falha.
 */

int triagem_repo_criar(int paciente_id, int tipo_triagem, int pontuacao,
                       const char *classificacao);
int triagem_repo_listar_json(char *buffer, int tamanho);
int triagem_repo_desativar(int id);
int triagem_repo_contar_ativos(void);

#endif
