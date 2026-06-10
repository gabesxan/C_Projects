#include "exame.h"
#include "triagem.h"
#include "sqlite_db.h"

static int buscarPacienteAtivo(int pacienteId)
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

static int buscarMedicoAtivo(int medicoId)
{
    for (int i = 0; i < totalMedicos; i++)
    {
        if (medicos[i].id == medicoId && medicos[i].ativo == 1)
        {
            return i;
        }
    }

    return -1;
}

static int buscarProntuarioAtivo(int prontuarioId)
{
    for (int i = 0; i < totalProntuarios; i++)
    {
        if (prontuarios[i].id == prontuarioId && prontuarios[i].ativo == 1)
        {
            return i;
        }
    }

    return -1;
}

static int buscarExameAtivo(int exameId)
{
    for (int i = 0; i < totalExames; i++)
    {
        if (exames[i].id == exameId && exames[i].ativo == 1)
        {
            return i;
        }
    }

    return -1;
}

static int textoVazio(const char texto[])
{
    return texto[0] == '\0';
}

static const char *nomePaciente(int pacienteId)
{
    for (int i = 0; i < totalPacientes; i++)
    {
        if (pacientes[i].id == pacienteId && pacientes[i].ativo == 1)
        {
            return pacientes[i].nome;
        }
    }

    return "Paciente nao encontrado";
}

static const char *nomeMedico(int medicoId)
{
    for (int i = 0; i < totalMedicos; i++)
    {
        if (medicos[i].id == medicoId && medicos[i].ativo == 1)
        {
            return medicos[i].nome;
        }
    }

    return "Medico nao encontrado";
}

static int prontuarioCompativel(int prontuarioId, int pacienteId, int medicoId)
{
    int indiceProntuario = buscarProntuarioAtivo(prontuarioId);

    if (indiceProntuario == -1)
    {
        return 0;
    }

    return prontuarios[indiceProntuario].pacienteId == pacienteId &&
           prontuarios[indiceProntuario].medicoId == medicoId;
}

static int exameUrgentePorTriagem(int pacienteId)
{
    int indiceTriagem = triagemAtual(pacienteId);

    if (indiceTriagem == -1)
    {
        return 0;
    }

    return ehUrgente(triagens[indiceTriagem].classificacao);
}

void exibirNomeTipoExame(int tipoExame)
{
    switch (tipoExame)
    {
    case EXAME_HEMOGRAMA:
        printf("Hemograma");
        break;
    case EXAME_RAIO_X:
        printf("Raio-X");
        break;
    case EXAME_TOMOGRAFIA:
        printf("Tomografia");
        break;
    case EXAME_RESSONANCIA:
        printf("Ressonancia");
        break;
    case EXAME_ELETROCARDIOGRAMA:
        printf("Eletrocardiograma");
        break;
    case EXAME_URINA:
        printf("Urina");
        break;
    case EXAME_ULTRASSONOGRAFIA:
        printf("Ultrassonografia");
        break;
    default:
        printf("Desconhecido");
        break;
    }
}

static void exibirExame(const Exame *exame)
{
    printf("\nID: %d\n", exame->id);
    printf("Paciente: %s\n", nomePaciente(exame->pacienteId));
    printf("Medico: %s\n", nomeMedico(exame->medicoId));
    printf("Prontuario ID: %d\n", exame->prontuarioId);
    printf("Tipo: ");
    exibirNomeTipoExame(exame->tipoExame);
    printf("\nData da solicitacao: %s\n", exame->dataSolicitacao);
    printf("Data do resultado: %s\n", exame->dataResultado);
    printf("Resultado: %s\n", exame->resultado);
    printf("Status: %s\n", exame->status);
    printf("Urgente: %s\n", exame->urgente == 1 ? "Sim" : "Nao");
}

int copiarExames(Exame destino[], int maximo)
{
    int totalCopiados = 0;

    if (destino == NULL || maximo <= 0)
    {
        return 0;
    }

    for (int i = 0; i < totalExames && totalCopiados < maximo; i++)
    {
        if (exames[i].ativo == 1)
        {
            destino[totalCopiados] = exames[i];
            totalCopiados++;
        }
    }

    return totalCopiados;
}

