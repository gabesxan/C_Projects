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
#include "auditoria_repository.h"
#include "prescricao_repository.h"
#include "triagem_service.h"
#include "relatorio_service.h"
#include "credencial_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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

/* Diretorio do front buildado (servido para rotas que nao sao da API). */
#define DIR_PUBLICO "public"

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
    if (c >= 'A' && c <= 'Z')
        return c - 'A';
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 26;
    if (c >= '0' && c <= '9')
        return c - '0' + 52;
    if (c == '+')
        return 62;
    if (c == '/')
        return 63;
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

/* Busca 'prefixo' em 'texto' ignorando maiusculas/minusculas e devolve o
 * ponteiro logo apos o prefixo, ou NULL. Necessario porque proxies e HTTP/2
 * normalizam o nome do cabecalho (ex.: "authorization:" em minusculas). */
static const char *aposCabecalho(const char *texto, const char *prefixo)
{
    size_t plen = strlen(prefixo);
    size_t i;

    for (i = 0; texto[i] != '\0'; i++)
    {
        size_t j = 0;

        while (j < plen && texto[i + j] != '\0' &&
               tolower((unsigned char)texto[i + j]) ==
                   tolower((unsigned char)prefixo[j]))
        {
            j++;
        }

        if (j == plen)
        {
            return texto + i + plen;
        }
    }

    return NULL;
}

/* Autentica o request pelo cabecalho 'Authorization: Basic'. Em sucesso
 * preenche papel/vinculos, o login e o id do usuario e retorna 1; senao 0.
 * login_out/usuario_id podem ser NULL quando o chamador nao precisa deles. */
static int autenticarRequest(const char *requisicao, char *papel, int papel_tam,
                             int *paciente_id, int *medico_id,
                             char *login_out, int login_tam, int *usuario_id)
{
    const char *prefixo = "Authorization: Basic ";
    const char *inicio = aposCabecalho(requisicao, prefixo);
    char b64[512];
    char credenciais[512];
    char *separador;
    int n = 0;

    if (inicio == NULL)
    {
        return 0;
    }

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

    if (login_out != NULL && login_tam > 0)
    {
        strncpy(login_out, credenciais, (size_t)login_tam - 1);
        login_out[login_tam - 1] = '\0';
    }

    return usuario_repo_autenticar(credenciais, separador + 1, papel, papel_tam,
                                   paciente_id, medico_id, usuario_id);
}

static int comecaCom(const char *texto, const char *prefixo)
{
    return strncmp(texto, prefixo, strlen(prefixo)) == 0;
}

static int terminaCom(const char *texto, const char *sufixo)
{
    size_t lt = strlen(texto);
    size_t ls = strlen(sufixo);
    return lt >= ls && strcmp(texto + lt - ls, sufixo) == 0;
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
           comecaCom(caminho, "/prescricoes") || comecaCom(caminho, "/relatorios");
}

/* Politica central de acesso por papel. 1 = permitido, 0 = negado.
 * Escopo "ver so o seu" (paciente/medico) e tratado pelas rotas sob /me. */
static int autorizado(const char *metodo, const char *caminho, const char *papel)
{
    if (strcmp(papel, "ADMIN") == 0)
    {
        return 1;
    }

    /* Qualquer usuario autenticado acessa o proprio perfil/sessao e seus dados. */
    if (ehRotaMe(caminho) || strcmp(caminho, "/login") == 0)
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
        /* Triagem e o fluxo principal da enfermagem: classificar risco e
         * registrar triagem (buscar/cadastrar paciente faz parte do fluxo). */
        if (comecaCom(caminho, "/triagens") || comecaCom(caminho, "/triagem/"))
        {
            return strcmp(metodo, "GET") == 0 || strcmp(metodo, "POST") == 0;
        }

        /* Pacientes: buscar (GET) e cadastrar (POST) dentro da triagem. */
        if (comecaCom(caminho, "/pacientes"))
        {
            return strcmp(metodo, "GET") == 0 || strcmp(metodo, "POST") == 0;
        }

        /* Enfermagem gerencia status de leito e transfere internacao. */
        if (comecaCom(caminho, "/leitos"))
        {
            return strcmp(metodo, "GET") == 0 ||
                   (strcmp(metodo, "POST") == 0 && terminaCom(caminho, "/status"));
        }

        if (comecaCom(caminho, "/internacoes"))
        {
            return strcmp(metodo, "GET") == 0 ||
                   (strcmp(metodo, "POST") == 0 && terminaCom(caminho, "/transferir"));
        }

        /* Acompanhamento de alas/medicos/prescricoes: somente leitura. */
        return strcmp(metodo, "GET") == 0 &&
               (comecaCom(caminho, "/alas") ||
                comecaCom(caminho, "/medicos") ||
                comecaCom(caminho, "/prescricoes"));
    }

    /* PACIENTE: somente as rotas sob /me (ja liberadas acima). */
    return 0;
}

/* ----------------------------------------------------------------------- */
/* Sessao do usuario autenticado e auditoria                                */
/* ----------------------------------------------------------------------- */

/* Identidade do usuario autenticado no request atual. Usada para auditar
 * acoes sensiveis e para derivar o autor de registros clinicos (em vez de
 * confiar num medico_id vindo do cliente). */
typedef struct
{
    char papel[32];
    char login[128];
    int usuario_id;
    int paciente_id;
    int medico_id;
} Sessao;

/* Registra uma acao sensivel na trilha de auditoria. Falha de auditoria
 * nunca interrompe a operacao principal (retorno ignorado de proposito). */
static void auditar(const Sessao *s, const char *acao, const char *entidade,
                    int entidade_id, const char *detalhe)
{
    auditoria_registrar(s->usuario_id, s->login, acao, entidade, entidade_id,
                        detalhe);
}

/* Deriva o medico autor de um registro clinico: um MEDICO so registra em
 * nome de si mesmo (ignora qualquer medico_id vindo do cliente); ADMIN pode
 * informar o medico_id explicitamente. Retorna 0 quando indeterminado. */
