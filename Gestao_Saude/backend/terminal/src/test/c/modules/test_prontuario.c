#include <assert.h>
#include <string.h>
#include "prontuario.h"
#include "sqlite_db.h"

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
    Prontuario lista[MAX_PRONTUARIOS];
    int totalCopiados;

    assert(executarSQLSQLite("DELETE FROM exames;") == 1);
    assert(executarSQLSQLite("DELETE FROM prontuarios;") == 1);
    assert(executarSQLSQLite("DELETE FROM agendamentos;") == 1);
    assert(executarSQLSQLite("DELETE FROM triagens;") == 1);
    assert(executarSQLSQLite("DELETE FROM pacientes;") == 1);
    assert(executarSQLSQLite("DELETE FROM medicos;") == 1);
    assert(executarSQLSQLite("INSERT INTO pacientes (id, nome, cpf, idade, telefone, sexo, regiao_administrativa, ativo) VALUES (701, 'Paciente Prontuario', '701.701.701-70', 45, '(61) 97010-0000', 'F', 1, 1);") == 1);
    assert(executarSQLSQLite("INSERT INTO medicos (id, nome, crm, especialidade, regiao_administrativa, ativo) VALUES (801, 'Medico Prontuario', '801', 'Cardiologia', 1, 1);") == 1);

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

    assert(criarProntuarioAutomatico(1, 1, "09/06/2026") == 1);
    assert(totalProntuarios == 1);
    assert(prontuarios[0].id == 1);
    assert(prontuarios[0].pacienteId == 1);
    assert(prontuarios[0].medicoId == 1);
    assert(strcmp(prontuarios[0].data, "09/06/2026") == 0);
    assert(strcmp(prontuarios[0].observacoes, "Prontuario base criado automaticamente pelo sistema.") == 0);
    assert(strcmp(prontuarios[0].diagnostico, "Aguardando avaliacao medica.") == 0);
    assert(strcmp(prontuarios[0].conduta, "Registrar evolucao do atendimento.") == 0);
    assert(prontuarios[0].alertaImportante == 0);
    assert(prontuarios[0].ativo == 1);

    assert(complementarProntuario(1,
                                  "Paciente acolhido e avaliado.",
                                  "Quadro inicial em observacao.",
                                  "Manter monitoramento clinico.",
                                  1) == 1);
    assert(strcmp(prontuarios[0].observacoes, "Paciente acolhido e avaliado.") == 0);
    assert(strcmp(prontuarios[0].diagnostico, "Quadro inicial em observacao.") == 0);
    assert(strcmp(prontuarios[0].conduta, "Manter monitoramento clinico.") == 0);
    assert(prontuarios[0].alertaImportante == 1);
    assert(complementarProntuario(99, "Obs", "Diag", "Conduta", 0) == 0);
    assert(complementarProntuario(1, "", "Diag", "Conduta", 0) == 0);

    assert(registrarProntuario(1, 1, "10/06/2026",
                               "Paciente com dor toracica",
                               "Suspeita clinica inicial",
                               "Solicitar observacao e exames",
                               1) == 1);

    assert(totalProntuarios == 2);
    assert(prontuarios[1].id == 2);
    assert(prontuarios[1].pacienteId == 1);
    assert(prontuarios[1].medicoId == 1);
    assert(strcmp(prontuarios[1].data, "10/06/2026") == 0);
    assert(strcmp(prontuarios[1].observacoes, "Paciente com dor toracica") == 0);
    assert(strcmp(prontuarios[1].diagnostico, "Suspeita clinica inicial") == 0);
    assert(strcmp(prontuarios[1].conduta, "Solicitar observacao e exames") == 0);
    assert(prontuarios[1].alertaImportante == 1);
    assert(prontuarios[1].ativo == 1);

    assert(registrarProntuario(2, 2, "11/06/2026",
                               "Paciente com dor no joelho",
                               "Lesao ortopedica",
                               "Repouso e avaliacao",
                               0) == 1);

    assert(totalProntuarios == 3);
    assert(prontuarios[2].id == 3);
    assert(prontuarios[2].pacienteId == 2);
    assert(prontuarios[2].medicoId == 2);
    assert(strcmp(prontuarios[2].data, "11/06/2026") == 0);
    assert(strcmp(prontuarios[2].observacoes, "Paciente com dor no joelho") == 0);
    assert(strcmp(prontuarios[2].diagnostico, "Lesao ortopedica") == 0);
    assert(strcmp(prontuarios[2].conduta, "Repouso e avaliacao") == 0);
    assert(prontuarios[2].alertaImportante == 0);
    assert(prontuarios[2].ativo == 1);

    assert(registrarProntuario(99, 1, "12/06/2026",
                               "Observacao",
                               "Diagnostico",
                               "Conduta",
                               0) == 0);
    assert(totalProntuarios == 3);

    assert(registrarProntuario(1, 99, "12/06/2026",
                               "Observacao",
                               "Diagnostico",
                               "Conduta",
                               0) == 0);
    assert(totalProntuarios == 3);
    assert(registrarProntuario(1, 1, "",
                               "Observacao",
                               "Diagnostico",
                               "Conduta",
                               0) == 0);
    assert(totalProntuarios == 3);
    assert(registrarProntuario(1, 1, "12/06/2026",
                               "",
                               "Diagnostico",
                               "Conduta",
                               0) == 0);
    assert(totalProntuarios == 3);

    pacientes[1].ativo = 0;

    assert(registrarProntuario(2, 1, "12/06/2026",
                               "Paciente inativo",
                               "Diagnostico",
                               "Conduta",
                               0) == 0);
    assert(totalProntuarios == 3);

    pacientes[1].ativo = 1;
    medicos[1].ativo = 0;

    assert(registrarProntuario(1, 2, "12/06/2026",
                               "Medico inativo",
                               "Diagnostico",
                               "Conduta",
                               0) == 0);
    assert(totalProntuarios == 3);

    medicos[1].ativo = 1;

    assert(registrarProntuario(1, 2, "13/06/2026",
                               "Retorno clinico",
                               "Acompanhamento",
                               "Manter observacao",
                               0) == 1);

    assert(totalProntuarios == 4);
    assert(prontuarios[3].id == 4);
    assert(prontuarios[3].pacienteId == 1);
    assert(prontuarios[3].medicoId == 2);
    assert(strcmp(prontuarios[3].data, "13/06/2026") == 0);
    assert(strcmp(prontuarios[3].observacoes, "Retorno clinico") == 0);
    assert(strcmp(prontuarios[3].diagnostico, "Acompanhamento") == 0);
    assert(strcmp(prontuarios[3].conduta, "Manter observacao") == 0);
    assert(prontuarios[3].alertaImportante == 0);
    assert(prontuarios[3].ativo == 1);

    assert(contarProntuariosPorPaciente(1) == 3);
    assert(contarProntuariosPorPaciente(2) == 1);
    assert(contarProntuariosPorPaciente(99) == 0);
    assert(contarProntuariosPorMedico(1) == 2);
    assert(contarProntuariosPorMedico(2) == 2);
    assert(contarProntuariosPorMedico(99) == 0);
    assert(contarProntuariosPorEspecialidade("Cardiologia") == 2);
    assert(contarProntuariosPorEspecialidade("Ortopedia") == 2);
    assert(contarProntuariosPorEspecialidade("Pediatria") == 0);
    assert(contarProntuariosPorEspecialidade("Inexistente") == 0);
    assert(contarProntuariosPorEspecialidade(NULL) == 0);

    totalCopiados = copiarProntuarios(lista, MAX_PRONTUARIOS);
    assert(totalCopiados == 4);
    assert(lista[0].id == 1);
    assert(lista[3].id == 4);

    totalCopiados = copiarProntuariosPorPaciente(1, lista, MAX_PRONTUARIOS);
    assert(totalCopiados == 3);
    assert(lista[0].pacienteId == 1);
    assert(lista[1].pacienteId == 1);
    assert(lista[2].pacienteId == 1);

    totalCopiados = copiarProntuariosPorMedico(2, lista, MAX_PRONTUARIOS);
    assert(totalCopiados == 2);
    assert(lista[0].medicoId == 2);
    assert(lista[1].medicoId == 2);

    totalCopiados = copiarProntuariosPorEspecialidade("Cardiologia", lista, MAX_PRONTUARIOS);
    assert(totalCopiados == 2);
    assert(lista[0].medicoId == 1);
    assert(lista[1].medicoId == 1);

    assert(copiarProntuarios(NULL, MAX_PRONTUARIOS) == 0);
    assert(copiarProntuariosPorPaciente(1, NULL, MAX_PRONTUARIOS) == 0);
    assert(copiarProntuariosPorMedico(2, lista, 0) == 0);
    assert(copiarProntuariosPorEspecialidade(NULL, lista, MAX_PRONTUARIOS) == 0);

    prontuarios[0].id = 701;
    prontuarios[0].pacienteId = 701;
    prontuarios[0].medicoId = 801;
    strcpy(prontuarios[0].data, "21/06/2026");
    strcpy(prontuarios[0].observacoes, "Observacao SQLite");
    strcpy(prontuarios[0].diagnostico, "Diagnostico SQLite");
    strcpy(prontuarios[0].conduta, "Conduta SQLite");
    prontuarios[0].alertaImportante = 1;
    prontuarios[0].ativo = 1;

    assert(salvarProntuarioNoBanco(&prontuarios[0]) == 1);
    assert(salvarProntuarioNoBanco(NULL) == 0);
    totalCopiados = carregarProntuariosDoBanco(lista, MAX_PRONTUARIOS);
    assert(totalCopiados == 1);
    assert(lista[0].id == 701);
    assert(lista[0].pacienteId == 701);
    assert(lista[0].medicoId == 801);
    return 0;
}
