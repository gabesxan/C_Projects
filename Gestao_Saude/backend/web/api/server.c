#include "database.h"
#include "paciente_repository.h"
#include "medico_repository.h"
#include "ala_repository.h"
#include "leito_repository.h"
#include "triagem_repository.h"
#include "agendamento_repository.h"
#include "prontuario_repository.h"
#include "exame_repository.h"
#include "internacao_repository.h"
#include "usuario_repository.h"
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

/* Responde com o JSON produzido por uma funcao de listagem do repository. */
static void responderLista(int cliente, int (*listar)(char *, int), const char *erro)
{
    char *json = malloc(TAM_JSON);

    if (json != NULL && listar(json, TAM_JSON) == 1)
    {
        responder(cliente, "200 OK", json);
    }
    else
    {
        responder(cliente, "500 Internal Server Error", erro);
    }

    free(json);
}

/* Responde {"ativos":N} a partir de uma funcao de contagem do repository. */
static void responderContagem(int cliente, int (*contar)(void))
{
    char corpo[64];

    snprintf(corpo, sizeof(corpo), "{\"ativos\":%d}", contar());
    responder(cliente, "200 OK", corpo);
}

/* Resposta padrao de criacao: 201 em sucesso, 400 com 'erro' caso contrario. */
static void responderCriacao(int cliente, int ok, const char *erro)
{
    if (ok)
    {
        responder(cliente, "201 Created", "{\"status\":\"criado\"}");
    }
    else
    {
        responder(cliente, "400 Bad Request", erro);
    }
}

