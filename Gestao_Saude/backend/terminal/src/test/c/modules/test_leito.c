#include <assert.h>
#include <string.h>
#include "leito.h"
#include "sqlite_db.h"

Ala alas[MAX_ALAS];
Leito leitos[MAX_LEITOS];

int totalAlas = 0;
int totalLeitos = 0;

static void prepararAlaAtiva(void)
{
    totalAlas = 1;

    alas[0].id = 1;
    strcpy(alas[0].nome, "Clinica Medica");
    alas[0].tipo = ALA_INTERNACAO;
    alas[0].totalLeitos = 10;
    alas[0].leitosOcupados = 0;
    alas[0].ativo = 1;
}

int main(void)
{
    Leito lista[MAX_LEITOS];
    int totalCarregados;

    assert(definirCaminhoBancoSQLite("/tmp/sigeh_test_leito.db") == 1);
    assert(reinicializarBancoSQLite() == 1);
    assert(executarSQLSQLite("INSERT INTO alas (id, nome, tipo, total_leitos, leitos_ocupados, ativo) VALUES (1, 'Clinica Medica', 1, 10, 0, 1);") == 1);

    prepararAlaAtiva();

    assert(cadastrarLeito(1, 101) == 1);
    assert(totalLeitos == 1);
    assert(leitos[0].id == 1);
    assert(leitos[0].alaId == 1);
    assert(leitos[0].numero == 101);
    assert(leitos[0].ocupado == 0);
    assert(leitos[0].pacienteId == 0);
    assert(leitos[0].ativo == 1);

    assert(cadastrarLeito(99, 102) == 0);
    assert(totalLeitos == 1);

    alas[0].ativo = 0;
    assert(cadastrarLeito(1, 103) == 0);
    assert(totalLeitos == 1);

    alas[0].ativo = 1;
    assert(excluirLeito(1) == 1);
    assert(leitos[0].ativo == 0);
    assert(excluirLeito(1) == 0);
    assert(excluirLeito(99) == 0);

    assert(cadastrarLeito(1, 104) == 1);
    assert(totalLeitos == 2);
    leitos[1].ocupado = 1;
    leitos[1].pacienteId = 1;

    assert(excluirLeito(2) == 0);
    assert(leitos[1].ativo == 1);

    assert(reinicializarBancoSQLite() == 1);
    assert(executarSQLSQLite("INSERT INTO alas (id, nome, tipo, total_leitos, leitos_ocupados, ativo) VALUES (1, 'Clinica Medica', 1, 10, 0, 1);") == 1);
    leitos[0].id = 888;
    leitos[0].alaId = 1;
    leitos[0].numero = 205;
    leitos[0].ocupado = 1;
    leitos[0].pacienteId = 9;
    leitos[0].ativo = 1;
    assert(salvarLeitoNoBanco(&leitos[0]) == 1);
    assert(salvarLeitoNoBanco(NULL) == 0);
    totalCarregados = carregarLeitosDoBanco(lista, MAX_LEITOS);
    assert(totalCarregados == 1);
    assert(lista[0].id == 888);
    assert(lista[0].numero == 205);
    assert(lista[0].pacienteId == 9);

    return 0;
}
