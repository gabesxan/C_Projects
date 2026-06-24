#include "notificacao_repository.h"
#include "usuario_repository.h"
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

    /* Usuario 1 = MEDICO, usuario 2 = ENFERMAGEM (ids sequenciais). */
    assert(usuario_repo_criar("Med", "med", "x", "MEDICO", 0, 1) == 1);
    assert(usuario_repo_criar("Enf", "enf", "x", "ENFERMAGEM", 0, 0) == 1);

    assert(notificacao_contar_nao_lidas(1) == 0);

    /* Notificacao direcionada a um usuario (id de notificacao 1). */
    assert(notificacao_criar_para_usuario(1, "Bem-vindo", "ola", "INFO", "", 0) == 1);
    assert(notificacao_contar_nao_lidas(1) == 1);

    /* Fan-out por papel: o unico MEDICO (usuario 1) recebe (notif id 2). */
    assert(notificacao_criar_para_papel("MEDICO", "Novo na fila", "senha C001",
                                        "FILA", "checkin", 7) == 1);
    assert(notificacao_contar_nao_lidas(1) == 2);

    /* Fan-out para ENFERMAGEM nao afeta o medico (notif id 3 -> usuario 2). */
    assert(notificacao_criar_para_papel("ENFERMAGEM", "Triagem", "risco alto",
                                        "TRIAGEM", "triagem", 3) == 1);
    assert(notificacao_contar_nao_lidas(1) == 2);
    assert(notificacao_contar_nao_lidas(2) == 1);

    /* Listagem do usuario traz o conteudo e o vinculo. */
    assert(notificacao_listar_por_usuario_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "Novo na fila") != NULL);
    assert(strstr(json, "\"entidade\":\"checkin\"") != NULL);
    assert(strstr(json, "\"lida\":0") != NULL);

    /* Marcar uma como lida reduz a contagem; usuario errado nao afeta. */
    assert(notificacao_marcar_lida(1, 2) == 0); /* notif 1 e do usuario 1 */
    assert(notificacao_marcar_lida(1, 1) == 1);
    assert(notificacao_contar_nao_lidas(1) == 1);

    /* Marcar todas zera o usuario 1, sem tocar no usuario 2. */
    assert(notificacao_marcar_todas_lidas(1) == 1);
    assert(notificacao_contar_nao_lidas(1) == 0);
    assert(notificacao_contar_nao_lidas(2) == 1);

    printf("test_notificacao_repository: OK\n");
    return 0;
}
