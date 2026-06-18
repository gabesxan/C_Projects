#ifndef TRIAGEM_REPOSITORY_H
#define TRIAGEM_REPOSITORY_H

/*
 * Repository de triagens do backend web do SIGEH-DF.
 * Usa database.h (SQLite) com prepared statements. Escrita: 1 = ok, 0 = falha.
 */

int triagem_repo_criar(int paciente_id, int tipo_triagem, int pontuacao,
                       const char *classificacao);

/* Como triagem_repo_criar, porem registra tambem os itens do checklist, a
 * queixa principal e os sinais vitais (texto livre; podem ser ""). A pontuacao
 * recebida e o nivel derivado do checklist. 1 = ok, 0 = falha. */
int triagem_repo_criar_completa(int paciente_id, int tipo_triagem, int pontuacao,
                                const char *classificacao, const char *itens,
                                const char *queixa, const char *pressao,
                                const char *temperatura, const char *freq_cardiaca,
                                const char *saturacao);
int triagem_repo_listar_json(char *buffer, int tamanho);

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
