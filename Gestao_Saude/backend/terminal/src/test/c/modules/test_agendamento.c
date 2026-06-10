#include <assert.h>
#include <string.h>
#include "agendamento.h"
#include "sqlite_db.h"

Paciente pacientes[MAX_PACIENTES];
Medico medicos[MAX_MEDICOS];
Agendamento agendamentos[MAX_AGENDAMENTOS];
Triagem triagens[MAX_TRIAGENS];
Prontuario prontuarios[MAX_PRONTUARIOS];

Ala alas[MAX_ALAS];
Leito leitos[MAX_LEITOS];
Internacao internacoes[MAX_INTERNACOES];

int totalPacientes = 0;
int totalMedicos = 0;
int totalAgendamentos = 0;
int totalTriagens = 0;
int totalProntuarios = 0;

int totalAlas = 0;
int totalLeitos = 0;
int totalInternacoes = 0;

static void prepararPaciente(int id, int regiao)
{
    pacientes[totalPacientes].id = id;
    strcpy(pacientes[totalPacientes].nome, "Paciente Teste");
    strcpy(pacientes[totalPacientes].cpf, "000.000.000-00");
    pacientes[totalPacientes].idade = 30;
    strcpy(pacientes[totalPacientes].telefone, "(61) 99999-0000");
    pacientes[totalPacientes].sexo = 'F';
    pacientes[totalPacientes].regiaoAdministrativa = regiao;
    pacientes[totalPacientes].ativo = 1;
    totalPacientes++;
}

static void prepararMedico(int id, const char especialidade[], int regiao, int ativo)
{
    medicos[totalMedicos].id = id;
    strcpy(medicos[totalMedicos].nome, "Medico Teste");
    strcpy(medicos[totalMedicos].crm, "12345");
    strcpy(medicos[totalMedicos].especialidade, especialidade);
    medicos[totalMedicos].regiaoAdministrativa = regiao;
    medicos[totalMedicos].ativo = ativo;
    totalMedicos++;
}

static void prepararTriagem(int id, int pacienteId, int tipoTriagem, const char classificacao[], int ativo)
{
    triagens[totalTriagens].id = id;
    triagens[totalTriagens].pacienteId = pacienteId;
    triagens[totalTriagens].tipoTriagem = tipoTriagem;
    triagens[totalTriagens].pontuacao = 0;
    strcpy(triagens[totalTriagens].classificacao, classificacao);
    triagens[totalTriagens].ativo = ativo;
    totalTriagens++;
}

static void prepararAgendamento(int id, int pacienteId, int medicoId, const char data[],
                                const char horario[], const char status[])
{
    agendamentos[totalAgendamentos].id = id;
    agendamentos[totalAgendamentos].pacienteId = pacienteId;
    agendamentos[totalAgendamentos].medicoId = medicoId;
    strcpy(agendamentos[totalAgendamentos].data, data);
    strcpy(agendamentos[totalAgendamentos].horario, horario);
    strcpy(agendamentos[totalAgendamentos].status, status);
    totalAgendamentos++;
}

static void resetarDados(void)
{
    totalPacientes = 0;
    totalMedicos = 0;
    totalAgendamentos = 0;
    totalTriagens = 0;
    totalProntuarios = 0;
    totalAlas = 0;
    totalLeitos = 0;
    totalInternacoes = 0;
}

