#include "sessao_repository.h"
#include "usuario_repository.h"
#include "database.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *BANCO_TESTE = "build/test_sigeh_repository.db";
static const char *SCHEMA = "../data/schema_v3.sql";

int main(void)
{
    char papel[32] = "";
    char login[128] = "";
    char token[128] = "";
    char token2[128] = "";
    int pacienteId = 0;
    int medicoId = 0;
    int usuarioId = 0;
    int bloqueado = -1;
    int i;

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    /* Usuario de teste vinculado a um medico. */
    assert(usuario_repo_criar("Dra. Ana", "ana", "senha123", "MEDICO", 0, 7) == 1);

    /* Login correto: autentica, zera contadores, devolve papel/vinculos. */
    assert(usuario_repo_autenticar_com_bloqueio("ana", "senha123", papel, sizeof(papel),
                                                &pacienteId, &medicoId, &usuarioId,
                                                &bloqueado) == 1);
    assert(strcmp(papel, "MEDICO") == 0);
    assert(medicoId == 7);
    assert(usuarioId > 0);
    assert(bloqueado == 0);

    /* Cria a sessao e valida o token. */
    assert(sessao_repo_criar(usuarioId, 8, token, sizeof(token)) == 1);
    assert(strlen(token) == 64);

    papel[0] = '\0';
    medicoId = 0;
    assert(sessao_repo_validar(token, papel, sizeof(papel), &pacienteId, &medicoId,
                               &usuarioId, login, sizeof(login)) == 1);
    assert(strcmp(papel, "MEDICO") == 0);
    assert(medicoId == 7);
    assert(strcmp(login, "ana") == 0);

    /* Token desconhecido nao valida. */
    assert(sessao_repo_validar("naoexiste", papel, sizeof(papel), NULL, NULL, NULL,
                               NULL, 0) == 0);

    /* Logout: remove a sessao e o token deixa de valer. */
    assert(sessao_repo_remover(token) == 1);
    assert(sessao_repo_validar(token, papel, sizeof(papel), NULL, NULL, NULL,
                               NULL, 0) == 0);
    /* Remover de novo nao altera nada. */
    assert(sessao_repo_remover(token) == 0);

    /* Bloqueio por tentativas: 4 erros nao bloqueiam; o 5o bloqueia. */
    assert(usuario_repo_criar("Bob", "bob", "certa", "ENFERMAGEM", 0, 0) == 1);
    for (i = 0; i < 4; i++)
    {
        bloqueado = -1;
        assert(usuario_repo_autenticar_com_bloqueio("bob", "errada", NULL, 0,
                                                    NULL, NULL, NULL, &bloqueado) == 0);
        assert(bloqueado == 0);
    }
    bloqueado = -1;
    assert(usuario_repo_autenticar_com_bloqueio("bob", "errada", NULL, 0,
                                                NULL, NULL, NULL, &bloqueado) == 0);
    assert(bloqueado == 1);

    /* Mesmo com a senha certa, o login permanece bloqueado pela janela. */
    bloqueado = -1;
    assert(usuario_repo_autenticar_com_bloqueio("bob", "certa", NULL, 0,
                                                NULL, NULL, NULL, &bloqueado) == 0);
    assert(bloqueado == 1);

    /* Login inexistente: falha sem bloquear. */
    bloqueado = -1;
    assert(usuario_repo_autenticar_com_bloqueio("ninguem", "x", NULL, 0,
                                                NULL, NULL, NULL, &bloqueado) == 0);
    assert(bloqueado == 0);

    /* A sessao da 'ana' segue funcionando independente do bloqueio do 'bob'. */
    assert(sessao_repo_criar(usuarioId, 8, token2, sizeof(token2)) == 1);
    assert(sessao_repo_validar(token2, NULL, 0, NULL, NULL, NULL, NULL, 0) == 1);

    /* Troca de senha e exigencia de troca no 1o acesso. */
    {
        int carolId = 0;
        assert(usuario_repo_criar("Carol", "carol", "inicial1", "CADASTRO", 0, 0) == 1);
        /* Usuario criado pelo admin comeca exigindo troca. */
        assert(usuario_repo_autenticar_com_bloqueio("carol", "inicial1", NULL, 0,
                                                    NULL, NULL, &carolId, NULL) == 1);
        assert(usuario_repo_precisa_trocar_senha(carolId) == 1);

        /* Senha atual errada nao troca. */
        assert(usuario_repo_trocar_senha(carolId, "errada", "novaSenha2") == 0);
        /* Troca valida zera a exigencia. */
        assert(usuario_repo_trocar_senha(carolId, "inicial1", "novaSenha2") == 1);
        assert(usuario_repo_precisa_trocar_senha(carolId) == 0);

        /* A senha antiga deixa de valer; a nova autentica. */
        assert(usuario_repo_autenticar_com_bloqueio("carol", "inicial1", NULL, 0,
                                                    NULL, NULL, NULL, NULL) == 0);
        assert(usuario_repo_autenticar_com_bloqueio("carol", "novaSenha2", NULL, 0,
                                                    NULL, NULL, NULL, NULL) == 1);
    }

    printf("test_sessao_repository: OK\n");
    return 0;
}
