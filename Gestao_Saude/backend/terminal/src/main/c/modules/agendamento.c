#include "agendamento.h"
#include "prontuario.h"
#include "triagem.h"
#include "sqlite_db.h"

static int buscarPaciente(int pacienteId)
{
    for (int i = 0; i < totalPacientes; i++)
    {
        if (pacientes[i].id == pacienteId && pacientes[i].ativo == 1)
        {
            return i;
        }
    }

    return -1;
}

static int mudarStatus(int id, const char status[])
{
    for (int i = 0; i < totalAgendamentos; i++)
    {
        if (agendamentos[i].id == id &&
            strcmp(agendamentos[i].status, "AGENDADO") == 0)
        {
            char statusAnterior[20];

            strcpy(statusAnterior, agendamentos[i].status);
            strcpy(agendamentos[i].status, status);
            if (salvarAgendamentoNoBanco(&agendamentos[i]) == 0)
            {
                strcpy(agendamentos[i].status, statusAnterior);
                return 0;
            }
            return 1;
        }
    }

    return 0;
}

static int buscarAgendamentoPorId(int id)
{
    for (int i = 0; i < totalAgendamentos; i++)
    {
        if (agendamentos[i].id == id)
        {
            return i;
        }
    }

    return -1;
}

static void exibirAgendamento(const Agendamento *agendamento)
{
    printf("\nID: %d\n", agendamento->id);
    printf("Paciente ID: %d\n", agendamento->pacienteId);
    printf("Medico ID: %d\n", agendamento->medicoId);
    printf("Data: %s\n", agendamento->data);
    printf("Horario: %s\n", agendamento->horario);
    printf("Status: %s\n", agendamento->status);
}

static void listarTodosAgendamentos(void)
{
    Agendamento lista[MAX_AGENDAMENTOS];
    int totalCopiados = copiarAgendamentos(lista, MAX_AGENDAMENTOS);

    if (totalCopiados == 0)
    {
        printf("\nNenhum agendamento cadastrado.\n");
        return;
    }

    printf("\nLista de Agendamentos:\n");

    for (int i = 0; i < totalCopiados; i++)
    {
        exibirAgendamento(&lista[i]);
    }
}

int buscarAgenda(int medicoId, char data[], char horario[])
{
    for (int i = 0; i < totalAgendamentos; i++)
    {
        if (agendamentos[i].medicoId == medicoId &&
            strcmp(agendamentos[i].data, data) == 0 &&
            strcmp(agendamentos[i].horario, horario) == 0 &&
            strcmp(agendamentos[i].status, "AGENDADO") == 0)
        {
            return i;
        }
    }

    return -1;
}

int medicoOcupado(int medicoId, char data[], char horario[])
{
    return buscarAgenda(medicoId, data, horario) != -1;
}

static int nivelRegiao(int regiaoPaciente, int regiaoMedico)
{
    if (regiaoPaciente == regiaoMedico)
        return 0;

    if ((regiaoPaciente == 1 && regiaoMedico == 7) ||
        (regiaoPaciente == 7 && regiaoMedico == 1) ||
        (regiaoPaciente == 2 && regiaoMedico == 3) ||
        (regiaoPaciente == 3 && regiaoMedico == 2) ||
        (regiaoPaciente == 3 && regiaoMedico == 8) ||
        (regiaoPaciente == 8 && regiaoMedico == 3) ||
        (regiaoPaciente == 4 && regiaoMedico == 2) ||
        (regiaoPaciente == 2 && regiaoMedico == 4) ||
        (regiaoPaciente == 5 && regiaoMedico == 4) ||
        (regiaoPaciente == 4 && regiaoMedico == 5) ||
        (regiaoPaciente == 6 && regiaoMedico == 1) ||
        (regiaoPaciente == 1 && regiaoMedico == 6))
        return 1;

    return 2;
}

int cancelarAgendamento(int id)
{
    return mudarStatus(id, "CANCELADO");
}

