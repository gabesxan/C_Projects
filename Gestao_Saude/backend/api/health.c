#include "database.h"
#include "paciente_repository.h"
#include "medico_repository.h"
#include "triagem_service.h"
#include "relatorio_service.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

/*
 * Servidor HTTP minimo do backend web do SIGEH-DF.
 * Sockets POSIX puros, sem dependencia externa alem da database.h/sqlite3.
 *
 * Rotas:
 *   GET    /health
 *   GET    /pacientes              GET /pacientes/contar
 *   POST   /pacientes?nome=..&cpf=..&idade=..&telefone=..&sexo=..&regiao=..
 *   DELETE /pacientes/{id}
 *   GET    /medicos                GET /medicos/contar
 *   POST   /medicos?nome=..&crm=..&especialidade=..&regiao=..
 *   DELETE /medicos/{id}
 */

#define PORTA_PADRAO 8080
#define TAM_REQUISICAO 2048
#define TAM_JSON 65536

/* ----------------------------------------------------------------------- */
/* Utilitarios HTTP                                                         */
/* ----------------------------------------------------------------------- */

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

/* Decodifica %XX e '+' de um trecho de query para 'destino'. */
static void urlDecode(char *destino, int tamanho, const char *origem, int len)
{
    int i = 0;
    int j = 0;

    while (i < len && j < tamanho - 1)
    {
        if (origem[i] == '%' && i + 2 < len)
        {
            char hex[3];
            hex[0] = origem[i + 1];
            hex[1] = origem[i + 2];
            hex[2] = '\0';
            destino[j++] = (char)strtol(hex, NULL, 16);
            i += 3;
        }
        else if (origem[i] == '+')
        {
            destino[j++] = ' ';
            i++;
        }
        else
        {
            destino[j++] = origem[i++];
        }
    }

    destino[j] = '\0';
}

/* Extrai o valor do parametro 'chave' da query string. Retorna 1/0. */
static int extrairParam(const char *consulta, const char *chave,
                        char *destino, int tamanho)
{
    char alvo[64];
    const char *p;
    int n = snprintf(alvo, sizeof(alvo), "%s=", chave);

    destino[0] = '\0';

    if (consulta == NULL || n <= 0 || n >= (int)sizeof(alvo))
    {
        return 0;
    }

    p = consulta;
    while (p != NULL && *p != '\0')
    {
        if (strncmp(p, alvo, (size_t)n) == 0)
        {
            const char *inicio = p + n;
            const char *fim = strchr(inicio, '&');
            int len = fim != NULL ? (int)(fim - inicio) : (int)strlen(inicio);
            urlDecode(destino, tamanho, inicio, len);
            return 1;
        }

        p = strchr(p, '&');
        if (p != NULL)
        {
            p++;
        }
    }

    return 0;
}

/* ----------------------------------------------------------------------- */
/* Handlers de rota                                                         */
/* ----------------------------------------------------------------------- */

static void rotaHealth(int cliente)
{
    if (db_executar("SELECT 1 FROM pacientes LIMIT 1;"))
    {
        responder(cliente, "200 OK", "{\"status\":\"ok\",\"database\":\"ok\"}");
    }
    else
    {
        responder(cliente, "503 Service Unavailable",
                  "{\"status\":\"degraded\",\"database\":\"down\"}");
    }
}

