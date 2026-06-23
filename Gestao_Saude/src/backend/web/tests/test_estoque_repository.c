#include "estoque_repository.h"
#include "medicamento_repository.h"
#include "database.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static const char *BANCO_TESTE = "build/test_sigeh_estoque.db";
static const char *SCHEMA = "../data/schema_v3.sql";

int main(void)
{
    char json[4096];
    char validadeProxima[16];
    time_t agora = time(NULL);
    struct tm *tm = localtime(&agora);

    assert(tm != NULL);
    tm->tm_mday += 10;
    assert(mktime(tm) != (time_t)-1);
    assert(strftime(validadeProxima, sizeof(validadeProxima), "%Y-%m-%d", tm) > 0);

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    /* Medicamento pai (id 1) exigido pela FK. */
    assert(medicamento_criar("Dipirona", "500mg", "comprimido", 10, 150) == 1);

    assert(estoque_saldo(1) == 0);

    /* Parametros invalidos. */
    assert(estoque_entrada(0, "L1", "2026-01-01", 10, "", 1, "admin") == 0);
    assert(estoque_entrada(1, "L1", "2026-01-01", 0, "", 1, "admin") == 0);

    /* Entradas: dois lotes; o L2 vence antes (FIFO deve consumi-lo primeiro). */
    assert(estoque_entrada(1, "L1", "2026-01-01", 50, "Prateleira A", 1, "admin") == 1);
    assert(estoque_entrada(1, "L2", "2025-06-01", 30, "Prateleira A", 1, "admin") == 1);
    /* Entrada de lote identico soma na quantidade (L1 -> 70). */
    assert(estoque_entrada(1, "L1", "2026-01-01", 20, "", 1, "admin") == 1);
    assert(estoque_saldo(1) == 100);

    assert(estoque_itens_listar_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "\"lote\":\"L1\"") != NULL);
    assert(strstr(json, "\"lote\":\"L2\"") != NULL);
    assert(strstr(json, "\"quantidade\":70") != NULL);
    /* L2 (validade mais proxima) listado antes de L1 (FIFO). */
    assert(strstr(json, "\"lote\":\"L2\"") < strstr(json, "\"lote\":\"L1\""));

    /* Saldo insuficiente: nao baixa nem altera o estoque. */
    assert(estoque_baixar(1, 200, "SAIDA", "demais", 1, "admin") == 0);
    assert(estoque_saldo(1) == 100);

    /* Baixa por lote especifico preserva a rastreabilidade do item fisico. */
    assert(estoque_baixar_lote(1, "L1", "2026-01-01", 5,
                               "SAIDA", "aplicacao", 1, "admin") == 1);
    assert(estoque_saldo(1) == 95);
    assert(estoque_baixar_lote(1, "L1", "2026-01-01", 999,
                               "SAIDA", "excesso", 1, "admin") == 0);
    assert(estoque_baixar_lote(1, "X", "2026-01-01", 1,
                               "SAIDA", "lote inexistente", 1, "admin") == 0);
    assert(estoque_saldo(1) == 95);

    /* Baixa FIFO de 40: consome L2 (30) e depois L1 (10) -> saldo 55. */
    assert(estoque_baixar(1, 40, "SAIDA", "teste", 1, "admin") == 1);
    assert(estoque_saldo(1) == 55);

    /* L2 zerou e sai da listagem; L1 fica com 55. */
    assert(estoque_itens_listar_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "\"lote\":\"L2\"") == NULL);
    assert(strstr(json, "\"quantidade\":55") != NULL);

    /* AJUSTE tambem debita (ex.: perda de inventario). */
    assert(estoque_baixar(1, 10, "AJUSTE", "perda", 1, "admin") == 1);
    assert(estoque_saldo(1) == 45);

    /* Trilha de movimentacoes: entradas, saida e ajuste registrados. */
    assert(movimentacao_listar_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "\"tipo\":\"ENTRADA\"") != NULL);
    assert(strstr(json, "\"tipo\":\"SAIDA\"") != NULL);
    assert(strstr(json, "\"tipo\":\"AJUSTE\"") != NULL);

    /* Relatorio/alertas: saldo consolidado, estoque baixo e validade proxima. */
    assert(medicamento_criar("Amoxicilina", "250mg", "frasco", 30, 1200) == 1);
    assert(estoque_entrada(2, "AMX1", validadeProxima, 10, "Geladeira", 1, "admin") == 1);
    assert(estoque_alertas_json(json, sizeof(json)) == 1);
    assert(strstr(json, "\"saldos\"") != NULL);
    assert(strstr(json, "\"estoqueBaixo\"") != NULL);
    assert(strstr(json, "\"validadeProxima\"") != NULL);
    assert(strstr(json, "\"nome\":\"Dipirona\"") != NULL);
    assert(strstr(json, "\"saldo\":45") != NULL);
    assert(strstr(json, "\"nome\":\"Amoxicilina\"") != NULL);
    assert(strstr(json, "\"saldo\":10") != NULL);
    assert(strstr(json, "\"estoqueMinimo\":30") != NULL);
    assert(strstr(json, "\"lote\":\"AMX1\"") != NULL);
    assert(strstr(json, validadeProxima) != NULL);

    printf("test_estoque_repository: OK\n");
    return 0;
}
