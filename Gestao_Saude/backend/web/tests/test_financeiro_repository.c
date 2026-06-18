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

    /* Convenios. */
    assert(convenio_contar_ativos() == 0);
    assert(convenio_criar("Unimed") == 1);
    assert(convenio_criar("") == 0);
    assert(convenio_contar_ativos() == 1);
    assert(paciente_repo_definir_convenio(1, 1) == 1);

    /* Cobrancas: convenio exige convenio_id; valor > 0. */
    assert(cobranca_criar(1, 0, "CONVENIO", "agendamento #1", "Consulta", 15000) == 0);
    assert(cobranca_criar(1, 1, "CONVENIO", "agendamento #1", "Consulta", 0) == 0);
    assert(cobranca_criar(1, 1, "CONVENIO", "agendamento #1", "Consulta", 15000) == 1);
    assert(cobranca_criar(1, 0, "PARTICULAR", "exame", "Hemograma", 8000) == 1);
    assert(cobranca_contar_pendentes() == 2);

    /* Status: GLOSADA exige motivo; fluxo ate PAGA. */
    assert(cobranca_atualizar_status(1, "GLOSADA", "") == 0);
    assert(cobranca_atualizar_status(1, "AUTORIZADA", "") == 1);
    assert(cobranca_atualizar_status(1, "PAGA", "") == 1);
    /* Cobranca paga (terminal) nao muda mais. */
    assert(cobranca_atualizar_status(1, "CANCELADA", "erro") == 0);
    /* A particular (id 2) e cancelada com motivo. */
    assert(cobranca_atualizar_status(2, "CANCELADA", "Paciente desistiu") == 1);
    assert(cobranca_contar_pendentes() == 0);

    /* Listagem e demonstrativo. */
    assert(cobranca_listar_json(json, sizeof(json)) == 1);
    assert(strstr(json, "Hemograma") != NULL);
    assert(cobranca_listar_por_paciente_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "\"forma\":\"CONVENIO\"") != NULL);

    assert(cobranca_demonstrativo_json(json, sizeof(json)) == 1);
    assert(strstr(json, "\"recebidoCentavos\":15000") != NULL);
    assert(strstr(json, "porStatus") != NULL);

    printf("test_financeiro_repository: OK\n");
    return 0;
}
