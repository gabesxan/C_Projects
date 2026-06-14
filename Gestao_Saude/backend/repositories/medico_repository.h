#ifndef MEDICO_REPOSITORY_H
#define MEDICO_REPOSITORY_H

/*
 * Repository de medicos do backend web do SIGEH-DF.
 * Usa a camada database.h (SQLite) com prepared statements.
 * Escrita: retorna 1 = sucesso, 0 = falha.
 */

int medico_repo_criar(const char *nome,
                      const char *crm,
                      const char *especialidade,
                      int regiao_administrativa);

int medico_repo_listar_json(char *buffer, int tamanho);

int medico_repo_desativar(int id);

int medico_repo_contar_ativos(void);

#endif
