#include "medicamento_repository.h"
#include "database.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *BANCO_TESTE = "build/test_sigeh_medicamento.db";
static const char *SCHEMA = "../data/schema_v3.sql";

int main(void)
{
    char json[4096];

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    /* Catalogo vazio. */
    assert(medicamento_contar_ativos() == 0);

    /* Nome obrigatorio. */
    assert(medicamento_criar("", "500mg comprimido", "comprimido", 10, 150) == 0);
    assert(medicamento_criar(NULL, "x", "x", 1, 1) == 0);

    /* Cadastros validos (estoque_minimo e preco negativos viram 0). */
    assert(medicamento_criar("Dipirona", "500mg comprimido", "comprimido", 20, 150) == 1);
    assert(medicamento_criar("Amoxicilina", "250mg/5ml suspensao", "ml", -5, -10) == 1);
    assert(medicamento_contar_ativos() == 2);

    /* Listagem ordenada por nome, com os campos esperados. */
    assert(medicamento_listar_json(json, sizeof(json)) == 1);
    assert(strstr(json, "\"nome\":\"Amoxicilina\"") != NULL);
    assert(strstr(json, "\"nome\":\"Dipirona\"") != NULL);
    assert(strstr(json, "\"apresentacao\":\"500mg comprimido\"") != NULL);
    assert(strstr(json, "\"estoqueMinimo\":20") != NULL);
    assert(strstr(json, "\"estoqueMinimo\":0") != NULL); /* negativo normalizado */
    assert(strstr(json, "\"precoCentavos\":150") != NULL);
    assert(strstr(json, "\"precoCentavos\":0") != NULL); /* preco negativo normalizado */
    /* Amoxicilina (A) vem antes de Dipirona (D). */
    assert(strstr(json, "Amoxicilina") < strstr(json, "Dipirona"));

    /* Vigencia e preco por id (id 1 = Dipirona). */
    assert(medicamento_ativo(1) == 1);
    assert(medicamento_ativo(999) == 0);
    assert(medicamento_ativo(0) == 0);
    assert(medicamento_preco_centavos(1) == 150);
    assert(medicamento_preco_centavos(999) == -1);

    /* Desativacao (soft delete): some da listagem e da contagem. */
    assert(medicamento_desativar(1) == 1);
    assert(medicamento_desativar(1) == 0); /* ja inativo */
    assert(medicamento_desativar(999) == 0);
    assert(medicamento_contar_ativos() == 1);
    assert(medicamento_ativo(1) == 0);

    printf("test_medicamento_repository: OK\n");
    return 0;
}
