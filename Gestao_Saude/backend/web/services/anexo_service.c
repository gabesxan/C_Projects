#include "anexo_service.h"
#include "anexo_repository.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* Diretorios (relativos ao cwd do servidor, backend/web). O caminho guardado no
 * banco e relativo a DATA_DIR (ex.: "anexos/12_laudo.pdf"); o arquivo fisico
 * fica em DATA_DIR/anexos/. */
#define DATA_DIR ".."  "/data"
#define ANEXO_SUBDIR "anexos"

static const char ALLOWED_MIME[][24] = {
    "application/pdf",
    "image/png",
    "image/jpeg",
};

static int mime_permitido(const char *mime)
{
    size_t i;
    size_t n = sizeof(ALLOWED_MIME) / sizeof(ALLOWED_MIME[0]);
    for (i = 0; i < n; i++)
    {
        if (strcmp(mime, ALLOWED_MIME[i]) == 0) return 1;
    }
    return 0;
}

/* Mantem apenas caracteres seguros no nome de arquivo (evita path traversal e
 * caracteres problematicos); os demais viram '_'. Nunca devolve vazio. */
static void sanitizar_nome(const char *nome, char *saida, size_t cap)
{
    size_t i = 0;
    size_t j = 0;

    for (; nome[i] != '\0' && j + 1 < cap; i++)
    {
        char c = nome[i];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') || c == '.' || c == '_' || c == '-')
        {
            saida[j++] = c;
        }
        else
        {
            saida[j++] = '_';
        }
    }
    if (j == 0) saida[j++] = 'a';
    saida[j] = '\0';
}

static int b64_valor(char c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

/* Decodifica base64 (ignora espacos e quebras de linha) em 'saida'. Retorna o
 * numero de bytes decodificados, ou -1 em conteudo invalido ou estouro de
 * 'cap_saida'. */
static long base64_decode(const char *entrada, unsigned char *saida, long cap_saida)
{
    int quad[4];
    int qn = 0;
    int padding = 0;
    long out = 0;
    const char *p;

    for (p = entrada; *p != '\0'; p++)
    {
        if (*p == '\r' || *p == '\n' || *p == ' ' || *p == '\t') continue;

        if (*p == '=')
        {
            quad[qn++] = 0;
            padding++;
        }
        else
        {
            int v = b64_valor(*p);
            if (v < 0 || padding > 0) return -1; /* simbolo invalido ou apos '=' */
            quad[qn++] = v;
        }

        if (qn == 4)
        {
            unsigned int triple = ((unsigned int)quad[0] << 18) |
                                  ((unsigned int)quad[1] << 12) |
                                  ((unsigned int)quad[2] << 6) |
                                  (unsigned int)quad[3];
            int bytes = 3 - padding;
            int i;
            for (i = 0; i < bytes; i++)
            {
                if (out >= cap_saida) return -1;
                saida[out++] = (unsigned char)((triple >> (16 - 8 * i)) & 0xFF);
            }
            qn = 0;
            if (padding > 0) break; /* padding so aparece no fim */
        }
    }

    if (qn != 0) return -1; /* comprimento nao multiplo de 4 */
    return out;
}

void anexo_service_apagar_arquivo(const char *caminho)
{
    char fisico[600];

    if (caminho == NULL || caminho[0] == '\0' || strstr(caminho, "..") != NULL)
    {
        return;
    }
    if (snprintf(fisico, sizeof(fisico), "%s/%s", DATA_DIR, caminho) < (int)sizeof(fisico))
    {
        remove(fisico);
    }
}

int anexo_service_criar(const char *entidade, int entidade_id, const char *nome,
                        const char *mime, const char *conteudo_b64,
                        int autor_id, const char *autor_login,
                        char *buffer, int tamanho, int *novo_id)
{
    unsigned char *dados = NULL;
    long bytes;
    char slug[160];
    char rel[256];
    char fisico[600];
    int id = 0;
    FILE *f;

    if (novo_id != NULL) *novo_id = 0;
    if (buffer == NULL || tamanho <= 0) return 0;

    if (entidade == NULL || entidade[0] == '\0' || entidade_id <= 0 ||
        nome == NULL || nome[0] == '\0' ||
        mime == NULL || mime[0] == '\0' ||
        conteudo_b64 == NULL || conteudo_b64[0] == '\0')
    {
        snprintf(buffer, (size_t)tamanho, "{\"erro\":\"dados invalidos\"}");
        return 0;
    }

    if (!mime_permitido(mime))
    {
        snprintf(buffer, (size_t)tamanho,
                 "{\"erro\":\"tipo nao permitido (use pdf, png ou jpeg)\"}");
        return 0;
    }

    dados = malloc(ANEXO_MAX_BYTES);
    if (dados == NULL)
    {
        snprintf(buffer, (size_t)tamanho, "{\"erro\":\"falha interna\"}");
        return 0;
    }

    bytes = base64_decode(conteudo_b64, dados, ANEXO_MAX_BYTES);
    if (bytes < 0)
    {
        free(dados);
        snprintf(buffer, (size_t)tamanho,
                 "{\"erro\":\"conteudo invalido ou acima de 5 MB\"}");
        return 0;
    }
    if (bytes == 0)
    {
        free(dados);
        snprintf(buffer, (size_t)tamanho, "{\"erro\":\"arquivo vazio\"}");
        return 0;
    }

    sanitizar_nome(nome, slug, sizeof(slug));

    /* Insere o metadado com caminho provisorio para obter o id, que compoe o
     * nome do arquivo (garante unicidade). O caminho e finalizado em seguida. */
    if (anexo_criar(entidade, entidade_id, nome, mime, bytes, "pendente",
                    autor_id, autor_login, &id) == 0 || id <= 0)
    {
        free(dados);
        snprintf(buffer, (size_t)tamanho, "{\"erro\":\"falha ao registrar anexo\"}");
        return 0;
    }

    snprintf(rel, sizeof(rel), "%s/%d_%s", ANEXO_SUBDIR, id, slug);
    snprintf(fisico, sizeof(fisico), "%s/%s", DATA_DIR, rel);

    /* Garante o diretorio de anexos (idempotente). */
    mkdir(DATA_DIR "/" ANEXO_SUBDIR, 0755);

    f = fopen(fisico, "wb");
    if (f == NULL || fwrite(dados, 1, (size_t)bytes, f) != (size_t)bytes)
    {
        if (f != NULL) fclose(f);
        free(dados);
        anexo_remover(id, NULL, 0); /* desfaz o metadado */
        snprintf(buffer, (size_t)tamanho, "{\"erro\":\"falha ao gravar arquivo\"}");
        return 0;
    }
    fclose(f);
    free(dados);

    if (anexo_definir_caminho(id, rel) == 0)
    {
        remove(fisico);
        anexo_remover(id, NULL, 0);
        snprintf(buffer, (size_t)tamanho, "{\"erro\":\"falha ao finalizar anexo\"}");
        return 0;
    }

    if (novo_id != NULL) *novo_id = id;
    snprintf(buffer, (size_t)tamanho,
             "{\"id\":%d,\"tamanho\":%ld}", id, bytes);
    return 1;
}
