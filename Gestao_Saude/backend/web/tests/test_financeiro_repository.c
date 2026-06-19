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
    char json[4096];

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    assert(paciente_repo_criar("Ana", "1990-01-01", "111", "CPF", "61", "F", 1, "", "") == 1);

    /* Convenios: Unimed cobre 70% (paciente paga 30% de coparticipacao). */
    assert(convenio_contar_ativos() == 0);
    assert(convenio_criar("Unimed", 70) == 1);
    assert(convenio_criar("", 50) == 0);
    assert(convenio_contar_ativos() == 1);
    assert(paciente_repo_definir_convenio(1, 1) == 1);
    assert(convenio_listar_json(json, sizeof(json)) == 1);
    assert(strstr(json, "\"coberturaPct\":70") != NULL);

    /* Cobrancas: convenio exige convenio_id valido; valor > 0. */
    assert(cobranca_criar(1, 0, "CONVENIO", "agendamento #1", "Consulta", 15000, "", "", "") == 0);
    assert(cobranca_criar(1, 1, "CONVENIO", "agendamento #1", "Consulta", 0, "", "", "") == 0);
    assert(cobranca_criar(1, 99, "CONVENIO", "x", "y", 15000, "", "", "") == 0); /* convenio inexistente */
    /* id 1: convenio, vencimento no passado (vencida) + guia. */
    assert(cobranca_criar(1, 1, "CONVENIO", "agendamento #1", "Consulta", 15000,
                          "2020-01-01", "GUIA-1", "2020-12-31") == 1);
    /* id 2: particular (paciente paga tudo). */
    assert(cobranca_criar(1, 0, "PARTICULAR", "exame", "Hemograma", 8000, "", "", "") == 1);
    assert(cobranca_contar_pendentes() == 2);

    /* Coparticipacao: convenio 70% -> coberto 10500 / copart 4500; particular
     * -> copart 8000. Guia e flag de vencida presentes. */
    assert(cobranca_listar_json(json, sizeof(json)) == 1);
    assert(strstr(json, "\"cobertoCentavos\":10500") != NULL);
    assert(strstr(json, "\"copartCentavos\":4500") != NULL);
    assert(strstr(json, "\"copartCentavos\":8000") != NULL);
    assert(strstr(json, "\"guia\":\"GUIA-1\"") != NULL);
    assert(strstr(json, "\"vencida\":true") != NULL);

    /* Demonstrativo: id 1 esta em aberto e vencida -> total vencido 15000. */
    assert(cobranca_demonstrativo_json(json, sizeof(json)) == 1);
    assert(strstr(json, "\"vencidoCentavos\":15000") != NULL);

    /* Status: GLOSADA exige motivo; fluxo ate PAGA. */
    assert(cobranca_atualizar_status(1, "GLOSADA", "") == 0);
    assert(cobranca_atualizar_status(1, "AUTORIZADA", "") == 1);
    assert(cobranca_atualizar_status(1, "PAGA", "") == 1);
    /* Cobranca paga (terminal) nao muda mais. */
    assert(cobranca_atualizar_status(1, "CANCELADA", "erro") == 0);
    /* A particular (id 2) e cancelada com motivo. */
    assert(cobranca_atualizar_status(2, "CANCELADA", "Paciente desistiu") == 1);
    assert(cobranca_contar_pendentes() == 0);

    /* Apos paga, nao ha mais valor vencido em aberto. */
    assert(cobranca_demonstrativo_json(json, sizeof(json)) == 1);
    assert(strstr(json, "\"recebidoCentavos\":15000") != NULL);
    assert(strstr(json, "\"vencidoCentavos\":0") != NULL);
    assert(strstr(json, "porStatus") != NULL);

    assert(cobranca_listar_por_paciente_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "\"forma\":\"CONVENIO\"") != NULL);

    printf("test_financeiro_repository: OK\n");
    return 0;
}
