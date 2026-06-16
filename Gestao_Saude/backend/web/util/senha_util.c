#include "senha_util.h"

#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define SENHA_SALT_BYTES 8

/* Numero de iteracoes do PBKDF2. Quanto maior, mais caro um ataque de forca
 * bruta. 210000 segue a recomendacao do OWASP para PBKDF2-HMAC-SHA256. */
#define SENHA_PBKDF2_ITERACOES 210000

/* Tamanho da chave derivada em bytes (32 -> 64 caracteres hex). */
#define SENHA_HASH_BYTES 32

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
    unsigned char derivada[SENHA_HASH_BYTES];
    int i;

    if (salt == NULL || senha == NULL || hash == NULL ||
        tam < SENHA_HASH_BYTES * 2 + 1)
    {
        return 0;
    }

    /* PBKDF2-HMAC-SHA256 sobre a senha, usando o salt (em hex) como sal.
     * Derivacao lenta e proposital: encarece ataques de dicionario. */
    if (PKCS5_PBKDF2_HMAC(senha, (int)strlen(senha),
                          (const unsigned char *)salt, (int)strlen(salt),
                          SENHA_PBKDF2_ITERACOES, EVP_sha256(),
                          SENHA_HASH_BYTES, derivada) != 1)
    {
        return 0;
    }

    for (i = 0; i < SENHA_HASH_BYTES; i++)
    {
        sprintf(hash + i * 2, "%02x", derivada[i]);
    }

    hash[SENHA_HASH_BYTES * 2] = '\0';
    return 1;
}
