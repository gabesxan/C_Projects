#ifndef INTERNACAO_REPOSITORY_H
#define INTERNACAO_REPOSITORY_H

/*
 * Repository de internacoes do backend web do SIGEH-DF.
 * A tabela usa 'status' (nao 'ativo'); dar alta = status 'ALTA' + data_alta.
 * Usa database.h (SQLite) com prepared statements. Escrita: 1 = ok, 0 = falha.
 */

/* Interna o paciente: exige leito DISPONIVEL (que passa a OCUPADO) e registra
 * o responsavel. 1 = ok, 0 = falha (inclui leito indisponivel). */
int internacao_repo_criar(int paciente_id, int ala_id, int leito_id,
                          const char *data_entrada, const char *responsavel);
int internacao_repo_listar_json(char *buffer, int tamanho);

/* Da alta: exige resumo clinico e diagnostico final; libera o leito para
 * HIGIENIZACAO. 1 = ok, 0 = falha. */
int internacao_repo_dar_alta(int id, const char *data_alta, const char *resumo,
                             const char *diagnostico, const char *orientacoes);

/* Transfere a internacao para outro leito DISPONIVEL, liberando o de origem e
 * registrando a transferencia. 1 = ok, 0 = falha. */
int internacao_repo_transferir(int id, int novo_leito_id, const char *data,
                               const char *responsavel);
int internacao_repo_contar_ativos(void);

/* Escreve (JSON) a distribuicao de internacoes por status, no formato
 * [{"status":"INTERNADO","total":3},...]. 1 = ok, 0 = erro. */
int internacao_repo_distribuicao_por_status_json(char *buffer, int tamanho);

#endif
