#include <assert.h>
#include <string.h>
#include "paciente.h"

Paciente pacientes[MAX_PACIENTES];
int totalPacientes = 0;

int main(void)
{
    assert(cadastrarPaciente("Maria Aparecida Santos", "123.456.789-00", 62, "(61) 99999-0000", 'F', 2) == 1);
    assert(totalPacientes == 1);
    assert(pacientes[0].id == 1);
    assert(strcmp(pacientes[0].nome, "Maria Aparecida Santos") == 0);
    assert(strcmp(pacientes[0].cpf, "123.456.789-00") == 0);
    assert(pacientes[0].idade == 62);
    assert(strcmp(pacientes[0].telefone, "(61) 99999-0000") == 0);
    assert(pacientes[0].sexo == 'F');
    assert(pacientes[0].regiaoAdministrativa == 2);
    assert(pacientes[0].ativo == 1);

    assert(excluirPaciente(1) == 1);
    assert(pacientes[0].ativo == 0);
    assert(excluirPaciente(1) == 0);
    assert(excluirPaciente(99) == 0);

    return 0;
}

