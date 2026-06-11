#include <assert.h>
#include <string.h>
#include "ala.h"
#include "sqlite_db.h"

Ala alas[MAX_ALAS];
int totalAlas = 0;

int main(void)
{
    Ala lista[MAX_ALAS];
    int totalCarregados;

    assert(definirCaminhoBancoSQLite("/tmp/sigeh_test_ala.db") == 1);
    assert(reinicializarBancoSQLite() == 1);

    assert(cadastrarAla("Clinica Medica", ALA_INTERNACAO, 20) == 1);
    assert(cadastrarAla("UTI Central", ALA_UTI, 10) == 1);
    assert(cadastrarAla("Sala de Observacao", ALA_OBSERVACAO, 8) == 1);

    assert(totalAlas == 3);

    assert(alas[0].id == 1);
    assert(strcmp(alas[0].nome, "Clinica Medica") == 0);
    assert(alas[0].tipo == ALA_INTERNACAO);
    assert(alas[0].totalLeitos == 20);
    assert(alas[0].leitosOcupados == 0);
    assert(alas[0].ativo == 1);

    assert(alas[1].id == 2);
    assert(strcmp(alas[1].nome, "UTI Central") == 0);
    assert(alas[1].tipo == ALA_UTI);
    assert(alas[1].totalLeitos == 10);
    assert(alas[1].leitosOcupados == 0);
    assert(alas[1].ativo == 1);

    assert(alas[2].id == 3);
    assert(strcmp(alas[2].nome, "Sala de Observacao") == 0);
    assert(alas[2].tipo == ALA_OBSERVACAO);
    assert(alas[2].totalLeitos == 8);
    assert(alas[2].leitosOcupados == 0);
    assert(alas[2].ativo == 1);

    assert(contarAlasPorTipo(ALA_INTERNACAO) == 1);
    assert(contarAlasPorTipo(ALA_UTI) == 1);
    assert(contarAlasPorTipo(ALA_OBSERVACAO) == 1);
    assert(contarAlasPorTipo(ALA_PEDIATRIA) == 0);

    assert(excluirAla(2) == 1);
    assert(alas[1].ativo == 0);

    assert(contarAlasPorTipo(ALA_UTI) == 0);

    assert(excluirAla(2) == 0);
    assert(excluirAla(99) == 0);

    assert(reinicializarBancoSQLite() == 1);
    alas[0].id = 777;
    strcpy(alas[0].nome, "Ala SQLite");
    alas[0].tipo = ALA_CIRURGICA;
    alas[0].totalLeitos = 12;
    alas[0].leitosOcupados = 3;
    alas[0].ativo = 1;
    assert(salvarAlaNoBanco(&alas[0]) == 1);
    assert(salvarAlaNoBanco(NULL) == 0);
    totalCarregados = carregarAlasDoBanco(lista, MAX_ALAS);
    assert(totalCarregados == 1);
    assert(lista[0].id == 777);
    assert(strcmp(lista[0].nome, "Ala SQLite") == 0);
    assert(lista[0].tipo == ALA_CIRURGICA);
    assert(lista[0].leitosOcupados == 3);

    return 0;
}
