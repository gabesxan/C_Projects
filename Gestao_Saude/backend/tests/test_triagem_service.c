#include "triagem_service.h"
#include "triagem_repository.h"
#include "paciente_repository.h"
#include "medico_repository.h"
#include "prontuario_repository.h"
#include "exame_repository.h"
#include "agendamento_repository.h"
#include "database.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const char *BANCO_TESTE = "tests/test_sigeh_repository.db";
static const char *SCHEMA = "data/schema_v2.sql";

int main(void)
{
    char json[2048];

    assert(db_definir_caminho(BANCO_TESTE) == 1);
    assert(db_resetar_com_schema(SCHEMA) == 1);

    /* Paciente sem triagem ativa -> 0. */
    assert(triagem_service_avaliar_json(1, json, sizeof(json)) == 0);

    /* Cardiologia + Emergencia -> especialidade Cardiologia, prioridade 5. */
    assert(triagem_repo_criar(1, 3, 8, "Emergencia") == 1);
    assert(triagem_service_avaliar_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "\"pacienteId\":1") != NULL);
    assert(strstr(json, "\"especialidadeProvavel\":\"Cardiologia\"") != NULL);
    assert(strstr(json, "\"prioridade\":5") != NULL);

    /* Outro tipo: Pediatria + Prioritario -> Pediatria, prioridade 3. */
    assert(triagem_repo_criar(2, 5, 3, "Prioritario") == 1);
    assert(triagem_service_avaliar_json(2, json, sizeof(json)) == 1);
    assert(strstr(json, "\"especialidadeProvavel\":\"Pediatria\"") != NULL);
    assert(strstr(json, "\"prioridade\":3") != NULL);

    /* Deve usar a triagem MAIS RECENTE do paciente. */
    assert(triagem_repo_criar(1, 2, 1, "Comum") == 1);
    assert(triagem_service_avaliar_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "\"especialidadeProvavel\":\"Ortopedia\"") != NULL);
    assert(strstr(json, "\"prioridade\":2") != NULL);

    /* Parametros invalidos -> 0. */
    assert(triagem_service_avaliar_json(0, json, sizeof(json)) == 0);

    /* --- Sugestao de medicos por especialidade + regiao --- */
    assert(db_resetar_com_schema(SCHEMA) == 1);

    /* Paciente da regiao 7 (recebe id 1). */
    assert(paciente_repo_criar("Joao", "11122233344", 40, "61999990000", "M", 7) == 1);
    /* Triagem cardiologica para o paciente 1. */
    assert(triagem_repo_criar(1, 3, 8, "Emergencia") == 1);

    /* Medicos: so o cardiologista da regiao 7 deve casar. */
    assert(medico_repo_criar("Dr Cardio Local", "CRM1", "Cardiologia", 7) == 1);
    assert(medico_repo_criar("Dr Cardio Longe", "CRM2", "Cardiologia", 3) == 1);
    assert(medico_repo_criar("Dr Orto Local", "CRM3", "Ortopedia", 7) == 1);

    assert(triagem_service_sugerir_medicos_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "\"especialidadeProvavel\":\"Cardiologia\"") != NULL);
    assert(strstr(json, "\"regiao\":7") != NULL);
    assert(strstr(json, "Dr Cardio Local") != NULL);
    assert(strstr(json, "Dr Cardio Longe") == NULL); /* regiao diferente */
    assert(strstr(json, "Dr Orto Local") == NULL);   /* especialidade diferente */

    /* Paciente sem triagem -> 0. */
    assert(triagem_service_sugerir_medicos_json(999, json, sizeof(json)) == 0);

    /* --- Historico clinico (prontuarios + exames anteriores) --- */
    assert(db_resetar_com_schema(SCHEMA) == 1);
    assert(prontuario_repo_criar(1, 1, "2026-06-01", "Estavel", "Gripe",
                                 "Repouso", 0) == 1);
    assert(exame_repo_criar(1, 1, 1, 1, "2026-06-01", 0) == 1);

    assert(triagem_service_historico_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "\"prontuarios\":") != NULL);
    assert(strstr(json, "\"exames\":") != NULL);
    assert(strstr(json, "Gripe") != NULL);

    /* Paciente sem historico -> retorna 1 com listas vazias. */
    assert(triagem_service_historico_json(2, json, sizeof(json)) == 1);
    assert(strstr(json, "\"prontuarios\":[]") != NULL);
    assert(strstr(json, "\"exames\":[]") != NULL);

    /* --- Sugestao de exames iniciais por tipo de triagem --- */
    assert(db_resetar_com_schema(SCHEMA) == 1);
    assert(triagem_repo_criar(1, 3, 8, "Emergencia") == 1); /* Cardiologia */
    assert(triagem_service_sugerir_exames_json(1, json, sizeof(json)) == 1);
    assert(strstr(json, "Eletrocardiograma") != NULL);
    assert(triagem_service_sugerir_exames_json(999, json, sizeof(json)) == 0);

    /* --- Agendamento automatico --- */
    assert(db_resetar_com_schema(SCHEMA) == 1);
    assert(paciente_repo_criar("Joao", "11122233344", 40, "61999990000", "M", 7) == 1);
    assert(triagem_repo_criar(1, 3, 8, "Emergencia") == 1); /* Cardiologia */
    assert(medico_repo_criar("Dr Cardio", "CRM1", "Cardiologia", 7) == 1); /* id 1 */

    /* Horario livre -> agenda com o medico 1. */
    assert(triagem_service_agendar_json(1, "2026-06-20", "09:00", json, sizeof(json)) == 1);
    assert(strstr(json, "\"agendado\":true") != NULL);
    assert(strstr(json, "\"medicoId\":1") != NULL);
    assert(agendamento_repo_contar_ativos() == 1);

    /* Mesmo horario, unico medico ocupado -> nao agenda. */
    assert(triagem_service_agendar_json(1, "2026-06-20", "09:00", json, sizeof(json)) == 0);
    assert(strstr(json, "\"agendado\":false") != NULL);
    assert(agendamento_repo_contar_ativos() == 1);

    /* Outro horario livre -> agenda de novo. */
    assert(triagem_service_agendar_json(1, "2026-06-20", "10:00", json, sizeof(json)) == 1);
    assert(agendamento_repo_contar_ativos() == 2);

    /* --- Encaminhamento para outra especialidade --- */
    assert(db_resetar_com_schema(SCHEMA) == 1);
    assert(paciente_repo_criar("Joao", "11122233344", 40, "61999990000", "M", 7) == 1);
    assert(medico_repo_criar("Dr Orto", "CRM9", "Ortopedia", 7) == 1); /* id 1 */

    /* Encaminha para Ortopedia (independe da triagem) -> agenda. */
    assert(triagem_service_encaminhar_json(1, "Ortopedia", "2026-06-22", "08:00",
                                           json, sizeof(json)) == 1);
    assert(strstr(json, "\"encaminhado\":true") != NULL);
    assert(strstr(json, "\"especialidade\":\"Ortopedia\"") != NULL);
    assert(strstr(json, "\"medicoId\":1") != NULL);
    assert(agendamento_repo_contar_ativos() == 1);

    /* Especialidade sem medico na regiao -> nao encaminha. */
    assert(triagem_service_encaminhar_json(1, "Neurologia", "2026-06-22", "08:00",
                                           json, sizeof(json)) == 0);
    assert(strstr(json, "\"encaminhado\":false") != NULL);

    printf("test_triagem_service: OK\n");
    return 0;
}
