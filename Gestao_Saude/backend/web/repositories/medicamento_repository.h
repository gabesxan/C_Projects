#ifndef MEDICAMENTO_REPOSITORY_H
#define MEDICAMENTO_REPOSITORY_H

/*
 * Repository do catalogo de medicamentos da farmacia (SIGEH-DF).
 * Camada database.h (SQLite) com prepared statements. Escrita: 1 = sucesso,
 * 0 = falha; listas via *_json(buffer, tamanho). O estoque fisico (lotes) e as
 * movimentacoes ficam em repositories proprios (sub-etapas 5b/5c).
 */

/* Cadastra um medicamento ativo. 'nome' e obrigatorio; 'estoque_minimo' e
 * 'preco_centavos' < 0 sao tratados como 0. Retorna 1 em sucesso, 0 em falha. */
int medicamento_criar(const char *nome, const char *apresentacao,
                      const char *unidade, int estoque_minimo,
                      int preco_centavos);

/* Lista (JSON) os medicamentos ativos, ordenados por nome. 1 = ok, 0 = erro. */
int medicamento_listar_json(char *buffer, int tamanho);

/* Desativa (soft delete) o medicamento ativo. 1 se desativou, 0 caso contrario. */
int medicamento_desativar(int id);

/* Quantidade de medicamentos ativos (-1 em erro). */
int medicamento_contar_ativos(void);

/* 1 se existe um medicamento ativo com o id informado; 0 caso contrario. */
int medicamento_ativo(int id);

/* Preco unitario (centavos) do medicamento ativo; -1 se inexistente/inativo. */
int medicamento_preco_centavos(int id);

#endif
