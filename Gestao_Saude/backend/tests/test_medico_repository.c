#include "medico_repository.h"
#include "database.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Caminhos relativos ao diretorio backend/ (cwd quando o make roda o teste). */
static const char *BANCO_TESTE = "tests/test_sigeh_repository.db";
static const char *SCHEMA = "data/schema_v2.sql";

int main(void)
{
    char json[4096];
    int antes;
    int depois;

    /* Aponta a camada de banco para um arquivo de teste isolado e o recria
     * do zero a partir do schema, para nao depender de dados antigos. */
    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    /* Banco recem-criado: nenhum medico ativo. */
    assert(medico_repo_contar_ativos() == 0);

    /* Criacao valida. */
    assert(medico_repo_criar("Dra. Helena", "CRM12345", "Cardiologia", 7) == 1);
    assert(medico_repo_contar_ativos() == 1);

    /* Falha: nome vazio. */
    assert(medico_repo_criar("", "CRM00000", "Ortopedia", 1) == 0);

    /* Falha: especialidade vazia. */
    assert(medico_repo_criar("Dr. Mateus", "CRM67890", "", 1) == 0);

    /* As falhas nao alteraram a contagem. */
    assert(medico_repo_contar_ativos() == 1);

    /* Listagem em JSON contem o medico criado. */
    assert(medico_repo_listar_json(json, sizeof(json)) == 1);
    assert(json[0] == '[');
    assert(strstr(json, "Dra. Helena") != NULL);

    /* Segundo medico para validar a desativacao. */
    assert(medico_repo_criar("Dr. Bruno", "CRM55555", "Pediatria", 9) == 1);
    antes = medico_repo_contar_ativos();
    assert(antes == 2);

    /* Exclusao logica do primeiro medico (id = 1). */
    assert(medico_repo_desativar(1) == 1);
    depois = medico_repo_contar_ativos();
    assert(depois == antes - 1);

    /* Desativar id inexistente nao deve contar como sucesso. */
    assert(medico_repo_desativar(9999) == 0);

    printf("test_medico_repository: OK\n");
    return 0;
}