int concluirAgendamento(int id)
{
    int indiceAgendamento = buscarAgendamentoPorId(id);

    if (indiceAgendamento == -1)
    {
        return 0;
    }

    if (strcmp(agendamentos[indiceAgendamento].status, "AGENDADO") != 0)
    {
        return 0;
    }

    strcpy(agendamentos[indiceAgendamento].status, "CONCLUIDO");

    if (criarProntuarioAutomatico(agendamentos[indiceAgendamento].pacienteId,
                                  agendamentos[indiceAgendamento].medicoId,
                                  agendamentos[indiceAgendamento].data) == 0)
    {
        strcpy(agendamentos[indiceAgendamento].status, "AGENDADO");
        return 0;
    }

    if (salvarAgendamentoNoBanco(&agendamentos[indiceAgendamento]) == 0)
    {
        strcpy(agendamentos[indiceAgendamento].status, "AGENDADO");
        return 0;
    }

    return 1;
}

int criarAgendamentoTriagem(int pacienteId, char data[], char horario[], int *agendamentoId, int *medicoId)
{
    int indicePaciente = buscarPaciente(pacienteId);
    int medicoSelecionado;

    if (indicePaciente == -1)
    {
        return 0;
    }

    medicoSelecionado = agendarTriagem(pacienteId, data, horario);

    if (medicoSelecionado == 0)
    {
        return 0;
    }

    if (agendamentoId != NULL)
    {
        *agendamentoId = agendamentos[totalAgendamentos - 1].id;
    }

    if (medicoId != NULL)
    {
        *medicoId = medicoSelecionado;
    }

    return 1;
}

int salvarAgendamentoNoBanco(const Agendamento *agendamento)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO agendamentos "
        "(id, paciente_id, medico_id, data, horario, status) "
        "VALUES (?, ?, ?, ?, ?, ?) "
        "ON CONFLICT(id) DO UPDATE SET "
        "paciente_id = excluded.paciente_id, "
        "medico_id = excluded.medico_id, "
        "data = excluded.data, "
        "horario = excluded.horario, "
        "status = excluded.status;";

    if (agendamento == NULL)
    {
        return 0;
    }

    if (abrirBancoSQLite(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        fecharBancoSQLite(db);
        return 0;
    }

    sqlite3_bind_int(stmt, 1, agendamento->id);
    sqlite3_bind_int(stmt, 2, agendamento->pacienteId);
    sqlite3_bind_int(stmt, 3, agendamento->medicoId);
    sqlite3_bind_text(stmt, 4, agendamento->data, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, agendamento->horario, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, agendamento->status, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        fecharBancoSQLite(db);
        return 0;
    }

    sqlite3_finalize(stmt);
    fecharBancoSQLite(db);
    return 1;
}

int carregarAgendamentosDoBanco(Agendamento destino[], int maximo)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, paciente_id, medico_id, data, horario, status "
        "FROM agendamentos ORDER BY id;";
    int totalCarregados = 0;

    if (destino == NULL || maximo <= 0)
    {
        return 0;
    }

    if (abrirBancoSQLite(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        fecharBancoSQLite(db);
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW && totalCarregados < maximo)
    {
        destino[totalCarregados].id = sqlite3_column_int(stmt, 0);
        destino[totalCarregados].pacienteId = sqlite3_column_int(stmt, 1);
        destino[totalCarregados].medicoId = sqlite3_column_int(stmt, 2);
        strcpy(destino[totalCarregados].data, (const char *)sqlite3_column_text(stmt, 3));
        strcpy(destino[totalCarregados].horario, (const char *)sqlite3_column_text(stmt, 4));
        strcpy(destino[totalCarregados].status, (const char *)sqlite3_column_text(stmt, 5));
        totalCarregados++;
    }

    sqlite3_finalize(stmt);
    fecharBancoSQLite(db);
    return totalCarregados;
}

