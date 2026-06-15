#ifndef PACIENTE_REPOSITORY_H
#define PACIENTE_REPOSITORY_H

/*
 * Repository de pacientes do backend web do SIGEH-DF.
 * Usa a camada database.h (SQLite) com prepared statements.
 * Escrita: retorna 1 = sucesso, 0 = falha.
 */

int paciente_repo_criar(const char *nome,
                        const char *cpf,
                        int idade,
                        const char *telefone,
                        const char *sexo,
                        int regiao_administrativa);

int paciente_repo_listar_json(char *buffer, int tamanho);

int paciente_repo_desativar(int id);

int paciente_repo_contar_ativos(void);

/* Regiao administrativa do paciente ativo, ou -1 se nao existir. */
int paciente_repo_regiao(int id);

#endif
