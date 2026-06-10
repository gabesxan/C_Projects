#include "medico.h"
#include "sqlite_db.h"

static void listarRegioes(void)
{
    printf("1. Plano Piloto\n");
    printf("2. Ceilandia\n");
    printf("3. Taguatinga\n");
    printf("4. Samambaia\n");
    printf("5. Gama\n");
    printf("6. Sobradinho\n");
    printf("7. Guara\n");
    printf("8. Aguas Claras\n");
}

static const char *nomeRegiao(int regiaoAdministrativa)
{
    switch (regiaoAdministrativa)
    {
    case 1:
        return "Plano Piloto";
    case 2:
        return "Ceilandia";
    case 3:
        return "Taguatinga";
    case 4:
        return "Samambaia";
    case 5:
        return "Gama";
    case 6:
        return "Sobradinho";
    case 7:
        return "Guara";
    case 8:
        return "Aguas Claras";
    default:
        return "Nao informada";
    }
}

static int buscarMedico(int id)
{
    for (int i = 0; i < totalMedicos; i++)
    {
        if (medicos[i].id == id && medicos[i].ativo == 1)
        {
            return i;
        }
    }

    return -1;
}

static void exibirMedico(const Medico *medico)
{
    printf("\nID: %d\n", medico->id);
    printf("Nome: Dr(a). %s\n", medico->nome);
    printf("CRM: CRM%s\n", medico->crm);
    printf("Especialidade: %s\n", medico->especialidade);
    printf("Regiao administrativa: %s\n", nomeRegiao(medico->regiaoAdministrativa));
}

int cadastrarMedico(const char nome[], const char crm[], const char especialidade[], int regiaoAdministrativa)
{
    if (totalMedicos >= MAX_MEDICOS)
    {
        return 0;
    }

    if (regiaoAdministrativa < 1 || regiaoAdministrativa > 8)
    {
        return 0;
    }

    medicos[totalMedicos].id = totalMedicos + 1;
    strcpy(medicos[totalMedicos].nome, nome);
    strcpy(medicos[totalMedicos].crm, crm);
    strcpy(medicos[totalMedicos].especialidade, especialidade);
    medicos[totalMedicos].regiaoAdministrativa = regiaoAdministrativa;
    medicos[totalMedicos].ativo = 1;

    totalMedicos++;

    return 1;
}

int excluirMedico(int id)
{
    for (int i = 0; i < totalMedicos; i++)
    {
        if (medicos[i].id == id && medicos[i].ativo == 1)
        {
            medicos[i].ativo = 0;
            return 1;
        }
    }

    return 0;
}

int salvarMedicoNoBanco(const Medico *medico)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT OR REPLACE INTO medicos "
        "(id, nome, crm, especialidade, regiao_administrativa, ativo) "
        "VALUES (?, ?, ?, ?, ?, ?);";

    if (medico == NULL)
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

    sqlite3_bind_int(stmt, 1, medico->id);
    sqlite3_bind_text(stmt, 2, medico->nome, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, medico->crm, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, medico->especialidade, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, medico->regiaoAdministrativa);
    sqlite3_bind_int(stmt, 6, medico->ativo);

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

int carregarMedicosDoBanco(Medico destino[], int maximo)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, nome, crm, especialidade, regiao_administrativa, ativo "
        "FROM medicos ORDER BY id;";
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
        strcpy(destino[totalCarregados].nome, (const char *)sqlite3_column_text(stmt, 1));
        strcpy(destino[totalCarregados].crm, (const char *)sqlite3_column_text(stmt, 2));
        strcpy(destino[totalCarregados].especialidade, (const char *)sqlite3_column_text(stmt, 3));
        destino[totalCarregados].regiaoAdministrativa = sqlite3_column_int(stmt, 4);
        destino[totalCarregados].ativo = sqlite3_column_int(stmt, 5);
        totalCarregados++;
    }

    sqlite3_finalize(stmt);
    fecharBancoSQLite(db);
    return totalCarregados;
}

