#include "ala.h"
#include "sqlite_db.h"

static const char *nomeTipoAla(int tipo)
{
    switch (tipo)
    {
    case ALA_INTERNACAO:
        return "Internacao";

    case ALA_UTI:
        return "UTI";

    case ALA_OBSERVACAO:
        return "Observacao";

    case ALA_PEDIATRIA:
        return "Pediatria";

    case ALA_CIRURGICA:
        return "Cirurgica";
        
    default:
        return "Nao informado";
    }
}

static int tipoAlaValido(int tipo)
{
    return tipo >= ALA_INTERNACAO && tipo <= ALA_CIRURGICA;
}

static void exibirOpcoesTipoAla(void)
{
    printf("\nTipos de ala:\n");
    printf("1. Internacao\n");
    printf("2. UTI\n");
    printf("3. Observacao\n");
    printf("4. Pediatria\n");
    printf("5. Cirurgica\n");
}

static void exibirAla(const Ala *ala)
{
    printf("\nID: %d\n", ala->id);
    printf("Nome: %s\n", ala->nome);
    printf("Tipo: %s\n", nomeTipoAla(ala->tipo));
    printf("Total de leitos: %d\n", ala->totalLeitos);
    printf("Leitos ocupados: %d\n", ala->leitosOcupados);
}

int cadastrarAla(const char nome[], int tipo, int totalLeitosAla)
{
    Ala *novaAla = NULL;

    if (totalAlas >= MAX_ALAS)
    {
        return 0;
    }

    if (tipoAlaValido(tipo) == 0)
    {
        return 0;
    }

    novaAla = &alas[totalAlas];
    novaAla->id = totalAlas + 1;
    strcpy(novaAla->nome, nome);
    novaAla->tipo = tipo;
    novaAla->totalLeitos = totalLeitosAla;
    novaAla->leitosOcupados = 0;
    novaAla->ativo = 1;

    if (salvarAlaNoBanco(novaAla) == 0)
    {
        return 0;
    }

    totalAlas++;

    return 1;
}

int excluirAla(int id)
{
    for (int i = 0; i < totalAlas; i++)
    {
        if (alas[i].id == id && alas[i].ativo == 1)
        {
            alas[i].ativo = 0;
            if (salvarAlaNoBanco(&alas[i]) == 0)
            {
                alas[i].ativo = 1;
                return 0;
            }
            return 1;
        }
    }

    return 0;
}

int salvarAlaNoBanco(const Ala *ala)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO alas "
        "(id, nome, tipo, total_leitos, leitos_ocupados, ativo) "
        "VALUES (?, ?, ?, ?, ?, ?) "
        "ON CONFLICT(id) DO UPDATE SET "
        "nome = excluded.nome, "
        "tipo = excluded.tipo, "
        "total_leitos = excluded.total_leitos, "
        "leitos_ocupados = excluded.leitos_ocupados, "
        "ativo = excluded.ativo;";

    if (ala == NULL)
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

    sqlite3_bind_int(stmt, 1, ala->id);
    sqlite3_bind_text(stmt, 2, ala->nome, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, ala->tipo);
    sqlite3_bind_int(stmt, 4, ala->totalLeitos);
    sqlite3_bind_int(stmt, 5, ala->leitosOcupados);
    sqlite3_bind_int(stmt, 6, ala->ativo);

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

int carregarAlasDoBanco(Ala destino[], int maximo)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT id, nome, tipo, total_leitos, leitos_ocupados, ativo "
        "FROM alas ORDER BY id;";
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
        destino[totalCarregados].tipo = sqlite3_column_int(stmt, 2);
        destino[totalCarregados].totalLeitos = sqlite3_column_int(stmt, 3);
        destino[totalCarregados].leitosOcupados = sqlite3_column_int(stmt, 4);
        destino[totalCarregados].ativo = sqlite3_column_int(stmt, 5);
        totalCarregados++;
    }

    sqlite3_finalize(stmt);
    fecharBancoSQLite(db);
    return totalCarregados;
}

