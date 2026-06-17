/*
 * Ferramenta de seed do SIGEH-DF.
 * Recria o banco oficial a partir do schema versionado e popula dados minimos
 * para o sistema ser usavel: um usuario por papel (com senha com hash) e alguns
 * cadastros de exemplo. Reaproveita os repositories reais do backend.
 *
 * Uso (a partir de backend/web/):
 *   ./build/seed [caminho_schema] [caminho_banco]
 * Padroes: ../data/schema_v3.sql e ../data/sigeh_v3.db (caminho default do db).
 */

#include "database.h"
#include "usuario_repository.h"
#include "paciente_repository.h"
#include "medico_repository.h"
#include "ala_repository.h"
#include "leito_repository.h"

#include <stdio.h>

#define SCHEMA_PADRAO "../data/schema_v3.sql"

/* Cria um usuario e relata no stdout; aborta o seed em caso de falha. */
static int criar_usuario(const char *nome, const char *login, const char *senha,
                         const char *papel, int paciente_id, int medico_id)
{
    if (usuario_repo_criar(nome, login, senha, papel, paciente_id, medico_id) != 1)
    {
        fprintf(stderr, "falha ao criar usuario '%s'\n", login);
        return 0;
    }

    printf("  usuario: %-10s papel=%-10s senha=%s\n", login, papel, senha);
    return 1;
}

int main(int argc, char *argv[])
{
    const char *schema = argc > 1 ? argv[1] : SCHEMA_PADRAO;

    /* O segundo argumento permite gerar um banco em outro caminho (testes). */
    if (argc > 2 && db_definir_caminho(argv[2]) != 1)
    {
        fprintf(stderr, "caminho de banco invalido: %s\n", argv[2]);
        return 1;
    }

    printf("Recriando banco em %s a partir de %s\n", db_caminho(), schema);

    if (db_resetar_com_schema(schema) != 1)
    {
        fprintf(stderr, "falha ao resetar banco com schema %s\n", schema);
        return 1;
    }

    /* Cadastros de exemplo (id 1 em cada tabela apos o reset). */
    if (medico_repo_criar("Dra. Helena Prado", "CRM-DF 1001",
                          "Clinica Geral", 1) != 1)
    {
        fprintf(stderr, "falha ao criar medico de exemplo\n");
        return 1;
    }

    if (paciente_repo_criar("Joao da Silva", "1981-05-12", "12345678900", "CPF",
                            "61999990000", "M", 1, "", "Penicilina") != 1)
    {
        fprintf(stderr, "falha ao criar paciente de exemplo\n");
        return 1;
    }

    /* Uma ala com tres leitos para a gestao de internacao/enfermagem. */
    if (ala_repo_criar("Ala A - Clinica Medica", 1, 3) != 1 ||
        leito_repo_criar(1, 101) != 1 ||
        leito_repo_criar(1, 102) != 1 ||
        leito_repo_criar(1, 103) != 1)
    {
        fprintf(stderr, "falha ao criar ala/leitos de exemplo\n");
        return 1;
    }

    /* Um usuario por papel. PACIENTE/MEDICO ficam vinculados aos cadastros. */
    printf("Usuarios criados:\n");
    if (!criar_usuario("Administrador", "admin", "admin123", "ADMIN", 0, 0) ||
        !criar_usuario("Recepcao", "cadastro", "cadastro123", "CADASTRO", 0, 0) ||
        !criar_usuario("Dra. Helena Prado", "medico", "medico123", "MEDICO", 0, 1) ||
        !criar_usuario("Equipe Enfermagem", "enfermagem", "enfermagem123",
                       "ENFERMAGEM", 0, 0) ||
        !criar_usuario("Joao da Silva", "paciente", "paciente123", "PACIENTE", 1, 0))
    {
        return 1;
    }

    printf("Seed concluido.\n");
    return 0;
}
