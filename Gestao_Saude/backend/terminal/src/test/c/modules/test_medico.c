#include <assert.h>
#include <string.h>
#include "medico.h"
#include "sqlite_db.h"

Medico medicos[MAX_MEDICOS];
int totalMedicos = 0;

int main(void)
{
    Medico medicoTeste;
    Medico lista[MAX_MEDICOS];
    int totalCarregados;

    assert(definirCaminhoBancoSQLite("/tmp/sigeh_test_medico.db") == 1);
    assert(reinicializarBancoSQLite() == 1);

    assert(cadastrarMedico("Carlos Henrique Almeida", "12345", "Cardiologia", 3) == 1);
    assert(totalMedicos == 1);
    assert(medicos[0].id == 1);
    assert(strcmp(medicos[0].nome, "Carlos Henrique Almeida") == 0);
    assert(strcmp(medicos[0].crm, "12345") == 0);
    assert(strcmp(medicos[0].especialidade, "Cardiologia") == 0);
    assert(medicos[0].regiaoAdministrativa == 3);
    assert(medicos[0].ativo == 1);

    assert(cadastrarMedico("Ana Paula Souza", "98765", "Clinica Geral", 1) == 1);
    assert(totalMedicos == 2);
    assert(medicos[1].id == 2);
    assert(medicos[1].regiaoAdministrativa == 1);
    assert(medicos[1].ativo == 1);

    assert(excluirMedico(1) == 1);
    assert(medicos[0].ativo == 0);
    assert(medicos[1].ativo == 1);
    assert(excluirMedico(1) == 0);
    assert(excluirMedico(99) == 0);

    medicoTeste.id = 888;
    strcpy(medicoTeste.nome, "Medico SQLite");
    strcpy(medicoTeste.crm, "88888");
    strcpy(medicoTeste.especialidade, "Clinico Geral");
    medicoTeste.regiaoAdministrativa = 2;
    medicoTeste.ativo = 1;

    assert(reinicializarBancoSQLite() == 1);

    assert(salvarMedicoNoBanco(&medicoTeste) == 1);
    assert(salvarMedicoNoBanco(NULL) == 0);
    totalCarregados = carregarMedicosDoBanco(lista, MAX_MEDICOS);
    assert(totalCarregados == 1);
    assert(lista[0].id == 888);
    assert(strcmp(lista[0].nome, "Medico SQLite") == 0);

    return 0;
}
