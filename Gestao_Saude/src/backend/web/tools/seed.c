/*
 * Ferramenta de seed do SIGEH-DF.
 * Recria o banco oficial a partir do schema versionado e popula dados minimos
 * para o sistema ser usavel: um usuario por papel (com senha com hash) e alguns
 * cadastros de exemplo. Reaproveita os repositories reais do backend.
 *
 * Uso (a partir de src/backend/web/):
 *   ./build/seed [caminho_schema] [caminho_banco]
 * Padroes: ../data/schema_v3.sql e ../data/sigeh_v3.db (caminho default do db).
 */

#include "database.h"
#include "usuario_repository.h"
#include "paciente_repository.h"
#include "medico_repository.h"
#include "ala_repository.h"
#include "leito_repository.h"
#include "checkin_repository.h"
#include "triagem_repository.h"
#include "agendamento_repository.h"
#include "prontuario_repository.h"
#include "exame_repository.h"
#include "analito_repository.h"
#include "financeiro_repository.h"
#include "lote_repository.h"
#include "medicamento_repository.h"
#include "estoque_repository.h"
#include "prescricao_repository.h"
#include "enfermagem_repository.h"
#include "farmacia_service.h"
#include "vacina_repository.h"
#include "internacao_repository.h"
#include "solicitacao_repository.h"

#include <stdio.h>
#include <stdlib.h>

#define SCHEMA_PADRAO "../data/schema_v3.sql"

