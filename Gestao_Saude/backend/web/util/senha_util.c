#include "senha_util.h"

#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define SENHA_SALT_BYTES 8

int senha_gerar_salt(char *salt, int tam)
{
    unsigned char bytes[SENHA_SALT_BYTES];
    int i;

    if (salt == NULL || tam < (int)(sizeof(bytes) * 2 + 1))
    {
        return 0;
    }

    if (RAND_bytes(bytes, sizeof(bytes)) != 1)
    {
        return 0;
    }

    for (i = 0; i < (int)sizeof(bytes); i++)
    {
        sprintf(salt + i * 2, "%02x", bytes[i]);
    }

    salt[sizeof(bytes) * 2] = '\0';
    return 1;
}

int senha_hash(const char *salt, const char *senha, char *hash, int tam)
{
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int dlen = 0;
    char entrada[512];
    EVP_MD_CTX *ctx;
    unsigned int i;

    if (salt == NULL || senha == NULL || hash == NULL || tam < 65)
    {
        return 0;
    }

    if (snprintf(entrada, sizeof(entrada), "%s%s", salt, senha) >= (int)sizeof(entrada))
    {
        return 0;
    }

    ctx = EVP_MD_CTX_new();

    if (ctx == NULL)
    {
        return 0;
    }

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) != 1 ||
        EVP_DigestUpdate(ctx, entrada, strlen(entrada)) != 1 ||
        EVP_DigestFinal_ex(ctx, digest, &dlen) != 1)
    {
        EVP_MD_CTX_free(ctx);
        return 0;
    }

    EVP_MD_CTX_free(ctx);

    for (i = 0; i < dlen; i++)
    {
        sprintf(hash + i * 2, "%02x", digest[i]);
    }

    hash[dlen * 2] = '\0';
    return 1;
}