int copiarExamesPorPaciente(int pacienteId, Exame destino[], int maximo)
{
    int totalCopiados = 0;

    if (destino == NULL || maximo <= 0)
    {
        return 0;
    }

    for (int i = 0; i < totalExames && totalCopiados < maximo; i++)
    {
        if (exames[i].ativo == 1 && exames[i].pacienteId == pacienteId)
        {
            destino[totalCopiados] = exames[i];
            totalCopiados++;
        }
    }

    return totalCopiados;
}

int copiarExamesPorMedico(int medicoId, Exame destino[], int maximo)
{
    int totalCopiados = 0;

    if (destino == NULL || maximo <= 0)
    {
        return 0;
    }

    for (int i = 0; i < totalExames && totalCopiados < maximo; i++)
    {
        if (exames[i].ativo == 1 && exames[i].medicoId == medicoId)
        {
            destino[totalCopiados] = exames[i];
            totalCopiados++;
        }
    }

    return totalCopiados;
}

int copiarExamesPorProntuario(int prontuarioId, Exame destino[], int maximo)
{
    int totalCopiados = 0;

    if (destino == NULL || maximo <= 0)
    {
        return 0;
    }

    for (int i = 0; i < totalExames && totalCopiados < maximo; i++)
    {
        if (exames[i].ativo == 1 && exames[i].prontuarioId == prontuarioId)
        {
            destino[totalCopiados] = exames[i];
            totalCopiados++;
        }
    }

    return totalCopiados;
}

int copiarExamesUrgentes(Exame destino[], int maximo)
{
    int totalCopiados = 0;

    if (destino == NULL || maximo <= 0)
    {
        return 0;
    }

    for (int i = 0; i < totalExames && totalCopiados < maximo; i++)
    {
        if (exames[i].ativo == 1 && exames[i].urgente == 1)
        {
            destino[totalCopiados] = exames[i];
            totalCopiados++;
        }
    }

    return totalCopiados;
}

int salvarExameNoBanco(const Exame *exame)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO exames "
        "(id, paciente_id, medico_id, prontuario_id, tipo_exame, data_solicitacao, data_resultado, resultado, status, urgente, ativo) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
        "ON CONFLICT(id) DO UPDATE SET "
        "paciente_id = excluded.paciente_id, "
        "medico_id = excluded.medico_id, "
        "prontuario_id = excluded.prontuario_id, "
        "tipo_exame = excluded.tipo_exame, "
        "data_solicitacao = excluded.data_solicitacao, "
        "data_resultado = excluded.data_resultado, "
        "resultado = excluded.resultado, "
        "status = excluded.status, "
        "urgente = excluded.urgente, "
        "ativo = excluded.ativo;";

    if (exame == NULL)
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

    sqlite3_bind_int(stmt, 1, exame->id);
    sqlite3_bind_int(stmt, 2, exame->pacienteId);
    sqlite3_bind_int(stmt, 3, exame->medicoId);
    sqlite3_bind_int(stmt, 4, exame->prontuarioId);
    sqlite3_bind_int(stmt, 5, exame->tipoExame);
    sqlite3_bind_text(stmt, 6, exame->dataSolicitacao, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, exame->dataResultado, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 8, exame->resultado, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 9, exame->status, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 10, exame->urgente);
    sqlite3_bind_int(stmt, 11, exame->ativo);

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

