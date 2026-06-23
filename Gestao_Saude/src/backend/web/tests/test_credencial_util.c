#include "credencial_util.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main(void)
{
    char out[64];

    /* Normalizacao: minusculo, sem acentos, sem espacos. */
    assert(credencial_normalizar("João Silva", out, sizeof(out)) == 1);
    assert(strcmp(out, "joaosilva") == 0);

    assert(credencial_normalizar("Ção Êxé Ãñ", out, sizeof(out)) == 1);
    /* 'ñ' nao e mapeado (nao e acento portugues padrao) e e descartado. */
    assert(strcmp(out, "caoexea") == 0);

    /* Senha: 3 letras do nome + nascimento sem separadores + 4 digitos. */
    assert(credencial_gerar_senha("João Silva", "1980-05-12", out, sizeof(out)) == 1);
    assert(strncmp(out, "joa19800512", 11) == 0);
    assert(strlen(out) == 15); /* joa + 8 digitos da data + 4 aleatorios */
    /* Sem espacos e tudo minusculo. */
    {
        size_t i;
        for (i = 0; i < strlen(out); i++)
        {
            assert(out[i] != ' ');
            assert(!isupper((unsigned char)out[i]));
        }
        /* Os 4 ultimos sao digitos. */
        for (i = strlen(out) - 4; i < strlen(out); i++)
        {
            assert(isdigit((unsigned char)out[i]));
        }
    }

    printf("test_credencial_util: OK\n");
    return 0;
}
