#include <assert.h>
#include <string.h>
#include "triagem.h"
#include "sqlite_db.h"

Paciente pacientes[MAX_PACIENTES];
Triagem triagens[MAX_TRIAGENS];

int totalPacientes = 0;
int totalTriagens = 0;

int main(void)
{
    char classificacao[30];
    Triagem lista[MAX_TRIAGENS];
    int totalCarregados;

    assert(definirCaminhoBancoSQLite("/tmp/sigeh_test_triagem.db") == 1);
    assert(reinicializarBancoSQLite() == 1);
    assert(executarSQLSQLite("INSERT INTO pacientes (id, nome, cpf, idade, telefone, sexo, regiao_administrativa, ativo) VALUES (777, 'Paciente Triagem', '777.777.777-77', 25, '(61) 90000-7777', 'M', 1, 1);") == 1);

    classificarTriagem(0, classificacao);
    assert(strcmp(classificacao, "Orientacao basica") == 0);

    classificarTriagem(2, classificacao);
    assert(strcmp(classificacao, "Comum") == 0);

    classificarTriagem(4, classificacao);
    assert(strcmp(classificacao, "Prioritario") == 0);

    classificarTriagem(6, classificacao);
    assert(strcmp(classificacao, "Muito prioritario") == 0);

    classificarTriagem(10, classificacao);
    assert(strcmp(classificacao, "Emergencia") == 0);

    assert(nivelPrioridade("Emergencia") == 5);
    assert(nivelPrioridade("Muito prioritario") == 4);
    assert(nivelPrioridade("Prioritario") == 3);
    assert(nivelPrioridade("Comum") == 2);
    assert(nivelPrioridade("Orientacao basica") == 1);
    assert(nivelPrioridade("Invalida") == 0);

    assert(ehUrgente("Emergencia") == 1);
    assert(ehUrgente("Muito prioritario") == 1);
    assert(ehUrgente("Prioritario") == 0);
    assert(ehUrgente("Comum") == 0);
    assert(ehUrgente("Orientacao basica") == 0);

    totalTriagens = 1;
    triagens[0].id = 1;
    triagens[0].pacienteId = 1;
    triagens[0].tipoTriagem = TRIAGEM_ORTOPEDIA;
    triagens[0].pontuacao = 9;
    strcpy(triagens[0].classificacao, "Emergencia");
    triagens[0].ativo = 1;

    assert(triagens[0].ativo == 1);
    assert(triagens[0].tipoTriagem == TRIAGEM_ORTOPEDIA);
    assert(excluirTriagem(1) == 1);
    assert(triagens[0].ativo == 0);
    assert(excluirTriagem(1) == 0);
    assert(excluirTriagem(99) == 0);

    totalTriagens = 0;
    triagens[0].id = 1;
    triagens[0].pacienteId = 2;
    triagens[0].tipoTriagem = TRIAGEM_GERAL;
    triagens[0].pontuacao = 2;
    strcpy(triagens[0].classificacao, "Comum");
    triagens[0].ativo = 1;

    triagens[1].id = 2;
    triagens[1].pacienteId = 2;
    triagens[1].tipoTriagem = TRIAGEM_CARDIOLOGIA;
    triagens[1].pontuacao = 9;
    strcpy(triagens[1].classificacao, "Emergencia");
    triagens[1].ativo = 1;

    triagens[2].id = 3;
    triagens[2].pacienteId = 3;
    triagens[2].tipoTriagem = TRIAGEM_GERAL;
    triagens[2].pontuacao = 1;
    strcpy(triagens[2].classificacao, "Comum");
    triagens[2].ativo = 0;
    totalTriagens = 3;

    assert(triagemAtual(2) == 1);
    assert(triagemAtual(3) == -1);
    assert(triagemAtual(99) == -1);

    assert(reinicializarBancoSQLite() == 1);
    assert(executarSQLSQLite("INSERT INTO pacientes (id, nome, cpf, idade, telefone, sexo, regiao_administrativa, ativo) VALUES (777, 'Paciente Triagem', '777.777.777-77', 25, '(61) 90000-7777', 'M', 1, 1);") == 1);

    triagens[0].id = 777;
    triagens[0].pacienteId = 777;
    triagens[0].tipoTriagem = TRIAGEM_GERAL;
    triagens[0].pontuacao = 6;
    strcpy(triagens[0].classificacao, "Muito prioritario");
    triagens[0].ativo = 1;

    assert(salvarTriagemNoBanco(&triagens[0]) == 1);
    assert(salvarTriagemNoBanco(NULL) == 0);
    totalCarregados = carregarTriagensDoBanco(lista, MAX_TRIAGENS);
    assert(totalCarregados == 1);
    assert(lista[0].id == 777);
    assert(lista[0].pacienteId == 777);
    assert(strcmp(lista[0].classificacao, "Muito prioritario") == 0);

    return 0;
}