int carregarExamesDoBanco(Exame destino[], int maximo)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, paciente_id, medico_id, prontuario_id, tipo_exame, data_solicitacao, data_resultado, resultado, status, urgente, ativo "
        "FROM exames ORDER BY id;";
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
        destino[totalCarregados].prontuarioId = sqlite3_column_int(stmt, 3);
        destino[totalCarregados].tipoExame = sqlite3_column_int(stmt, 4);
        strcpy(destino[totalCarregados].dataSolicitacao, (const char *)sqlite3_column_text(stmt, 5));
        strcpy(destino[totalCarregados].dataResultado, (const char *)sqlite3_column_text(stmt, 6));
        strcpy(destino[totalCarregados].resultado, (const char *)sqlite3_column_text(stmt, 7));
        strcpy(destino[totalCarregados].status, (const char *)sqlite3_column_text(stmt, 8));
        destino[totalCarregados].urgente = sqlite3_column_int(stmt, 9);
        destino[totalCarregados].ativo = sqlite3_column_int(stmt, 10);
        totalCarregados++;
    }

    sqlite3_finalize(stmt);
    fecharBancoSQLite(db);
    return totalCarregados;
}

int escolherTipoExame(void)
{
    int tipoExame;

    printf("\nTipos de exame:\n");
    printf("1. Hemograma\n");
    printf("2. Raio-X\n");
    printf("3. Tomografia\n");
    printf("4. Ressonancia\n");
    printf("5. Eletrocardiograma\n");
    printf("6. Urina\n");
    printf("7. Ultrassonografia\n");
    printf("0. Cancelar\n");
    printf("Escolha uma opcao: ");

    if (lerInteiro(&tipoExame) == 0)
    {
        printf("\nTipo de exame invalido.\n");
        return 0;
    }

    if (tipoExame < 0 || tipoExame > EXAME_ULTRASSONOGRAFIA)
    {
        printf("\nTipo de exame invalido.\n");
        return 0;
    }

    return tipoExame;
}

int solicitarExame(int pacienteId, int medicoId, int prontuarioId,
                   int tipoExame, const char dataSolicitacao[])
{
    if (totalExames >= MAX_EXAMES)
    {
        return 0;
    }

    if (buscarPacienteAtivo(pacienteId) == -1 ||
        buscarMedicoAtivo(medicoId) == -1)
    {
        return 0;
    }

    if (prontuarioCompativel(prontuarioId, pacienteId, medicoId) == 0)
    {
        return 0;
    }

    if (tipoExame < EXAME_HEMOGRAMA || tipoExame > EXAME_ULTRASSONOGRAFIA)
    {
        return 0;
    }

    if (textoVazio(dataSolicitacao))
    {
        return 0;
    }

    exames[totalExames].id = totalExames + 1;
    exames[totalExames].pacienteId = pacienteId;
    exames[totalExames].medicoId = medicoId;
    exames[totalExames].prontuarioId = prontuarioId;
    exames[totalExames].tipoExame = tipoExame;
    strcpy(exames[totalExames].dataSolicitacao, dataSolicitacao);
    strcpy(exames[totalExames].dataResultado, "");
    strcpy(exames[totalExames].resultado, "");
    strcpy(exames[totalExames].status, "Solicitado");
    exames[totalExames].urgente = exameUrgentePorTriagem(pacienteId);
    exames[totalExames].ativo = 1;

    if (salvarExameNoBanco(&exames[totalExames]) == 0)
    {
        return 0;
    }

    totalExames++;

    return 1;
}

int registrarResultadoExame(int exameId, const char dataResultado[],
                            const char resultado[])
{
    int indiceExame = buscarExameAtivo(exameId);

    if (indiceExame == -1)
    {
        return 0;
    }

    if (strcmp(exames[indiceExame].status, "Solicitado") != 0)
    {
        return 0;
    }

    if (textoVazio(dataResultado) || textoVazio(resultado))
    {
        return 0;
    }

    strcpy(exames[indiceExame].dataResultado, dataResultado);
    strcpy(exames[indiceExame].resultado, resultado);
    strcpy(exames[indiceExame].status, "Realizado");

    if (salvarExameNoBanco(&exames[indiceExame]) == 0)
    {
        return 0;
    }

    return 1;
}

