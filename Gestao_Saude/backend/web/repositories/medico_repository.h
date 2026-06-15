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

/* Lista (JSON) os medicos ativos de uma especialidade numa regiao. */
int medico_repo_listar_por_especialidade_regiao_json(const char *especialidade,
                                                     int regiao,
                                                     char *buffer, int tamanho);

/* Preenche 'ids' com os ids dos medicos ativos da especialidade/regiao
 * (ate 'max'). Retorna a quantidade encontrada. */
int medico_repo_ids_por_especialidade_regiao(const char *especialidade,
                                             int regiao, int *ids, int max);

/* Escreve (JSON) a distribuicao de medicos ativos por especialidade, no
 * formato [{"especialidade":"Cardiologia","total":2},...]. 1 = ok, 0 = erro. */
int medico_repo_distribuicao_por_especialidade_json(char *buffer, int tamanho);

/* Copia para 'destino' a especialidade do medico ativo de id informado.
 * Retorna 1 se encontrou (e coube), 0 caso contrario. */
int medico_repo_especialidade(int id, char *destino, int tamanho);

#endif
