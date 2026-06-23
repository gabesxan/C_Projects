#ifndef ANALITO_REPOSITORY_H
#define ANALITO_REPOSITORY_H

/*
 * Repository do laboratorio do SIGEH-DF: catalogo de analitos (cada medida de um
 * exame, com unidade e faixa de referencia) e composicao dos paineis (quais
 * analitos formam cada tipo de exame). Base para o resultado estruturado por
 * analito nas etapas seguintes. Usa database.h (SQLite). Escrita: 1 = ok,
 * 0 = falha; listas via *_json(buffer, tamanho).
 */

/* Cria um analito ativo. Exige codigo e nome; recusa codigo ja usado por outro
 * analito ativo. Retorna 1 em sucesso, 0 em falha/duplicado. */
int analito_criar(const char *codigo, const char *nome, const char *unidade,
                  double ref_min, double ref_max, const char *metodo);

/* Lista (JSON) os analitos ativos ordenados por codigo. 1/0. */
int analito_listar_json(char *buffer, int tamanho);

/* Desativa um analito (soft delete) e o remove de todos os paineis. 1/0. */
int analito_desativar(int id);

/* Conta os analitos ativos. Retorna o total, ou -1 em erro. */
int analito_contar_ativos(void);

/* Vincula um analito ativo a um tipo de exame (painel), na ordem dada. Recusa
 * vinculo duplicado ou analito inexistente/inativo. 1 = ok, 0 = falha. */
int painel_adicionar_analito(int tipo_exame, int analito_id, int ordem);

/* Remove um analito de um painel. 1 = ok, 0 = vinculo inexistente. */
int painel_remover_analito(int tipo_exame, int analito_id);

/* Lista (JSON) os analitos de um painel (tipo_exame), na ordem do painel,
 * com unidade e faixa de referencia. 1/0. */
int painel_listar_json(int tipo_exame, char *buffer, int tamanho);

#endif
