#include "leito.h"
#include "sqlite_db.h"

int cadastrarLeito(int alaId, int numero)
{
    int alaEncontrada = 0;
    Leito *novoLeito = NULL;

    if (totalLeitos >= MAX_LEITOS)
    {
        return 0;
    }

    for (int i = 0; i < totalAlas; i++)
    {
        if (alas[i].id == alaId && alas[i].ativo == 1)
        {
            alaEncontrada = 1;
            break;
        }
    }

    if (alaEncontrada == 0)
    {
        return 0;
    }

    novoLeito = &leitos[totalLeitos];
    novoLeito->id = totalLeitos + 1;
    novoLeito->alaId = alaId;
    novoLeito->numero = numero;
    novoLeito->ocupado = 0;
    novoLeito->pacienteId = 0;
    novoLeito->ativo = 1;

    if (salvarLeitoNoBanco(novoLeito) == 0)
    {
        return 0;
    }

    totalLeitos++;

    return 1;
}

int excluirLeito(int id)
{
    for (int i = 0; i < totalLeitos; i++)
    {
        if (leitos[i].id == id && leitos[i].ativo == 1)
        {
            if (leitos[i].ocupado == 1)
            {
                return 0;
            }

            leitos[i].ativo = 0;
            if (salvarLeitoNoBanco(&leitos[i]) == 0)
            {
                leitos[i].ativo = 1;
                return 0;
            }
            return 1;
        }
    }

    return 0;
}

int salvarLeitoNoBanco(const Leito *leito)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO leitos "
        "(id, ala_id, numero, ocupado, paciente_id, ativo) "
        "VALUES (?, ?, ?, ?, ?, ?) "
        "ON CONFLICT(id) DO UPDATE SET "
        "ala_id = excluded.ala_id, "
        "numero = excluded.numero, "
        "ocupado = excluded.ocupado, "
        "paciente_id = excluded.paciente_id, "
        "ativo = excluded.ativo;";

    if (leito == NULL)
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

    sqlite3_bind_int(stmt, 1, leito->id);
    sqlite3_bind_int(stmt, 2, leito->alaId);
    sqlite3_bind_int(stmt, 3, leito->numero);
    sqlite3_bind_int(stmt, 4, leito->ocupado);
    sqlite3_bind_int(stmt, 5, leito->pacienteId);
    sqlite3_bind_int(stmt, 6, leito->ativo);

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

int carregarLeitosDoBanco(Leito destino[], int maximo)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, ala_id, numero, ocupado, paciente_id, ativo "
        "FROM leitos ORDER BY id;";
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
        destino[totalCarregados].alaId = sqlite3_column_int(stmt, 1);
        destino[totalCarregados].numero = sqlite3_column_int(stmt, 2);
        destino[totalCarregados].ocupado = sqlite3_column_int(stmt, 3);
        destino[totalCarregados].pacienteId = sqlite3_column_int(stmt, 4);
        destino[totalCarregados].ativo = sqlite3_column_int(stmt, 5);
        totalCarregados++;
    }

    sqlite3_finalize(stmt);
    fecharBancoSQLite(db);
    return totalCarregados;
}

void menuLeitos(void)
{
    int caso5;

    do
    {
        printf("\n=============================================\n");
        printf("MENU GERENCIAMENTO DE LEITOS\n");
        printf("=============================================\n");
        printf("1. Cadastrar Leito\n");
        printf("2. Listar Leitos\n");
        printf("3. Excluir Leito\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("---------------------------------------------\n");
        printf("Escolha uma opcao: ");
        if (lerInteiro(&caso5) == 0)
        {
            printf("\nOpcao invalida. Tente novamente.\n");
            continue;
        }

        switch (caso5)
        {
        
        case 1:
        {
            int alaId;
            int numero;

            printf("\nID da ala: ");
            scanf("%d", &alaId);

            printf("Numero do leito: ");
            scanf("%d", &numero);

            if (cadastrarLeito(alaId, numero) == 0)
            {
                printf("\nNao foi possivel cadastrar o leito. Verifique se a ala existe, esta ativa ou se o limite foi atingido.\n");
                break;
            }

            printf("\nLeito cadastrado com sucesso! ID: %d\n", leitos[totalLeitos - 1].id);
            break;
        }

        
        case 2:
        {
            int encontrouAtivo = 0;

            if (totalLeitos == 0)
            {
                printf("\nNenhum leito cadastrado.\n");
                break;
            }

            printf("\nLista de Leitos:\n");

            for (int i = 0; i < totalLeitos; i++)
            {
                if (leitos[i].ativo == 1)
                {
                    encontrouAtivo = 1;

                    printf("\nID: %d\n", leitos[i].id);
                    printf("Ala ID: %d\n", leitos[i].alaId);
                    printf("Numero: %d\n", leitos[i].numero);
                    printf("Ocupado: %s\n", leitos[i].ocupado == 1 ? "Sim" : "Nao");
                    printf("Paciente ID: %d\n", leitos[i].pacienteId);
                }
            }

            if (encontrouAtivo == 0)
            {
                printf("\nNenhum leito ativo cadastrado.\n");
            }

            break;
        }

        case 3:
        {
            int idBusca;

            printf("\nDigite o ID do leito que deseja excluir: ");
            scanf("%d", &idBusca);

            if (excluirLeito(idBusca) == 1)
            {
                printf("\nLeito removido com sucesso.\n");
            }
            else
            {
                printf("\nLeito nao encontrado, ja inativo ou ocupado.\n");
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

    } while (caso5 != 0);
}