static int medicoAutor(const Sessao *s, const char *medicoIdCliente)
{
    if (strcmp(s->papel, "MEDICO") == 0)
    {
        return s->medico_id;
    }

    return atoi(medicoIdCliente);
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

static void rotaCriarPaciente(int cliente, const char *consulta, const Sessao *s)
{
    char nome[128];
    char nascimento[16];
    char documento[40];
    char tipoDocumento[16];
    char telefone[32];
    char sexo[8];
    char regiaoStr[16];
    char responsavel[128];
    char alergias[256];

    extrairParam(consulta, "nome", nome, sizeof(nome));
    extrairParam(consulta, "nascimento", nascimento, sizeof(nascimento));
    extrairParam(consulta, "documento", documento, sizeof(documento));
    extrairParam(consulta, "tipo_documento", tipoDocumento, sizeof(tipoDocumento));
    extrairParam(consulta, "telefone", telefone, sizeof(telefone));
    extrairParam(consulta, "sexo", sexo, sizeof(sexo));
    extrairParam(consulta, "regiao", regiaoStr, sizeof(regiaoStr));
    extrairParam(consulta, "responsavel", responsavel, sizeof(responsavel));
    extrairParam(consulta, "alergias", alergias, sizeof(alergias));

    if (paciente_repo_criar(nome, nascimento, documento, tipoDocumento, telefone,
                            sexo, atoi(regiaoStr), responsavel, alergias) == 1)
    {
        auditar(s, "CRIAR", "paciente", 0, nome);
        responder(cliente, "201 Created", "{\"status\":\"criado\"}");
    }
    else
    {
        responder(cliente, "400 Bad Request",
                  "{\"erro\":\"dados invalidos (verifique nascimento, documento, "
                  "responsavel de menor ou CPF ja cadastrado)\"}");
    }
}

static void rotaDesativarPaciente(int cliente, int id, const Sessao *s)
{
    if (paciente_repo_desativar(id) == 1)
    {
        auditar(s, "DESATIVAR", "paciente", id, "");
        responder(cliente, "200 OK", "{\"status\":\"desativado\"}");
    }
    else
    {
        responder(cliente, "404 Not Found",
                  "{\"erro\":\"paciente nao encontrado ou ja inativo\"}");
    }
}

static void rotaBuscarPacientes(int cliente, const char *consulta)
{
    char termo[128];
    char *json;

    extrairParam(consulta, "q", termo, sizeof(termo));

    json = malloc(TAM_JSON);

    if (json != NULL && paciente_repo_buscar_json(termo, json, TAM_JSON) == 1)
    {
        responder(cliente, "200 OK", json);
    }
    else
    {
        responder(cliente, "500 Internal Server Error",
                  "{\"erro\":\"falha na busca de pacientes\"}");
    }

    free(json);
}

static void rotaDetalhePaciente(int cliente, int id)
{
    char *json = malloc(TAM_JSON);

    if (json != NULL && paciente_repo_detalhe_json(id, json, TAM_JSON) == 1)
    {
        responder(cliente, "200 OK", json);
    }
    else
    {
        responder(cliente, "404 Not Found",
                  "{\"erro\":\"paciente nao encontrado\"}");
    }

    free(json);
}

/* Historico clinico agregado de um paciente: prontuarios, exames, prescricoes,
 * triagens e agendamentos num unico objeto. Dados clinicos: restrito a equipe
 * assistencial (CADASTRO nao ve). */
static void rotaHistoricoPaciente(int cliente, int id, const Sessao *s)
{
    char *prontuarios = malloc(TAM_JSON);
    char *exames = malloc(TAM_JSON);
    char *prescricoes = malloc(TAM_JSON);
    char *triagens = malloc(TAM_JSON);
    char *agendamentos = malloc(TAM_JSON);
    char *saida = malloc(TAM_JSON);
    int ok = 0;

    if (strcmp(s->papel, "CADASTRO") == 0)
    {
        responder(cliente, "403 Forbidden",
                  "{\"erro\":\"sem acesso ao historico clinico\"}");
        free(prontuarios);
        free(exames);
        free(prescricoes);
        free(triagens);
        free(agendamentos);
        free(saida);
        return;
    }

    if (prontuarios && exames && prescricoes && triagens && agendamentos && saida &&
        prontuario_repo_listar_por_paciente_json(id, prontuarios, TAM_JSON) == 1 &&
        exame_repo_listar_por_paciente_json(id, exames, TAM_JSON) == 1 &&
        prescricao_repo_listar_por_paciente_json(id, prescricoes, TAM_JSON) == 1 &&
        triagem_service_historico_json(id, triagens, TAM_JSON) == 1 &&
        agendamento_repo_listar_por_paciente_json(id, agendamentos, TAM_JSON) == 1)
    {
        int n = snprintf(saida, TAM_JSON,
                         "{\"prontuarios\":%s,\"exames\":%s,\"prescricoes\":%s,"
                         "\"triagens\":%s,\"agendamentos\":%s}",
                         prontuarios, exames, prescricoes, triagens, agendamentos);
        ok = (n > 0 && n < TAM_JSON);
    }

    if (ok)
    {
        responder(cliente, "200 OK", saida);
    }
    else
    {
        responder(cliente, "500 Internal Server Error",
                  "{\"erro\":\"falha ao montar historico\"}");
    }

    free(prontuarios);
    free(exames);
    free(prescricoes);
    free(triagens);
    free(agendamentos);
    free(saida);
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

/* Fluxo de triagem: cadastra um paciente novo E ja cria seu acesso PACIENTE,
 * gerando login unico (pac-<nome><sufixo>) e senha automatica, devolvendo as
 * credenciais para exibicao imediata. */
static void rotaTriagemCadastrarPaciente(int cliente, const char *consulta,
                                         const Sessao *s)
{
    char nome[128];
    char nascimento[16];
    char documento[40];
    char tipoDocumento[16];
    char telefone[32];
    char sexo[8];
    char regiaoStr[16];
    char responsavel[128];
    char alergias[256];
    char norm[128];
    char login[180];
    char senha[40];
    char corpo[512];
    int pacienteId = 0;
    int sufixo = 1;

    extrairParam(consulta, "nome", nome, sizeof(nome));
    extrairParam(consulta, "nascimento", nascimento, sizeof(nascimento));
    extrairParam(consulta, "documento", documento, sizeof(documento));
    extrairParam(consulta, "tipo_documento", tipoDocumento, sizeof(tipoDocumento));
    extrairParam(consulta, "telefone", telefone, sizeof(telefone));
    extrairParam(consulta, "sexo", sexo, sizeof(sexo));
    extrairParam(consulta, "regiao", regiaoStr, sizeof(regiaoStr));
    extrairParam(consulta, "responsavel", responsavel, sizeof(responsavel));
    extrairParam(consulta, "alergias", alergias, sizeof(alergias));

    if (paciente_repo_criar_retornando_id(nome, nascimento, documento,
                                          tipoDocumento, telefone, sexo, atoi(regiaoStr), responsavel,
                                          alergias, &pacienteId) != 1 ||
        pacienteId <= 0)
    {
        responder(cliente, "400 Bad Request",
                  "{\"erro\":\"dados invalidos (verifique nascimento, documento, "
                  "responsavel de menor ou CPF ja cadastrado)\"}");
        return;
    }

    /* Login unico: pac-<nome-normalizado>, com sufixo numerico se ja existir. */
    if (credencial_normalizar(nome, norm, sizeof(norm)) == 0 || norm[0] == '\0')
    {
        strcpy(norm, "paciente");
    }
    snprintf(login, sizeof(login), "pac-%s", norm);
    while (usuario_repo_login_existe(login))
    {
        sufixo++;
        snprintf(login, sizeof(login), "pac-%s%d", norm, sufixo);
    }

    if (credencial_gerar_senha(nome, nascimento, senha, sizeof(senha)) == 0)
    {
        responder(cliente, "500 Internal Server Error",
                  "{\"erro\":\"falha ao gerar senha\"}");
        return;
    }

    if (usuario_repo_criar(nome, login, senha, "PACIENTE", pacienteId, 0) != 1)
    {
        /* Paciente foi criado; sinaliza que o acesso nao pode ser gerado. */
        snprintf(corpo, sizeof(corpo),
                 "{\"pacienteId\":%d,\"erro\":\"paciente criado, mas falha ao criar acesso\"}",
                 pacienteId);
        responder(cliente, "409 Conflict", corpo);
        return;
    }

    auditar(s, "CRIAR", "paciente", pacienteId, nome);
    auditar(s, "CRIAR", "usuario", pacienteId, login);

    snprintf(corpo, sizeof(corpo),
             "{\"pacienteId\":%d,\"login\":\"%s\",\"senha\":\"%s\"}",
             pacienteId, login, senha);
    responder(cliente, "201 Created", corpo);
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

static void rotaListarTriagens(int cliente, const char *papel, int medico_id)
{
    char *json = malloc(TAM_JSON);
    int ok;

    if (json == NULL)
    {
        responder(cliente, "500 Internal Server Error",
                  "{\"erro\":\"sem memoria\"}");
        return;
    }

    /* MEDICO ve apenas as triagens da sua especialidade (escopo por identidade);
     * o tipo da triagem define a especialidade provavel. */
    if (strcmp(papel, "MEDICO") == 0)
    {
        char especialidade[64];

        if (medico_repo_especialidade(medico_id, especialidade,
                                      sizeof(especialidade)) == 1)
        {
            ok = triagem_service_listar_por_especialidade_json(
                especialidade, json, TAM_JSON);
        }
        else
        {
            ok = 0;
        }
    }
    else
    {
        ok = triagem_repo_listar_json(json, TAM_JSON);
    }

    if (ok == 1)
    {
        responder(cliente, "200 OK", json);
    }
    else
    {
        responder(cliente, "500 Internal Server Error",
                  "{\"erro\":\"falha ao listar triagens\"}");
    }

    free(json);
}

static void rotaListarProntuarios(int cliente, const char *papel, int medico_id)
{
    char *json = malloc(TAM_JSON);
    int ok;

    if (json == NULL)
    {
        responder(cliente, "500 Internal Server Error",
                  "{\"erro\":\"sem memoria\"}");
        return;
    }

    /* MEDICO ve apenas os prontuarios que ele assinou (escopo por identidade). */
    if (strcmp(papel, "MEDICO") == 0)
    {
        ok = prontuario_repo_listar_por_medico_json(medico_id, json, TAM_JSON);
    }
    else
    {
        ok = prontuario_repo_listar_json(json, TAM_JSON);
    }

    if (ok == 1)
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

static void rotaListarExames(int cliente, const char *papel, int medico_id)
{
    char *json = malloc(TAM_JSON);
    int ok;

    if (json == NULL)
    {
        responder(cliente, "500 Internal Server Error",
                  "{\"erro\":\"sem memoria\"}");
        return;
    }

    /* MEDICO ve apenas os exames que ele solicitou (escopo por identidade). */
    if (strcmp(papel, "MEDICO") == 0)
    {
        ok = exame_repo_listar_por_medico_json(medico_id, json, TAM_JSON);
    }
    else
    {
        ok = exame_repo_listar_json(json, TAM_JSON);
    }

    if (ok == 1)
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

static void rotaListarPrescricoes(int cliente, const char *papel, int medico_id)
{
    char *json = malloc(TAM_JSON);
    int ok;

    if (json == NULL)
    {
        responder(cliente, "500 Internal Server Error",
                  "{\"erro\":\"sem memoria\"}");
        return;
    }

    /* MEDICO ve apenas as proprias prescricoes; ENFERMAGEM e ADMIN veem todas
     * as ativas (a fila de "remedios a aplicar"). */
    if (strcmp(papel, "MEDICO") == 0)
    {
        ok = prescricao_repo_listar_por_medico_json(medico_id, json, TAM_JSON);
    }
    else
    {
        ok = prescricao_repo_listar_json(json, TAM_JSON);
    }

    if (ok == 1)
    {
        responder(cliente, "200 OK", json);
    }
    else
    {
        responder(cliente, "500 Internal Server Error",
                  "{\"erro\":\"falha ao listar prescricoes\"}");
    }

    free(json);
}

static void rotaCriarPrescricao(int cliente, const char *consulta, const Sessao *s)
{
    char pacienteId[16];
    char medicoId[16];
    char medicamento[128];
    char dosagem[64];
    char frequencia[64];
    char via[48];
    char duracao[48];
    char observacoes[512];
    int medico;
    int ok;

    extrairParam(consulta, "paciente_id", pacienteId, sizeof(pacienteId));
    extrairParam(consulta, "medico_id", medicoId, sizeof(medicoId));
    extrairParam(consulta, "medicamento", medicamento, sizeof(medicamento));
    extrairParam(consulta, "dosagem", dosagem, sizeof(dosagem));
    extrairParam(consulta, "frequencia", frequencia, sizeof(frequencia));
    extrairParam(consulta, "via", via, sizeof(via));
    extrairParam(consulta, "duracao", duracao, sizeof(duracao));
    extrairParam(consulta, "observacoes", observacoes, sizeof(observacoes));

    medico = medicoAutor(s, medicoId);

    if (medico <= 0)
    {
        responder(cliente, "400 Bad Request",
                  "{\"erro\":\"medico prescritor indefinido\"}");
        return;
    }

    ok = prescricao_repo_criar(atoi(pacienteId), medico, medicamento,
                               dosagem, frequencia, via, duracao,
                               observacoes) == 1;

    if (ok)
    {
        auditar(s, "PRESCREVER", "prescricao", atoi(pacienteId), medicamento);
    }

    responderCriacao(cliente, ok,
                     "{\"erro\":\"dados invalidos ou paciente alergico ao medicamento\"}");
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

static void rotaCriarTriagem(int cliente, const char *consulta, const Sessao *s)
{
    char pacienteId[16];
    char tipo[16];
    char pontuacao[16];
    char classificacao[64];
    char queixa[256];
    char pressao[32];
    char temperatura[32];
    char freqCardiaca[32];
    char saturacao[32];
    int ok;

    extrairParam(consulta, "paciente_id", pacienteId, sizeof(pacienteId));
    extrairParam(consulta, "tipo", tipo, sizeof(tipo));
    extrairParam(consulta, "pontuacao", pontuacao, sizeof(pontuacao));
    extrairParam(consulta, "classificacao", classificacao, sizeof(classificacao));
    extrairParam(consulta, "queixa", queixa, sizeof(queixa));
    extrairParam(consulta, "pressao", pressao, sizeof(pressao));
    extrairParam(consulta, "temperatura", temperatura, sizeof(temperatura));
    extrairParam(consulta, "freq_cardiaca", freqCardiaca, sizeof(freqCardiaca));
    extrairParam(consulta, "saturacao", saturacao, sizeof(saturacao));

    ok = triagem_repo_criar_completa(atoi(pacienteId), atoi(tipo),
                                     atoi(pontuacao), classificacao, queixa, pressao, temperatura,
                                     freqCardiaca, saturacao) == 1;

    if (ok)
    {
        auditar(s, "TRIAGEM", "triagem", atoi(pacienteId), classificacao);
    }

    responderCriacao(cliente, ok, "{\"erro\":\"dados invalidos para triagem\"}");
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

static void rotaCriarProntuario(int cliente, const char *consulta, const Sessao *s)
{
    char pacienteId[16];
    char medicoId[16];
    char data[32];
    char observacoes[512];
    char diagnostico[256];
    char conduta[256];
    char alerta[16];
    int medico;
    int ok;

    extrairParam(consulta, "paciente_id", pacienteId, sizeof(pacienteId));
    extrairParam(consulta, "medico_id", medicoId, sizeof(medicoId));
    extrairParam(consulta, "data", data, sizeof(data));
    extrairParam(consulta, "observacoes", observacoes, sizeof(observacoes));
    extrairParam(consulta, "diagnostico", diagnostico, sizeof(diagnostico));
    extrairParam(consulta, "conduta", conduta, sizeof(conduta));
    extrairParam(consulta, "alerta_importante", alerta, sizeof(alerta));

    medico = medicoAutor(s, medicoId);

    if (medico <= 0)
    {
        responder(cliente, "400 Bad Request",
                  "{\"erro\":\"medico autor do prontuario indefinido\"}");
        return;
    }

    ok = prontuario_repo_criar(atoi(pacienteId), medico, data,
                               observacoes, diagnostico, conduta,
                               atoi(alerta)) == 1;

    if (ok)
    {
        auditar(s, "CRIAR", "prontuario", atoi(pacienteId), diagnostico);
    }

    responderCriacao(cliente, ok,
                     "{\"erro\":\"dados invalidos para prontuario\"}");
}

static void rotaCriarExame(int cliente, const char *consulta, const Sessao *s)
{
    char pacienteId[16];
    char medicoId[16];
    char prontuarioId[16];
    char tipo[16];
    char dataSolicitacao[32];
    char urgente[16];
    int medico;
    int ok;

    extrairParam(consulta, "paciente_id", pacienteId, sizeof(pacienteId));
    extrairParam(consulta, "medico_id", medicoId, sizeof(medicoId));
    extrairParam(consulta, "prontuario_id", prontuarioId, sizeof(prontuarioId));
    extrairParam(consulta, "tipo", tipo, sizeof(tipo));
    extrairParam(consulta, "data_solicitacao", dataSolicitacao, sizeof(dataSolicitacao));
    extrairParam(consulta, "urgente", urgente, sizeof(urgente));

    medico = medicoAutor(s, medicoId);

    if (medico <= 0)
    {
        responder(cliente, "400 Bad Request",
                  "{\"erro\":\"medico solicitante do exame indefinido\"}");
        return;
    }

    ok = exame_repo_criar(atoi(pacienteId), medico, atoi(prontuarioId),
                          atoi(tipo), dataSolicitacao, atoi(urgente)) == 1;

    if (ok)
    {
        auditar(s, "CRIAR", "exame", atoi(pacienteId), tipo);
    }

    responderCriacao(cliente, ok, "{\"erro\":\"dados invalidos para exame\"}");
}

static void rotaCriarInternacao(int cliente, const char *consulta, const Sessao *s)
{
    char pacienteId[16];
    char alaId[16];
    char leitoId[16];
    char dataEntrada[32];
    char responsavel[128];
    int ok;

    extrairParam(consulta, "paciente_id", pacienteId, sizeof(pacienteId));
    extrairParam(consulta, "ala_id", alaId, sizeof(alaId));
    extrairParam(consulta, "leito_id", leitoId, sizeof(leitoId));
    extrairParam(consulta, "data_entrada", dataEntrada, sizeof(dataEntrada));
    extrairParam(consulta, "responsavel", responsavel, sizeof(responsavel));

    ok = internacao_repo_criar(atoi(pacienteId), atoi(alaId), atoi(leitoId),
                               dataEntrada, responsavel) == 1;

    if (ok)
    {
        auditar(s, "INTERNAR", "internacao", atoi(pacienteId), responsavel);
    }

    responderCriacao(cliente, ok,
        "{\"erro\":\"dados invalidos ou leito indisponivel\"}");
}

static void rotaInternacaoAlta(int cliente, int id, const char *consulta,
                               const Sessao *s)
{
    char data[32];
    char resumo[512];
    char diagnostico[256];
    char orientacoes[512];

    extrairParam(consulta, "data", data, sizeof(data));
    extrairParam(consulta, "resumo", resumo, sizeof(resumo));
    extrairParam(consulta, "diagnostico", diagnostico, sizeof(diagnostico));
    extrairParam(consulta, "orientacoes", orientacoes, sizeof(orientacoes));

    if (internacao_repo_dar_alta(id, data, resumo, diagnostico, orientacoes) == 1)
    {
        auditar(s, "ALTA", "internacao", id, diagnostico);
        responder(cliente, "200 OK", "{\"status\":\"alta registrada\"}");
    }
    else
    {
        responder(cliente, "409 Conflict",
                  "{\"erro\":\"internacao invalida, sem alta, ou falta resumo/diagnostico\"}");
    }
}

static void rotaInternacaoTransferir(int cliente, int id, const char *consulta,
                                     const Sessao *s)
{
    char leito[16];
    char data[32];
    char responsavel[128];
    int ok;

    extrairParam(consulta, "leito_id", leito, sizeof(leito));
    extrairParam(consulta, "data", data, sizeof(data));
    extrairParam(consulta, "responsavel", responsavel, sizeof(responsavel));

    ok = internacao_repo_transferir(id, atoi(leito), data, responsavel) == 1;

    if (ok)
    {
        auditar(s, "TRANSFERIR", "internacao", id, responsavel);
    }

    responderRemocao(cliente, ok,
        "{\"erro\":\"transferencia invalida (internacao/leito destino)\"}");
}

static void rotaLeitoStatus(int cliente, int id, const char *consulta,
                            const Sessao *s)
{
    char valor[32];
    int ok;

    extrairParam(consulta, "valor", valor, sizeof(valor));
    ok = leito_repo_registrar_status(id, valor, s->login) == 1;

    if (ok)
    {
        auditar(s, "LEITO_STATUS", "leito", id, valor);
    }

    responderRemocao(cliente, ok,
        "{\"erro\":\"status de leito invalido\"}");
}

static void rotaCriarUsuario(int cliente, const char *consulta, const Sessao *s)
{
    char nome[128];
    char login[128];
    char senha[128];
    char papel[32];
    char pacienteId[16];
    char medicoId[16];
    int ok;

    extrairParam(consulta, "nome", nome, sizeof(nome));
    extrairParam(consulta, "login", login, sizeof(login));
    extrairParam(consulta, "senha", senha, sizeof(senha));
    extrairParam(consulta, "papel", papel, sizeof(papel));
    extrairParam(consulta, "paciente_id", pacienteId, sizeof(pacienteId));
    extrairParam(consulta, "medico_id", medicoId, sizeof(medicoId));

    ok = usuario_repo_criar(nome, login, senha, papel, atoi(pacienteId),
                            atoi(medicoId)) == 1;

    if (ok)
    {
        char detalhe[192];
        snprintf(detalhe, sizeof(detalhe), "login=%s papel=%s", login, papel);
        auditar(s, "CRIAR", "usuario", 0, detalhe);
    }

    responderCriacao(cliente, ok, "{\"erro\":\"dados invalidos para usuario\"}");
}

static void rotaMe(int cliente, const char *papel, int paciente_id, int medico_id)
{
    char corpo[160];

    snprintf(corpo, sizeof(corpo),
             "{\"papel\":\"%s\",\"pacienteId\":%d,\"medicoId\":%d}",
             papel, paciente_id, medico_id);
    responder(cliente, "200 OK", corpo);
}

/* Confirma a sessao e registra o evento de login na auditoria. O front chama
 * esta rota uma vez no login (diferente de /me, usado a cada navegacao). */
static void rotaLogin(int cliente, const Sessao *s)
{
    char corpo[200];

    auditar(s, "LOGIN", "usuario", s->usuario_id, s->login);
    snprintf(corpo, sizeof(corpo),
             "{\"papel\":\"%s\",\"login\":\"%s\",\"pacienteId\":%d,\"medicoId\":%d}",
             s->papel, s->login, s->paciente_id, s->medico_id);
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

static void rotaMeReceitas(int cliente, int paciente_id)
{
    char *json = malloc(TAM_JSON);

    if (json != NULL && prescricao_repo_listar_por_paciente_json(paciente_id, json, TAM_JSON) == 1)
    {
        responder(cliente, "200 OK", json);
    }
    else
    {
        responder(cliente, "500 Internal Server Error",
                  "{\"erro\":\"falha ao listar receitas\"}");
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

static void rotaMeResumo(int cliente, int medico_id)
{
    char *json = malloc(TAM_JSON);

    if (json != NULL && relatorio_service_resumo_medico_json(medico_id, json, TAM_JSON) == 1)
    {
        responder(cliente, "200 OK", json);
    }
    else
    {
        responder(cliente, "500 Internal Server Error",
                  "{\"erro\":\"falha ao gerar resumo do medico\"}");
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
/* Conteudo estatico (front buildado)                                       */
/* ----------------------------------------------------------------------- */

/* Mapeia a extensao do arquivo para o Content-Type apropriado. */
static const char *tipoConteudo(const char *caminho)
{
    const char *ext = strrchr(caminho, '.');

    if (ext == NULL)
        return "application/octet-stream";
    if (strcmp(ext, ".html") == 0)
        return "text/html; charset=utf-8";
    if (strcmp(ext, ".js") == 0)
        return "text/javascript";
    if (strcmp(ext, ".css") == 0)
        return "text/css";
    if (strcmp(ext, ".svg") == 0)
        return "image/svg+xml";
    if (strcmp(ext, ".json") == 0)
        return "application/json";
    if (strcmp(ext, ".ico") == 0)
        return "image/x-icon";
    if (strcmp(ext, ".png") == 0)
        return "image/png";
    if (strcmp(ext, ".woff2") == 0)
        return "font/woff2";
    return "application/octet-stream";
}

/* Envia o arquivo (binario) com cabecalho HTTP. Retorna 1 se enviou, 0 se
 * o arquivo nao existe ou houve erro. */
static int enviarArquivo(int cliente, const char *caminhoArquivo)
{
    FILE *f = fopen(caminhoArquivo, "rb");
    char cabecalho[256];
    char *conteudo;
    long tam;
    int n;

    if (f == NULL)
        return 0;

    if (fseek(f, 0, SEEK_END) != 0 || (tam = ftell(f)) < 0 ||
        fseek(f, 0, SEEK_SET) != 0)
    {
        fclose(f);
        return 0;
    }

    conteudo = malloc((size_t)tam);
    if (conteudo == NULL)
    {
        fclose(f);
        return 0;
    }

    if (fread(conteudo, 1, (size_t)tam, f) != (size_t)tam)
    {
        fclose(f);
        free(conteudo);
        return 0;
    }
    fclose(f);

    n = snprintf(cabecalho, sizeof(cabecalho),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: %s\r\n"
                 "Content-Length: %ld\r\n"
                 "Connection: close\r\n"
                 "\r\n",
                 tipoConteudo(caminhoArquivo), tam);

    if (n > 0)
    {
        write(cliente, cabecalho, (size_t)n);
        write(cliente, conteudo, (size_t)tam);
    }

    free(conteudo);
    return 1;
}

/* Serve um arquivo de DIR_PUBLICO; se nao existir e a rota nao tiver extensao,
 * cai no index.html (fallback de SPA). */
static void servirEstatico(int cliente, const char *caminho)
{
    char arquivo[512];

    /* Seguranca: recusa qualquer tentativa de path traversal. */
    if (strstr(caminho, "..") != NULL)
    {
        responder(cliente, "400 Bad Request", "{\"erro\":\"caminho invalido\"}");
        return;
    }

    if (strcmp(caminho, "/") == 0)
    {
        snprintf(arquivo, sizeof(arquivo), "%s/index.html", DIR_PUBLICO);
    }
    else
    {
        snprintf(arquivo, sizeof(arquivo), "%s%s", DIR_PUBLICO, caminho);
    }

    if (enviarArquivo(cliente, arquivo))
    {
        return;
    }

    /* Rota de SPA (sem extensao) que nao e arquivo real -> index.html. */
    if (strrchr(caminho, '.') == NULL)
    {
        char index[512];
        snprintf(index, sizeof(index), "%s/index.html", DIR_PUBLICO);
        if (enviarArquivo(cliente, index))
        {
            return;
        }
    }

    responder(cliente, "404 Not Found", "{\"erro\":\"recurso nao encontrado\"}");
}

/* Verdadeiro se o caminho pertence a API (e nao ao front estatico). As rotas
 * de relatorio usam "/relatorios/" com barra para liberar "/relatorios" ao SPA. */
static int ehRotaApi(const char *caminho)
{
    return strcmp(caminho, "/health") == 0 ||
           comecaCom(caminho, "/pacientes") || comecaCom(caminho, "/medicos") ||
           comecaCom(caminho, "/alas") || comecaCom(caminho, "/leitos") ||
           comecaCom(caminho, "/triagens") || comecaCom(caminho, "/triagem/") ||
           comecaCom(caminho, "/agendamentos") || comecaCom(caminho, "/prontuarios") ||
           comecaCom(caminho, "/exames") || comecaCom(caminho, "/internacoes") ||
           comecaCom(caminho, "/prescricoes") ||
           comecaCom(caminho, "/relatorios/") || comecaCom(caminho, "/usuarios") ||
           comecaCom(caminho, "/auditoria") || comecaCom(caminho, "/me");
}

/* ----------------------------------------------------------------------- */
/* Roteamento                                                               */
/* ----------------------------------------------------------------------- */

static void rotear(int cliente, const char *metodo, char *caminho,
                   const char *requisicao)
{
    char *consulta = strchr(caminho, '?');
    char acao[32];
    Sessao s;
    char *papel = s.papel;
    int authPacienteId;
    int authMedicoId;
    int id;

    memset(&s, 0, sizeof(s));

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

    /* Front estatico (publico): qualquer GET que nao seja rota de API.
     * Cobre "/", "/login", "/r/...", "/assets/..." com fallback de SPA. */
    if (strcmp(metodo, "GET") == 0 && !ehRotaApi(caminho))
    {
        servirEstatico(cliente, caminho);
        return;
    }

    /* Todas as demais rotas exigem autenticacao (HTTP Basic). */
    if (autenticarRequest(requisicao, s.papel, sizeof(s.papel),
                          &s.paciente_id, &s.medico_id,
                          s.login, sizeof(s.login), &s.usuario_id) == 0)
    {
        responder(cliente, "401 Unauthorized",
                  "{\"erro\":\"credenciais invalidas\"}");
        return;
    }

    authPacienteId = s.paciente_id;
    authMedicoId = s.medico_id;

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
        rotaCriarPaciente(cliente, consulta, &s);
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/pacientes/buscar") == 0)
    {
        rotaBuscarPacientes(cliente, consulta);
    }
    else if (strcmp(metodo, "GET") == 0 && sscanf(caminho, "/pacientes/%d/%31s", &id, acao) == 2 &&
             strcmp(acao, "historico") == 0)
    {
        rotaHistoricoPaciente(cliente, id, &s);
    }
    else if (strcmp(metodo, "GET") == 0 && sscanf(caminho, "/pacientes/%d", &id) == 1)
    {
        rotaDetalhePaciente(cliente, id);
    }
    else if (strcmp(metodo, "DELETE") == 0 && sscanf(caminho, "/pacientes/%d", &id) == 1)
    {
        rotaDesativarPaciente(cliente, id, &s);
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
    else if (strcmp(metodo, "POST") == 0 && strcmp(caminho, "/triagem/pacientes") == 0)
    {
        rotaTriagemCadastrarPaciente(cliente, consulta, &s);
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
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/leitos/ocupacao") == 0)
    {
        responderLista(cliente, leito_repo_ocupacao_json,
                       "{\"erro\":\"falha ao calcular ocupacao\"}");
    }
    else if (strcmp(metodo, "POST") == 0 && strcmp(caminho, "/leitos") == 0)
    {
        rotaCriarLeito(cliente, consulta);
    }
    else if (strcmp(metodo, "POST") == 0 && sscanf(caminho, "/leitos/%d/%31s", &id, acao) == 2 &&
             strcmp(acao, "status") == 0)
    {
        rotaLeitoStatus(cliente, id, consulta, &s);
    }
    else if (strcmp(metodo, "DELETE") == 0 && sscanf(caminho, "/leitos/%d", &id) == 1)
    {
        responderRemocao(cliente, leito_repo_desativar(id) == 1, "{\"erro\":\"leito nao encontrado\"}");
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/triagens") == 0)
    {
        rotaListarTriagens(cliente, papel, authMedicoId);
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/triagens/contar") == 0)
    {
        responderContagem(cliente, triagem_repo_contar_ativos);
    }
    else if (strcmp(metodo, "POST") == 0 && strcmp(caminho, "/triagens") == 0)
    {
        rotaCriarTriagem(cliente, consulta, &s);
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
    else if (strcmp(metodo, "POST") == 0 && sscanf(caminho, "/agendamentos/%d/%31s", &id, acao) == 2 &&
             strcmp(acao, "reagendar") == 0)
    {
        char data[32];
        char horario[16];
        int ok;
        extrairParam(consulta, "data", data, sizeof(data));
        extrairParam(consulta, "horario", horario, sizeof(horario));
        ok = agendamento_repo_reagendar(id, data, horario) == 1;
        if (ok)
            auditar(&s, "REAGENDAR", "agendamento", id, data);
        responderRemocao(cliente, ok,
                         "{\"erro\":\"reagendamento invalido (status, grade ou conflito)\"}");
    }
    else if (strcmp(metodo, "DELETE") == 0 && sscanf(caminho, "/agendamentos/%d", &id) == 1)
    {
        char motivo[256];
        int ok;
        extrairParam(consulta, "motivo", motivo, sizeof(motivo));
        ok = agendamento_repo_cancelar(id, motivo) == 1;
        if (ok)
            auditar(&s, "CANCELAR", "agendamento", id, motivo);
        responderRemocao(cliente, ok,
                         "{\"erro\":\"agendamento nao encontrado ou motivo ausente\"}");
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/prontuarios") == 0)
    {
        rotaListarProntuarios(cliente, papel, authMedicoId);
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/prontuarios/contar") == 0)
    {
        responderContagem(cliente, prontuario_repo_contar_ativos);
    }
    else if (strcmp(metodo, "POST") == 0 && strcmp(caminho, "/prontuarios") == 0)
    {
        rotaCriarProntuario(cliente, consulta, &s);
    }
    /* Sem rota de remocao de prontuario: registro clinico nao e apagado. */
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/exames") == 0)
    {
        rotaListarExames(cliente, papel, authMedicoId);
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/exames/contar") == 0)
    {
        responderContagem(cliente, exame_repo_contar_ativos);
    }
    else if (strcmp(metodo, "POST") == 0 && strcmp(caminho, "/exames") == 0)
    {
        rotaCriarExame(cliente, consulta, &s);
    }
    else if (strcmp(metodo, "POST") == 0 && sscanf(caminho, "/exames/%d/%31s", &id, acao) == 2 &&
             strcmp(acao, "status") == 0)
    {
        char valor[32];
        int ok;
        extrairParam(consulta, "valor", valor, sizeof(valor));
        ok = exame_repo_atualizar_status(id, valor) == 1;
        if (ok)
            auditar(&s, "EXAME_STATUS", "exame", id, valor);
        responderRemocao(cliente, ok,
                         "{\"erro\":\"transicao de status invalida\"}");
    }
    else if (strcmp(metodo, "POST") == 0 && sscanf(caminho, "/exames/%d/%31s", &id, acao) == 2 &&
             strcmp(acao, "resultado") == 0)
    {
        char resultado[512];
        char critico[8];
        int ok;
        extrairParam(consulta, "resultado", resultado, sizeof(resultado));
        extrairParam(consulta, "critico", critico, sizeof(critico));
        ok = exame_repo_registrar_resultado(id, resultado, atoi(critico)) == 1;
        if (ok)
        {
            auditar(&s, "EXAME_RESULTADO", "exame", id,
                    atoi(critico) ? "CRITICO" : "");
        }
        responderRemocao(cliente, ok,
                         "{\"erro\":\"nao foi possivel registrar resultado (status/coleta)\"}");
    }
    else if (strcmp(metodo, "DELETE") == 0 && sscanf(caminho, "/exames/%d", &id) == 1)
    {
        char motivo[256];
        int ok;
        extrairParam(consulta, "motivo", motivo, sizeof(motivo));
        ok = exame_repo_cancelar(id, motivo) == 1;
        if (ok)
            auditar(&s, "CANCELAR", "exame", id, motivo);
        responderRemocao(cliente, ok,
                         "{\"erro\":\"exame nao cancelado (motivo ausente ou ja concluido)\"}");
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
        rotaCriarInternacao(cliente, consulta, &s);
    }
    else if (strcmp(metodo, "POST") == 0 && sscanf(caminho, "/internacoes/%d/%31s", &id, acao) == 2 &&
             strcmp(acao, "alta") == 0)
    {
        rotaInternacaoAlta(cliente, id, consulta, &s);
    }
    else if (strcmp(metodo, "POST") == 0 && sscanf(caminho, "/internacoes/%d/%31s", &id, acao) == 2 &&
             strcmp(acao, "transferir") == 0)
    {
        rotaInternacaoTransferir(cliente, id, consulta, &s);
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/prescricoes") == 0)
    {
        rotaListarPrescricoes(cliente, papel, authMedicoId);
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/prescricoes/contar") == 0)
    {
        responderContagem(cliente, prescricao_repo_contar_ativos);
    }
    else if (strcmp(metodo, "POST") == 0 && strcmp(caminho, "/prescricoes") == 0)
    {
        rotaCriarPrescricao(cliente, consulta, &s);
    }
    else if (strcmp(metodo, "DELETE") == 0 && sscanf(caminho, "/prescricoes/%d", &id) == 1)
    {
        char motivo[256];
        int ok;
        extrairParam(consulta, "motivo", motivo, sizeof(motivo));
        ok = prescricao_repo_desativar(id, motivo) == 1;
        if (ok)
            auditar(&s, "SUSPENDER", "prescricao", id, motivo);
        responderRemocao(cliente, ok,
                         "{\"erro\":\"prescricao nao encontrada ou motivo ausente\"}");
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
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/relatorios/triagens") == 0)
    {
        responderLista(cliente, triagem_repo_distribuicao_por_classificacao_json,
                       "{\"erro\":\"falha ao gerar relatorio de triagens\"}");
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/relatorios/internacoes") == 0)
    {
        responderLista(cliente, internacao_repo_distribuicao_por_status_json,
                       "{\"erro\":\"falha ao gerar relatorio de internacoes\"}");
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/relatorios/ocupacao") == 0)
    {
        responderLista(cliente, leito_repo_ocupacao_json,
                       "{\"erro\":\"falha ao calcular ocupacao\"}");
    }
    else if (strcmp(metodo, "POST") == 0 && strcmp(caminho, "/login") == 0)
    {
        rotaLogin(cliente, &s);
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
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/me/receitas") == 0)
    {
        rotaMeReceitas(cliente, authPacienteId);
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/me/agendamentos") == 0)
    {
        char *json = malloc(TAM_JSON);
        if (json != NULL &&
            agendamento_repo_listar_por_paciente_json(authPacienteId, json, TAM_JSON) == 1)
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
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/me/perfil") == 0)
    {
        char *json = malloc(TAM_JSON);
        if (json != NULL &&
            paciente_repo_detalhe_json(authPacienteId, json, TAM_JSON) == 1)
        {
            responder(cliente, "200 OK", json);
        }
        else
        {
            responder(cliente, "404 Not Found",
                      "{\"erro\":\"perfil nao encontrado\"}");
        }
        free(json);
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/me/agenda") == 0)
    {
        rotaMeAgenda(cliente, authMedicoId);
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/me/pacientes") == 0)
    {
        rotaMePacientes(cliente, authMedicoId);
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/me/resumo") == 0)
    {
        rotaMeResumo(cliente, authMedicoId);
    }
    else if (strcmp(metodo, "POST") == 0 && strcmp(caminho, "/usuarios") == 0)
    {
        rotaCriarUsuario(cliente, consulta, &s);
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
        int ok = usuario_repo_desativar(id) == 1;
        if (ok)
            auditar(&s, "DESATIVAR", "usuario", id, "");
        responderRemocao(cliente, ok, "{\"erro\":\"usuario nao encontrado\"}");
    }
    else if (strcmp(metodo, "POST") == 0 && sscanf(caminho, "/usuarios/%d/%31s", &id, acao) == 2 &&
             strcmp(acao, "reativar") == 0)
    {
        int ok = usuario_repo_reativar(id) == 1;
        if (ok)
            auditar(&s, "REATIVAR", "usuario", id, "");
        responderRemocao(cliente, ok, "{\"erro\":\"usuario nao encontrado ou ja ativo\"}");
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/auditoria") == 0)
    {
        responderLista(cliente, auditoria_listar_json, "{\"erro\":\"falha ao listar auditoria\"}");
    }
    else if (strcmp(metodo, "GET") == 0 && strcmp(caminho, "/auditoria/contar") == 0)
    {
        responderContagem(cliente, auditoria_contar);
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
