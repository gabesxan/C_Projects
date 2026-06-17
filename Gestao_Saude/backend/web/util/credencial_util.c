#include "credencial_util.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

/* Mapeia a segunda metade de uma sequencia UTF-8 iniciada por 0xC3 (acentos
 * latinos comuns no portugues) para a letra base minuscula. 0 = nao mapeado. */
static char baseDeAcento(unsigned char b)
{
    switch (b)
    {
        case 0xA1: case 0xA0: case 0xA2: case 0xA3: case 0xA4: /* a com acento */
        case 0x81: case 0x80: case 0x82: case 0x83: case 0x84: /* A com acento */
            return 'a';
        case 0xA9: case 0xA8: case 0xAA: case 0xAB:
        case 0x89: case 0x88: case 0x8A: case 0x8B:
            return 'e';
        case 0xAD: case 0xAC: case 0xAE: case 0xAF:
        case 0x8D: case 0x8C: case 0x8E: case 0x8F:
            return 'i';
        case 0xB3: case 0xB2: case 0xB4: case 0xB5: case 0xB6:
        case 0x93: case 0x92: case 0x94: case 0x95: case 0x96:
            return 'o';
        case 0xBA: case 0xB9: case 0xBB: case 0xBC:
        case 0x9A: case 0x99: case 0x9B: case 0x9C:
            return 'u';
        case 0xA7: case 0x87: /* c cedilha */
            return 'c';
        default:
            return 0;
    }
}

int credencial_normalizar(const char *nome, char *out, int tam)
{
    int j = 0;
    int i;

    if (nome == NULL || out == NULL || tam <= 0)
    {
        return 0;
    }

    for (i = 0; nome[i] != '\0'; i++)
    {
        unsigned char c = (unsigned char)nome[i];
        char destino = 0;

        if (c == 0xC3 && nome[i + 1] != '\0')
        {
            destino = baseDeAcento((unsigned char)nome[i + 1]);
            i++; /* consome o segundo byte da sequencia UTF-8 */
        }
        else if (isalpha(c))
        {
            destino = (char)tolower(c);
        }
        else if (isdigit(c))
        {
            destino = (char)c;
        }

        if (destino != 0)
        {
            if (j >= tam - 1)
            {
                return 0;
            }
            out[j++] = destino;
        }
    }

    out[j] = '\0';
    return 1;
}

int credencial_gerar_senha(const char *nome, const char *nascimento,
                           char *out, int tam)
{
    static int semeado = 0;
    char normalizado[128];
    char prefixo[4];
    char digitos[16];
    int d = 0;
    int len;
    int i;
    int escrito;

    if (out == NULL || tam < 20 || nascimento == NULL)
    {
        return 0;
    }

    if (credencial_normalizar(nome, normalizado, sizeof(normalizado)) == 0 ||
        normalizado[0] == '\0')
    {
        return 0;
    }

    /* 3 primeiras letras do nome (completa com 'x' se o nome for curto). */
    len = (int)strlen(normalizado);
    for (i = 0; i < 3; i++)
    {
        prefixo[i] = i < len ? normalizado[i] : 'x';
    }
    prefixo[3] = '\0';

    /* Data de nascimento sem separadores (apenas digitos). */
    for (i = 0; nascimento[i] != '\0' && d < (int)sizeof(digitos) - 1; i++)
    {
        if (isdigit((unsigned char)nascimento[i]))
        {
            digitos[d++] = nascimento[i];
        }
    }
    digitos[d] = '\0';

    if (!semeado)
    {
        srand((unsigned int)time(NULL));
        semeado = 1;
    }

    /* 4 digitos aleatorios (0000-9999). */
    escrito = snprintf(out, (size_t)tam, "%s%s%04d",
                       prefixo, digitos, rand() % 10000);

    return (escrito > 0 && escrito < tam) ? 1 : 0;
}
