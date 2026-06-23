#include "vacina_repository.h"
#include "estoque_repository.h"
#include "medicamento_repository.h"
#include "paciente_repository.h"
#include "database.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *BANCO_TESTE = "build/test_sigeh_vacina.db";
static const char *SCHEMA = "../data/schema_v3.sql";

int main(void)
{
    char json[4096];

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    assert(medicamento_criar("Imunizante Influenza", "dose", "dose", 2, 0) == 1);
    assert(estoque_entrada(1, "VAC1", "2027-05-31", 5, "Geladeira", 1, "admin") == 1);
    assert(paciente_repo_criar("Maria Vacina", "1990-01-01", "11122233344",
                               "CPF", "61999990000", "F", 1, "", "") == 1);
    assert(paciente_repo_criar("Joao Imune", "1988-03-02", "22233344455",
                               "CPF", "61988880000", "M", 1, "", "") == 1);

    assert(vacina_contar_ativas() == 0);

    assert(vacina_criar("", "Fiocruz", "COVID-19", 2, 28, 180, 1) == 0);
    assert(vacina_criar(NULL, "Fiocruz", "COVID-19", 2, 28, 180, 1) == 0);

    assert(vacina_criar("COVID-19 bivalente", "Fiocruz", "COVID-19", 2, 28, 180, 1) == 1);
    assert(vacina_criar("Influenza", "Butantan", "Gripe", 1, 0, 365, 1) == 1);
    assert(vacina_criar("Hepatite B", "", "Hepatite B", -2, -1, -5, 0) == 1);
    assert(vacina_contar_ativas() == 3);

    assert(vacina_listar_json(json, sizeof(json)) == 1);
    assert(strstr(json, "\"nome\":\"COVID-19 bivalente\"") != NULL);
    assert(strstr(json, "\"fabricante\":\"Fiocruz\"") != NULL);
    assert(strstr(json, "\"doencasAlvo\":\"COVID-19\"") != NULL);
    assert(strstr(json, "\"dosesPrevistas\":2") != NULL);
    assert(strstr(json, "\"intervaloDias\":28") != NULL);
    assert(strstr(json, "\"reforcoDias\":180") != NULL);
    assert(strstr(json, "\"medicamentoId\":1") != NULL);
    assert(strstr(json, "\"nome\":\"Hepatite B\"") != NULL);
    assert(strstr(json, "\"dosesPrevistas\":1") != NULL);
    assert(strstr(json, "\"intervaloDias\":0") != NULL);
    assert(strstr(json, "\"reforcoDias\":0") != NULL);
    assert(strstr(json, "COVID-19 bivalente") < strstr(json, "Hepatite B"));

    assert(vacina_ativa(1) == 1);
    assert(vacina_ativa(999) == 0);
    assert(vacina_ativa(0) == 0);

    {
        int aplicacaoId = 0;

        assert(vacina_aplicar(1, 2, 1, "VAC1", "2027-05-31",
                              7, "enf", "campanha", &aplicacaoId) == 1);
        assert(aplicacaoId > 0);
        assert(estoque_saldo(1) == 4);
        assert(vacina_aplicacoes_listar_json(json, sizeof(json)) == 1);
        assert(strstr(json, "\"pacienteNome\":\"Maria Vacina\"") != NULL);
        assert(strstr(json, "\"vacinaNome\":\"Influenza\"") != NULL);
        assert(strstr(json, "\"doseNumero\":1") != NULL);
        assert(strstr(json, "\"lote\":\"VAC1\"") != NULL);
        assert(strstr(json, "\"validade\":\"2027-05-31\"") != NULL);
        assert(strstr(json, "\"aplicadorLogin\":\"enf\"") != NULL);

        assert(vacina_aplicar(2, 1, 1, "VAC1", "2027-05-31",
                              7, "enf", "outro paciente", NULL) == 1);
        assert(estoque_saldo(1) == 3);
        assert(vacina_aplicacoes_listar_por_paciente_json(1, json, sizeof(json)) == 1);
        assert(strstr(json, "\"pacienteNome\":\"Maria Vacina\"") != NULL);
        assert(strstr(json, "\"vacinaNome\":\"Influenza\"") != NULL);
        assert(strstr(json, "\"pacienteNome\":\"Joao Imune\"") == NULL);

        assert(vacina_aplicar(999, 2, 1, "VAC1", "2027-05-31",
                              7, "enf", "", NULL) == 0);
        assert(vacina_aplicar(1, 3, 1, "VAC1", "2027-05-31",
                              7, "enf", "", NULL) == 0);
        assert(vacina_aplicar(1, 2, 1, "SEMLOTE", "2027-05-31",
                              7, "enf", "", NULL) == 0);
        assert(estoque_saldo(1) == 3);
    }

    assert(vacina_desativar(1) == 1);
    assert(vacina_desativar(1) == 0);
    assert(vacina_desativar(999) == 0);
    assert(vacina_contar_ativas() == 2);
    assert(vacina_ativa(1) == 0);

    printf("test_vacina_repository: OK\n");
    return 0;
}
