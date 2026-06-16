#include "usuario_repository.h"
#include "database.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *BANCO_TESTE = "build/test_sigeh_repository.db";
static const char *SCHEMA = "../data/schema_v3.sql";

int main(void)
{
    char json[2048];
    char papel[32];
    int pacienteId = -1;
    int medicoId = -1;

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    assert(usuario_repo_contar_ativos() == 0);

    /* Criacao valida. */
    assert(usuario_repo_criar("Administrador", "admin", "secreta", "ADMIN", 0, 0) == 1);
    assert(usuario_repo_contar_ativos() == 1);

    /* Login duplicado (UNIQUE) -> falha. */
    assert(usuario_repo_criar("Administrador", "admin", "outra", "ADMIN", 0, 0) == 0);
    /* Papel invalido -> falha. */
    assert(usuario_repo_criar("Joao", "joao", "x", "ROOT", 0, 0) == 0);
    /* Nome/login/senha vazios -> falha. */
    assert(usuario_repo_criar("", "semnome", "x", "ADMIN", 0, 0) == 0);
    assert(usuario_repo_criar("Nome", "", "x", "ADMIN", 0, 0) == 0);
    assert(usuario_repo_criar("Ana", "ana", "", "ADMIN", 0, 0) == 0);
    assert(usuario_repo_contar_ativos() == 1);

    /* Autenticacao correta preenche papel/vinculos. */
    assert(usuario_repo_autenticar("admin", "secreta", papel, sizeof(papel),
                                   &pacienteId, &medicoId) == 1);
    assert(strcmp(papel, "ADMIN") == 0);
    assert(pacienteId == 0 && medicoId == 0);

    /* Senha errada -> falha. */
    assert(usuario_repo_autenticar("admin", "errada", papel, sizeof(papel),
                                   NULL, NULL) == 0);
    /* Login inexistente -> falha. */
    assert(usuario_repo_autenticar("ninguem", "secreta", papel, sizeof(papel),
                                   NULL, NULL) == 0);

    /* Usuario MEDICO vinculado a um medico. */
    assert(usuario_repo_criar("Dr House", "drhouse", "house123", "MEDICO", 0, 5) == 1);
    assert(usuario_repo_autenticar("drhouse", "house123", papel, sizeof(papel),
                                   &pacienteId, &medicoId) == 1);
    assert(strcmp(papel, "MEDICO") == 0);
    assert(medicoId == 5);

    /* Listagem nao expoe senha/hash/salt. */
    assert(usuario_repo_listar_json(json, sizeof(json)) == 1);
    assert(strstr(json, "admin") != NULL);
    assert(strstr(json, "secreta") == NULL);
    assert(strstr(json, "senha") == NULL);
    assert(strstr(json, "salt") == NULL);

    /* Desativacao bloqueia autenticacao. */
    assert(usuario_repo_desativar(1) == 1);
    assert(usuario_repo_autenticar("admin", "secreta", papel, sizeof(papel),
                                   NULL, NULL) == 0);
    assert(usuario_repo_contar_ativos() == 1);

    printf("test_usuario_repository: OK\n");
    return 0;
}
