#include "triagem_repository.h"
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
    int antes;

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    /* Pacientes pais (ids 1 e 2) exigidos pela FK de triagens. */
    assert(paciente_repo_criar("Ana", "1990-01-01", "111", "CPF", "61", "F", 1, "", "") == 1);
    assert(paciente_repo_criar("Bia", "1980-01-01", "222", "CPF", "61", "F", 2, "", "") == 1);

    assert(triagem_repo_contar_ativos() == 0);

    assert(triagem_repo_criar(1, 1, 8, "Emergencia") == 1);
    assert(triagem_repo_contar_ativos() == 1);

    /* Falhas de validacao. */
    assert(triagem_repo_criar(0, 1, 8, "Emergencia") == 0);
    assert(triagem_repo_criar(1, 0, 8, "Emergencia") == 0);
    assert(triagem_repo_criar(1, 1, 8, "") == 0);
    assert(triagem_repo_contar_ativos() == 1);

    assert(triagem_repo_listar_json(json, sizeof(json)) == 1);
    assert(json[0] == '[');
    assert(strstr(json, "Emergencia") != NULL);

    assert(triagem_repo_especialidades_json(json, sizeof(json)) == 1);
    assert(strstr(json, "Cardiologia") != NULL);
    assert(strstr(json, "Pneumologia") != NULL);

    assert(triagem_repo_problemas_por_especialidade_json(3, json, sizeof(json)) == 1);
    assert(strstr(json, "dor no peito") != NULL);
    assert(strstr(json, "Eletrocardiograma") != NULL);

    assert(triagem_repo_criar(2, 3, 4, "Prioritario") == 1);
    antes = triagem_repo_contar_ativos();
    assert(antes == 2);

    /* Filtro por tipos: triagem 1 e tipo 1; triagem 2 e tipo 3. */
    {
        int tiposCardio[1] = {3};
        int tiposGeral[1] = {1};
        int tiposVarios[2] = {1, 3};

        assert(triagem_repo_listar_por_tipos_json(tiposCardio, 1, json, sizeof(json)) == 1);
        assert(strstr(json, "\"tipoTriagem\":3") != NULL);
        assert(strstr(json, "\"tipoTriagem\":1") == NULL);

        assert(triagem_repo_listar_por_tipos_json(tiposVarios, 2, json, sizeof(json)) == 1);
        assert(strstr(json, "\"tipoTriagem\":1") != NULL);
        assert(strstr(json, "\"tipoTriagem\":3") != NULL);

        /* n <= 0 devolve uma lista vazia valida. */
        assert(triagem_repo_listar_por_tipos_json(tiposGeral, 0, json, sizeof(json)) == 1);
        assert(strcmp(json, "[]") == 0);
    }

    /* Reclassificacao versionada: nova versao vigente preserva a anterior. */
    /* Triagem 2 (Prioritario, nivel 4) piora para Emergencia (nivel 5). */
    assert(triagem_repo_reclassificar(2, "", 5, "dor_toracica", "x") == 0); /* classe vazia */
    assert(triagem_repo_reclassificar(2, "Vermelho", 5, "dor_toracica", "") == 0); /* sem justificativa */
    assert(triagem_repo_reclassificar(2, "Vermelho", 5, "dor_toracica", "Piora do quadro") == 1);
    /* Contagem de vigentes nao muda (substitui a versao). */
    assert(triagem_repo_contar_ativos() == antes);
    /* A fila vigente mostra a nova classificacao. */
    assert(triagem_repo_listar_json(json, sizeof(json)) == 1);
    assert(strstr(json, "Vermelho") != NULL);
    assert(strstr(json, "Prioritario") == NULL); /* versao antiga saiu do vigente */

    assert(triagem_repo_desativar(1) == 1);
    assert(triagem_repo_contar_ativos() == antes - 1);
    assert(triagem_repo_desativar(9999) == 0);

    {
        int triagemId = triagem_repo_criar_clinica(1, 77, 3, 1, "Azul", "",
                                                   "dor no peito", "observacao",
                                                   "", "", "", "");
        int pacienteId = 0;
        int profissionalId = 0;

        assert(triagemId > 0);
        assert(triagem_repo_paciente_id(triagemId, &pacienteId) == 1);
        assert(pacienteId == 1);
        assert(triagem_repo_profissional_id(triagemId, &profissionalId) == 1);
        assert(profissionalId == 77);
        assert(triagem_repo_adicionar_problema(triagemId, 1, 1, "principal") == 1);
        assert(triagem_repo_adicionar_problema(triagemId, 13, 0, "suspeita respiratoria") == 1);
        assert(triagem_repo_atualizar_resultado(triagemId, 3, 5, "Vermelho") == 1);
        assert(triagem_repo_detalhar_json(triagemId, json, sizeof(json)) == 1);
        assert(strstr(json, "\"especialidadePrincipal\":\"Cardiologia\"") != NULL);
        assert(strstr(json, "dor no peito") != NULL);
        assert(strstr(json, "falta de ar") != NULL);
        assert(triagem_repo_remover_problema(triagemId, 13) == 1);
        assert(triagem_repo_detalhar_json(triagemId, json, sizeof(json)) == 1);
        assert(strstr(json, "suspeita respiratoria") == NULL);
    }

    printf("test_triagem_repository: OK\n");
    return 0;
}
