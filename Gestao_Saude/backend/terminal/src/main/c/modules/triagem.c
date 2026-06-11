#include "triagem.h"
#include "sqlite_db.h"

static int pontosResposta(int resposta, int pontos)
{
    return resposta == 1 ? pontos : 0;
}

static int bonusIdade(int idadePaciente)
{
    return (idadePaciente >= 60 || idadePaciente <= 5) ? 2 : 0;
}

static int bonusIdoso(int idadePaciente)
{
    return idadePaciente >= 60 ? 2 : 0;
}

static int bonusCrianca(int idadePaciente)
{
    return idadePaciente <= 5 ? 2 : 0;
}

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

static int pontuarTriagem(int tipoTriagem, int idadePaciente)
{
    switch (tipoTriagem)
    {
    case TRIAGEM_GERAL:
        return triagemGeral(idadePaciente);
    case TRIAGEM_ORTOPEDIA:
        return triagemOrtopedia(idadePaciente);
    case TRIAGEM_CARDIOLOGIA:
        return triagemCardiologia(idadePaciente);
    case TRIAGEM_PNEUMOLOGIA:
        return triagemPneumologia(idadePaciente);
    case TRIAGEM_PEDIATRIA:
        return triagemPediatria(idadePaciente);
    default:
        return -1;
    }
}

static void exibirTriagem(const Triagem *triagem)
{
    printf("\nID: %d\n", triagem->id);
    printf("Paciente ID: %d\n", triagem->pacienteId);
    printf("Tipo de triagem: ");
    exibirTipo(triagem->tipoTriagem);
    printf("\nPontuacao: %d\n", triagem->pontuacao);
    printf("Classificacao: %s\n", triagem->classificacao);
}

void classificarTriagem(int pontuacao, char classificacao[])
{
    if (pontuacao >= 8)
        strcpy(classificacao, "Emergencia");
    else if (pontuacao >= 5)
        strcpy(classificacao, "Muito prioritario");
    else if (pontuacao >= 3)
        strcpy(classificacao, "Prioritario");
    else if (pontuacao >= 1)
        strcpy(classificacao, "Comum");
    else
        strcpy(classificacao, "Orientacao basica");
}

int nivelPrioridade(const char classificacao[])
{
    if (strcmp(classificacao, "Emergencia") == 0)
        return 5;
    if (strcmp(classificacao, "Muito prioritario") == 0)
        return 4;
    if (strcmp(classificacao, "Prioritario") == 0)
        return 3;
    if (strcmp(classificacao, "Comum") == 0)
        return 2;
    if (strcmp(classificacao, "Orientacao basica") == 0)
        return 1;

    return 0;
}

int ehUrgente(const char classificacao[])
{
    return strcmp(classificacao, "Emergencia") == 0 ||
           strcmp(classificacao, "Muito prioritario") == 0;
}

int triagemAtual(int pacienteId)
{
    for (int i = totalTriagens - 1; i >= 0; i--)
    {
        if (triagens[i].pacienteId == pacienteId && triagens[i].ativo == 1)
        {
            return i;
        }
    }

    return -1;
}

int excluirTriagem(int id)
{
    for (int i = 0; i < totalTriagens; i++)
    {
        if (triagens[i].id == id && triagens[i].ativo == 1)
        {
            triagens[i].ativo = 0;
            if (salvarTriagemNoBanco(&triagens[i]) == 0)
            {
                triagens[i].ativo = 1;
                return -1;
            }
            return 1;
        }
    }

    return 0;
}

int salvarTriagemNoBanco(const Triagem *triagem)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO triagens "
        "(id, paciente_id, tipo_triagem, pontuacao, classificacao, ativo) "
        "VALUES (?, ?, ?, ?, ?, ?) "
        "ON CONFLICT(id) DO UPDATE SET "
        "paciente_id = excluded.paciente_id, "
        "tipo_triagem = excluded.tipo_triagem, "
        "pontuacao = excluded.pontuacao, "
        "classificacao = excluded.classificacao, "
        "ativo = excluded.ativo;";

    if (triagem == NULL)
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

    sqlite3_bind_int(stmt, 1, triagem->id);
    sqlite3_bind_int(stmt, 2, triagem->pacienteId);
    sqlite3_bind_int(stmt, 3, triagem->tipoTriagem);
    sqlite3_bind_int(stmt, 4, triagem->pontuacao);
    sqlite3_bind_text(stmt, 5, triagem->classificacao, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, triagem->ativo);

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