/* Resposta padrao de remocao: 200 em sucesso, 404 com 'erro' caso contrario. */
static void responderRemocao(int cliente, int ok, const char *erro)
{
    if (ok)
    {
        responder(cliente, "200 OK", "{\"status\":\"removido\"}");
    }
    else
    {
        responder(cliente, "404 Not Found", erro);
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
/* Autenticacao (HTTP Basic)                                                */
/* ----------------------------------------------------------------------- */

static int base64Valor(char c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

static int base64Decodificar(const char *entrada, char *saida, int tam)
{
    int acumulador = 0;
    int bits = 0;
    int n = 0;
    int i;

    for (i = 0; entrada[i] != '\0'; i++)
    {
        int v;

        if (entrada[i] == '=')
        {
            break;
        }

        v = base64Valor(entrada[i]);

        if (v < 0)
        {
            continue;
        }

        acumulador = (acumulador << 6) | v;
        bits += 6;

        if (bits >= 8)
        {
            bits -= 8;
            if (n >= tam - 1)
            {
                return 0;
            }
            saida[n++] = (char)((acumulador >> bits) & 0xFF);
        }
    }

    saida[n] = '\0';
    return 1;
}

/* Autentica o request pelo cabecalho 'Authorization: Basic'. Em sucesso
 * preenche papel/vinculos e retorna 1; caso contrario 0. */
static int autenticarRequest(const char *requisicao, char *papel, int papel_tam,
                             int *paciente_id, int *medico_id)
{
    const char *prefixo = "Authorization: Basic ";
    const char *inicio = strstr(requisicao, prefixo);
    char b64[512];
    char credenciais[512];
    char *separador;
    int n = 0;

    if (inicio == NULL)
    {
        return 0;
    }

    inicio += strlen(prefixo);

    while (inicio[n] != '\0' && inicio[n] != '\r' && inicio[n] != '\n' &&
           inicio[n] != ' ' && n < (int)sizeof(b64) - 1)
    {
        b64[n] = inicio[n];
        n++;
    }
    b64[n] = '\0';

    if (base64Decodificar(b64, credenciais, sizeof(credenciais)) == 0)
    {
        return 0;
    }

    separador = strchr(credenciais, ':');

    if (separador == NULL)
    {
        return 0;
    }

    *separador = '\0';

    return usuario_repo_autenticar(credenciais, separador + 1, papel, papel_tam,
                                   paciente_id, medico_id);
}

static int comecaCom(const char *texto, const char *prefixo)
{
    return strncmp(texto, prefixo, strlen(prefixo)) == 0;
}

static int ehRotaMe(const char *caminho)
{
    return strcmp(caminho, "/me") == 0 || comecaCom(caminho, "/me/");
}

static int ehCadastro(const char *caminho)
{
    return comecaCom(caminho, "/pacientes") || comecaCom(caminho, "/medicos") ||
           comecaCom(caminho, "/alas") || comecaCom(caminho, "/leitos");
}

static int ehClinico(const char *caminho)
{
    return comecaCom(caminho, "/triagens") || comecaCom(caminho, "/agendamentos") ||
           comecaCom(caminho, "/prontuarios") || comecaCom(caminho, "/exames") ||
           comecaCom(caminho, "/internacoes") || comecaCom(caminho, "/triagem/") ||
           comecaCom(caminho, "/relatorios");
}

/* Politica central de acesso por papel. 1 = permitido, 0 = negado.
 * Escopo "ver so o seu" (paciente/medico) e tratado pelas rotas sob /me. */
static int autorizado(const char *metodo, const char *caminho, const char *papel)
{
    if (strcmp(papel, "ADMIN") == 0)
    {
        return 1;
    }

    /* Qualquer usuario autenticado acessa o proprio perfil e seus dados. */
    if (ehRotaMe(caminho))
    {
        return 1;
    }

    if (strcmp(papel, "CADASTRO") == 0)
    {
        return ehCadastro(caminho);
    }

    if (strcmp(papel, "MEDICO") == 0)
    {
        if (strcmp(metodo, "GET") == 0 && ehCadastro(caminho))
        {
            return 1;
        }
        return ehClinico(caminho);
    }

    if (strcmp(papel, "ENFERMAGEM") == 0)
    {
        return strcmp(metodo, "GET") == 0 &&
               (comecaCom(caminho, "/internacoes") ||
                comecaCom(caminho, "/leitos") ||
                comecaCom(caminho, "/alas"));
    }

    /* PACIENTE: somente as rotas sob /me (ja liberadas acima). */
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

static void rotaListarPacientes(int cliente, const char *papel, int medico_id)
{
    char *json = malloc(TAM_JSON);
    int ok;

    if (json == NULL)
    {
        responder(cliente, "500 Internal Server Error",
                  "{\"erro\":\"sem memoria\"}");
        return;
    }

    /* MEDICO ve apenas os proprios pacientes (escopo por identidade). */
    if (strcmp(papel, "MEDICO") == 0)
    {
        ok = paciente_repo_listar_por_medico_json(medico_id, json, TAM_JSON);
    }
    else
    {
        ok = paciente_repo_listar_json(json, TAM_JSON);
    }

    if (ok == 1)
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

static void rotaRelatorioDistribuicao(int cliente)
{
    char *json = malloc(TAM_JSON);

    if (json != NULL && relatorio_service_distribuicao_json(json, TAM_JSON) == 1)
    {
        responder(cliente, "200 OK", json);
    }
    else
    {
        responder(cliente, "500 Internal Server Error",
                  "{\"erro\":\"falha ao gerar distribuicao\"}");
    }

    free(json);
}

static void rotaRelatorioAgendamentos(int cliente, const char *consulta)
{
    char inicio[16];
    char fim[16];
    char *json;

    extrairParam(consulta, "inicio", inicio, sizeof(inicio));
    extrairParam(consulta, "fim", fim, sizeof(fim));

    if (inicio[0] == '\0' || fim[0] == '\0')
    {
        responder(cliente, "400 Bad Request",
                  "{\"erro\":\"informe inicio e fim (YYYY-MM-DD)\"}");
        return;
    }

    json = malloc(TAM_JSON);

    if (json != NULL &&
        relatorio_service_agendamentos_periodo_json(inicio, fim, json, TAM_JSON) == 1)
    {
        responder(cliente, "200 OK", json);
    }
    else
    {
        responder(cliente, "500 Internal Server Error",
                  "{\"erro\":\"falha ao gerar relatorio por periodo\"}");
    }

    free(json);
}

static void rotaListarAgendamentos(int cliente, const char *papel, int medico_id)
{
    char *json = malloc(TAM_JSON);
    int ok;

    if (json == NULL)
    {
        responder(cliente, "500 Internal Server Error",
                  "{\"erro\":\"sem memoria\"}");
        return;
    }

    /* MEDICO ve apenas a propria agenda (escopo por identidade). */
    if (strcmp(papel, "MEDICO") == 0)
    {
        ok = agendamento_repo_listar_por_medico_json(medico_id, json, TAM_JSON);
    }
    else
    {
        ok = agendamento_repo_listar_json(json, TAM_JSON);
    }

    if (ok == 1)
    {
        responder(cliente, "200 OK", json);
    }
    else
    {
        responder(cliente, "500 Internal Server Error",
                  "{\"erro\":\"falha ao listar agendamentos\"}");
    }

    free(json);
}

static void rotaCriarAla(int cliente, const char *consulta)
{
    char nome[128];
    char tipo[16];
    char totalLeitos[16];

    extrairParam(consulta, "nome", nome, sizeof(nome));
    extrairParam(consulta, "tipo", tipo, sizeof(tipo));
    extrairParam(consulta, "total_leitos", totalLeitos, sizeof(totalLeitos));

    responderCriacao(cliente,
        ala_repo_criar(nome, atoi(tipo), atoi(totalLeitos)) == 1,
        "{\"erro\":\"dados invalidos para ala\"}");
}

static void rotaCriarLeito(int cliente, const char *consulta)
{
    char alaId[16];
    char numero[16];

    extrairParam(consulta, "ala_id", alaId, sizeof(alaId));
    extrairParam(consulta, "numero", numero, sizeof(numero));

    responderCriacao(cliente,
        leito_repo_criar(atoi(alaId), atoi(numero)) == 1,
        "{\"erro\":\"dados invalidos para leito\"}");
}

static void rotaCriarTriagem(int cliente, const char *consulta)
{
    char pacienteId[16];
    char tipo[16];
    char pontuacao[16];
    char classificacao[64];

    extrairParam(consulta, "paciente_id", pacienteId, sizeof(pacienteId));
    extrairParam(consulta, "tipo", tipo, sizeof(tipo));
    extrairParam(consulta, "pontuacao", pontuacao, sizeof(pontuacao));
    extrairParam(consulta, "classificacao", classificacao, sizeof(classificacao));

    responderCriacao(cliente,
        triagem_repo_criar(atoi(pacienteId), atoi(tipo), atoi(pontuacao),
                           classificacao) == 1,
        "{\"erro\":\"dados invalidos para triagem\"}");
}

static void rotaCriarAgendamento(int cliente, const char *consulta)
{
    char pacienteId[16];
    char medicoId[16];
    char data[32];
    char horario[16];

    extrairParam(consulta, "paciente_id", pacienteId, sizeof(pacienteId));
    extrairParam(consulta, "medico_id", medicoId, sizeof(medicoId));
    extrairParam(consulta, "data", data, sizeof(data));
    extrairParam(consulta, "horario", horario, sizeof(horario));

    responderCriacao(cliente,
        agendamento_repo_criar(atoi(pacienteId), atoi(medicoId), data, horario) == 1,
        "{\"erro\":\"dados invalidos para agendamento\"}");
}

static void rotaCriarProntuario(int cliente, const char *consulta)
{
    char pacienteId[16];
    char medicoId[16];
    char data[32];
    char observacoes[512];
    char diagnostico[256];
    char conduta[256];
    char alerta[16];

    extrairParam(consulta, "paciente_id", pacienteId, sizeof(pacienteId));
    extrairParam(consulta, "medico_id", medicoId, sizeof(medicoId));
    extrairParam(consulta, "data", data, sizeof(data));
    extrairParam(consulta, "observacoes", observacoes, sizeof(observacoes));
    extrairParam(consulta, "diagnostico", diagnostico, sizeof(diagnostico));
    extrairParam(consulta, "conduta", conduta, sizeof(conduta));
    extrairParam(consulta, "alerta_importante", alerta, sizeof(alerta));

    responderCriacao(cliente,
        prontuario_repo_criar(atoi(pacienteId), atoi(medicoId), data,
                              observacoes, diagnostico, conduta,
                              atoi(alerta)) == 1,
        "{\"erro\":\"dados invalidos para prontuario\"}");
}

static void rotaCriarExame(int cliente, const char *consulta)
{
    char pacienteId[16];
    char medicoId[16];
    char prontuarioId[16];
    char tipo[16];
    char dataSolicitacao[32];
    char urgente[16];

    extrairParam(consulta, "paciente_id", pacienteId, sizeof(pacienteId));
    extrairParam(consulta, "medico_id", medicoId, sizeof(medicoId));
    extrairParam(consulta, "prontuario_id", prontuarioId, sizeof(prontuarioId));
    extrairParam(consulta, "tipo", tipo, sizeof(tipo));
    extrairParam(consulta, "data_solicitacao", dataSolicitacao, sizeof(dataSolicitacao));
    extrairParam(consulta, "urgente", urgente, sizeof(urgente));

    responderCriacao(cliente,
        exame_repo_criar(atoi(pacienteId), atoi(medicoId), atoi(prontuarioId),
                         atoi(tipo), dataSolicitacao, atoi(urgente)) == 1,
        "{\"erro\":\"dados invalidos para exame\"}");
}

static void rotaCriarInternacao(int cliente, const char *consulta)
{
    char pacienteId[16];
    char alaId[16];
    char leitoId[16];
    char dataEntrada[32];

    extrairParam(consulta, "paciente_id", pacienteId, sizeof(pacienteId));
    extrairParam(consulta, "ala_id", alaId, sizeof(alaId));
    extrairParam(consulta, "leito_id", leitoId, sizeof(leitoId));
    extrairParam(consulta, "data_entrada", dataEntrada, sizeof(dataEntrada));

    responderCriacao(cliente,
        internacao_repo_criar(atoi(pacienteId), atoi(alaId), atoi(leitoId),
                              dataEntrada) == 1,
        "{\"erro\":\"dados invalidos para internacao\"}");
}

static void rotaInternacaoAlta(int cliente, int id, const char *consulta)
{
    char data[32];

    extrairParam(consulta, "data", data, sizeof(data));

    if (internacao_repo_dar_alta(id, data) == 1)
    {
        responder(cliente, "200 OK", "{\"status\":\"alta registrada\"}");
    }
    else
    {
        responder(cliente, "409 Conflict",
                  "{\"erro\":\"internacao nao encontrada ou ja com alta\"}");
    }
}

static void rotaCriarUsuario(int cliente, const char *consulta)
{
    char login[128];
    char senha[128];
    char papel[32];
    char pacienteId[16];
    char medicoId[16];

    extrairParam(consulta, "login", login, sizeof(login));
    extrairParam(consulta, "senha", senha, sizeof(senha));
    extrairParam(consulta, "papel", papel, sizeof(papel));
    extrairParam(consulta, "paciente_id", pacienteId, sizeof(pacienteId));
    extrairParam(consulta, "medico_id", medicoId, sizeof(medicoId));

    responderCriacao(cliente,
        usuario_repo_criar(login, senha, papel, atoi(pacienteId), atoi(medicoId)) == 1,
        "{\"erro\":\"dados invalidos para usuario\"}");
}

static void rotaMe(int cliente, const char *papel, int paciente_id, int medico_id)
{
    char corpo[160];

    snprintf(corpo, sizeof(corpo),
        "{\"papel\":\"%s\",\"pacienteId\":%d,\"medicoId\":%d}",
        papel, paciente_id, medico_id);
    responder(cliente, "200 OK", corpo);
}

static void rotaMeExames(int cliente, int paciente_id)
{
    char *json = malloc(TAM_JSON);

    if (json != NULL && exame_repo_listar_por_paciente_json(paciente_id, json, TAM_JSON) == 1)
    {
        responder(cliente, "200 OK", json);
    }
    else
    {
        responder(cliente, "500 Internal Server Error",
                  "{\"erro\":\"falha ao listar exames\"}");
    }

    free(json);
}

static void rotaMeProntuarios(int cliente, int paciente_id)
{
    char *json = malloc(TAM_JSON);

    if (json != NULL && prontuario_repo_listar_por_paciente_json(paciente_id, json, TAM_JSON) == 1)
    {
        responder(cliente, "200 OK", json);
    }
    else
    {
        responder(cliente, "500 Internal Server Error",
                  "{\"erro\":\"falha ao listar prontuarios\"}");
    }

    free(json);
}

static void rotaMeAgenda(int cliente, int medico_id)
{
    char *json = malloc(TAM_JSON);

    if (json != NULL && agendamento_repo_listar_por_medico_json(medico_id, json, TAM_JSON) == 1)
    {
        responder(cliente, "200 OK", json);
    }
    else
    {
        responder(cliente, "500 Internal Server Error",
                  "{\"erro\":\"falha ao listar agenda\"}");
    }

    free(json);
}

static void rotaMePacientes(int cliente, int medico_id)
{
    char *json = malloc(TAM_JSON);

    if (json != NULL && paciente_repo_listar_por_medico_json(medico_id, json, TAM_JSON) == 1)
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

/* ----------------------------------------------------------------------- */
/* Roteamento                                                               */
/* ----------------------------------------------------------------------- */

static void rotear(int cliente, const char *metodo, char *caminho,
                   const char *requisicao)
{
    char *consulta = strchr(caminho, '?');
    char acao[32];
    char papel[32];
    int authPacienteId = 0;
    int authMedicoId = 0;
    int id;

    if (consulta != NULL)
    {
        *consulta = '\0';
        consulta++;
    }

    /* /health e publico (liveness). */
    if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/health") == 0)
    {
        rotaHealth(cliente);
        return;
    }

    /* Todas as demais rotas exigem autenticacao (HTTP Basic). */
    if (autenticarRequest(requisicao, papel, sizeof(papel),
                          &authPacienteId, &authMedicoId) == 0)
    {
        responder(cliente, "401 Unauthorized",
                  "{\"erro\":\"credenciais invalidas\"}");
        return;
    }

    /* Politica de acesso por papel. */
    if (autorizado(metodo, caminho, papel) == 0)
    {
        responder(cliente, "403 Forbidden", "{\"erro\":\"acesso negado\"}");
        return;
    }

    if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/pacientes") == 0)
    {
        rotaListarPacientes(cliente, papel, authMedicoId);
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
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/alas") == 0)
    {
        responderLista(cliente, ala_repo_listar_json, "{\"erro\":\"falha ao listar alas\"}");
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/alas/contar") == 0)
    {
        responderContagem(cliente, ala_repo_contar_ativos);
    }
    else if (strcmp(metodo, "POST") == 0 && strcmp(caminho, "/alas") == 0)
    {
        rotaCriarAla(cliente, consulta);
    }
    else if (strcmp(metodo, "DELETE") == 0 && sscanf(caminho, "/alas/%d", &id) == 1)
    {
        responderRemocao(cliente, ala_repo_desativar(id) == 1, "{\"erro\":\"ala nao encontrada\"}");
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/leitos") == 0)
    {
        responderLista(cliente, leito_repo_listar_json, "{\"erro\":\"falha ao listar leitos\"}");
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/leitos/contar") == 0)
    {
        responderContagem(cliente, leito_repo_contar_ativos);
    }
    else if (strcmp(metodo, "POST") == 0 && strcmp(caminho, "/leitos") == 0)
    {
        rotaCriarLeito(cliente, consulta);
    }
    else if (strcmp(metodo, "DELETE") == 0 && sscanf(caminho, "/leitos/%d", &id) == 1)
    {
        responderRemocao(cliente, leito_repo_desativar(id) == 1, "{\"erro\":\"leito nao encontrado\"}");
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/triagens") == 0)
    {
        responderLista(cliente, triagem_repo_listar_json, "{\"erro\":\"falha ao listar triagens\"}");
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/triagens/contar") == 0)
    {
        responderContagem(cliente, triagem_repo_contar_ativos);
    }
    else if (strcmp(metodo, "POST") == 0 && strcmp(caminho, "/triagens") == 0)
    {
        rotaCriarTriagem(cliente, consulta);
    }
    else if (strcmp(metodo, "DELETE") == 0 && sscanf(caminho, "/triagens/%d", &id) == 1)
    {
        responderRemocao(cliente, triagem_repo_desativar(id) == 1, "{\"erro\":\"triagem nao encontrada\"}");
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/agendamentos") == 0)
    {
        rotaListarAgendamentos(cliente, papel, authMedicoId);
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/agendamentos/contar") == 0)
    {
        responderContagem(cliente, agendamento_repo_contar_ativos);
    }
    else if (strcmp(metodo, "POST") == 0 && strcmp(caminho, "/agendamentos") == 0)
    {
        rotaCriarAgendamento(cliente, consulta);
    }
    else if (strcmp(metodo, "DELETE") == 0 && sscanf(caminho, "/agendamentos/%d", &id) == 1)
    {
        responderRemocao(cliente, agendamento_repo_cancelar(id) == 1, "{\"erro\":\"agendamento nao encontrado\"}");
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/prontuarios") == 0)
    {
        responderLista(cliente, prontuario_repo_listar_json, "{\"erro\":\"falha ao listar prontuarios\"}");
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/prontuarios/contar") == 0)
    {
        responderContagem(cliente, prontuario_repo_contar_ativos);
    }
    else if (strcmp(metodo, "POST") == 0 && strcmp(caminho, "/prontuarios") == 0)
    {
        rotaCriarProntuario(cliente, consulta);
    }
    else if (strcmp(metodo, "DELETE") == 0 && sscanf(caminho, "/prontuarios/%d", &id) == 1)
    {
        responderRemocao(cliente, prontuario_repo_desativar(id) == 1, "{\"erro\":\"prontuario nao encontrado\"}");
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/exames") == 0)
    {
        responderLista(cliente, exame_repo_listar_json, "{\"erro\":\"falha ao listar exames\"}");
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/exames/contar") == 0)
    {
        responderContagem(cliente, exame_repo_contar_ativos);
    }
    else if (strcmp(metodo, "POST") == 0 && strcmp(caminho, "/exames") == 0)
    {
        rotaCriarExame(cliente, consulta);
    }
    else if (strcmp(metodo, "DELETE") == 0 && sscanf(caminho, "/exames/%d", &id) == 1)
    {
        responderRemocao(cliente, exame_repo_desativar(id) == 1, "{\"erro\":\"exame nao encontrado\"}");
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/internacoes") == 0)
    {
        responderLista(cliente, internacao_repo_listar_json, "{\"erro\":\"falha ao listar internacoes\"}");
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/internacoes/contar") == 0)
    {
        responderContagem(cliente, internacao_repo_contar_ativos);
    }
    else if (strcmp(metodo, "POST") == 0 && strcmp(caminho, "/internacoes") == 0)
    {
        rotaCriarInternacao(cliente, consulta);
    }
    else if (strcmp(metodo, "POST") == 0 && sscanf(caminho, "/internacoes/%d/%31s", &id, acao) == 2 &&
             strcmp(acao, "alta") == 0)
    {
        rotaInternacaoAlta(cliente, id, consulta);
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/relatorios/indicadores") == 0)
    {
        rotaRelatorioIndicadores(cliente);
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/relatorios/distribuicao") == 0)
    {
        rotaRelatorioDistribuicao(cliente);
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/relatorios/agendamentos") == 0)
    {
        rotaRelatorioAgendamentos(cliente, consulta);
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/me") == 0)
    {
        rotaMe(cliente, papel, authPacienteId, authMedicoId);
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/me/exames") == 0)
    {
        rotaMeExames(cliente, authPacienteId);
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/me/prontuarios") == 0)
    {
        rotaMeProntuarios(cliente, authPacienteId);
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/me/agenda") == 0)
    {
        rotaMeAgenda(cliente, authMedicoId);
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/me/pacientes") == 0)
    {
        rotaMePacientes(cliente, authMedicoId);
    }
    else if (strcmp(metodo, "POST") == 0 && strcmp(caminho, "/usuarios") == 0)
    {
        rotaCriarUsuario(cliente, consulta);
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/usuarios") == 0)
    {
        responderLista(cliente, usuario_repo_listar_json, "{\"erro\":\"falha ao listar usuarios\"}");
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/usuarios/contar") == 0)
    {
        responderContagem(cliente, usuario_repo_contar_ativos);
    }
    else if (strcmp(metodo, "DELETE") == 0 && sscanf(caminho, "/usuarios/%d", &id) == 1)
    {
        responderRemocao(cliente, usuario_repo_desativar(id) == 1, "{\"erro\":\"usuario nao encontrado\"}");
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

        rotear(cliente, metodo, caminho, requisicao);
        close(cliente);
    }

    close(servidor);
    return 0;
}
