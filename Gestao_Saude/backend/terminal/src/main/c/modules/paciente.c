#include "paciente.h"
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

static int buscarPaciente(int id)
{
    for (int i = 0; i < totalPacientes; i++)
    {
        if (pacientes[i].id == id && pacientes[i].ativo == 1)
        {
            return i;
        }
    }

    return -1;
}

static void exibirPaciente(const Paciente *paciente)
{
    printf("\nID: %d\n", paciente->id);
    printf("Nome: %s\n", paciente->nome);
    printf("CPF: %s\n", paciente->cpf);
    printf("Idade: %d\n", paciente->idade);
    printf("Telefone: %s\n", paciente->telefone);
    printf("Sexo: %c\n", paciente->sexo);
    printf("Regiao administrativa: %s\n", nomeRegiao(paciente->regiaoAdministrativa));
}

int cadastrarPaciente(const char nome[], const char cpf[], int idade, const char telefone[], char sexo, int regiaoAdministrativa)
{
    Paciente *novoPaciente = NULL;

    if (totalPacientes >= MAX_PACIENTES)
    {
        return 0;
    }

    if (regiaoAdministrativa < 1 || regiaoAdministrativa > 8)
    {
        return 0;
    }

    novoPaciente = &pacientes[totalPacientes];
    novoPaciente->id = totalPacientes + 1;
    strcpy(novoPaciente->nome, nome);
    strcpy(novoPaciente->cpf, cpf);
    novoPaciente->idade = idade;
    strcpy(novoPaciente->telefone, telefone);
    novoPaciente->sexo = sexo;
    novoPaciente->regiaoAdministrativa = regiaoAdministrativa;
    novoPaciente->ativo = 1;

    if (salvarPacienteNoBanco(novoPaciente) == 0)
    {
        return 0;
    }

    totalPacientes++;

    return 1;
}

int excluirPaciente(int id)
{
    for (int i = 0; i < totalPacientes; i++)
    {
        if (pacientes[i].id == id && pacientes[i].ativo == 1)
        {
            pacientes[i].ativo = 0;
            if (salvarPacienteNoBanco(&pacientes[i]) == 0)
            {
                pacientes[i].ativo = 1;
                return 0;
            }
            return 1;
        }
    }

    return 0;
}

int salvarPacienteNoBanco(const Paciente *paciente)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO pacientes "
        "(id, nome, cpf, idade, telefone, sexo, regiao_administrativa, ativo) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?) "
        "ON CONFLICT(id) DO UPDATE SET "
        "nome = excluded.nome, "
        "cpf = excluded.cpf, "
        "idade = excluded.idade, "
        "telefone = excluded.telefone, "
        "sexo = excluded.sexo, "
        "regiao_administrativa = excluded.regiao_administrativa, "
        "ativo = excluded.ativo;";

    if (paciente == NULL)
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
    sqlite3_bind_int(stmt, 1, paciente->id);
    sqlite3_bind_text(stmt, 2, paciente->nome, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, paciente->cpf, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, paciente->idade);
    sqlite3_bind_text(stmt, 5, paciente->telefone, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, &paciente->sexo, 1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 7, paciente->regiaoAdministrativa);
    sqlite3_bind_int(stmt, 8, paciente->ativo);

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

int carregarPacientesDoBanco(Paciente destino[], int maximo)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, nome, cpf, idade, telefone, sexo, regiao_administrativa, ativo "
        "FROM pacientes ORDER BY id;";
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
        strcpy(destino[totalCarregados].cpf, (const char *)sqlite3_column_text(stmt, 2));
        destino[totalCarregados].idade = sqlite3_column_int(stmt, 3);
        strcpy(destino[totalCarregados].telefone, (const char *)sqlite3_column_text(stmt, 4));
        destino[totalCarregados].sexo = ((const char *)sqlite3_column_text(stmt, 5))[0];
        destino[totalCarregados].regiaoAdministrativa = sqlite3_column_int(stmt, 6);
        destino[totalCarregados].ativo = sqlite3_column_int(stmt, 7);
        totalCarregados++;
    }

    sqlite3_finalize(stmt);
    fecharBancoSQLite(db);
    return totalCarregados;
}