int cancelarExame(int exameId)
{
    int indiceExame = buscarExameAtivo(exameId);

    if (indiceExame == -1)
    {
        return 0;
    }

    if (strcmp(exames[indiceExame].status, "Solicitado") != 0)
    {
        return 0;
    }

    strcpy(exames[indiceExame].status, "Cancelado");
    if (salvarExameNoBanco(&exames[indiceExame]) == 0)
    {
        strcpy(exames[indiceExame].status, "Solicitado");
        return 0;
    }
    return 1;
}

int excluirExame(int exameId)
{
    int indiceExame = buscarExameAtivo(exameId);

    if (indiceExame == -1)
    {
        return 0;
    }

    exames[indiceExame].ativo = 0;
    if (salvarExameNoBanco(&exames[indiceExame]) == 0)
    {
        exames[indiceExame].ativo = 1;
        return 0;
    }
    return 1;
}

void listarExames(void)
{
    Exame lista[MAX_EXAMES];
    int totalCopiados = copiarExames(lista, MAX_EXAMES);

    if (totalCopiados == 0)
    {
        printf("\nNenhum exame cadastrado.\n");
        return;
    }

    for (int i = 0; i < totalCopiados; i++)
    {
        exibirExame(&lista[i]);
    }
}

void listarExamesPorPaciente(int pacienteId)
{
    Exame lista[MAX_EXAMES];
    int totalCopiados = copiarExamesPorPaciente(pacienteId, lista, MAX_EXAMES);

    if (totalCopiados == 0)
    {
        printf("\nNenhum exame encontrado para esse paciente.\n");
        return;
    }

    for (int i = 0; i < totalCopiados; i++)
    {
        exibirExame(&lista[i]);
    }
}

void listarExamesPorMedico(int medicoId)
{
    Exame lista[MAX_EXAMES];
    int totalCopiados = copiarExamesPorMedico(medicoId, lista, MAX_EXAMES);

    if (totalCopiados == 0)
    {
        printf("\nNenhum exame encontrado para esse medico.\n");
        return;
    }

    for (int i = 0; i < totalCopiados; i++)
    {
        exibirExame(&lista[i]);
    }
}

void listarExamesPorProntuario(int prontuarioId)
{
    Exame lista[MAX_EXAMES];
    int totalCopiados = copiarExamesPorProntuario(prontuarioId, lista, MAX_EXAMES);

    if (totalCopiados == 0)
    {
        printf("\nNenhum exame encontrado para esse prontuario.\n");
        return;
    }

    for (int i = 0; i < totalCopiados; i++)
    {
        exibirExame(&lista[i]);
    }
}

void listarExamesUrgentes(void)
{
    Exame lista[MAX_EXAMES];
    int totalCopiados = copiarExamesUrgentes(lista, MAX_EXAMES);

    if (totalCopiados == 0)
    {
        printf("\nNenhum exame urgente encontrado.\n");
        return;
    }

    for (int i = 0; i < totalCopiados; i++)
    {
        exibirExame(&lista[i]);
    }
}

int contarExamesPorStatus(const char status[])
{
    int total = 0;

    for (int i = 0; i < totalExames; i++)
    {
        if (exames[i].ativo == 1 && strcmp(exames[i].status, status) == 0)
        {
            total++;
        }
    }

    return total;
}

int contarExamesPorTipo(int tipoExame)
{
    int total = 0;

    for (int i = 0; i < totalExames; i++)
    {
        if (exames[i].ativo == 1 && exames[i].tipoExame == tipoExame)
        {
            total++;
        }
    }

    return total;
}

int contarExamesUrgentes(void)
{
    int total = 0;

    for (int i = 0; i < totalExames; i++)
    {
        if (exames[i].ativo == 1 && exames[i].urgente == 1)
        {
            total++;
        }
    }

    return total;
}

