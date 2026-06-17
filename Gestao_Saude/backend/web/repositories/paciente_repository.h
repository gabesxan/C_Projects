#ifndef PACIENTE_REPOSITORY_H
#define PACIENTE_REPOSITORY_H

/*
 * Repository de pacientes do backend web do SIGEH-DF.
 * Usa a camada database.h (SQLite) com prepared statements.
 * Escrita: retorna 1 = sucesso, 0 = falha.
 */

/* Cria um paciente. Regras: nome/documento/telefone obrigatorios; nascimento
 * (YYYY-MM-DD) valido e nao futuro; menor de idade exige responsavel; CPF
 * unico entre pacientes ativos (tipo_documento "CPF"). A idade e derivada do
 * nascimento, nunca armazenada. Retorna 1 em sucesso, 0 em falha. */
int paciente_repo_criar(const char *nome,
                        const char *nascimento,
                        const char *documento,
                        const char *tipo_documento,
                        const char *telefone,
                        const char *sexo,
                        int regiao_administrativa,
                        const char *responsavel,
                        const char *alergias);

/* Como paciente_repo_criar, mas devolve em *novo_id o id gerado (usado no
 * fluxo de triagem para vincular o usuario PACIENTE). 1 = ok, 0 = falha. */
int paciente_repo_criar_retornando_id(const char *nome,
                                      const char *nascimento,
                                      const char *documento,
                                      const char *tipo_documento,
                                      const char *telefone,
                                      const char *sexo,
                                      int regiao_administrativa,
                                      const char *responsavel,
                                      const char *alergias,
                                      int *novo_id);

int paciente_repo_listar_json(char *buffer, int tamanho);

/* Busca (JSON) pacientes ativos cujo nome ou documento contem 'termo'. */
int paciente_repo_buscar_json(const char *termo, char *buffer, int tamanho);

/* Escreve (JSON) um unico paciente por id (ativo ou nao, para o historico).
 * Retorna 1 se encontrou, 0 caso contrario. */
int paciente_repo_detalhe_json(int id, char *buffer, int tamanho);

/* Lista (JSON) os pacientes ativos que tem agendamento com o medico. */
int paciente_repo_listar_por_medico_json(int medico_id, char *buffer, int tamanho);

int paciente_repo_desativar(int id);

int paciente_repo_contar_ativos(void);

/* Conta pacientes ativos com agendamento nao-cancelado com o medico.
 * Retorna o total, ou -1 em erro. */
int paciente_repo_contar_por_medico(int medico_id);

/* Regiao administrativa do paciente ativo, ou -1 se nao existir. */
int paciente_repo_regiao(int id);

/* Escreve (JSON) a distribuicao de pacientes ativos por regiao administrativa,
 * no formato [{"regiao":1,"total":3},...]. Retorna 1 em sucesso, 0 em erro. */
int paciente_repo_distribuicao_por_regiao_json(char *buffer, int tamanho);

#endif