int carregarTriagensDoBanco(Triagem destino[], int maximo)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, paciente_id, tipo_triagem, pontuacao, classificacao, ativo "
        "FROM triagens ORDER BY id;";
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
        destino[totalCarregados].tipoTriagem = sqlite3_column_int(stmt, 2);
        destino[totalCarregados].pontuacao = sqlite3_column_int(stmt, 3);
        const char *classificacao = (const char *)sqlite3_column_text(stmt, 4);
        if (classificacao != NULL)
        {
            strcpy(destino[totalCarregados].classificacao, classificacao);
        }
        else
        {
            destino[totalCarregados].classificacao[0] = '\0';
        }
        destino[totalCarregados].ativo = sqlite3_column_int(stmt, 5);
        totalCarregados++;
    }

    sqlite3_finalize(stmt);
    fecharBancoSQLite(db);
    return totalCarregados;
}

void exibirTipo(int tipoTriagem)
{
    switch (tipoTriagem)
    {
    case TRIAGEM_GERAL:
        printf("Geral");
        break;
    case TRIAGEM_ORTOPEDIA:
        printf("Ortopedia");
        break;
    case TRIAGEM_CARDIOLOGIA:
        printf("Cardiologia");
        break;
    case TRIAGEM_PNEUMOLOGIA:
        printf("Pneumologia");
        break;
    case TRIAGEM_PEDIATRIA:
        printf("Pediatria");
        break;
    default:
        printf("Desconhecido");
        break;
    }
}

int escolherTriagem(void)
{
    int tipoTriagem;

    printf("\n=============================================\n");
    printf("SELECIONE O TIPO DE TRIAGEM\n");
    printf("=============================================\n");
    printf("1. Geral\n");
    printf("2. Ortopedia\n");
    printf("3. Cardiologia\n");
    printf("4. Pneumologia\n");
    printf("5. Pediatria\n");
    printf("0. Cancelar\n");
    printf("---------------------------------------------\n");
    printf("Escolha uma opcao: ");
    if (lerInteiro(&tipoTriagem) == 0)
    {
        printf("\nTipo de triagem invalido.\n");
        return -1;
    }

    if (tipoTriagem < 0 || tipoTriagem > 5)
    {
        printf("\nTipo de triagem invalido.\n");
        return -1;
    }

    return tipoTriagem;
}

int triagemGeral(int idadePaciente)
{
    int febre = 0;
    int faltaAr = 0;
    int dorIntensa = 0;
    int pressaoAlta = 0;
    int pontuacao = 0;

    printf("\nTRIAGEM GERAL\n");

    printf("Febre? (1-Sim / 0-Nao): ");
    lerInteiro(&febre);

    printf("Falta de ar? (1-Sim / 0-Nao): ");
    lerInteiro(&faltaAr);

    printf("Dor intensa? (1-Sim / 0-Nao): ");
    lerInteiro(&dorIntensa);

    printf("Pressao alta? (1-Sim / 0-Nao): ");
    lerInteiro(&pressaoAlta);

    pontuacao += pontosResposta(febre, 2);
    pontuacao += pontosResposta(faltaAr, 5);
    pontuacao += pontosResposta(dorIntensa, 4);
    pontuacao += pontosResposta(pressaoAlta, 2);
    pontuacao += bonusIdade(idadePaciente);

    return pontuacao;
}

int triagemOrtopedia(int idadePaciente)
{
    int suspeitaFratura = 0;
    int deformidadeVisivel = 0;
    int dificuldadeMovimento = 0;
    int dorIntensa = 0;
    int inchacoIntenso = 0;
    int sangramento = 0;
    int perdaSensibilidade = 0;
    int pontuacao = 0;

    printf("\nTRIAGEM ORTOPEDICA\n");

    printf("Suspeita de fratura? (1-Sim / 0-Nao): ");
    lerInteiro(&suspeitaFratura);

    printf("Deformidade visivel? (1-Sim / 0-Nao): ");
    lerInteiro(&deformidadeVisivel);

    printf("Dificuldade ou impossibilidade de andar/mover? (1-Sim / 0-Nao): ");
    lerInteiro(&dificuldadeMovimento);

    printf("Dor intensa? (1-Sim / 0-Nao): ");
    lerInteiro(&dorIntensa);

    printf("Inchaco intenso? (1-Sim / 0-Nao): ");
    lerInteiro(&inchacoIntenso);

    printf("Sangramento? (1-Sim / 0-Nao): ");
    lerInteiro(&sangramento);

    printf("Perda de sensibilidade? (1-Sim / 0-Nao): ");
    lerInteiro(&perdaSensibilidade);

    pontuacao += pontosResposta(suspeitaFratura, 5);
    pontuacao += pontosResposta(deformidadeVisivel, 5);
    pontuacao += pontosResposta(dificuldadeMovimento, 4);
    pontuacao += pontosResposta(dorIntensa, 4);
    pontuacao += pontosResposta(inchacoIntenso, 2);
    pontuacao += pontosResposta(sangramento, 3);
    pontuacao += pontosResposta(perdaSensibilidade, 5);
    pontuacao += bonusIdade(idadePaciente);

    return pontuacao;
}