static void rotaListarPacientes(int cliente)
{
    char *json = malloc(TAM_JSON);

    if (json != NULL && paciente_repo_listar_json(json, TAM_JSON) == 1)
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

static void rotaContarPacientes(int cliente)
{
    char corpo[64];
    int total = paciente_repo_contar_ativos();

    snprintf(corpo, sizeof(corpo), "{\"ativos\":%d}", total);
    responder(cliente, "200 OK", corpo);
}

static void rotaCriarPaciente(int cliente, const char *consulta)
{
    char nome[128];
    char cpf[32];
    char telefone[32];
    char sexo[8];
    char idadeStr[16];
    char regiaoStr[16];

    extrairParam(consulta, "nome", nome, sizeof(nome));
    extrairParam(consulta, "cpf", cpf, sizeof(cpf));
    extrairParam(consulta, "idade", idadeStr, sizeof(idadeStr));
    extrairParam(consulta, "telefone", telefone, sizeof(telefone));
    extrairParam(consulta, "sexo", sexo, sizeof(sexo));
    extrairParam(consulta, "regiao", regiaoStr, sizeof(regiaoStr));

    if (paciente_repo_criar(nome, cpf, atoi(idadeStr), telefone, sexo,
                            atoi(regiaoStr)) == 1)
    {
        responder(cliente, "201 Created", "{\"status\":\"criado\"}");
    }
    else
    {
        responder(cliente, "400 Bad Request",
                  "{\"erro\":\"dados invalidos para paciente\"}");
    }
}

static void rotaDesativarPaciente(int cliente, int id)
{
    if (paciente_repo_desativar(id) == 1)
    {
        responder(cliente, "200 OK", "{\"status\":\"desativado\"}");
    }
    else
    {
        responder(cliente, "404 Not Found",
                  "{\"erro\":\"paciente nao encontrado ou ja inativo\"}");
    }
}

static void rotaListarMedicos(int cliente)
{
    char *json = malloc(TAM_JSON);

    if (json != NULL && medico_repo_listar_json(json, TAM_JSON) == 1)
    {
        responder(cliente, "200 OK", json);
    }
    else
    {
        responder(cliente, "500 Internal Server Error",
                  "{\"erro\":\"falha ao listar medicos\"}");
    }

    free(json);
}

static void rotaContarMedicos(int cliente)
{
    char corpo[64];
    int total = medico_repo_contar_ativos();

    snprintf(corpo, sizeof(corpo), "{\"ativos\":%d}", total);
    responder(cliente, "200 OK", corpo);
}

static void rotaCriarMedico(int cliente, const char *consulta)
{
    char nome[128];
    char crm[32];
    char especialidade[64];
    char regiaoStr[16];

    extrairParam(consulta, "nome", nome, sizeof(nome));
    extrairParam(consulta, "crm", crm, sizeof(crm));
    extrairParam(consulta, "especialidade", especialidade, sizeof(especialidade));
    extrairParam(consulta, "regiao", regiaoStr, sizeof(regiaoStr));

    if (medico_repo_criar(nome, crm, especialidade, atoi(regiaoStr)) == 1)
    {
        responder(cliente, "201 Created", "{\"status\":\"criado\"}");
    }
    else
    {
        responder(cliente, "400 Bad Request",
                  "{\"erro\":\"dados invalidos para medico\"}");
    }
}

static void rotaDesativarMedico(int cliente, int id)
{
    if (medico_repo_desativar(id) == 1)
    {
        responder(cliente, "200 OK", "{\"status\":\"desativado\"}");
    }
    else
    {
        responder(cliente, "404 Not Found",
                  "{\"erro\":\"medico nao encontrado ou ja inativo\"}");
    }
}

static void rotaTriagemAvaliacao(int cliente, int paciente_id)
{
    char *json = malloc(TAM_JSON);

    if (json != NULL && triagem_service_avaliar_json(paciente_id, json, TAM_JSON) == 1)
    {
        responder(cliente, "200 OK", json);
    }
    else
    {
        responder(cliente, "404 Not Found",
                  "{\"erro\":\"sem triagem ativa para o paciente\"}");
    }

    free(json);
}

static void rotaTriagemMedicos(int cliente, int paciente_id)
{
    char *json = malloc(TAM_JSON);

    if (json != NULL && triagem_service_sugerir_medicos_json(paciente_id, json, TAM_JSON) == 1)
    {
        responder(cliente, "200 OK", json);
    }
    else
    {
        responder(cliente, "404 Not Found",
                  "{\"erro\":\"sem triagem ativa para o paciente\"}");
    }

    free(json);
}

static void rotaTriagemHistorico(int cliente, int paciente_id)
{
    char *json = malloc(TAM_JSON);

    if (json != NULL && triagem_service_historico_json(paciente_id, json, TAM_JSON) == 1)
    {
        responder(cliente, "200 OK", json);
    }
    else
    {
        responder(cliente, "404 Not Found",
                  "{\"erro\":\"paciente invalido\"}");
    }

    free(json);
}

static void rotaTriagemExames(int cliente, int paciente_id)
{
    char *json = malloc(TAM_JSON);

    if (json != NULL && triagem_service_sugerir_exames_json(paciente_id, json, TAM_JSON) == 1)
    {
        responder(cliente, "200 OK", json);
    }
    else
    {
        responder(cliente, "404 Not Found",
                  "{\"erro\":\"sem triagem ativa para o paciente\"}");
    }

    free(json);
}

static void rotaTriagemAgendar(int cliente, int paciente_id, const char *consulta)
{
    char data[32];
    char horario[16];
    char *json = malloc(TAM_JSON);

    if (json == NULL)
    {
        responder(cliente, "500 Internal Server Error",
                  "{\"erro\":\"sem memoria\"}");
        return;
    }

    json[0] = '\0';
    extrairParam(consulta, "data", data, sizeof(data));
    extrairParam(consulta, "horario", horario, sizeof(horario));

    if (triagem_service_agendar_json(paciente_id, data, horario, json, TAM_JSON) == 1)
    {
        responder(cliente, "201 Created", json);
    }
    else if (json[0] != '\0')
    {
        responder(cliente, "409 Conflict", json);
    }
    else
    {
        responder(cliente, "400 Bad Request",
                  "{\"erro\":\"sem triagem ativa ou data/horario invalidos\"}");
    }

    free(json);
}

static void rotaTriagemEncaminhar(int cliente, int paciente_id, const char *consulta)
{
    char especialidade[64];
    char data[32];
    char horario[16];
    char *json = malloc(TAM_JSON);

    if (json == NULL)
    {
        responder(cliente, "500 Internal Server Error",
                  "{\"erro\":\"sem memoria\"}");
        return;
    }

    json[0] = '\0';
    extrairParam(consulta, "especialidade", especialidade, sizeof(especialidade));
    extrairParam(consulta, "data", data, sizeof(data));
    extrairParam(consulta, "horario", horario, sizeof(horario));

    if (triagem_service_encaminhar_json(paciente_id, especialidade, data, horario,
                                        json, TAM_JSON) == 1)
    {
        responder(cliente, "201 Created", json);
    }
    else if (json[0] != '\0')
    {
        responder(cliente, "409 Conflict", json);
    }
    else
    {
        responder(cliente, "400 Bad Request",
                  "{\"erro\":\"especialidade/data/horario invalidos\"}");
    }

    free(json);
}

static void rotaRelatorioIndicadores(int cliente)
{
    char *json = malloc(TAM_JSON);

    if (json != NULL && relatorio_service_indicadores_json(json, TAM_JSON) == 1)
    {
        responder(cliente, "200 OK", json);
    }
    else
    {
        responder(cliente, "500 Internal Server Error",
                  "{\"erro\":\"falha ao gerar indicadores\"}");
    }

    free(json);
}

/* ----------------------------------------------------------------------- */
/* Roteamento                                                               */
/* ----------------------------------------------------------------------- */

static void rotear(int cliente, const char *metodo, char *caminho)
{
    char *consulta = strchr(caminho, '?');
    char acao[32];
    int id;

    if (consulta != NULL)
    {
        *consulta = '\0';
        consulta++;
    }

    if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/health") == 0)
    {
        rotaHealth(cliente);
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/pacientes") == 0)
    {
        rotaListarPacientes(cliente);
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/pacientes/contar") == 0)
    {
        rotaContarPacientes(cliente);
    }
    else if (strcmp(metodo, "POST") == 0 && strcmp(caminho, "/pacientes") == 0)
    {
        rotaCriarPaciente(cliente, consulta);
    }
    else if (strcmp(metodo, "DELETE") == 0 && sscanf(caminho, "/pacientes/%d", &id) == 1)
    {
        rotaDesativarPaciente(cliente, id);
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/medicos") == 0)
    {
        rotaListarMedicos(cliente);
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/medicos/contar") == 0)
    {
        rotaContarMedicos(cliente);
    }
    else if (strcmp(metodo, "POST") == 0 && strcmp(caminho, "/medicos") == 0)
    {
        rotaCriarMedico(cliente, consulta);
    }
    else if (strcmp(metodo, "DELETE") == 0 && sscanf(caminho, "/medicos/%d", &id) == 1)
    {
        rotaDesativarMedico(cliente, id);
    }
    else if (sscanf(caminho, "/triagem/%d/%31s", &id, acao) == 2)
    {
        if (strcmp(metodo, "GET") == 0 && strcmp(acao, "avaliacao") == 0)
        {
            rotaTriagemAvaliacao(cliente, id);
        }
        else if (strcmp(metodo, "GET") == 0 && strcmp(acao, "medicos") == 0)
        {
            rotaTriagemMedicos(cliente, id);
        }
        else if (strcmp(metodo, "GET") == 0 && strcmp(acao, "historico") == 0)
        {
            rotaTriagemHistorico(cliente, id);
        }
        else if (strcmp(metodo, "GET") == 0 && strcmp(acao, "exames") == 0)
        {
            rotaTriagemExames(cliente, id);
        }
        else if (strcmp(metodo, "POST") == 0 && strcmp(acao, "agendar") == 0)
        {
            rotaTriagemAgendar(cliente, id, consulta);
        }
        else if (strcmp(metodo, "POST") == 0 && strcmp(acao, "encaminhar") == 0)
        {
            rotaTriagemEncaminhar(cliente, id, consulta);
        }
        else
        {
            responder(cliente, "404 Not Found",
                      "{\"erro\":\"rota de triagem nao encontrada\"}");
        }
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/relatorios/indicadores") == 0)
    {
        rotaRelatorioIndicadores(cliente);
    }
    else
    {
        responder(cliente, "404 Not Found",
                  "{\"erro\":\"rota nao encontrada\"}");
    }
}

/* ----------------------------------------------------------------------- */
/* Servidor                                                                 */
/* ----------------------------------------------------------------------- */

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

    printf("SIGEH-DF API ouvindo em http://localhost:%d\n", porta);
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

        rotear(cliente, metodo, caminho);
        close(cliente);
    }

    close(servidor);
    return 0;
}
