#include "paciente_repository.h"
#include "database.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Caminhos relativos ao diretorio src/backend/ (cwd quando o make roda o teste). */
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

    /* Criacao valida (adulto, CPF). */
    assert(paciente_repo_criar("Joao Silva", "1990-05-10", "12345678900", "CPF",
                               "61999990000", "M", 7, "", "") == 1);
    assert(paciente_repo_contar_ativos() == 1);

    /* Falha: nome vazio. */
    assert(paciente_repo_criar("", "1990-01-01", "00000000000", "CPF",
                               "6100000000", "F", 1, "", "") == 0);

    /* Falha: nascimento invalido. */
    assert(paciente_repo_criar("Maria Souza", "data-ruim", "98765432100", "CPF",
                               "6100000000", "F", 1, "", "") == 0);

    /* Falha: nascimento no futuro. */
    assert(paciente_repo_criar("Futuro", "2999-01-01", "55566677788", "CPF",
                               "6100000000", "M", 1, "", "") == 0);

    /* Falha: menor de idade sem responsavel. */
    assert(paciente_repo_criar("Crianca", "2020-01-01", "44455566677", "CPF",
                               "6100000000", "F", 1, "", "") == 0);

    /* Sucesso: menor de idade COM responsavel. */
    assert(paciente_repo_criar("Crianca", "2020-01-01", "44455566677", "CPF",
                               "6100000000", "F", 1, "Mae da Crianca", "") == 1);

    /* Falha: CPF duplicado entre pacientes ATIVOS. */
    assert(paciente_repo_criar("Outro Joao", "1985-01-01", "12345678900", "CPF",
                               "6100000000", "M", 1, "", "") == 0);

    /* Documento alternativo (nao-CPF) nao entra na regra de unicidade. */
    assert(paciente_repo_criar("Sem CPF", "1970-01-01", "PASSAPORTE123", "OUTRO",
                               "6100000000", "M", 1, "", "") == 1);

    /* As falhas nao alteraram a contagem (3 sucessos ate aqui). */
    assert(paciente_repo_contar_ativos() == 3);

    /* Listagem em JSON contem o paciente e a idade calculada (campo "idade"). */
    assert(paciente_repo_listar_json(json, sizeof(json)) == 1);
    assert(json[0] == '[');
    assert(strstr(json, "Joao Silva") != NULL);
    assert(strstr(json, "\"idade\":") != NULL);
    /* A idade NAO e mais um parametro de entrada, e derivada do nascimento. */
    assert(strstr(json, "\"nascimento\":\"1990-05-10\"") != NULL);

    /* Detalhe por id. */
    assert(paciente_repo_detalhe_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "Joao Silva") != NULL);
    assert(paciente_repo_detalhe_json(9999, json, sizeof(json)) == 0);

    antes = paciente_repo_contar_ativos();
    assert(antes == 3);

    /* Exclusao logica do primeiro paciente (id = 1). */
    assert(paciente_repo_desativar(1) == 1);
    depois = paciente_repo_contar_ativos();
    assert(depois == antes - 1);

    /* Apos desativar o Joao, o mesmo CPF pode ser cadastrado de novo (ativo). */
    assert(paciente_repo_criar("Joao Renovado", "1990-05-10", "12345678900",
                               "CPF", "61999990000", "M", 7, "", "") == 1);

    /* Desativar id inexistente nao deve contar como sucesso. */
    assert(paciente_repo_desativar(9999) == 0);

    printf("test_paciente_repository: OK\n");
    return 0;
}
