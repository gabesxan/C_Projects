#include "database.h"
#include "paciente_repository.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

/*
 * Servidor HTTP minimo do backend web do SIGEH-DF.
 * Expoe apenas GET /health, que confirma que o banco esta acessivel.
 * Sockets POSIX puros, sem dependencia externa alem da database.h/sqlite3.
 */

#define PORTA_PADRAO 8080
#define TAM_REQUISICAO 2048

/* Health real: tenta uma consulta trivial na tabela pacientes.
 * Retorna 1 se o schema responde, 0 caso contrario. */
static int bancoSaudavel(void)
{
    return db_executar("SELECT 1 FROM pacientes LIMIT 1;");
}

static void responder(int cliente, const char *status, const char *corpo)
{
    char cabecalho[256];
    int n = snprintf(cabecalho, sizeof(cabecalho),
        "HTTP/1.1 %s\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n",
        status, (int)strlen(corpo));

    if (n > 0)
    {
        write(cliente, cabecalho, (size_t)n);
        write(cliente, corpo, strlen(corpo));
    }
}

int main(int argc, char *argv[])
{
    int porta = PORTA_PADRAO;
    int servidor;
    int opcao = 1;
    struct sockaddr_in endereco;

    if (argc > 1)
    {
        porta = atoi(argv[1]);
        if (porta <= 0 || porta > 65535)
        {
            porta = PORTA_PADRAO;
        }
    }

    servidor = socket(AF_INET, SOCK_STREAM, 0);
    if (servidor < 0)
    {
        perror("socket");
        return 1;
    }

    setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &opcao, sizeof(opcao));

    memset(&endereco, 0, sizeof(endereco));
    endereco.sin_family = AF_INET;
    endereco.sin_addr.s_addr = htonl(INADDR_ANY);
    endereco.sin_port = htons((unsigned short)porta);

    if (bind(servidor, (struct sockaddr *)&endereco, sizeof(endereco)) < 0)
    {
        perror("bind");
        close(servidor);
        return 1;
    }

    if (listen(servidor, 16) < 0)
    {
        perror("listen");
        close(servidor);
        return 1;
    }

    printf("SIGEH-DF API ouvindo em http://localhost:%d/health\n", porta);
    fflush(stdout);

    for (;;)
    {
        char requisicao[TAM_REQUISICAO];
        char metodo[8];
        char caminho[256];
        int cliente = accept(servidor, NULL, NULL);
        ssize_t lidos;

        if (cliente < 0)
        {
            continue;
        }

        lidos = read(cliente, requisicao, sizeof(requisicao) - 1);
        if (lidos <= 0)
        {
            close(cliente);
            continue;
        }

        requisicao[lidos] = '\0';

        if (sscanf(requisicao, "%7s %255s", metodo, caminho) != 2)
        {
            responder(cliente, "400 Bad Request",
                      "{\"erro\":\"requisicao invalida\"}");
            close(cliente);
            continue;
        }

        if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/health") == 0)
        {
            if (bancoSaudavel())
            {
                responder(cliente, "200 OK",
                          "{\"status\":\"ok\",\"database\":\"ok\"}");
            }
            else
            {
                responder(cliente, "503 Service Unavailable",
                          "{\"status\":\"degraded\",\"database\":\"down\"}");
            }
        }
        else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/pacientes") == 0)
        {
            char *json = malloc(65536);

            if (json != NULL && paciente_repo_listar_json(json, 65536) == 1)
            {
                responder(cliente, "200 OK", json);
            }
            else
            {
                responder(cliente, "500 Internal Server Error",
                          "{\"erro\":\"falha ao listar pacientes\"}");
            }

            free(json);
        }
        else
        {
            responder(cliente, "404 Not Found",
                      "{\"erro\":\"rota nao encontrada\"}");
        }

        close(cliente);
    }

    close(servidor);
    return 0;
}
