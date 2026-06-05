#include <assert.h>
#include <string.h>
#include "medico.h"

Medico medicos[MAX_MEDICOS];
int totalMedicos = 0;

int main(void)
{
    assert(cadastrarMedico("Carlos Henrique Almeida", "12345", "Cardiologia") == 1);
    assert(totalMedicos == 1);
    assert(medicos[0].id == 1);
    assert(strcmp(medicos[0].nome, "Carlos Henrique Almeida") == 0);
    assert(strcmp(medicos[0].crm, "12345") == 0);
    assert(strcmp(medicos[0].especialidade, "Cardiologia") == 0);
    assert(medicos[0].ativo == 1);

    assert(excluirMedico(1) == 1);
    assert(medicos[0].ativo == 0);
    assert(excluirMedico(1) == 0);
    assert(excluirMedico(99) == 0);

    return 0;
}
