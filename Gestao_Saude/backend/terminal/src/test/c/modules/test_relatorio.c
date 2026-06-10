#include <assert.h>
#include <string.h>
#include "relatorio.h"
#include "exame.h"

Paciente pacientes[MAX_PACIENTES];
Medico medicos[MAX_MEDICOS];
Agendamento agendamentos[MAX_AGENDAMENTOS];
Ala alas[MAX_ALAS];
Leito leitos[MAX_LEITOS];
Internacao internacoes[MAX_INTERNACOES];
Triagem triagens[MAX_TRIAGENS];
Prontuario prontuarios[MAX_PRONTUARIOS];
Exame exames[MAX_EXAMES];

int totalPacientes = 0;
int totalMedicos = 0;
int totalAgendamentos = 0;
int totalInternacoes = 0;
int totalAlas = 0;
int totalLeitos = 0;
int totalTriagens = 0;
int totalProntuarios = 0;
int totalExames = 0;

int main(void)
{
    totalPacientes = 4;

    pacientes[0].id = 1;
    pacientes[0].ativo = 1;
    pacientes[0].regiaoAdministrativa = 1;

    pacientes[1].id = 2;
    pacientes[1].ativo = 1;
    pacientes[1].regiaoAdministrativa = 1;

    pacientes[2].id = 3;
    pacientes[2].ativo = 1;
    pacientes[2].regiaoAdministrativa = 3;

    pacientes[3].id = 4;
    pacientes[3].ativo = 0;
    pacientes[3].regiaoAdministrativa = 1;

    totalAlas = 1;
    alas[0].id = 1;
    strcpy(alas[0].nome, "Clinica Medica");
    alas[0].tipo = ALA_INTERNACAO;
    alas[0].totalLeitos = 4;
    alas[0].leitosOcupados = 2;
    alas[0].ativo = 1;

    totalLeitos = 5;
    leitos[0].ativo = 1;
    leitos[0].ocupado = 1;
    leitos[1].ativo = 1;
    leitos[1].ocupado = 1;
    leitos[2].ativo = 1;
    leitos[2].ocupado = 0;
    leitos[3].ativo = 1;
    leitos[3].ocupado = 0;
    leitos[4].ativo = 0;
    leitos[4].ocupado = 1;

    totalTriagens = 4;

    triagens[0].pacienteId = 1;
    triagens[0].ativo = 1;
    strcpy(triagens[0].classificacao, "Emergencia");

    triagens[1].pacienteId = 2;
    triagens[1].ativo = 1;
    strcpy(triagens[1].classificacao, "Muito prioritario");

    triagens[2].pacienteId = 3;
    triagens[2].ativo = 1;
    strcpy(triagens[2].classificacao, "Comum");

    triagens[3].pacienteId = 1;
    triagens[3].ativo = 0;
    strcpy(triagens[3].classificacao, "Emergencia");

    totalMedicos = 4;

    medicos[0].id = 1;
    medicos[0].ativo = 1;
    medicos[0].regiaoAdministrativa = 1;
    strcpy(medicos[0].especialidade, "Cardiologia");

    medicos[1].id = 2;
    medicos[1].ativo = 1;
    medicos[1].regiaoAdministrativa = 1;
    strcpy(medicos[1].especialidade, "Cardiologia");

    medicos[2].id = 3;
    medicos[2].ativo = 1;
    medicos[2].regiaoAdministrativa = 4;
    strcpy(medicos[2].especialidade, "Ortopedia");

    medicos[3].id = 4;
    medicos[3].ativo = 0;
    medicos[3].regiaoAdministrativa = 1;
    strcpy(medicos[3].especialidade, "Pediatria");

    totalAgendamentos = 4;

    agendamentos[0].medicoId = 1;
    strcpy(agendamentos[0].status, "AGENDADO");

    agendamentos[1].medicoId = 2;
    strcpy(agendamentos[1].status, "CONCLUIDO");

    agendamentos[2].medicoId = 3;
    strcpy(agendamentos[2].status, "AGENDADO");

    agendamentos[3].medicoId = 4;
    strcpy(agendamentos[3].status, "CANCELADO");

    totalProntuarios = 3;
    prontuarios[0].ativo = 1;
    prontuarios[1].ativo = 1;
    prontuarios[2].ativo = 0;

    totalExames = 4;
    exames[0].ativo = 1;
    strcpy(exames[0].status, "Solicitado");
    exames[0].urgente = 1;

    exames[1].ativo = 1;
    strcpy(exames[1].status, "Realizado");
    exames[1].urgente = 0;

    exames[2].ativo = 1;
    strcpy(exames[2].status, "Cancelado");
    exames[2].urgente = 1;

    exames[3].ativo = 0;
    strcpy(exames[3].status, "Solicitado");
    exames[3].urgente = 1;

    assert(contarPacientesAtivos() == 3);
    assert(contarMedicosAtivos() == 3);
    assert(contarProntuariosAtivos() == 2);
    assert(contarExamesAtivos() == 3);
    assert(contarExamesPorStatus("Solicitado") == 1);
    assert(contarExamesPorStatus("Realizado") == 1);
    assert(contarExamesPorStatus("Cancelado") == 1);
    assert(contarExamesUrgentes() == 2);

    assert(contarLeitosOcupados() == 2);
    assert(contarLivres() == 2);
    assert(taxaAla(1) == 50.0f);
    assert(taxaAla(99) == 0.0f);

    assert(contarTriagens("Emergencia") == 1);
    assert(contarTriagens("Muito prioritario") == 1);
    assert(contarTriagens("Comum") == 1);
    assert(contarTriagens("Prioritario") == 0);

    assert(contarMedRegiao(1) == 2);
    assert(contarMedRegiao(4) == 1);
    assert(contarMedRegiao(8) == 0);

    assert(contarPacRegiao(1) == 2);
    assert(contarPacRegiao(3) == 1);
    assert(contarPacRegiao(8) == 0);

    assert(contarEsp("Cardiologia") == 2);
    assert(contarEsp("Ortopedia") == 1);
    assert(contarEsp("Pediatria") == 0);

    {
        char especialidade[50];
        espDemandada(especialidade);
        assert(strcmp(especialidade, "Cardiologia") == 0);
    }

    assert(contarCasosGravesRegiao(1) == 2);
    assert(contarCasosGravesRegiao(3) == 0);
    assert(contarCasosGravesRegiao(8) == 0);
    assert(regiaoMaisCasosGraves() == 1);

    return 0;
}