void menuExames(void)
{
    int opcao;

    do
    {
        printf("\n=============================================\n");
        printf("MENU EXAMES\n");
        printf("=============================================\n");
        printf("1. Solicitar exame\n");
        printf("2. Registrar resultado\n");
        printf("3. Cancelar exame\n");
        printf("4. Excluir exame\n");
        printf("5. Listar todos os exames\n");
        printf("6. Listar exames por paciente\n");
        printf("7. Listar exames por medico\n");
        printf("8. Listar exames por prontuario\n");
        printf("9. Listar exames urgentes\n");
        printf("0. Voltar ao menu principal\n");
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
            int medicoId;
            int prontuarioId;
            int tipoExame;
            char dataSolicitacao[11];

            printf("\nID do paciente: ");
            if (lerInteiro(&pacienteId) == 0)
            {
                printf("\nPaciente invalido.\n");
                break;
            }

            printf("ID do medico: ");
            if (lerInteiro(&medicoId) == 0)
            {
                printf("\nMedico invalido.\n");
                break;
            }

            printf("ID do prontuario: ");
            if (lerInteiro(&prontuarioId) == 0)
            {
                printf("\nProntuario invalido.\n");
                break;
            }

            tipoExame = escolherTipoExame();
            if (tipoExame == 0)
            {
                break;
            }

            printf("Data da solicitacao (DD/MM/AAAA): ");
            scanf(" %[^\n]", dataSolicitacao);

            if (solicitarExame(pacienteId, medicoId, prontuarioId,
                               tipoExame, dataSolicitacao) == 1)
            {
                printf("\nExame solicitado com sucesso.\n");
            }
            else
            {
                printf("\nNao foi possivel solicitar o exame.\n");
            }

            break;
        }

        case 2:
        {
            int exameId;
            char dataResultado[11];
            char resultado[300];

            printf("\nID do exame: ");
            if (lerInteiro(&exameId) == 0)
            {
                printf("\nExame invalido.\n");
                break;
            }

            printf("Data do resultado (DD/MM/AAAA): ");
            scanf(" %[^\n]", dataResultado);

            printf("Resultado: ");
            scanf(" %[^\n]", resultado);

            if (registrarResultadoExame(exameId, dataResultado, resultado) == 1)
            {
                printf("\nResultado registrado com sucesso.\n");
            }
            else
            {
                printf("\nNao foi possivel registrar o resultado.\n");
            }

            break;
        }

        case 3:
        {
            int exameId;

            printf("\nID do exame: ");
            if (lerInteiro(&exameId) == 0)
            {
                printf("\nExame invalido.\n");
                break;
            }

            if (cancelarExame(exameId) == 1)
            {
                printf("\nExame cancelado com sucesso.\n");
            }
            else
            {
                printf("\nNao foi possivel cancelar o exame.\n");
            }

            break;
        }

        case 4:
        {
            int exameId;

            printf("\nID do exame: ");
            if (lerInteiro(&exameId) == 0)
            {
                printf("\nExame invalido.\n");
                break;
            }

            if (excluirExame(exameId) == 1)
            {
                printf("\nExame excluido com sucesso.\n");
            }
            else
            {
                printf("\nNao foi possivel excluir o exame.\n");
            }

            break;
        }

        case 5:
            listarExames();
            break;

        case 6:
        {
            int pacienteId;

            printf("\nID do paciente: ");
            if (lerInteiro(&pacienteId) == 0)
            {
                printf("\nPaciente invalido.\n");
                break;
            }

            listarExamesPorPaciente(pacienteId);
            break;
        }

        case 7:
        {
            int medicoId;

            printf("\nID do medico: ");
            if (lerInteiro(&medicoId) == 0)
            {
                printf("\nMedico invalido.\n");
                break;
            }

            listarExamesPorMedico(medicoId);
            break;
        }

        case 8:
        {
            int prontuarioId;

            printf("\nID do prontuario: ");
            if (lerInteiro(&prontuarioId) == 0)
            {
                printf("\nProntuario invalido.\n");
                break;
            }

            listarExamesPorProntuario(prontuarioId);
            break;
        }

        case 9:
            listarExamesUrgentes();
            break;

        case 0:
            printf("\nVoltando ao menu principal...\n");
            break;

        default:
            printf("\nOpcao invalida. Tente novamente.\n");
            break;
        }

    } while (opcao != 0);
}
