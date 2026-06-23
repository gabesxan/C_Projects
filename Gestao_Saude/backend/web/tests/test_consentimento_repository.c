#include "consentimento_repository.h"
#include "database.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *BANCO_TESTE = "build/test_sigeh_consentimento.db";
static const char *SCHEMA = "../data/schema_v3.sql";

int main(void)
{
    char json[4096];
    char status[32];
    int pid = 0;
    int id1 = 0;
    int id2 = 0;
    int id_outro = 0;

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    /* Paciente sem consentimentos: lista vazia (so os colchetes). */
    assert(consentimento_listar_por_paciente_json(1, json, sizeof(json)) == 1);
    assert(strcmp(json, "[]") == 0);

    /* Validacoes de criacao: campos obrigatorios e paciente positivo. */
    assert(consentimento_criar(0, "tratamento", "v1", &id1) == 0);
    assert(consentimento_criar(1, "", "v1", &id1) == 0);
    assert(consentimento_criar(1, "tratamento", "", &id1) == 0);
    assert(id1 == 0);

    /* Criacoes validas: dois para o paciente 1, um para outro paciente. */
    assert(consentimento_criar(1, "tratamento", "v1", &id1) == 1);
    assert(id1 > 0);
    assert(consentimento_criar(1, "pesquisa", "v2", &id2) == 1);
    assert(id2 > id1);
    assert(consentimento_criar(9, "marketing", "v1", &id_outro) == 1);

    /* Listagem por paciente: mais novo primeiro; status nasce CONCEDIDO. */
    assert(consentimento_listar_por_paciente_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "\"finalidade\":\"tratamento\"") != NULL);
    assert(strstr(json, "\"finalidade\":\"pesquisa\"") != NULL);
    assert(strstr(json, "\"status\":\"CONCEDIDO\"") != NULL);
    assert(strstr(json, "\"versaoTermo\":\"v2\"") != NULL);
    assert(strstr(json, "\"ativo\":1") != NULL);
    /* Consentimento de outro paciente nao aparece. */
    assert(strstr(json, "marketing") == NULL);
    /* Ordem decrescente por id: pesquisa (id2) antes de tratamento (id1). */
    assert(strstr(json, "pesquisa") < strstr(json, "tratamento"));
    /* concedido_em preenchido; revogado_em ainda vazio. */
    assert(strstr(json, "\"concedidoEm\":\"\"") == NULL);
    assert(strstr(json, "\"revogadoEm\":\"\"") != NULL);

    /* Busca por id: devolve paciente dono e status atual. */
    assert(consentimento_buscar(id1, &pid, status, sizeof(status)) == 1);
    assert(pid == 1);
    assert(strcmp(status, "CONCEDIDO") == 0);
    assert(consentimento_buscar(999, &pid, status, sizeof(status)) == 0);
    assert(consentimento_buscar(0, &pid, status, sizeof(status)) == 0);

    /* Revogacao: exige motivo, preserva a linha e troca o estado logico. */
    assert(consentimento_revogar(id1, "") == 0);          /* motivo obrigatorio */
    assert(consentimento_revogar(999, "engano") == 0);    /* inexistente */
    assert(consentimento_revogar(id1, "titular solicitou") == 1);
    /* Idempotente / transicao invalida: revogar de novo nao altera. */
    assert(consentimento_revogar(id1, "de novo") == 0);

    /* A linha continua existindo, agora REVOGADO e inativo, com motivo. */
    assert(consentimento_buscar(id1, &pid, status, sizeof(status)) == 1);
    assert(strcmp(status, "REVOGADO") == 0);
    assert(consentimento_listar_por_paciente_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "\"status\":\"REVOGADO\"") != NULL);
    assert(strstr(json, "\"motivoRevogacao\":\"titular solicitou\"") != NULL);
    assert(strstr(json, "\"ativo\":0") != NULL);
    /* Historico imutavel: o id1 continua na listagem (nao foi apagado). */
    assert(strstr(json, "tratamento") != NULL);
    /* O id2 segue CONCEDIDO. */
    assert(consentimento_buscar(id2, &pid, status, sizeof(status)) == 1);
    assert(strcmp(status, "CONCEDIDO") == 0);

    printf("test_consentimento_repository: OK\n");
    return 0;
}