int triagemCardiologia(int idadePaciente)
{
    int dorPeito = 0;
    int faltaAr = 0;
    int desmaio = 0;
    int palpitacaoIntensa = 0;
    int pressaoAlta = 0;
    int pontuacao = 0;

    printf("\nTRIAGEM CARDIOLOGICA\n");

    printf("Dor no peito? (1-Sim / 0-Nao): ");
    lerInteiro(&dorPeito);

    printf("Falta de ar? (1-Sim / 0-Nao): ");
    lerInteiro(&faltaAr);

    printf("Desmaio ou quase desmaio? (1-Sim / 0-Nao): ");
    lerInteiro(&desmaio);

    printf("Palpitacao intensa? (1-Sim / 0-Nao): ");
    lerInteiro(&palpitacaoIntensa);

    printf("Pressao alta? (1-Sim / 0-Nao): ");
    lerInteiro(&pressaoAlta);

    pontuacao += pontosResposta(dorPeito, 5);
    pontuacao += pontosResposta(faltaAr, 4);
    pontuacao += pontosResposta(desmaio, 5);
    pontuacao += pontosResposta(palpitacaoIntensa, 3);
    pontuacao += pontosResposta(pressaoAlta, 2);
    pontuacao += bonusIdoso(idadePaciente);

    return pontuacao;
}

int triagemPneumologia(int idadePaciente)
{
    int faltaAr = 0;
    int chiadoPeito = 0;
    int tosseIntensa = 0;
    int dorRespirar = 0;
    int saturacaoBaixa = 0;
    int febre = 0;
    int pontuacao = 0;

    printf("\nTRIAGEM PNEUMOLOGICA\n");

    printf("Falta de ar? (1-Sim / 0-Nao): ");
    lerInteiro(&faltaAr);

    printf("Chiado no peito? (1-Sim / 0-Nao): ");
    lerInteiro(&chiadoPeito);

    printf("Tosse intensa? (1-Sim / 0-Nao): ");
    lerInteiro(&tosseIntensa);

    printf("Dor ao respirar? (1-Sim / 0-Nao): ");
    lerInteiro(&dorRespirar);

    printf("Saturacao baixa? (1-Sim / 0-Nao): ");
    lerInteiro(&saturacaoBaixa);

    printf("Febre? (1-Sim / 0-Nao): ");
    lerInteiro(&febre);

    pontuacao += pontosResposta(faltaAr, 5);
    pontuacao += pontosResposta(chiadoPeito, 3);
    pontuacao += pontosResposta(tosseIntensa, 2);
    pontuacao += pontosResposta(dorRespirar, 3);
    pontuacao += pontosResposta(saturacaoBaixa, 5);
    pontuacao += pontosResposta(febre, 2);
    pontuacao += bonusIdade(idadePaciente);

    return pontuacao;
}

