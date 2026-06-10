#include <assert.h>
#include <string.h>
#include "paciente.h"
#include "sqlite_db.h"

Paciente pacientes[MAX_PACIENTES];
int totalPacientes = 0;
Paciente pacienteTeste;

int main(void)
{
    Paciente lista[MAX_PACIENTES];
    int totalCarregados;

    assert(executarSQLSQLite("DELETE FROM exames;") == 1);
    assert(executarSQLSQLite("DELETE FROM prontuarios;") == 1);
    assert(executarSQLSQLite("DELETE FROM agendamentos;") == 1);
    assert(executarSQLSQLite("DELETE FROM triagens;") == 1);
    assert(executarSQLSQLite("DELETE FROM pacientes;") == 1);

    pacienteTeste.id = 999;
    strcpy(pacienteTeste.nome, "Paciente SQLite");
    strcpy(pacienteTeste.cpf, "999.999.999-99");
    pacienteTeste.idade = 30;
    strcpy(pacienteTeste.telefone, "(61) 99999-9999");
    pacienteTeste.sexo = 'F';
    pacienteTeste.regiaoAdministrativa = 1;
    pacienteTeste.ativo = 1;
    
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
    assert(salvarPacienteNoBanco(&pacienteTeste) == 1);
    assert(salvarPacienteNoBanco(NULL) == 0);
    totalCarregados = carregarPacientesDoBanco(lista, MAX_PACIENTES);
    assert(totalCarregados == 1);
    assert(lista[0].id == 999);
    assert(strcmp(lista[0].nome, "Paciente SQLite") == 0);
    return 0;
}
