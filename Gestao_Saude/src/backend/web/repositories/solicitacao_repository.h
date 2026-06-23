#ifndef SOLICITACAO_REPOSITORY_H
#define SOLICITACAO_REPOSITORY_H

/*
 * Solicitacoes abertas pelo paciente no portal.
 * O paciente solicita ajuda/consulta comum, mas nao cria triagem nem decide
 * conduta clinica. A equipe usa esses registros como fila administrativa.
 */

int solicitacao_repo_criar(int paciente_id, const char *tipo,
                           const char *mensagem, int *id_out);

int solicitacao_repo_listar_por_paciente_json(int paciente_id, char *buffer,
                                              int tamanho);

int solicitacao_repo_listar_json(char *buffer, int tamanho);

int solicitacao_repo_contar_abertas(void);

#endif
