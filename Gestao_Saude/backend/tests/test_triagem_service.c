#include "triagem_service.h"
#include "triagem_repository.h"
#include "database.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *BANCO_TESTE = "tests/test_sigeh_repository.db";
static const char *SCHEMA = "data/schema_v2.sql";

int main(void)
{
    char json[512];

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    /* Paciente sem triagem ativa -> 0. */
    assert(triagem_service_avaliar_json(1, json, sizeof(json)) == 0);

    /* Cardiologia + Emergencia -> especialidade Cardiologia, prioridade 5. */
    assert(triagem_repo_criar(1, 3, 8, "Emergencia") == 1);
    assert(triagem_service_avaliar_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "\"pacienteId\":1") != NULL);
    assert(strstr(json, "\"especialidadeProvavel\":\"Cardiologia\"") != NULL);
    assert(strstr(json, "\"prioridade\":5") != NULL);

    /* Outro tipo: Pediatria + Prioritario -> Pediatria, prioridade 3. */
    assert(triagem_repo_criar(2, 5, 3, "Prioritario") == 1);
    assert(triagem_service_avaliar_json(2, json, sizeof(json)) == 1);
    assert(strstr(json, "\"especialidadeProvavel\":\"Pediatria\"") != NULL);
    assert(strstr(json, "\"prioridade\":3") != NULL);

    /* Deve usar a triagem MAIS RECENTE do paciente. */
    assert(triagem_repo_criar(1, 2, 1, "Comum") == 1);
    assert(triagem_service_avaliar_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "\"especialidadeProvavel\":\"Ortopedia\"") != NULL);
    assert(strstr(json, "\"prioridade\":2") != NULL);

    /* Parametros invalidos -> 0. */
    assert(triagem_service_avaliar_json(0, json, sizeof(json)) == 0);

    printf("test_triagem_service: OK\n");
    return 0;
}
