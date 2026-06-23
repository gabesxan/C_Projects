#include "repo_json.h"

#include <stdio.h>
#include <string.h>

int repo_json_escapar(char *destino, int tamanho, const char *origem)
{
    int pos = 0;
    int i;

    if (tamanho < 3)
    {
        return 0;
    }

    destino[pos++] = '"';

    for (i = 0; origem != NULL && origem[i] != '\0'; i++)
    {
        char c = origem[i];

        if (c == '"' || c == '\\')
        {
            if (pos + 2 > tamanho - 2)
            {
                return 0;
            }
            destino[pos++] = '\\';
            destino[pos++] = c;
        }
        else
        {
            if (pos + 1 > tamanho - 2)
            {
                return 0;
            }
            destino[pos++] = c;
        }
    }

    destino[pos++] = '"';
    destino[pos] = '\0';
    return 1;
}

int repo_json_anexar(char *buffer, int tamanho, int *usado, const char *texto)
{
    int restante = tamanho - *usado;
    int escrito = snprintf(buffer + *usado, (size_t)restante, "%s", texto);

    if (escrito < 0 || escrito >= restante)
    {
        return 0;
    }

    *usado += escrito;
    return 1;
}
