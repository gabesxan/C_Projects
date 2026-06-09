#include <assert.h>
#include <string.h>
#include "exame.h"

Paciente pacientes[MAX_PACIENTES];
Medico medicos[MAX_MEDICOS];
Prontuario prontuarios[MAX_PRONTUARIOS];
Triagem triagens[MAX_TRIAGENS];
Exame exames[MAX_EXAMES];

Agendamento agendamentos[MAX_AGENDAMENTOS];
Ala alas[MAX_ALAS];
Leito leitos[MAX_LEITOS];
Internacao internacoes[MAX_INTERNACOES];

int totalPacientes = 0;
int totalMedicos = 0;
int totalProntuarios = 0;
int totalTriagens = 0;
int totalExames = 0;

int totalAgendamentos = 0;
int totalAlas = 0;
int totalLeitos = 0;
int totalInternacoes = 0;

static void resetarDados(void)
{
    totalPacientes = 0;
    totalMedicos = 0;
    totalProntuarios = 0;
    totalTriagens = 0;
    totalExames = 0;
    totalAgendamentos = 0;
    totalAlas = 0;
    totalLeitos = 0;
    totalInternacoes = 0;
}

int main(void)
{
    Exame lista[MAX_EXAMES];
    int totalCopiados;

    resetarDados();

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

    prontuarios[0].id = 1;
    prontuarios[0].pacienteId = 1;
    prontuarios[0].medicoId = 1;
    strcpy(prontuarios[0].data, "10/06/2026");
    strcpy(prontuarios[0].observacoes, "Base");
    strcpy(prontuarios[0].diagnostico, "Diag");
    strcpy(prontuarios[0].conduta, "Conduta");
    prontuarios[0].alertaImportante = 0;
    prontuarios[0].ativo = 1;

    prontuarios[1].id = 2;
    prontuarios[1].pacienteId = 2;
    prontuarios[1].medicoId = 2;
    strcpy(prontuarios[1].data, "11/06/2026");
    strcpy(prontuarios[1].observacoes, "Base");
    strcpy(prontuarios[1].diagnostico, "Diag");
    strcpy(prontuarios[1].conduta, "Conduta");
    prontuarios[1].alertaImportante = 0;
    prontuarios[1].ativo = 1;
    totalProntuarios = 2;

    triagens[0].id = 1;
    triagens[0].pacienteId = 1;
    triagens[0].tipoTriagem = TRIAGEM_CARDIOLOGIA;
    triagens[0].pontuacao = 9;
    strcpy(triagens[0].classificacao, "Emergencia");
    triagens[0].ativo = 1;

    triagens[1].id = 2;
    triagens[1].pacienteId = 2;
    triagens[1].tipoTriagem = TRIAGEM_ORTOPEDIA;
    triagens[1].pontuacao = 2;
    strcpy(triagens[1].classificacao, "Comum");
    triagens[1].ativo = 1;
    totalTriagens = 2;

    assert(solicitarExame(1, 1, 1, EXAME_TOMOGRAFIA, "12/06/2026") == 1);
    assert(totalExames == 1);
    assert(exames[0].id == 1);
    assert(exames[0].pacienteId == 1);
    assert(exames[0].medicoId == 1);
    assert(exames[0].prontuarioId == 1);
    assert(exames[0].tipoExame == EXAME_TOMOGRAFIA);
    assert(strcmp(exames[0].dataSolicitacao, "12/06/2026") == 0);
    assert(strcmp(exames[0].status, "Solicitado") == 0);
    assert(exames[0].urgente == 1);
    assert(exames[0].ativo == 1);

    assert(solicitarExame(2, 2, 2, EXAME_URINA, "13/06/2026") == 1);
    assert(totalExames == 2);
    assert(exames[1].urgente == 0);

    assert(solicitarExame(1, 2, 1, EXAME_RAIO_X, "14/06/2026") == 0);
    assert(solicitarExame(1, 1, 2, EXAME_RAIO_X, "14/06/2026") == 0);
    assert(solicitarExame(99, 1, 1, EXAME_RAIO_X, "14/06/2026") == 0);
    assert(solicitarExame(1, 99, 1, EXAME_RAIO_X, "14/06/2026") == 0);
    assert(solicitarExame(1, 1, 1, 99, "14/06/2026") == 0);
    assert(totalExames == 2);

    assert(registrarResultadoExame(1, "15/06/2026", "Sem alteracoes relevantes") == 1);
    assert(strcmp(exames[0].status, "Realizado") == 0);
    assert(strcmp(exames[0].dataResultado, "15/06/2026") == 0);
    assert(strcmp(exames[0].resultado, "Sem alteracoes relevantes") == 0);
    assert(registrarResultadoExame(1, "16/06/2026", "Outro") == 0);

    assert(cancelarExame(2) == 1);
    assert(strcmp(exames[1].status, "Cancelado") == 0);
    assert(cancelarExame(2) == 0);

    assert(excluirExame(2) == 1);
    assert(exames[1].ativo == 0);
    assert(excluirExame(2) == 0);

    assert(contarExamesPorStatus("Realizado") == 1);
    assert(contarExamesPorStatus("Solicitado") == 0);
    assert(contarExamesPorTipo(EXAME_TOMOGRAFIA) == 1);
    assert(contarExamesPorTipo(EXAME_URINA) == 0);
    assert(contarExamesUrgentes() == 1);

    totalCopiados = copiarExames(lista, MAX_EXAMES);
    assert(totalCopiados == 1);
    assert(lista[0].id == 1);
    assert(lista[0].pacienteId == 1);

    totalCopiados = copiarExamesPorPaciente(1, lista, MAX_EXAMES);
    assert(totalCopiados == 1);
    assert(lista[0].pacienteId == 1);

    totalCopiados = copiarExamesPorMedico(1, lista, MAX_EXAMES);
    assert(totalCopiados == 1);
    assert(lista[0].medicoId == 1);

    totalCopiados = copiarExamesPorProntuario(1, lista, MAX_EXAMES);
    assert(totalCopiados == 1);
    assert(lista[0].prontuarioId == 1);

    totalCopiados = copiarExamesUrgentes(lista, MAX_EXAMES);
    assert(totalCopiados == 1);
    assert(lista[0].urgente == 1);

    assert(copiarExames(NULL, MAX_EXAMES) == 0);
    assert(copiarExamesPorPaciente(1, NULL, MAX_EXAMES) == 0);
    assert(copiarExamesPorMedico(1, lista, 0) == 0);
    assert(copiarExamesPorProntuario(1, NULL, MAX_EXAMES) == 0);
    assert(copiarExamesUrgentes(NULL, MAX_EXAMES) == 0);

    return 0;
}