int main(void)
{
    int agendamentoId;
    int medicoSelecionado;
    Agendamento lista[MAX_AGENDAMENTOS];
    int totalCopiados;

    assert(definirCaminhoBancoSQLite("/tmp/sigeh_test_agendamento.db") == 1);
    assert(reinicializarBancoSQLite() == 1);
    assert(executarSQLSQLite("INSERT INTO pacientes (id, nome, cpf, idade, telefone, sexo, regiao_administrativa, ativo) VALUES (501, 'Paciente Agenda', '501.501.501-50', 30, '(61) 95010-0000', 'F', 2, 1);") == 1);
    assert(executarSQLSQLite("INSERT INTO medicos (id, nome, crm, especialidade, regiao_administrativa, ativo) VALUES (601, 'Medico Agenda', '601', 'Ortopedia', 2, 1);") == 1);

    assert(strcmp(obterEspecialidade(TRIAGEM_ORTOPEDIA), "Ortopedia") == 0);
    assert(strcmp(obterEspecialidade(TRIAGEM_CARDIOLOGIA), "Cardiologia") == 0);
    assert(strcmp(obterEspecialidade(TRIAGEM_PNEUMOLOGIA), "Pneumologia") == 0);
    assert(strcmp(obterEspecialidade(TRIAGEM_PEDIATRIA), "Pediatria") == 0);
    assert(strcmp(obterEspecialidade(TRIAGEM_GERAL), "Clinico Geral") == 0);

    resetarDados();
    prepararPaciente(1, 2);
    prepararMedico(1, "Ortopedia", 2, 1);
    prepararMedico(2, "Ortopedia", 3, 1);
    prepararMedico(3, "Cardiologia", 2, 1);
    prepararTriagem(1, 1, TRIAGEM_ORTOPEDIA, "Emergencia", 1);

    assert(buscarMedicoRegiao("Ortopedia", 2, "10/06/2026", "08:00") == 1);
    assert(buscarMedico("Ortopedia", "10/06/2026", "08:00") == 1);

    prepararAgendamento(1, 99, 1, "10/06/2026", "08:00", "AGENDADO");

    assert(medicoOcupado(1, "10/06/2026", "08:00") == 1);
    assert(buscarAgenda(1, "10/06/2026", "08:00") == 0);
    assert(buscarMedicoRegiao("Ortopedia", 2, "10/06/2026", "08:00") == 0);
    assert(buscarMedico("Ortopedia", "10/06/2026", "08:00") == 2);

    assert(cancelarAgendamento(1) == 1);
    assert(strcmp(agendamentos[0].status, "CANCELADO") == 0);
    assert(medicoOcupado(1, "10/06/2026", "08:00") == 0);
    assert(buscarAgenda(1, "10/06/2026", "08:00") == -1);

    resetarDados();
    prepararPaciente(1, 2);
    prepararMedico(1, "Ortopedia", 2, 1);
    prepararAgendamento(1, 1, 1, "10/06/2026", "09:30", "AGENDADO");

    assert(concluirAgendamento(1) == 1);
    assert(strcmp(agendamentos[0].status, "CONCLUIDO") == 0);
    assert(totalProntuarios == 1);
    assert(prontuarios[0].pacienteId == 1);
    assert(prontuarios[0].medicoId == 1);
    assert(strcmp(prontuarios[0].data, "10/06/2026") == 0);
    assert(strcmp(prontuarios[0].observacoes, "Prontuario base criado automaticamente pelo sistema.") == 0);
    assert(strcmp(prontuarios[0].diagnostico, "Aguardando avaliacao medica.") == 0);
    assert(strcmp(prontuarios[0].conduta, "Registrar evolucao do atendimento.") == 0);
    assert(prontuarios[0].alertaImportante == 0);
    assert(prontuarios[0].ativo == 1);
    assert(concluirAgendamento(1) == 0);
    assert(totalProntuarios == 1);

    resetarDados();
    prepararPaciente(1, 2);
    prepararMedico(1, "Ortopedia", 2, 1);
    prepararTriagem(1, 1, TRIAGEM_ORTOPEDIA, "Emergencia", 1);

    assert(criarAgendamentoTriagem(1, "11/06/2026", "09:00", &agendamentoId, &medicoSelecionado) == 1);
    assert(agendamentoId == 1);
    assert(medicoSelecionado == 1);
    assert(totalAgendamentos == 1);
    assert(agendamentos[0].pacienteId == 1);
    assert(agendamentos[0].medicoId == 1);
    assert(strcmp(agendamentos[0].status, "AGENDADO") == 0);
    totalCopiados = copiarAgendamentos(lista, MAX_AGENDAMENTOS);
    assert(totalCopiados == 1);
    assert(lista[0].id == 1);
    assert(lista[0].pacienteId == 1);
    assert(lista[0].medicoId == 1);
    assert(strcmp(lista[0].data, "11/06/2026") == 0);
    assert(strcmp(lista[0].horario, "09:00") == 0);
    assert(strcmp(lista[0].status, "AGENDADO") == 0);
    assert(copiarAgendamentos(NULL, MAX_AGENDAMENTOS) == 0);
    assert(copiarAgendamentos(lista, 0) == 0);

    resetarDados();
    prepararPaciente(1, 2);
    prepararPaciente(2, 3);
    prepararPaciente(3, 4);
    prepararMedico(1, "Ortopedia", 2, 1);
    prepararMedico(2, "Cardiologia", 3, 1);
    prepararAgendamento(1, 1, 1, "11/06/2026", "08:00", "AGENDADO");
    prepararAgendamento(2, 1, 2, "11/06/2026", "09:00", "CONCLUIDO");
    prepararAgendamento(3, 3, 2, "11/06/2026", "10:00", "CANCELADO");

    totalCopiados = copiarAgendamentosPorPaciente(1, lista, MAX_AGENDAMENTOS);
    assert(totalCopiados == 2);
    assert(lista[0].pacienteId == 1);
    assert(lista[1].pacienteId == 1);
    assert(lista[0].medicoId == 1);
    assert(lista[1].medicoId == 2);

    totalCopiados = copiarAgendamentosPorMedico(2, lista, MAX_AGENDAMENTOS);
    assert(totalCopiados == 2);
    assert(lista[0].medicoId == 2);
    assert(lista[1].medicoId == 2);
    assert(lista[0].pacienteId == 1);
    assert(lista[1].pacienteId == 3);

    assert(copiarAgendamentosPorPaciente(1, NULL, MAX_AGENDAMENTOS) == 0);
    assert(copiarAgendamentosPorMedico(2, lista, 0) == 0);

    assert(reinicializarBancoSQLite() == 1);
    assert(executarSQLSQLite("INSERT INTO pacientes (id, nome, cpf, idade, telefone, sexo, regiao_administrativa, ativo) VALUES (501, 'Paciente Agenda', '501.501.501-50', 30, '(61) 95010-0000', 'F', 2, 1);") == 1);
    assert(executarSQLSQLite("INSERT INTO medicos (id, nome, crm, especialidade, regiao_administrativa, ativo) VALUES (601, 'Medico Agenda', '601', 'Ortopedia', 2, 1);") == 1);

    resetarDados();
    prepararAgendamento(501, 501, 601, "21/06/2026", "08:30", "AGENDADO");
    assert(salvarAgendamentoNoBanco(&agendamentos[0]) == 1);
    assert(salvarAgendamentoNoBanco(NULL) == 0);
    totalCopiados = carregarAgendamentosDoBanco(lista, MAX_AGENDAMENTOS);
    assert(totalCopiados == 1);
    assert(lista[0].id == 501);
    assert(lista[0].pacienteId == 501);
    assert(lista[0].medicoId == 601);
    assert(strcmp(lista[0].status, "AGENDADO") == 0);

    resetarDados();
    prepararPaciente(1, 2);
    prepararMedico(1, "Ortopedia", 2, 1);
    prepararTriagem(1, 1, TRIAGEM_ORTOPEDIA, "Emergencia", 1);

    assert(agendarTriagem(1, "11/06/2026", "09:00") == 1);
    assert(totalAgendamentos == 1);
    assert(agendamentos[0].pacienteId == 1);
    assert(agendamentos[0].medicoId == 1);
    assert(strcmp(agendamentos[0].status, "AGENDADO") == 0);

    resetarDados();
    prepararPaciente(1, 2);
    prepararPaciente(2, 2);
    prepararMedico(1, "Ortopedia", 2, 1);
    prepararTriagem(1, 1, TRIAGEM_ORTOPEDIA, "Emergencia", 1);
    prepararTriagem(2, 2, TRIAGEM_ORTOPEDIA, "Comum", 1);
    prepararAgendamento(1, 2, 1, "12/06/2026", "10:00", "AGENDADO");

    assert(trocaHorario(1, 2) == 1);
    assert(agendarTriagem(1, "12/06/2026", "10:00") == 1);
    assert(totalAgendamentos == 2);
    assert(strcmp(agendamentos[0].status, "REMANEJADO") == 0);
    assert(agendamentos[1].pacienteId == 1);
    assert(agendamentos[1].medicoId == 1);
    assert(strcmp(agendamentos[1].status, "AGENDADO") == 0);
    totalCopiados = copiarAgendamentos(lista, MAX_AGENDAMENTOS);
    assert(totalCopiados == 2);
    assert(strcmp(lista[0].status, "REMANEJADO") == 0);
    assert(strcmp(lista[1].status, "AGENDADO") == 0);

    resetarDados();
    prepararPaciente(1, 2);
    prepararPaciente(2, 2);
    prepararMedico(1, "Ortopedia", 2, 1);
    prepararTriagem(1, 1, TRIAGEM_ORTOPEDIA, "Comum", 1);
    prepararTriagem(2, 2, TRIAGEM_ORTOPEDIA, "Emergencia", 1);
    prepararAgendamento(1, 2, 1, "13/06/2026", "10:00", "AGENDADO");

    assert(trocaHorario(1, 2) == 0);
    assert(agendarTriagem(1, "13/06/2026", "10:00") == 0);
    assert(totalAgendamentos == 1);
    assert(strcmp(agendamentos[0].status, "AGENDADO") == 0);

    resetarDados();
    prepararPaciente(1, 2);
    prepararPaciente(2, 2);
    prepararMedico(1, "Clinico Geral", 2, 1);
    prepararTriagem(1, 1, TRIAGEM_GERAL, "Emergencia", 1);
    prepararTriagem(2, 2, TRIAGEM_GERAL, "Muito prioritario", 1);
    prepararAgendamento(1, 2, 1, "14/06/2026", "11:00", "AGENDADO");

    assert(trocaHorario(1, 2) == 1);
    assert(agendarMedico(1, 1, "14/06/2026", "11:00") == 1);
    assert(totalAgendamentos == 2);
    assert(strcmp(agendamentos[0].status, "REMANEJADO") == 0);
    assert(agendamentos[1].pacienteId == 1);
    assert(strcmp(agendamentos[1].status, "AGENDADO") == 0);

    resetarDados();
    prepararPaciente(1, 2);
    prepararPaciente(2, 2);
    prepararMedico(1, "Clinico Geral", 2, 1);
    prepararTriagem(1, 1, TRIAGEM_GERAL, "Muito prioritario", 1);
    prepararTriagem(2, 2, TRIAGEM_GERAL, "Muito prioritario", 1);
    prepararAgendamento(1, 2, 1, "15/06/2026", "11:00", "AGENDADO");

    assert(trocaHorario(1, 2) == 0);
    assert(agendarMedico(1, 1, "15/06/2026", "11:00") == 0);
    assert(totalAgendamentos == 1);
    assert(strcmp(agendamentos[0].status, "AGENDADO") == 0);

    resetarDados();
    prepararPaciente(1, 2);
    prepararPaciente(2, 2);
    prepararMedico(1, "Ortopedia", 2, 1);
    prepararMedico(2, "Ortopedia", 3, 1);
    prepararTriagem(1, 1, TRIAGEM_ORTOPEDIA, "Emergencia", 1);
    prepararTriagem(2, 2, TRIAGEM_ORTOPEDIA, "Emergencia", 1);
    prepararAgendamento(1, 2, 1, "16/06/2026", "11:00", "AGENDADO");

    assert(agendarTriagem(1, "16/06/2026", "11:00") == 2);
    assert(totalAgendamentos == 2);
    assert(strcmp(agendamentos[0].status, "AGENDADO") == 0);
    assert(agendamentos[1].medicoId == 2);

    resetarDados();
    prepararPaciente(1, 2);
    prepararMedico(2, "Ortopedia", 3, 1);
    prepararTriagem(1, 1, TRIAGEM_ORTOPEDIA, "Muito prioritario", 1);

    assert(agendarTriagem(1, "17/06/2026", "12:00") == 2);
    assert(totalAgendamentos == 1);
    assert(agendamentos[0].medicoId == 2);

    resetarDados();
    prepararPaciente(1, 2);
    prepararPaciente(2, 3);
    prepararMedico(2, "Ortopedia", 3, 1);
    prepararTriagem(1, 1, TRIAGEM_ORTOPEDIA, "Muito prioritario", 1);
    prepararTriagem(2, 2, TRIAGEM_ORTOPEDIA, "Orientacao basica", 1);
    prepararAgendamento(1, 2, 2, "18/06/2026", "13:00", "AGENDADO");

    assert(agendarTriagem(1, "18/06/2026", "13:00") == 2);
    assert(totalAgendamentos == 2);
    assert(strcmp(agendamentos[0].status, "REMANEJADO") == 0);
    assert(agendamentos[1].pacienteId == 1);

    resetarDados();
    prepararPaciente(1, 2);
    prepararMedico(1, "Cardiologia", 2, 1);
    prepararTriagem(1, 1, TRIAGEM_ORTOPEDIA, "Emergencia", 1);

    assert(agendarTriagem(1, "19/06/2026", "14:00") == 0);

    resetarDados();
    prepararPaciente(1, 2);
    prepararMedico(1, "Ortopedia", 2, 1);

    assert(agendarTriagem(1, "20/06/2026", "15:00") == 0);

    return 0;
}