int triagemPediatria(int idadePaciente)
{
    int febreAlta = 0;
    int vomitosPersistentes = 0;
    int convulsao = 0;
    int prostracaoIntensa = 0;
    int dificuldadeRespirar = 0;
    int pontuacao = 0;

    printf("\nTRIAGEM PEDIATRICA\n");

    printf("Febre alta? (1-Sim / 0-Nao): ");
    lerInteiro(&febreAlta);

    printf("Vomitos persistentes? (1-Sim / 0-Nao): ");
    lerInteiro(&vomitosPersistentes);

    printf("Convulsao? (1-Sim / 0-Nao): ");
    lerInteiro(&convulsao);

    printf("Prostracao intensa? (1-Sim / 0-Nao): ");
    lerInteiro(&prostracaoIntensa);

    printf("Dificuldade para respirar? (1-Sim / 0-Nao): ");
    lerInteiro(&dificuldadeRespirar);

    pontuacao += pontosResposta(febreAlta, 3);
    pontuacao += pontosResposta(vomitosPersistentes, 3);
    pontuacao += pontosResposta(convulsao, 6);
    pontuacao += pontosResposta(prostracaoIntensa, 5);
    pontuacao += pontosResposta(dificuldadeRespirar, 5);
    pontuacao += bonusCrianca(idadePaciente);

    return pontuacao;
}

void menuTriagem(void)
{
    int caso7;

    do
    {
        printf("\n=============================================\n");
        printf("MENU GERENCIAMENTO DE TRIAGEM\n");
        printf("=============================================\n");
        printf("1. Realizar Triagem\n");
        printf("2. Listar Triagens\n");
        printf("3. Excluir Triagem\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("---------------------------------------------\n");
        printf("Escolha uma opcao: ");
        if (lerInteiro(&caso7) == 0)
        {
            printf("\nOpcao invalida. Tente novamente.\n");
            continue;
        }

        switch (caso7)
        {
        case 1:
        {
            int pacienteId;
            int indicePaciente;
            int idadePaciente;
            int tipoTriagem;
            int pontuacao;

            if (totalTriagens >= MAX_TRIAGENS)
            {
                printf("\nLimite de triagens atingido.\n");
                break;
            }

            printf("\nID do paciente: ");
            if (lerInteiro(&pacienteId) == 0)
            {
                printf("\nPaciente invalido.\n");
                break;
            }

            indicePaciente = buscarPaciente(pacienteId);

            if (indicePaciente == -1)
            {
                printf("\nPaciente nao encontrado ou inativo.\n");
                break;
            }

            idadePaciente = pacientes[indicePaciente].idade;

            tipoTriagem = escolherTriagem();

            if (tipoTriagem == 0)
            {
                printf("\nTriagem cancelada.\n");
                break;
            }

            if (tipoTriagem == -1)
            {
                break;
            }

            pontuacao = pontuarTriagem(tipoTriagem, idadePaciente);

            if (pontuacao < 0)
            {
                printf("\nTipo de triagem invalido.\n");
                break;
            }

            triagens[totalTriagens].id = totalTriagens + 1;
            triagens[totalTriagens].pacienteId = pacienteId;
            triagens[totalTriagens].tipoTriagem = tipoTriagem;
            triagens[totalTriagens].pontuacao = pontuacao;
            triagens[totalTriagens].ativo = 1;

            classificarTriagem(
                triagens[totalTriagens].pontuacao,
                triagens[totalTriagens].classificacao);

            if (salvarTriagemNoBanco(&triagens[totalTriagens]) == 0)
            {
                printf("\nNao foi possivel salvar a triagem no banco.\n");
                break;
            }

            printf("\nTriagem registrada com sucesso.\n");
            exibirTriagem(&triagens[totalTriagens]);

            totalTriagens++;
            break;
        }

        case 2:
        {
            int encontrouAtiva = 0;

            if (totalTriagens == 0)
            {
                printf("\nNenhuma triagem cadastrada.\n");
                break;
            }

            printf("\nLista de Triagens:\n");

            for (int i = 0; i < totalTriagens; i++)
            {
                if (triagens[i].ativo == 1)
                {
                    encontrouAtiva = 1;
                    exibirTriagem(&triagens[i]);
                }
            }

            if (encontrouAtiva == 0)
            {
                printf("\nNenhuma triagem ativa cadastrada.\n");
            }

            break;
        }

        case 3:
        {
            int idBusca;

            printf("\nDigite o ID da triagem que deseja excluir: ");
            if (lerInteiro(&idBusca) == 0)
            {
                printf("\nID invalido.\n");
                break;
            }

            int resultado = excluirTriagem(idBusca);

            if (resultado == 1)
            {
                printf("\nTriagem removida com sucesso.\n");
            }
            else if (resultado == -1)
            {
                printf("\nNao foi possivel salvar a exclusao no banco.\n");
            }
            else
            {
                printf("\nTriagem nao encontrada ou ja inativa.\n");
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

    } while (caso7 != 0);
}
