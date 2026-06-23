#include "lote_repository.h"
#include "financeiro_repository.h"
#include "paciente_repository.h"
#include "database.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *BANCO_TESTE = "build/test_sigeh_repository.db";
static const char *SCHEMA = "../data/schema_v3.sql";

int main(void)
{
    char json[8192];

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    assert(paciente_repo_criar("Ana", "1990-01-01", "111", "CPF", "61", "F", 1, "", "") == 1);
    assert(convenio_criar("Unimed", 100) == 1);   /* convenio 1 */
    assert(convenio_criar("Bradesco", 100) == 1); /* convenio 2 */

    /* Cobrancas: 1 e 2 sao CONVENIO conv1 (vao ao lote); 3 e de outro convenio;
     * 4 e particular; 5 fica pendente (nao autorizada). */
    assert(cobranca_criar(1, 1, "CONVENIO", "consulta", "C1", 10000, "", "", "") == 1);  /* id 1 */
    assert(cobranca_criar(1, 1, "CONVENIO", "exame", "C2", 20000, "", "", "") == 1);     /* id 2 */
    assert(cobranca_criar(1, 2, "CONVENIO", "consulta", "C3", 5000, "", "", "") == 1);   /* id 3 */
    assert(cobranca_criar(1, 0, "PARTICULAR", "consulta", "C4", 3000, "", "", "") == 1); /* id 4 */
    assert(cobranca_criar(1, 1, "CONVENIO", "exame", "C5", 7000, "", "", "") == 1);      /* id 5 */
    assert(cobranca_atualizar_status(1, "AUTORIZADA", "") == 1);
    assert(cobranca_atualizar_status(2, "AUTORIZADA", "") == 1);
    assert(cobranca_atualizar_status(3, "AUTORIZADA", "") == 1);
    assert(cobranca_atualizar_status(4, "AUTORIZADA", "") == 1);
    /* a 5 fica PENDENTE de proposito */

    /* Criacao do lote. */
    assert(lote_criar(0) == 0);
    assert(lote_criar(99) == 0); /* convenio inexistente/inativo */
    int lote = lote_criar(1);
    assert(lote > 0);

    /* Elegibilidade ao adicionar. */
    assert(lote_adicionar_cobranca(lote, 1) == 1);
    assert(lote_adicionar_cobranca(lote, 2) == 1);
    assert(lote_adicionar_cobranca(lote, 1) == 0); /* ja esta no lote */
    assert(lote_adicionar_cobranca(lote, 3) == 0); /* outro convenio */
    assert(lote_adicionar_cobranca(lote, 4) == 0); /* particular */
    assert(lote_adicionar_cobranca(lote, 5) == 0); /* nao autorizada */

    /* Fatura: total 30000, 2 itens. */
    assert(lote_fatura_json(lote, json, sizeof(json)) == 1);
    assert(strstr(json, "\"totalCentavos\":30000") != NULL);
    assert(strstr(json, "\"quantidade\":2") != NULL);
    assert(strstr(json, "\"status\":\"ABERTO\"") != NULL);

    /* Remocao volta a cobranca para fora do lote. */
    assert(lote_remover_cobranca(lote, 2) == 1);
    assert(lote_fatura_json(lote, json, sizeof(json)) == 1);
    assert(strstr(json, "\"totalCentavos\":10000") != NULL);
    assert(lote_adicionar_cobranca(lote, 2) == 1); /* readiciona */

    /* Fechar: lote vazio nao fecha; este (com 2) fecha. */
    int vazio = lote_criar(2);
    assert(vazio > 0);
    assert(lote_fechar(vazio) == 0);
    assert(lote_fechar(lote) == 1);
    assert(lote_fechar(lote) == 0); /* ja fechado */

    /* Apos fechar nao aceita mais cobrancas. */
    assert(cobranca_atualizar_status(5, "AUTORIZADA", "") == 1);
    assert(lote_adicionar_cobranca(lote, 5) == 0);

    /* Pagar: FECHADO -> PAGO e quita as cobrancas do lote. */
    assert(lote_pagar(vazio) == 0); /* nao esta fechado */
    assert(lote_pagar(lote) == 1);
    assert(lote_pagar(lote) == 0); /* ja pago */
    assert(lote_fatura_json(lote, json, sizeof(json)) == 1);
    assert(strstr(json, "\"status\":\"PAGO\"") != NULL);
    assert(strstr(json, "\"status\":\"PAGA\"") != NULL); /* itens quitados */

    /* Listagem traz o lote com convenio e total. */
    assert(lote_listar_json(json, sizeof(json)) == 1);
    assert(strstr(json, "Unimed") != NULL);
    assert(strstr(json, "\"totalCentavos\":30000") != NULL);

    printf("test_lote_repository: OK\n");
    return 0;
}