void menuMedicos(void)
{
    int caso2;

    do
    {
        printf("\n=============================================\n");
        printf("MENU GERENCIAMENTO DE MEDICOS\n");
        printf("=============================================\n");
        printf("1. Cadastrar Medico\n");
        printf("2. Listar Medicos\n");
        printf("3. Editar Medico\n");
        printf("4. Excluir Medico\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("---------------------------------------------\n");
        printf("Escolha uma opcao: ");

        if (lerInteiro(&caso2) == 0)
        {
            printf("\nOpcao invalida. Tente novamente.\n");
            continue;
        }

        switch (caso2)
        {
        case 1:
        {
            char nome[100];
            char crm[20];
            char especialidade[50];
            int regiaoAdministrativa;

            if (totalMedicos >= MAX_MEDICOS)
            {
                printf("\nLimite de medicos atingido.\n");
                break;
            }

            printf("\nNome do medico, sem Dr/Dr(a): ");
            scanf(" %[^\n]", nome);

            printf("Numero do CRM: ");
            scanf(" %[^\n]", crm);

            printf("Especialidade: ");
            scanf(" %[^\n]", especialidade);

            printf("Regiao administrativa do DF:\n");
            listarRegioes();
            printf("Escolha: ");
            scanf("%d", &regiaoAdministrativa);

            if (cadastrarMedico(nome, crm, especialidade, regiaoAdministrativa) == 0)
            {
                printf("\nNao foi possivel cadastrar o medico.\n");
                break;
            }

            printf("\nMedico cadastrado com sucesso! ID: %d\n", medicos[totalMedicos - 1].id);
            break;
        }

        case 2:
        {
            if (totalMedicos == 0)
            {
                printf("\nNenhum medico cadastrado.\n");
                break;
            }

            printf("\nLista de Medicos:\n");

            for (int i = 0; i < totalMedicos; i++)
            {
                if (medicos[i].ativo == 1)
                {
                    exibirMedico(&medicos[i]);
                }
            }

            break;
        }

        case 3:
        {
            int idBusca;
            int opcaoEditar;
            int indiceMedico;

            printf("\nDigite o ID do medico que deseja editar: ");
            scanf("%d", &idBusca);

            indiceMedico = buscarMedico(idBusca);

            if (indiceMedico == -1)
            {
                printf("\nMedico nao encontrado ou inativo.\n");
                break;
            }

            printf("\nMedico encontrado:\n");
            exibirMedico(&medicos[indiceMedico]);

            printf("\nO que deseja editar?\n");
            printf("1. Nome\n");
            printf("2. CRM\n");
            printf("3. Especialidade\n");
            printf("4. Regiao administrativa\n");
            printf("0. Cancelar\n");
            printf("Escolha uma opcao: ");
            if (lerInteiro(&opcaoEditar) == 0)
            {
                printf("\nOpcao invalida. Tente novamente.\n");
                break;
            }

            switch (opcaoEditar)
            {
            case 1:
                printf("Novo nome do medico, sem Dr/Dr(a): ");
                scanf(" %[^\n]", medicos[indiceMedico].nome);
                break;

            case 2:
                printf("Novo numero do CRM: ");
                scanf(" %[^\n]", medicos[indiceMedico].crm);
                break;

            case 3:
                printf("Nova especialidade: ");
                scanf(" %[^\n]", medicos[indiceMedico].especialidade);
                break;

            case 4:
                printf("Nova regiao administrativa do DF:\n");
                listarRegioes();
                printf("Escolha: ");
                scanf("%d", &medicos[indiceMedico].regiaoAdministrativa);
                break;

            case 0:
                printf("\nEdicao cancelada.\n");
                break;

            default:
                printf("\nOpcao invalida.\n");
                break;
            }

            if (opcaoEditar != 0)
            {
                printf("\nMedico atualizado com sucesso.\n");
            }

            break;
        }

        case 4:
        {
            int idBusca;

            printf("\nDigite o ID do medico que deseja excluir: ");
            scanf("%d", &idBusca);

            if (excluirMedico(idBusca) == 1)
            {
                printf("\nMedico removido com sucesso.\n");
            }
            else
            {
                printf("\nMedico nao encontrado ou ja inativo.\n");
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

    } while (caso2 != 0);
}
