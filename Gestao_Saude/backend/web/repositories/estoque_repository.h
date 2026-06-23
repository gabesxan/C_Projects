#ifndef ESTOQUE_REPOSITORY_H
#define ESTOQUE_REPOSITORY_H

/*
 * Repository de estoque da farmacia (SIGEH-DF): lotes (estoque_itens) e a
 * trilha imutavel de movimentacoes. Camada database.h (SQLite). Escrita:
 * 1 = sucesso, 0 = falha; listas via *_json(buffer, tamanho). A dispensacao
 * (que cruza estoque + financeiro) fica no farmacia_service.
 */

/* Registra uma ENTRADA de lote: soma na quantidade de um item de mesmo
 * medicamento/lote/validade se ja existir, senao cria o item. Grava a
 * movimentacao ENTRADA. quantidade > 0. Retorna 1 em sucesso, 0 em falha. */
int estoque_entrada(int medicamento_id, const char *lote, const char *validade,
                    int quantidade, const char *localizacao,
                    int usuario_id, const char *usuario_login);

/* Baixa 'quantidade' do estoque do medicamento, consumindo os lotes por
 * validade mais proxima primeiro (FIFO), tudo numa transacao. Grava a
 * movimentacao do tipo informado (SAIDA/AJUSTE). Retorna 1 se baixou tudo;
 * 0 se saldo insuficiente ou em erro (sem alterar o estoque). */
int estoque_baixar(int medicamento_id, int quantidade, const char *tipo,
                   const char *motivo, int usuario_id, const char *usuario_login);

/* Saldo total (soma das quantidades dos lotes) do medicamento; -1 em erro. */
int estoque_saldo(int medicamento_id);

/* Lista (JSON) os lotes do medicamento com saldo > 0, por validade (FIFO). */
int estoque_itens_listar_json(int medicamento_id, char *buffer, int tamanho);

/* Lista (JSON) as movimentacoes do medicamento, mais recentes primeiro. */
int movimentacao_listar_json(int medicamento_id, char *buffer, int tamanho);

/* Relatorio agregado da farmacia: saldos consolidados por medicamento, itens
 * com estoque baixo e lotes com validade nos proximos 30 dias. */
int estoque_alertas_json(char *buffer, int tamanho);

#endif
