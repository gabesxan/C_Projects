#include <assert.h>
#include <string.h>
#include "prontuario.h"

Paciente pacientes[MAX_PACIENTES];
Medico medicos[MAX_MEDICOS];
Prontuario prontuarios[MAX_PRONTUARIOS];

Agendamento agendamentos[MAX_AGENDAMENTOS];
Ala alas[MAX_ALAS];
Leito leitos[MAX_LEITOS];
Internacao internacoes[MAX_INTERNACOES];
Triagem triagens[MAX_TRIAGENS];

int totalPacientes = 0;
int totalMedicos = 0;
int totalProntuarios = 0;

int totalAgendamentos = 0;
int totalAlas = 0;
int totalLeitos = 0;
int totalInternacoes = 0;
int totalTriagens = 0;

int main(void)
{
    pacientes[0].id = 1;
    pacientes[0].ativo = 1;
    strcpy(pacientes[0].nome, "Maria Silva");

    pacientes[1].id = 2;
    pacientes[1].ativo = 1;
    strcpy(pacientes[1].nome, "Joao Souza");

    totalPacientes = 2;

    medicos[0].id = 1;
    medicos[0].ativo = 1;
    strcpy(medicos[0].nome, "Dra. Ana");
    strcpy(medicos[0].especialidade, "Cardiologia");

    medicos[1].id = 2;
    medicos[1].ativo = 1;
    strcpy(medicos[1].nome, "Dr. Pedro");
    strcpy(medicos[1].especialidade, "Ortopedia");

    totalMedicos = 2;

    assert(registrarProntuario(1, 1, "10/06/2026",
                               "Paciente com dor toracica",
                               "Suspeita clinica inicial",
                               "Solicitar observacao e exames",
                               1) == 1);

    assert(totalProntuarios == 1);
    assert(prontuarios[0].id == 1);
    assert(prontuarios[0].pacienteId == 1);
    assert(prontuarios[0].medicoId == 1);
    assert(strcmp(prontuarios[0].data, "10/06/2026") == 0);
    assert(strcmp(prontuarios[0].observacoes, "Paciente com dor toracica") == 0);
    assert(strcmp(prontuarios[0].diagnostico, "Suspeita clinica inicial") == 0);
    assert(strcmp(prontuarios[0].conduta, "Solicitar observacao e exames") == 0);
    assert(prontuarios[0].alertaImportante == 1);
    assert(prontuarios[0].ativo == 1);

    assert(registrarProntuario(2, 2, "11/06/2026",
                               "Paciente com dor no joelho",
                               "Lesao ortopedica",
                               "Repouso e avaliacao",
                               0) == 1);

    assert(totalProntuarios == 2);
    assert(prontuarios[1].id == 2);
    assert(prontuarios[1].pacienteId == 2);
    assert(prontuarios[1].medicoId == 2);
    assert(strcmp(prontuarios[1].data, "11/06/2026") == 0);
    assert(strcmp(prontuarios[1].observacoes, "Paciente com dor no joelho") == 0);
    assert(strcmp(prontuarios[1].diagnostico, "Lesao ortopedica") == 0);
    assert(strcmp(prontuarios[1].conduta, "Repouso e avaliacao") == 0);
    assert(prontuarios[1].alertaImportante == 0);
    assert(prontuarios[1].ativo == 1);

    assert(registrarProntuario(99, 1, "12/06/2026",
                               "Observacao",
                               "Diagnostico",
                               "Conduta",
                               0) == 0);

    assert(totalProntuarios == 2);

    assert(registrarProntuario(1, 99, "12/06/2026",
                               "Observacao",
                               "Diagnostico",
                               "Conduta",
                               0) == 0);

    assert(totalProntuarios == 2);

    pacientes[1].ativo = 0; 
    assert(registrarProntuario(2, 1, "12/06/2026", 
                                "Paciente Inativo",
                                "Diagnostico",
                                "Conduta",
                                0) == 0);
    
    assert(totalProntuarios == 2);

    pacientes[1].ativo = 1;
    medicos[1].ativo = 0;

    pacientes[1].ativo = 1;
    medicos[1].ativo = 0;

    assert(registrarProntuario(1, 2, "12/06/2026",
                               "Medico inativo",
                               "Diagnostico",
                               "Conduta",
                               0) == 0);

    assert(totalProntuarios == 2);

    medicos[1].ativo = 1;

    assert(registrarProntuario(1, 2, "13/06/2026",
                               "Retorno clinico",
                               "Acompanhamento",
                               "Manter observacao",
                               0) == 1);

    assert(totalProntuarios == 3);
    assert(prontuarios[2].id == 3);
    assert(prontuarios[2].pacienteId == 1);
    assert(prontuarios[2].medicoId == 2);
    assert(strcmp(prontuarios[2].data, "13/06/2026") == 0);
    assert(strcmp(prontuarios[2].observacoes, "Retorno clinico") == 0);
    assert(strcmp(prontuarios[2].diagnostico, "Acompanhamento") == 0);
    assert(strcmp(prontuarios[2].conduta, "Manter observacao") == 0);
    assert(prontuarios[2].alertaImportante == 0);
    assert(prontuarios[2].ativo == 1);
                                
    return 0;
}
