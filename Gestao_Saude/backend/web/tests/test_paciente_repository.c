#include "paciente_repository.h"
#include "database.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Caminhos relativos ao diretorio backend/ (cwd quando o make roda o teste). */
static const char *BANCO_TESTE = "build/test_sigeh_repository.db";
static const char *SCHEMA = "../data/schema_v3.sql";

int main(void)
{
    char json[4096];
    int antes;
    int depois;

    /* Aponta a camada de banco para um arquivo de teste isolado e o recria
     * do zero a partir do schema, para nao depender de dados antigos. */
    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    /* Banco recem-criado: nenhum paciente ativo. */
    assert(paciente_repo_contar_ativos() == 0);

    /* Criacao valida. */
    assert(paciente_repo_criar("Joao Silva", "12345678900", 30,
                               "61999990000", "M", 7) == 1);
    assert(paciente_repo_contar_ativos() == 1);

    /* Falha: nome vazio. */
    assert(paciente_repo_criar("", "00000000000", 20,
                               "6100000000", "F", 1) == 0);

    /* Falha: idade negativa. */
    assert(paciente_repo_criar("Maria Souza", "98765432100", -5,
                               "6100000000", "F", 1) == 0);

    /* As falhas nao alteraram a contagem. */
    assert(paciente_repo_contar_ativos() == 1);

    /* Listagem em JSON contem o paciente criado. */
    assert(paciente_repo_listar_json(json, sizeof(json)) == 1);
    assert(json[0] == '[');
    assert(strstr(json, "Joao Silva") != NULL);

    /* Segundo paciente para validar a desativacao. */
    assert(paciente_repo_criar("Ana Lima", "33344455566", 5,
                               "61888880000", "F", 9) == 1);
    antes = paciente_repo_contar_ativos();
    assert(antes == 2);

    /* Exclusao logica do primeiro paciente (id = 1). */
    assert(paciente_repo_desativar(1) == 1);
    depois = paciente_repo_contar_ativos();
    assert(depois == antes - 1);

    /* Desativar id inexistente nao deve contar como sucesso. */
    assert(paciente_repo_desativar(9999) == 0);

    printf("test_paciente_repository: OK\n");
    return 0;
}