int copiarAgendamentos(Agendamento destino[], int maximo)
{
    int totalCopiados = 0;

    if (destino == NULL || maximo <= 0)
    {
        return 0;
    }

    for (int i = 0; i < totalAgendamentos && totalCopiados < maximo; i++)
    {
        /* Agendamento preserva historico por status; a consulta retorna todos os registros. */
        destino[totalCopiados] = agendamentos[i];
        totalCopiados++;
    }

    return totalCopiados;
}

int copiarAgendamentosPorPaciente(int pacienteId, Agendamento destino[], int maximo)
{
    int totalCopiados = 0;

    if (destino == NULL || maximo <= 0)
    {
        return 0;
    }

    for (int i = 0; i < totalAgendamentos && totalCopiados < maximo; i++)
    {
        if (agendamentos[i].pacienteId == pacienteId)
        {
            destino[totalCopiados] = agendamentos[i];
            totalCopiados++;
        }
    }

    return totalCopiados;
}

int copiarAgendamentosPorMedico(int medicoId, Agendamento destino[], int maximo)
{
    int totalCopiados = 0;

    if (destino == NULL || maximo <= 0)
    {
        return 0;
    }

    for (int i = 0; i < totalAgendamentos && totalCopiados < maximo; i++)
    {
        if (agendamentos[i].medicoId == medicoId)
        {
            destino[totalCopiados] = agendamentos[i];
            totalCopiados++;
        }
    }

    return totalCopiados;
}

static int criarAgendamento(int pacienteId, int medicoId, char data[], char horario[])
{
    if (totalAgendamentos >= MAX_AGENDAMENTOS)
    {
        return 0;
    }

    agendamentos[totalAgendamentos].id = totalAgendamentos + 1;
    agendamentos[totalAgendamentos].pacienteId = pacienteId;
    agendamentos[totalAgendamentos].medicoId = medicoId;
    strcpy(agendamentos[totalAgendamentos].data, data);
    strcpy(agendamentos[totalAgendamentos].horario, horario);
    strcpy(agendamentos[totalAgendamentos].status, "AGENDADO");

    if (salvarAgendamentoNoBanco(&agendamentos[totalAgendamentos]) == 0)
    {
        return 0;
    }

    totalAgendamentos++;

    return medicoId;
}

const char *obterEspecialidade(int tipoTriagem)
{
    switch (tipoTriagem)
    {
    case TRIAGEM_ORTOPEDIA:
        return "Ortopedia";
    case TRIAGEM_CARDIOLOGIA:
        return "Cardiologia";
    case TRIAGEM_PNEUMOLOGIA:
        return "Pneumologia";
    case TRIAGEM_PEDIATRIA:
        return "Pediatria";
    case TRIAGEM_GERAL:
        return "Clinico Geral";
    default:
        return "";
    }
}

int buscarMedicoRegiao(const char especialidade[], int regiaoAdministrativa, char data[], char horario[])
{
    for (int i = 0; i < totalMedicos; i++)
    {
        if (medicos[i].ativo == 1 &&
            strcmp(medicos[i].especialidade, especialidade) == 0 &&
            medicos[i].regiaoAdministrativa == regiaoAdministrativa &&
            medicoOcupado(medicos[i].id, data, horario) == 0)
        {
            return medicos[i].id;
        }
    }

    return 0;
}

int buscarMedico(const char especialidade[], char data[], char horario[])
{
    for (int i = 0; i < totalMedicos; i++)
    {
        if (medicos[i].ativo == 1 &&
            strcmp(medicos[i].especialidade, especialidade) == 0 &&
            medicoOcupado(medicos[i].id, data, horario) == 0)
        {
            return medicos[i].id;
        }
    }

    return 0;
}

