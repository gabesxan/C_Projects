#ifndef TRIAGEM_REPOSITORY_H
#define TRIAGEM_REPOSITORY_H

/*
 * Repository de triagens do backend web do SIGEH-DF.
 * Usa database.h (SQLite) com prepared statements. Escrita: 1 = ok, 0 = falha.
 */

int triagem_repo_criar(int paciente_id, int tipo_triagem, int pontuacao,
                       const char *classificacao);

int triagem_repo_criar_clinica(int paciente_id, int profissional_id,
                               int especialidade_principal_id, int pontuacao,
                               const char *classificacao, const char *itens,
                               const char *queixa, const char *observacoes,
                               const char *pressao, const char *temperatura,
                               const char *freq_cardiaca,
                               const char *saturacao);

/* Como triagem_repo_criar, porem registra tambem os itens do checklist, a
 * queixa principal e os sinais vitais (texto livre; podem ser ""). A pontuacao
 * recebida e o nivel derivado do checklist. 1 = ok, 0 = falha. */
int triagem_repo_criar_completa(int paciente_id, int tipo_triagem, int pontuacao,
                                const char *classificacao, const char *itens,
                                const char *queixa, const char *pressao,
                                const char *temperatura, const char *freq_cardiaca,
                                const char *saturacao);
/* Reclassifica uma triagem ativa (novo risco/nivel/itens); exige justificativa.
 * 1 = ok, 0 = falha. */
int triagem_repo_reclassificar(int id, const char *classificacao, int nivel,
                               const char *itens, const char *justificativa);

int triagem_repo_listar_json(char *buffer, int tamanho);
int triagem_repo_detalhar_json(int id, char *buffer, int tamanho);
int triagem_repo_paciente_id(int id, int *paciente_id);
int triagem_repo_profissional_id(int id, int *profissional_id);

int triagem_repo_especialidades_json(char *buffer, int tamanho);
int triagem_repo_problemas_por_especialidade_json(int especialidade_id,
                                                  char *buffer, int tamanho);
int triagem_repo_adicionar_problema(int triagem_id, int problema_id,
                                    int principal, const char *observacao);
int triagem_repo_remover_problema(int triagem_id, int problema_id);
int triagem_repo_atualizar_resultado(int id, int especialidade_id, int prioridade,
                                     const char *classificacao);

/* Escreve (JSON) a distribuicao de triagens ativas por classificacao, no
 * formato [{"classificacao":"Vermelho","total":2},...]. 1 = ok, 0 = erro. */
int triagem_repo_distribuicao_por_classificacao_json(char *buffer, int tamanho);

/* Lista (JSON) as triagens ativas cujo tipo_triagem esta no vetor 'tipos'
 * (tamanho 'n'). Com n <= 0 devolve uma lista vazia. 1 = ok, 0 = erro. */
int triagem_repo_listar_por_tipos_json(const int *tipos, int n,
                                       char *buffer, int tamanho);

int triagem_repo_desativar(int id);
int triagem_repo_contar_ativos(void);

/* Conta triagens ativas com a classificacao informada. -1 em erro. */
int triagem_repo_contar_por_classificacao(const char *classificacao);

/* Carrega a triagem ativa mais recente do paciente nos ponteiros de saida
 * (qualquer um pode ser NULL). Retorna 1 se encontrou, 0 caso contrario. */
int triagem_repo_ultima_por_paciente(int paciente_id, int *tipo_triagem,
                                     int *pontuacao, char *classificacao,
                                     int classificacao_tam);

#endif