static int etapa_ok(const char *etapa, int ok)
{
    if (!ok)
    {
        fprintf(stderr, "falha ao popular etapa: %s\n", etapa);
        return 0;
    }
    printf("  ok: %s\n", etapa);
    return 1;
}

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
    char senha[16];
    char json[1024];
    int agendamentoId = 0;
    int medicoId = 0;
    int aplicacaoId = 0;
    int loteFinanceiroId = 0;

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

    /* Cadastros-base. O portal de paciente so oferta especialidades com medico
     * ativo, entao deixamos varias especialidades disponiveis para testes. */
    if (!etapa_ok("medicos",
        medico_repo_criar("Dra. Helena Prado", "CRM-DF 1001", "Clinico Geral", 1) == 1 &&
        medico_repo_criar("Dr. Marcos Vieira", "CRM-DF 1002", "Cardiologia", 1) == 1 &&
        medico_repo_criar("Dra. Livia Rocha", "CRM-DF 1003", "Ortopedia", 2) == 1 &&
        medico_repo_criar("Dr. Paulo Nunes", "CRM-DF 1004", "Pediatria", 5) == 1 &&
        medico_repo_criar("Dra. Aline Torres", "CRM-DF 1005", "Pneumologia", 1) == 1 &&
        medico_repo_criar("Dr. Renato Lima", "CRM-DF 1006", "Neurologia", 3) == 1 &&
        medico_repo_criar("Dra. Camila Bento", "CRM-DF 1007", "Gastroenterologia", 4) == 1))
    {
        return 1;
    }

    if (!etapa_ok("pacientes",
        paciente_repo_criar("Joao da Silva", "1981-05-12", "12345678900", "CPF",
                            "61999990000", "M", 1, "", "Penicilina") == 1 &&
        paciente_repo_criar("Maria Oliveira", "1990-08-22", "98765432100", "CPF",
                            "61988887777", "F", 1, "", "") == 1 &&
        paciente_repo_criar("Ana Souza", "2016-04-09", "55566677788", "CPF",
                            "61977776666", "F", 5, "Carla Souza", "Dipirona") == 1 &&
        paciente_repo_criar("Carlos Mendes", "1974-11-30", "44433322211", "CPF",
                            "61966665555", "M", 2, "", "") == 1 &&
        paciente_repo_criar("Beatriz Lima", "1955-02-18", "33322211100", "CPF",
                            "61955554444", "F", 3, "", "Iodo") == 1))
    {
        return 1;
    }

    if (!etapa_ok("alas e leitos",
        ala_repo_criar("Ala A - Clinica Medica", 1, 4) == 1 &&
        ala_repo_criar("Ala B - Pediatria", 2, 3) == 1 &&
        leito_repo_criar(1, 101) == 1 &&
        leito_repo_criar(1, 102) == 1 &&
        leito_repo_criar(1, 103) == 1 &&
        leito_repo_criar(1, 104) == 1 &&
        leito_repo_criar(2, 201) == 1 &&
        leito_repo_criar(2, 202) == 1 &&
        leito_repo_criar(2, 203) == 1 &&
        leito_repo_registrar_status(4, "MANUTENCAO", "seed") == 1))
    {
        return 1;
    }

    /* Um usuario por papel. PACIENTE/MEDICO ficam vinculados aos cadastros. */
    printf("Usuarios criados:\n");
    if (!criar_usuario("Administrador", "admin", "admin123", "ADMIN", 0, 0) ||
        !criar_usuario("Recepcao", "cadastro", "cadastro123", "CADASTRO", 0, 0) ||
        !criar_usuario("Dra. Helena Prado", "medico", "medico123", "MEDICO", 0, 1) ||
        !criar_usuario("Equipe Enfermagem", "enfermagem", "enfermagem123",
                       "ENFERMAGEM", 0, 0) ||
        !criar_usuario("Joao da Silva", "paciente", "paciente123", "PACIENTE", 1, 0) ||
        !criar_usuario("Maria Oliveira", "paciente2", "paciente123", "PACIENTE", 2, 0))
    {
        return 1;
    }

    /* Contas de demonstracao nao forcam troca no 1o acesso (mantem as
     * credenciais documentadas utilizaveis). A troca obrigatoria vale para
     * usuarios criados depois pelo admin. */
    db_executar("UPDATE usuarios SET trocar_senha = 0;");

    /* Recepcao/fila: uma senha em atendimento, uma aguardando e uma encerrada. */
    if (!etapa_ok("recepcao e check-in",
        checkin_repo_criar(1, "TRIAGEM", senha, sizeof(senha)) == 1 &&
        checkin_repo_chamar(1) == 1 &&
        checkin_repo_criar(2, "CONSULTA", senha, sizeof(senha)) == 1 &&
        checkin_repo_criar(3, "TRIAGEM", senha, sizeof(senha)) == 1 &&
        checkin_repo_chamar(3) == 1 &&
        checkin_repo_encerrar(3) == 1))
    {
        return 1;
    }

    if (!etapa_ok("triagem clinica",
        triagem_repo_criar_clinica(1, 4, 1, 3, "Pouco prioritario",
            "febre,mal-estar", "Febre e mal-estar ha 2 dias",
            "Hidratado, sem sinais de gravidade", "120/80", "37.8", "82", "98") > 0 &&
        triagem_repo_adicionar_problema(1, 22, 1, "febre") == 1 &&
        triagem_repo_criar_clinica(2, 4, 3, 5, "Emergencia",
            "dor_toracica,falta_ar", "Dor no peito e falta de ar",
            "Encaminhar para cardiologia", "150/95", "36.7", "108", "94") > 0 &&
        triagem_repo_adicionar_problema(2, 1, 1, "dor no peito") == 1 &&
        triagem_repo_criar_clinica(3, 4, 5, 3, "Prioritario",
            "febre_infantil,vomito", "Febre infantil com vomitos",
            "Acompanhada pela responsavel", "100/60", "38.4", "110", "97") > 0 &&
        triagem_repo_adicionar_problema(3, 17, 1, "febre infantil") == 1 &&
        triagem_repo_criar_clinica(4, 4, 2, 4, "Muito prioritario",
            "fratura_suspeita", "Queda com dor intensa no tornozelo",
            "Imobilizar e solicitar imagem", "130/85", "36.5", "96", "99") > 0 &&
        triagem_repo_adicionar_problema(4, 8, 1, "fratura suspeita") == 1))
    {
        return 1;
    }

    if (!etapa_ok("agenda de consultas",
        agendamento_repo_criar_por_especialidade(1, "Clinico Geral", "2026-07-01", "08:00", &agendamentoId, &medicoId) == 1 &&
        agendamento_repo_criar_por_especialidade(2, "Cardiologia", "2026-07-01", "09:00", &agendamentoId, &medicoId) == 1 &&
        agendamento_repo_criar_por_especialidade(3, "Pediatria", "2026-07-02", "10:00", &agendamentoId, &medicoId) == 1 &&
        agendamento_repo_criar_por_especialidade(4, "Ortopedia", "2026-07-03", "11:00", &agendamentoId, &medicoId) == 1 &&
        agendamento_repo_criar_por_especialidade(5, "Neurologia", "2026-07-04", "14:00", &agendamentoId, &medicoId) == 1))
    {
        return 1;
    }

    if (!etapa_ok("prontuarios e prescricoes",
        prontuario_repo_criar(1, 1, "2026-07-01", "Consulta inicial",
            "Sindrome gripal leve", "Hidratacao e sintomaticos", 0) == 1 &&
        prontuario_repo_criar(2, 2, "2026-07-01", "Atendimento cardiologico",
            "Dor toracica em investigacao", "Solicitar ECG e exames laboratoriais", 1) == 1 &&
        prontuario_repo_criar(4, 3, "2026-07-03", "Avaliacao ortopedica",
            "Entorse de tornozelo", "Imobilizacao e analgesia", 0) == 1 &&
        prescricao_repo_criar(1, 1, "Paracetamol", "750mg", "8/8h", "VO",
            "5 dias", "Usar se dor ou febre") == 1 &&
        prescricao_repo_criar(2, 2, "AAS", "100mg", "1x/dia", "VO",
            "15 dias", "Apos avaliacao cardiologica") == 1 &&
        administracao_criar(1, 4, "enfermagem", "Dose administrada sem intercorrencias") == 1 &&
        evolucao_criar(4, "enfermagem", "Paciente orientado, dor controlada apos imobilizacao.",
            "125/82", "36.6", "88", "98") == 1))
    {
        return 1;
    }

    if (!etapa_ok("laboratorio e exames",
        analito_criar("HGB", "Hemoglobina", "g/dL", 12.0, 16.0, "Automatizado") == 1 &&
        analito_criar("GLI", "Glicemia", "mg/dL", 70.0, 99.0, "Enzimatico") == 1 &&
        analito_criar("PCR", "Proteina C Reativa", "mg/L", 0.0, 5.0, "Imunoturbidimetria") == 1 &&
        painel_adicionar_analito(1, 1, 1) == 1 &&
        painel_adicionar_analito(1, 2, 2) == 1 &&
        painel_adicionar_analito(1, 3, 3) == 1 &&
        exame_repo_criar(2, 2, 2, 1, "2026-07-01", 1) == 1 &&
        exame_repo_atualizar_status(1, "AUTORIZADO") == 1 &&
        exame_repo_atualizar_status(1, "COLETADO") == 1 &&
        exame_repo_registrar_resultado_analito(1, 1, 13.8, "", "") == 1 &&
        exame_repo_registrar_resultado_analito(1, 2, 112.0, "", "Levemente elevada") == 1 &&
        exame_repo_registrar_resultado_analito(1, 3, 8.5, "", "Inflamacao discreta") == 1 &&
        exame_repo_registrar_resultado(1, "Sem evidencia de evento agudo; manter acompanhamento.", 0) == 1 &&
        exame_repo_criar(4, 3, 3, 3, "2026-07-03", 0) == 1))
    {
        return 1;
    }

    if (!etapa_ok("financeiro",
        convenio_criar("Saude DF Basico", 80) == 1 &&
        convenio_criar("Particular Assistido", 0) == 1 &&
        paciente_repo_definir_convenio(2, 1) == 1 &&
        paciente_repo_definir_convenio(5, 1) == 1 &&
        cobranca_criar(1, 0, "PARTICULAR", "consulta", "Consulta clinica particular",
            18000, "2026-07-10", "", "") == 1 &&
        cobranca_atualizar_status(1, "PAGA", "") == 1 &&
        cobranca_criar(2, 1, "CONVENIO", "consulta", "Consulta cardiologica",
            32000, "2026-07-20", "GUIA-DF-001", "2026-08-20") == 1 &&
        cobranca_atualizar_status(2, "AUTORIZADA", "") == 1 &&
        (loteFinanceiroId = lote_criar(1)) > 0 &&
        lote_adicionar_cobranca(loteFinanceiroId, 2) == 1 &&
        lote_fechar(loteFinanceiroId) == 1))
    {
        return 1;
    }

    if (!etapa_ok("farmacia e estoque",
        medicamento_criar("Dipirona", "500mg comprimido", "comprimido", 20, 150) == 1 &&
        medicamento_criar("Amoxicilina", "500mg capsula", "capsula", 15, 280) == 1 &&
        medicamento_criar("Vacina Influenza", "dose unica", "dose", 5, 0) == 1 &&
        medicamento_criar("Insulina NPH", "frasco 10ml", "frasco", 8, 4200) == 1 &&
        estoque_entrada(1, "DIP-A1", "2027-01-31", 120, "Farmacia A", 1, "admin") == 1 &&
        estoque_entrada(2, "AMX-B1", "2027-03-31", 60, "Farmacia A", 1, "admin") == 1 &&
        estoque_entrada(3, "FLU-26", "2026-12-31", 40, "Geladeira 1", 1, "admin") == 1 &&
        estoque_entrada(4, "INS-N1", "2026-09-30", 6, "Geladeira 2", 1, "admin") == 1 &&
        farmacia_service_dispensar(1, 1, 10, "dispensacao pos-consulta", 4,
            "enfermagem", json, sizeof(json)) == 1))
    {
        return 1;
    }

    if (!etapa_ok("vacinacao",
        vacina_criar("Influenza campanha 2026", "Butantan", "Gripe", 1, 0, 365, 3) == 1 &&
        vacina_aplicar(1, 1, 1, "FLU-26", "2026-12-31", 4, "enfermagem",
            "Aplicada em campanha interna", &aplicacaoId) == 1 &&
        vacina_aplicar(2, 1, 1, "FLU-26", "2026-12-31", 4, "enfermagem",
            "Aplicada antes da consulta cardiologica", &aplicacaoId) == 1))
    {
        return 1;
    }

    if (!etapa_ok("internacao e enfermagem",
        internacao_repo_criar(5, 1, 1, "2026-07-02", "Dra. Helena Prado") == 1 &&
        internacao_repo_criar(3, 2, 5, "2026-07-03", "Dr. Paulo Nunes") == 1 &&
        internacao_repo_dar_alta(2, "2026-07-05", "Quadro febril controlado",
            "Virose infantil", "Retornar se febre persistir") == 1 &&
        evolucao_criar(5, "enfermagem", "Paciente em observacao, sem queixas no momento.",
            "135/84", "36.8", "78", "97") == 1))
    {
        return 1;
    }

    if (!etapa_ok("solicitacoes do paciente",
        solicitacao_repo_criar(1, "AGENDAMENTO", "Desejo reagendar retorno clinico.", NULL) == 1 &&
        solicitacao_repo_criar(2, "AJUDA", "Tenho duvida sobre preparo do exame.", NULL) == 1))
    {
        return 1;
    }

    /* Garante o banco na ultima versao de schema (no-op se ja estiver). */
    if (db_migrar() != 1)
    {
        fprintf(stderr, "falha ao aplicar migracoes de schema\n");
        return 1;
    }

    printf("Seed concluido.\n");
    return 0;
}