void menuPacientes(void)
{
    int caso1;

    do
    {
        printf("\n=============================================\n");
        printf("MENU GERENCIAMENTO DE PACIENTES\n");
        printf("=============================================\n");
        printf("1. Cadastrar Paciente\n");
        printf("2. Listar Pacientes\n");
        printf("3. Editar Paciente\n");
        printf("4. Excluir Paciente\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("---------------------------------------------\n");
        printf("Escolha uma opcao: ");

        if (lerInteiro(&caso1) == 0)
        {
            printf("\nOpcao invalida. Tente novamente.\n");
            continue;
        }

        switch (caso1)
        {
        case 1:
        {
            char nome[100];
            char cpf[15];
            int idade;
            char telefone[20];
            char sexo;
            int regiaoAdministrativa;

            if (totalPacientes >= MAX_PACIENTES)
            {
                printf("\nLimite de pacientes atingido.\n");
                break;
            }

            printf("\nNome: ");
            scanf(" %[^\n]", nome);

            printf("CPF: ");
            scanf(" %[^\n]", cpf);

            printf("Idade: ");
            scanf("%d", &idade);

            printf("Telefone: ");
            scanf(" %[^\n]", telefone);

            printf("Sexo (M/F/O): ");
            scanf(" %c", &sexo);

            printf("Regiao administrativa do DF:\n");
            listarRegioes();
            printf("Escolha: ");
            scanf("%d", &regiaoAdministrativa);

            if (cadastrarPaciente(nome, cpf, idade, telefone, sexo, regiaoAdministrativa) == 0)
            {
                printf("\nNao foi possivel cadastrar o paciente.\n");
                break;
            }

            printf("\nPaciente cadastrado com sucesso! ID: %d\n", pacientes[totalPacientes - 1].id);
            break;
        }

        case 2:
        {
            if (totalPacientes == 0)
            {
                printf("\nNenhum paciente cadastrado.\n");
                break;
            }

            printf("\nLista de Pacientes:\n");

            for (int i = 0; i < totalPacientes; i++)
            {
                if (pacientes[i].ativo == 1)
                {
                    exibirPaciente(&pacientes[i]);
                }
            }

            break;
        }

        case 3:
        {
            int idBusca;
            int opcaoEditar;
            int indicePaciente;

            printf("\nDigite o ID do paciente que deseja editar: ");
            scanf("%d", &idBusca);

            indicePaciente = buscarPaciente(idBusca);

            if (indicePaciente == -1)
            {
                printf("\nPaciente nao encontrado ou inativo.\n");
                break;
            }

            printf("\nPaciente encontrado:\n");
            exibirPaciente(&pacientes[indicePaciente]);

            printf("\nO que deseja editar?\n");
            printf("1. Nome\n");
            printf("2. CPF\n");
            printf("3. Idade\n");
            printf("4. Telefone\n");
            printf("5. Sexo\n");
            printf("6. Regiao administrativa\n");
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
                printf("Novo nome: ");
                scanf(" %[^\n]", pacientes[indicePaciente].nome);
                break;

            case 2:
                printf("Novo CPF: ");
                scanf(" %[^\n]", pacientes[indicePaciente].cpf);
                break;

            case 3:
                printf("Nova idade: ");
                scanf("%d", &pacientes[indicePaciente].idade);
                break;

            case 4:
                printf("Novo telefone: ");
                scanf(" %[^\n]", pacientes[indicePaciente].telefone);
                break;

            case 5:
                printf("Novo sexo (M/F/O): ");
                scanf(" %c", &pacientes[indicePaciente].sexo);
                break;

            case 6:
                printf("Nova regiao administrativa do DF:\n");
                listarRegioes();
                printf("Escolha: ");
                scanf("%d", &pacientes[indicePaciente].regiaoAdministrativa);
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
                if (salvarPacienteNoBanco(&pacientes[indicePaciente]) == 0)
                {
                    printf("\nNao foi possivel salvar a atualizacao no banco.\n");
                    break;
                }
                printf("\nPaciente atualizado com sucesso.\n");
            }

            break;
        }

        case 4:
        {
            int idBusca;

            printf("\nDigite o ID do paciente que deseja excluir: ");
            scanf("%d", &idBusca);

            if (excluirPaciente(idBusca) == 1)
            {
                printf("\nPaciente removido com sucesso.\n");
            }
            else
            {
                printf("\nPaciente nao encontrado ou ja inativo.\n");
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

    } while (caso1 != 0);
}
