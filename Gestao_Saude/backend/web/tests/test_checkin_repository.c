#include "checkin_repository.h"
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
    char senha[16];

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    assert(paciente_repo_criar("Ana", "1990-01-01", "111", "CPF", "61", "F", 1, "", "") == 1);
    assert(paciente_repo_criar("Bia", "1980-01-01", "222", "CPF", "61", "F", 2, "", "") == 1);

    assert(checkin_repo_contar_aguardando() == 0);

    /* Check-in para triagem gera senha T001. */
    assert(checkin_repo_criar(1, "TRIAGEM", senha, sizeof(senha)) == 1);
    assert(strcmp(senha, "T001") == 0);
    /* Segundo check-in de triagem -> T002. */
    assert(checkin_repo_criar(2, "TRIAGEM", senha, sizeof(senha)) == 1);
    assert(strcmp(senha, "T002") == 0);
    /* Consulta usa outra serie -> C001. */
    assert(checkin_repo_criar(1, "CONSULTA", senha, sizeof(senha)) == 1);
    assert(strcmp(senha, "C001") == 0);

    /* Destino invalido falha. */
    assert(checkin_repo_criar(1, "QUALQUER", senha, sizeof(senha)) == 0);

    assert(checkin_repo_contar_aguardando() == 3);

    /* Fila lista os nao encerrados com nome do paciente. */
    assert(checkin_repo_listar_json(json, sizeof(json)) == 1);
    assert(strstr(json, "T001") != NULL);
    assert(strstr(json, "Ana") != NULL);
    assert(strstr(json, "\"destino\":\"TRIAGEM\"") != NULL);

    /* Chamar tira de AGUARDANDO; encerrar sai da fila. */
    assert(checkin_repo_chamar(1) == 1);
    assert(checkin_repo_contar_aguardando() == 2);
    assert(checkin_repo_encerrar(1) == 1);
    assert(checkin_repo_listar_json(json, sizeof(json)) == 1);
    assert(strstr(json, "T001") == NULL); /* encerrado saiu da fila */

    /* Chamar id inexistente falha. */
    assert(checkin_repo_chamar(999) == 0);

    /* Rechamada: so vale para quem esta EM_ATENDIMENTO. */
    assert(checkin_repo_chamar(2) == 1);           /* T002 -> EM_ATENDIMENTO */
    assert(checkin_repo_rechamar(2) == 1);
    assert(checkin_repo_rechamar(2) == 1);
    assert(checkin_repo_listar_json(json, sizeof(json)) == 1);
    assert(strstr(json, "\"rechamadas\":2") != NULL);
    assert(checkin_repo_rechamar(3) == 0);         /* C001 ainda AGUARDANDO */

    /* Falta e retorno a fila. */
    assert(checkin_repo_faltar(2) == 1);           /* EM_ATENDIMENTO -> FALTOU */
    assert(checkin_repo_contar_aguardando() == 1); /* so C001 aguardando */
    assert(checkin_repo_retornar(2) == 1);         /* FALTOU -> AGUARDANDO */
    assert(checkin_repo_contar_aguardando() == 2);
    assert(checkin_repo_retornar(2) == 0);         /* nao esta mais como falta */

    /* Cancelamento exige motivo e tira da fila. */
    assert(checkin_repo_cancelar(3, "") == 0);
    assert(checkin_repo_cancelar(3, "paciente desistiu") == 1);
    assert(checkin_repo_contar_aguardando() == 1);
    assert(checkin_repo_listar_json(json, sizeof(json)) == 1);
    assert(strstr(json, "C001") == NULL);          /* cancelado saiu da fila */
    assert(checkin_repo_cancelar(3, "de novo") == 0); /* ja cancelado */

    printf("test_checkin_repository: OK\n");
    return 0;
}
