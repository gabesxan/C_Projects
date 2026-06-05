#include <assert.h>
#include <string.h>
#include <time.h>
#include "agendamento.h"

Agendamento agendamentos[MAX_AGENDAMENTOS];
int totalAgendamentos = 0;

Paciente pacientes[MAX_PACIENTES];
int totalPacientes = 0;

Medico medicos[MAX_MEDICOS];
int totalMedicos = 0;

Ala alas[MAX_ALAS];
int totalAlas = 0;

Leito leitos[MAX_LEITOS];
int totalLeitos = 0;

Internacao internacoes[MAX_INTERNACOES];
int totalInternacoes = 0;

Triagem triagens[MAX_TRIAGENS];
int totalTriagens = 0;

static void obterDataAtual(char data[], int tamanho)
{
    time_t agora = time(NULL);
    struct tm *dataAtual = localtime(&agora);

    strftime(data, tamanho, "%d/%m/%Y", dataAtual);
}

static void prepararPaciente(void)
{
    totalPacientes = 1;

    pacientes[0].id = 1;
    strcpy(pacientes[0].nome, "Maria Aparecida Santos");
    strcpy(pacientes[0].cpf, "123.456.789-00");
    pacientes[0].idade = 62;
    strcpy(pacientes[0].telefone, "(61) 99999-0000");
    pacientes[0].regiaoAdministrativa = 1;
    pacientes[0].ativo = 1;
}

static void prepararMedicos(void)
{
    totalMedicos = 2;

    medicos[0].id = 1;
    strcpy(medicos[0].nome, "Dr. Carlos Henrique Almeida");
    strcpy(medicos[0].crm, "CRM-DF 12345");
    strcpy(medicos[0].especialidade, "Cardiologia");
    medicos[0].ativo = 1;

    medicos[1].id = 2;
    strcpy(medicos[1].nome, "Dra. Fernanda Lima Rocha");
    strcpy(medicos[1].crm, "CRM-DF 67890");
    strcpy(medicos[1].especialidade, "Cardiologia");
    medicos[1].ativo = 1;
}
static void prepararAgendamento(int indice, int pacienteId, int medicoId, const char data[], const char horario[], const char status[])
{
    agendamentos[indice].id = indice + 1;
    agendamentos[indice].pacienteId = pacienteId;
    agendamentos[indice].medicoId = medicoId;
    strcpy(agendamentos[indice].data, data);
    strcpy(agendamentos[indice].horario, horario);
    strcpy(agendamentos[indice].status, status);
}

int main(void)
{
    char dataConsulta[11];

    obterDataAtual(dataConsulta, sizeof(dataConsulta));

    totalAgendamentos = 0;
    prepararPaciente();
    prepararMedicos();

    assert(verificarConflitoMedico(1, dataConsulta, "08:30") == 0);

    totalAgendamentos = 1;
    prepararAgendamento(0, 1, 1, dataConsulta, "08:30", "AGENDADO");

    assert(verificarConflitoMedico(1, dataConsulta, "08:30") == 1);
    assert(verificarConflitoMedico(1, dataConsulta, "09:30") == 0);
    assert(verificarConflitoMedico(2, dataConsulta, "08:30") == 0);

    strcpy(agendamentos[0].status, "CANCELADO");
    assert(verificarConflitoMedico(1, dataConsulta, "08:30") == 0);

    return 0;
}