int contarAlasPorTipo(int tipo)
{
    int total = 0;

    for (int i = 0; i < totalAlas; i++)
    {
        if (alas[i].ativo == 1 && alas[i].tipo == tipo)
        {
            total++;
        }
    }

    return total;
}

void listarAlasPorTipo(int tipo)
{
    int encontrou = 0;

    printf("\nAlas do tipo %s:\n", nomeTipoAla(tipo));

    for (int i = 0; i < totalAlas; i++)
    {
        if (alas[i].ativo == 1 && alas[i].tipo == tipo)
        {
            exibirAla(&alas[i]);
            encontrou = 1;
        }
    }

    if (encontrou == 0)
    {
        printf("\nNenhuma ala encontrada para esse tipo.\n");
    }
}

void menuAlas(void)
{
    int caso4;

    do
    {
        printf("\n=============================================\n");
        printf("MENU GERENCIAMENTO DE ALAS\n");
        printf("=============================================\n");
        printf("1. Cadastrar Ala\n");
        printf("2. Listar Alas\n");
        printf("3. Excluir Ala\n");
        printf("4. Consultar Alas por Tipo\n");
        printf("0. Voltar ao Menu Principal\n");
        printf("---------------------------------------------\n");
        printf("Escolha uma opcao: ");
        if (lerInteiro(&caso4) == 0)
        {
            printf("\nOpcao invalida. Tente novamente.\n");
            continue;
        }

        switch (caso4)
        {
        case 1:
        {
            char nome[100];
            int tipo;
            int totalLeitosAla;

            if (totalAlas >= MAX_ALAS)
            {
                printf("\nLimite de alas atingido.\n");
                break;
            }

            printf("\nNome da ala: ");
            scanf(" %[^\n]", nome);

            exibirOpcoesTipoAla();
            printf("Escolha o tipo da ala: ");
            scanf("%d", &tipo);

            if (tipoAlaValido(tipo) == 0)
            {
                printf("\nTipo de ala invalido.\n");
                break;
            }

            printf("Total de leitos: ");
            scanf("%d", &totalLeitosAla);

            if (cadastrarAla(nome, tipo, totalLeitosAla) == 1)
            {
                printf("\nAla cadastrada com sucesso! ID: %d\n", alas[totalAlas - 1].id);
            }
            else
            {
                printf("\nNao foi possivel cadastrar a ala.\n");
            }

            break;
        }

        case 2:
        {
            if (totalAlas == 0)
            {
                printf("\nNenhuma ala cadastrada.\n");
                break;
            }

            printf("\nLista de Alas:\n");

            for (int i = 0; i < totalAlas; i++)
            {
                if (alas[i].ativo == 1)
                {
                    exibirAla(&alas[i]);
                }
            }

            break;
        }

        case 3:
        {
            int idBusca;

            printf("\nDigite o ID da ala que deseja excluir: ");
            scanf("%d", &idBusca);

            if (excluirAla(idBusca) == 1)
            {
                printf("\nAla removida com sucesso.\n");
            }
            else
            {
                printf("\nAla nao encontrada ou ja inativa.\n");
            }

            break;
        }

        case 4:
        {
            int tipo;

            exibirOpcoesTipoAla();
            printf("Escolha o tipo que deseja consultar: ");
            scanf("%d", &tipo);

            if (tipoAlaValido(tipo) == 0)
            {
                printf("\nTipo de ala invalido.\n");
                break;
            }

            printf("\nTotal de alas desse tipo: %d\n", contarAlasPorTipo(tipo));
            listarAlasPorTipo(tipo);
            break;
        }

        case 0:
            printf("\nVoltando ao menu principal...\n");
            break;

        default:
            printf("\nOpcao invalida. Tente novamente.\n");
            break;
        }

    } while (caso4 != 0);
}
