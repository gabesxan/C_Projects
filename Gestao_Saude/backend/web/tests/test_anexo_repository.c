#include "anexo_repository.h"
#include "database.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *BANCO_TESTE = "build/test_sigeh_anexo.db";
static const char *SCHEMA = "../data/schema_v3.sql";

int main(void)
{
    char json[4096];
    char nome[256];
    char mime[160];
    char caminho[512];
    int id1 = 0;
    int id2 = 0;

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    /* Entidade vazia: lista vazia (so os colchetes). */
    assert(anexo_listar_por_entidade_json("exame", 1, json, sizeof(json)) == 1);
    assert(strcmp(json, "[]") == 0);

    /* Validacoes de criacao: campos obrigatorios e tamanho nao-negativo. */
    assert(anexo_criar("", 1, "a.pdf", "application/pdf", 10, "anexos/1_a.pdf", 5, "ana", &id1) == 0);
    assert(anexo_criar("exame", 0, "a.pdf", "application/pdf", 10, "anexos/1_a.pdf", 5, "ana", &id1) == 0);
    assert(anexo_criar("exame", 1, "", "application/pdf", 10, "anexos/1_a.pdf", 5, "ana", &id1) == 0);
    assert(anexo_criar("exame", 1, "a.pdf", "", 10, "anexos/1_a.pdf", 5, "ana", &id1) == 0);
    assert(anexo_criar("exame", 1, "a.pdf", "application/pdf", 10, "", 5, "ana", &id1) == 0);
    assert(anexo_criar("exame", 1, "a.pdf", "application/pdf", -1, "anexos/1_a.pdf", 5, "ana", &id1) == 0);
    assert(id1 == 0);

    /* Criacoes validas em entidades distintas. */
    assert(anexo_criar("exame", 1, "laudo.pdf", "application/pdf", 2048,
                       "anexos/1_laudo.pdf", 5, "ana", &id1) == 1);
    assert(id1 > 0);
    assert(anexo_criar("exame", 1, "raio-x.png", "image/png", 4096,
                       "anexos/2_raio-x.png", 7, "bruno", &id2) == 1);
    assert(id2 > id1);
    /* Anexo de outra entidade nao deve aparecer na listagem do exame 1. */
    assert(anexo_criar("paciente", 9, "termo.pdf", "application/pdf", 512,
                       "anexos/3_termo.pdf", 5, "ana", NULL) == 1);

    /* Listagem por entidade: mais novo primeiro, sem expor o caminho interno. */
    assert(anexo_listar_por_entidade_json("exame", 1, json, sizeof(json)) == 1);
    assert(strstr(json, "\"nome\":\"laudo.pdf\"") != NULL);
    assert(strstr(json, "\"nome\":\"raio-x.png\"") != NULL);
    assert(strstr(json, "\"mime\":\"image/png\"") != NULL);
    assert(strstr(json, "\"tamanho\":2048") != NULL);
    assert(strstr(json, "\"autorLogin\":\"bruno\"") != NULL);
    assert(strstr(json, "caminho") == NULL);    /* caminho nunca vai pro cliente */
    assert(strstr(json, "termo.pdf") == NULL);   /* outra entidade fora */
    /* Ordem decrescente por id: raio-x (id2) antes de laudo (id1). */
    assert(strstr(json, "raio-x.png") < strstr(json, "laudo.pdf"));

    /* Finalizacao do caminho (passo pos-insert que usa o id gerado). */
    assert(anexo_definir_caminho(id1, "anexos/1_laudo-final.pdf") == 1);
    assert(anexo_definir_caminho(999, "anexos/x") == 0);
    assert(anexo_definir_caminho(id1, "") == 0);

    /* Busca por id: devolve nome, mime e caminho interno para o download. */
    assert(anexo_buscar(id1, nome, sizeof(nome), mime, sizeof(mime),
                        caminho, sizeof(caminho)) == 1);
    assert(strcmp(nome, "laudo.pdf") == 0);
    assert(strcmp(mime, "application/pdf") == 0);
    assert(strcmp(caminho, "anexos/1_laudo-final.pdf") == 0);
    assert(anexo_buscar(999, nome, sizeof(nome), mime, sizeof(mime),
                        caminho, sizeof(caminho)) == 0);
    assert(anexo_buscar(0, nome, sizeof(nome), mime, sizeof(mime),
                        caminho, sizeof(caminho)) == 0);

    /* Remocao: devolve o caminho para o chamador apagar o arquivo. */
    caminho[0] = '\0';
    assert(anexo_remover(id1, caminho, sizeof(caminho)) == 1);
    assert(strcmp(caminho, "anexos/1_laudo-final.pdf") == 0);
    assert(anexo_remover(id1, caminho, sizeof(caminho)) == 0); /* ja removido */
    assert(anexo_remover(0, caminho, sizeof(caminho)) == 0);
    assert(anexo_buscar(id1, nome, sizeof(nome), mime, sizeof(mime),
                        caminho, sizeof(caminho)) == 0);

    /* So o id1 saiu; id2 segue na listagem do exame 1. */
    assert(anexo_listar_por_entidade_json("exame", 1, json, sizeof(json)) == 1);
    assert(strstr(json, "raio-x.png") != NULL);
    assert(strstr(json, "laudo.pdf") == NULL);

    printf("test_anexo_repository: OK\n");
    return 0;
}
