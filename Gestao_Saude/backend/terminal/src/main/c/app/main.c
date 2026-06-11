#include "hospital.h"
#include "paciente.h"
#include "medico.h"
#include "agendamento.h"
#include "ala.h"
#include "leito.h"
#include "internacao.h"
#include "triagem.h"
#include "relatorio.h"
#include "prontuario.h"
#include "exame.h"
#include "sqlite_db.h"

Paciente pacientes[MAX_PACIENTES];
Medico medicos[MAX_MEDICOS];
Agendamento agendamentos[MAX_AGENDAMENTOS];
Ala alas[MAX_ALAS];
Leito leitos[MAX_LEITOS];
Internacao internacoes[MAX_INTERNACOES];
Triagem triagens[MAX_TRIAGENS];
Prontuario prontuarios[MAX_PRONTUARIOS];
Exame exames[MAX_EXAMES];

int totalPacientes = 0;
int totalMedicos = 0;
int totalAgendamentos = 0;
int totalAlas = 0;
int totalLeitos = 0;
int totalInternacoes = 0;
int totalTriagens = 0;
int totalProntuarios = 0;
int totalExames = 0;

static void carregarDadosIniciais(void)
{
    sqlite3 *db = NULL;

    if (abrirBancoSQLite(&db) == 0)
    {
        printf("\nAviso: nao foi possivel abrir o banco SQLite. O sistema iniciara apenas com dados em memoria.\n");
        return;
    }

    fecharBancoSQLite(db);

    totalPacientes = carregarPacientesDoBanco(pacientes, MAX_PACIENTES);
    totalMedicos = carregarMedicosDoBanco(medicos, MAX_MEDICOS);
    totalTriagens = carregarTriagensDoBanco(triagens, MAX_TRIAGENS);
    totalAlas = carregarAlasDoBanco(alas, MAX_ALAS);
    totalLeitos = carregarLeitosDoBanco(leitos, MAX_LEITOS);
    totalAgendamentos = carregarAgendamentosDoBanco(agendamentos, MAX_AGENDAMENTOS);
    totalInternacoes = carregarInternacoesDoBanco(internacoes, MAX_INTERNACOES);
    totalProntuarios = carregarProntuariosDoBanco(prontuarios, MAX_PRONTUARIOS);
    totalExames = carregarExamesDoBanco(exames, MAX_EXAMES);
}

static void exibirMenu(void)
{
    printf("\n=============================================\n");
    printf("SIGEH-DF - Sistema de Gestao Hospitalar do DF\n");
    printf("=============================================\n");
    printf("1. Gerenciar Pacientes\n");
    printf("2. Gerenciar Medicos\n");
    printf("3. Gerenciar Agendamentos\n");
    printf("4. Gerenciar Alas\n");
    printf("5. Gerenciar Leitos\n");
    printf("6. Gerenciar Internacoes\n");
    printf("7. Gerenciar Triagem\n");
    printf("8. Prontuarios\n");
    printf("9. Exames\n");
    printf("10. Gerar Relatorios\n");
    printf("0. Sair\n");
    printf("---------------------------------------------\n");
    printf("Escolha uma opcao: ");
}

static void executarOpcao(int opcao)
{
    switch (opcao)
    {
    case 1:
        menuPacientes();
        break;

    case 2:
        menuMedicos();
        break;

    case 3:
        menuAgendamentos();
        break;

    case 4:
        menuAlas();
        break;

    case 5:
        menuLeitos();
        break;

    case 6:
        menuInternacoes();
        break;

    case 7:
        menuTriagem();
        break;

    case 8:
        menuProntuarios();
        break;

    case 9:
        menuExames();
        break;

    case 10:
        menuRelatorios();
        break;

    case 0:
        printf("\nEncerrando o SIGEH-DF...\n");
        break;

    default:
        printf("\nOpcao invalida. Tente novamente.\n");
        break;
    }
}

int main(void)
{
    int opcao;

    carregarDadosIniciais();

    do
    {
        exibirMenu();
        if (lerInteiro(&opcao) == 0)
        {
            printf("\nOpcao invalida. Tente novamente.\n");
            continue;
        }

        executarOpcao(opcao);

    } while (opcao != 0);

    return 0;
}
