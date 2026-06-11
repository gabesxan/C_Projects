#include <assert.h>
#include <string.h>
#include "internacao.h"
#include "sqlite_db.h"

Paciente pacientes[MAX_PACIENTES];
Ala alas[MAX_ALAS];
Leito leitos[MAX_LEITOS];
Internacao internacoes[MAX_INTERNACOES];

int totalPacientes = 0;
int totalAlas = 0;
int totalLeitos = 0;
int totalInternacoes = 0;

static void prepararDados(void)
{
    totalPacientes = 1;
    pacientes[0].id = 1;
    strcpy(pacientes[0].nome, "Maria Aparecida Santos");
    pacientes[0].idade = 62;
    pacientes[0].ativo = 1;

    totalAlas = 1;
    alas[0].id = 1;
    strcpy(alas[0].nome, "Clinica Medica");
    alas[0].tipo = ALA_INTERNACAO;
    alas[0].totalLeitos = 10;
    alas[0].leitosOcupados = 0;
    alas[0].ativo = 1;

    totalLeitos = 1;
    leitos[0].id = 1;
    leitos[0].alaId = 1;
    leitos[0].numero = 101;
    leitos[0].ocupado = 0;
    leitos[0].pacienteId = 0;

    totalInternacoes = 0;
}

int main(void)
{
    Internacao lista[MAX_INTERNACOES];
    int totalCarregados;

    assert(definirCaminhoBancoSQLite("/tmp/sigeh_test_internacao.db") == 1);
    assert(reinicializarBancoSQLite() == 1);
    assert(executarSQLSQLite("INSERT INTO pacientes (id, nome, cpf, idade, telefone, sexo, regiao_administrativa, ativo) VALUES (1, 'Maria Aparecida Santos', '111.111.111-11', 62, '(61) 90000-0001', 'F', 1, 1);") == 1);
    assert(executarSQLSQLite("INSERT INTO alas (id, nome, tipo, total_leitos, leitos_ocupados, ativo) VALUES (1, 'Clinica Medica', 1, 10, 0, 1);") == 1);
    assert(executarSQLSQLite("INSERT INTO leitos (id, ala_id, numero, ocupado, paciente_id, ativo) VALUES (1, 1, 101, 0, 0, 1);") == 1);

    prepararDados();

    assert(internarPaciente(1, 1, "05/06/2026") == 1);
    assert(totalInternacoes == 1);
    assert(internacoes[0].id == 1);
    assert(internacoes[0].pacienteId == 1);
    assert(internacoes[0].alaId == 1);
    assert(internacoes[0].leitoId == 1);
    assert(strcmp(internacoes[0].status, "INTERNADO") == 0);
    assert(leitos[0].ocupado == 1);
    assert(leitos[0].pacienteId == 1);
    assert(alas[0].leitosOcupados == 1);

    assert(internarPaciente(1, 1, "05/06/2026") == 0);
    assert(internarPaciente(99, 1, "05/06/2026") == 0);
    assert(darAltaInternacao(1, "06/06/2026") == 1);
    assert(strcmp(internacoes[0].status, "ALTA") == 0);
    assert(strcmp(internacoes[0].dataAlta, "06/06/2026") == 0);
    assert(leitos[0].ocupado == 0);
    assert(leitos[0].pacienteId == 0);
    assert(alas[0].leitosOcupados == 0);
    assert(darAltaInternacao(1, "07/06/2026") == 0);

    assert(reinicializarBancoSQLite() == 1);
    assert(executarSQLSQLite("INSERT INTO pacientes (id, nome, cpf, idade, telefone, sexo, regiao_administrativa, ativo) VALUES (501, 'Paciente SQLite', '501.501.501-50', 40, '(61) 95010-0000', 'F', 2, 1);") == 1);
    assert(executarSQLSQLite("INSERT INTO alas (id, nome, tipo, total_leitos, leitos_ocupados, ativo) VALUES (601, 'Ala SQLite', 1, 8, 1, 1);") == 1);
    assert(executarSQLSQLite("INSERT INTO leitos (id, ala_id, numero, ocupado, paciente_id, ativo) VALUES (701, 601, 12, 1, 501, 1);") == 1);
    internacoes[0].id = 901;
    internacoes[0].pacienteId = 501;
    internacoes[0].alaId = 601;
    internacoes[0].leitoId = 701;
    strcpy(internacoes[0].dataEntrada, "21/06/2026");
    strcpy(internacoes[0].dataAlta, "00/00/0000");
    strcpy(internacoes[0].status, "INTERNADO");
    assert(salvarInternacaoNoBanco(&internacoes[0]) == 1);
    assert(salvarInternacaoNoBanco(NULL) == 0);
    totalCarregados = carregarInternacoesDoBanco(lista, MAX_INTERNACOES);
    assert(totalCarregados == 1);
    assert(lista[0].id == 901);
    assert(lista[0].pacienteId == 501);
    assert(strcmp(lista[0].status, "INTERNADO") == 0);

    return 0;
}
