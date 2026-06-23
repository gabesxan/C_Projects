#ifndef LOTE_REPOSITORY_H
#define LOTE_REPOSITORY_H

/*
 * Repository de lotes de faturamento do SIGEH-DF.
 * Um lote agrupa cobrancas CONVENIO autorizadas de um mesmo convenio para
 * envio/pagamento. Ciclo: ABERTO -> FECHADO -> PAGO. Pagar o lote quita (PAGA)
 * todas as cobrancas que ele contem. Valores em centavos.
 */

/* Cria um lote ABERTO para um convenio ativo. Retorna o id do lote (>0) em
 * sucesso, 0 em falha. */
int lote_criar(int convenio_id);

/* Adiciona uma cobranca ao lote ABERTO. A cobranca precisa ser CONVENIO do
 * mesmo convenio do lote, estar AUTORIZADA e ainda nao pertencer a lote algum.
 * 1 = ok, 0 = invalido. */
int lote_adicionar_cobranca(int lote_id, int cobranca_id);

/* Remove uma cobranca de um lote ABERTO (volta a ficar sem lote). 1/0. */
int lote_remover_cobranca(int lote_id, int cobranca_id);

/* Fecha um lote ABERTO que tenha ao menos uma cobranca. 1/0. */
int lote_fechar(int lote_id);

/* Paga um lote FECHADO: marca o lote como PAGO e todas as suas cobrancas como
 * PAGA (numa transacao). 1/0. */
int lote_pagar(int lote_id);

/* Lista (JSON) os lotes com convenio, status, quantidade e total. 1/0. */
int lote_listar_json(char *buffer, int tamanho);

/* Fatura (JSON) de um lote: cabecalho (convenio, status, datas, total,
 * quantidade) + itens (cobrancas). 1 em sucesso, 0 se o lote nao existir. */
int lote_fatura_json(int lote_id, char *buffer, int tamanho);

#endif