int trocaHorario(int pacienteNovo, int pacienteAtual)
{
    int indiceNova = triagemAtual(pacienteNovo);
    int indiceAtual = triagemAtual(pacienteAtual);
    int prioridadeNova;
    int prioridadeAtual;

    if (indiceNova == -1 || indiceAtual == -1)
    {
        return 0;
    }

    if (ehUrgente(triagens[indiceNova].classificacao) == 0)
    {
        return 0;
    }

    prioridadeNova = nivelPrioridade(triagens[indiceNova].classificacao);
    prioridadeAtual = nivelPrioridade(triagens[indiceAtual].classificacao);

    return prioridadeNova > prioridadeAtual;
}

int agendarMedico(int medicoId, int pacienteId, char data[], char horario[])
{
    int agendamentoAtual = buscarAgenda(medicoId, data, horario);
    int pacienteAtualId;

    if (agendamentoAtual == -1)
    {
        return criarAgendamento(pacienteId, medicoId, data, horario);
    }

    pacienteAtualId = agendamentos[agendamentoAtual].pacienteId;

    if (trocaHorario(pacienteId, pacienteAtualId) == 0)
    {
        return 0;
    }

    strcpy(agendamentos[agendamentoAtual].status, "REMANEJADO");

    return criarAgendamento(pacienteId, medicoId, data, horario);
}

static int remanejarRegiao(const char especialidade[], int regiaoAdministrativa, int pacienteId, char data[], char horario[])
{
    int medicoSelecionado;

    for (int i = 0; i < totalMedicos; i++)
    {
        if (medicos[i].ativo == 1 &&
            strcmp(medicos[i].especialidade, especialidade) == 0 &&
            medicos[i].regiaoAdministrativa == regiaoAdministrativa &&
            medicoOcupado(medicos[i].id, data, horario) == 1)
        {
            medicoSelecionado = agendarMedico(medicos[i].id, pacienteId, data, horario);

            if (medicoSelecionado != 0)
            {
                return medicoSelecionado;
            }
        }
    }

    return 0;
}

static int buscarProxima(const char especialidade[], int regiaoAdministrativa, char data[], char horario[])
{
    int melhorMedico = 0;
    int melhorPrioridade = 99;

    for (int i = 0; i < totalMedicos; i++)
    {
        if (medicos[i].ativo == 1 &&
            strcmp(medicos[i].especialidade, especialidade) == 0 &&
            medicos[i].regiaoAdministrativa != regiaoAdministrativa &&
            medicoOcupado(medicos[i].id, data, horario) == 0)
        {
            int prioridade = nivelRegiao(regiaoAdministrativa, medicos[i].regiaoAdministrativa);

            if (prioridade < melhorPrioridade)
            {
                melhorPrioridade = prioridade;
                melhorMedico = medicos[i].id;
            }
        }
    }

    return melhorMedico;
}

static int remanejarExterno(const char especialidade[], int regiaoAdministrativa, int pacienteId, char data[], char horario[])
{
    int medicoSelecionado;

    for (int i = 0; i < totalMedicos; i++)
    {
        if (medicos[i].ativo == 1 &&
            strcmp(medicos[i].especialidade, especialidade) == 0 &&
            medicos[i].regiaoAdministrativa != regiaoAdministrativa &&
            medicoOcupado(medicos[i].id, data, horario) == 1)
        {
            medicoSelecionado = agendarMedico(medicos[i].id, pacienteId, data, horario);

            if (medicoSelecionado != 0)
            {
                return medicoSelecionado;
            }
        }
    }

    return 0;
}

