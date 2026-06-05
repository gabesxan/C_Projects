#include <assert.h>
#include <string.h>
#include "agendamento.h"

Paciente pacientes[MAX_PACIENTES];
Medico medicos[MAX_MEDICOS];
Agendamento agendamentos[MAX_AGENDAMENTOS];
Triagem triagens[MAX_TRIAGENS];

Ala alas[MAX_ALAS];
Leito leitos[MAX_LEITOS];
Internacao internacoes[MAX_INTERNACOES];

int totalPacientes = 0;
int totalMedicos = 0;
int totalAgendamentos = 0;
int totalTriagens = 0;

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

static void prepararTriagem(int id, int pacienteId, int tipoTriagem, int ativo)
{
    triagens[totalTriagens].id = id;
    triagens[totalTriagens].pacienteId = pacienteId;
    triagens[totalTriagens].tipoTriagem = tipoTriagem;
    triagens[totalTriagens].pontuacao = 8;
    strcpy(triagens[totalTriagens].classificacao, "Emergencia");
    triagens[totalTriagens].ativo = ativo;
    totalTriagens++;
}

static void prepararAgendamento(int id, int pacienteId, int medicoId, const char data[], const char horario[], const char status[])
{
    agendamentos[totalAgendamentos].id = id;
    agendamentos[totalAgendamentos].pacienteId = pacienteId;
    agendamentos[totalAgendamentos].medicoId = medicoId;
    strcpy(agendamentos[totalAgendamentos].data, data);
    strcpy(agendamentos[totalAgendamentos].horario, horario);
    strcpy(agendamentos[totalAgendamentos].status, status);
    totalAgendamentos++;
}

int main(void)
{
    totalPacientes = 0;
    totalMedicos = 0;
    totalAgendamentos = 0;
    totalTriagens = 0;

    assert(strcmp(especialidadeTriagem(TRIAGEM_ORTOPEDIA), "Ortopedia") == 0);
    assert(strcmp(especialidadeTriagem(TRIAGEM_CARDIOLOGIA), "Cardiologia") == 0);
    assert(strcmp(especialidadeTriagem(TRIAGEM_PNEUMOLOGIA), "Pneumologia") == 0);
    assert(strcmp(especialidadeTriagem(TRIAGEM_PEDIATRIA), "Pediatria") == 0);
    assert(strcmp(especialidadeTriagem(TRIAGEM_GERAL), "Clinico Geral") == 0);

    prepararPaciente(1, 2);
    prepararMedico(1, "Ortopedia", 2, 1);
    prepararMedico(2, "Ortopedia", 3, 1);
    prepararMedico(3, "Cardiologia", 2, 1);
    prepararTriagem(1, 1, TRIAGEM_ORTOPEDIA, 1);

    assert(buscarMedicoRegiao("Ortopedia", 2, "10/06/2026", "08:00") == 1);
    assert(buscarMedico("Ortopedia", "10/06/2026", "08:00") == 1);

    prepararAgendamento(1, 99, 1, "10/06/2026", "08:00", "AGENDADO");

    assert(conflitoMedico(1, "10/06/2026", "08:00") == 1);
    assert(buscarMedicoRegiao("Ortopedia", 2, "10/06/2026", "08:00") == 0);
    assert(buscarMedico("Ortopedia", "10/06/2026", "08:00") == 2);

    assert(cancelarAgendamento(1) == 1);
    assert(strcmp(agendamentos[0].status, "CANCELADO") == 0);
    assert(conflitoMedico(1, "10/06/2026", "08:00") == 0);

    totalAgendamentos = 0;
    assert(agendarTriagem(1, "11/06/2026", "09:00") == 1);
    assert(totalAgendamentos == 1);
    assert(agendamentos[0].pacienteId == 1);
    assert(agendamentos[0].medicoId == 1);
    assert(strcmp(agendamentos[0].status, "AGENDADO") == 0);

    totalAgendamentos = 0;
    prepararAgendamento(1, 77, 1, "12/06/2026", "10:00", "AGENDADO");
    assert(agendarTriagem(1, "12/06/2026", "10:00") == 2);

    totalPacientes = 0;
    totalMedicos = 0;
    totalAgendamentos = 0;
    totalTriagens = 0;

    prepararPaciente(1, 2);
    prepararMedico(1, "Cardiologia", 2, 1);
    prepararTriagem(1, 1, TRIAGEM_ORTOPEDIA, 1);

    assert(agendarTriagem(1, "13/06/2026", "11:00") == 0);

    totalPacientes = 0;
    totalMedicos = 0;
    totalAgendamentos = 0;
    totalTriagens = 0;

    prepararPaciente(1, 2);
    prepararMedico(1, "Ortopedia", 2, 1);

    assert(agendarTriagem(1, "14/06/2026", "12:00") == 0);

    return 0;
}