int agendarTriagem(int pacienteId, char data[], char horario[])
{
    int regiaoPaciente = 0;
    int triagemId;
    int medicoId = 0;
    const char *especialidade;

    if (totalAgendamentos >= MAX_AGENDAMENTOS)
    {
        return 0;
    }

    for (int i = 0; i < totalPacientes; i++)
    {
        if (pacientes[i].id == pacienteId && pacientes[i].ativo == 1)
        {
            regiaoPaciente = pacientes[i].regiaoAdministrativa;
            break;
        }
    }

    if (regiaoPaciente == 0)
    {
        return 0;
    }

    triagemId = triagemAtual(pacienteId);

    if (triagemId == -1)
    {
        return 0;
    }

    especialidade = obterEspecialidade(triagens[triagemId].tipoTriagem);

    if (strlen(especialidade) == 0)
    {
        return 0;
    }

    medicoId = buscarMedicoRegiao(especialidade, regiaoPaciente, data, horario);

    if (medicoId != 0)
    {
        return agendarMedico(medicoId, pacienteId, data, horario);
    }

    medicoId = remanejarRegiao(especialidade, regiaoPaciente, pacienteId, data, horario);

    if (medicoId != 0)
    {
        return medicoId;
    }

    medicoId = buscarProxima(especialidade, regiaoPaciente, data, horario);

    if (medicoId != 0)
    {
        return agendarMedico(medicoId, pacienteId, data, horario);
    }

    return remanejarExterno(especialidade, regiaoPaciente, pacienteId, data, horario);
}

void menuAgendamentos(void)
{
    int opcao;

    do
    {
        printf("\n=============================================\n");
        printf("MENU GERENCIAMENTO DE AGENDAMENTOS\n");
        printf("=============================================\n");
        printf("1. Criar Agendamento\n");
        printf("2. Listar Agendamentos\n");
        printf("3. Cancelar Agendamento\n");
        printf("4. Concluir Agendamento\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("---------------------------------------------\n");
        printf("Escolha uma opcao: ");

        if (lerInteiro(&opcao) == 0)
        {
            printf("\nOpcao invalida. Tente novamente.\n");
            continue;
        }

        switch (opcao)
        {
        case 1:
        {
            int pacienteId;
            int agendamentoId;
            int medicoSelecionado;
            char data[11];
            char horario[6];

            if (totalAgendamentos >= MAX_AGENDAMENTOS)
            {
                printf("\nLimite de agendamentos atingido.\n");
                break;
            }

            printf("\nID do paciente: ");
            if (lerInteiro(&pacienteId) == 0)
            {
                printf("\nID invalido.\n");
                break;
            }

            if (buscarPaciente(pacienteId) == -1)
            {
                printf("\nPaciente nao encontrado ou inativo.\n");
                break;
            }

            printf("Data (DD/MM/AAAA): ");
            scanf(" %[^\n]", data);

            printf("Horario (HH:MM): ");
            scanf(" %[^\n]", horario);

            if (criarAgendamentoTriagem(pacienteId, data, horario,
                                        &agendamentoId, &medicoSelecionado) == 0)
            {
                printf("\nNao foi possivel criar o agendamento.\n");
                printf("Verifique se o paciente possui triagem ativa e se ha medico disponivel.\n");
                break;
            }

            printf("\nAgendamento criado com sucesso! ID: %d\n", agendamentoId);
            printf("Medico selecionado automaticamente. ID: %d\n", medicoSelecionado);
            break;
        }

        case 2:
            listarTodosAgendamentos();
            break;

        case 3:
        {
            int idBusca;

            printf("\nDigite o ID do agendamento que deseja cancelar: ");
            if (lerInteiro(&idBusca) == 0)
            {
                printf("\nID invalido.\n");
                break;
            }

            if (cancelarAgendamento(idBusca) == 1)
            {
                printf("\nAgendamento cancelado com sucesso.\n");
            }
            else
            {
                printf("\nNao foi possivel cancelar o agendamento.\n");
            }

            break;
        }

        case 4:
        {
            int idBusca;

            printf("\nDigite o ID do agendamento que deseja concluir: ");
            if (lerInteiro(&idBusca) == 0)
            {
                printf("\nID invalido.\n");
                break;
            }

            if (concluirAgendamento(idBusca) == 1)
            {
                printf("\nAgendamento concluido com sucesso.\n");
            }
            else
            {
                printf("\nNao foi possivel concluir o agendamento.\n");
            }

            break;
        }

        case 0:
            printf("\nVoltando ao menu principal...\n");
            break;

        default:
            printf("\nOpcao invalida. Tente novamente.\n");
            break;
        }

    } while (opcao != 0);
}
