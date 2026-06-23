#include "lgpd_service.h"
#include "auditoria_repository.h"

#include <stdio.h>
#include <string.h>

int lgpd_service_relatorio_acessos_json(int paciente_id, char *buffer,
                                        int tamanho)
{
    int n;
    int usado;

    if (buffer == NULL || tamanho <= 0)
    {
        return 0;
    }

    if (paciente_id <= 0)
    {
        snprintf(buffer, (size_t)tamanho, "{\"erro\":\"paciente invalido\"}");
        return 0;
    }

    /* Envelope com o filtro de paciente; o array de acessos e escrito logo
     * apos o prefixo para reaproveitar um unico buffer (o relatorio pode ser
     * grande, ate 500 registros). */
    n = snprintf(buffer, (size_t)tamanho, "{\"pacienteId\":%d,\"acessos\":",
                 paciente_id);
    if (n < 0 || n >= tamanho)
    {
        snprintf(buffer, (size_t)tamanho,
                 "{\"erro\":\"falha ao montar relatorio de acessos\"}");
        return 0;
    }

    if (auditoria_acessos_paciente_json(paciente_id, buffer + n,
                                        tamanho - n) == 0)
    {
        snprintf(buffer, (size_t)tamanho,
                 "{\"erro\":\"falha ao montar relatorio de acessos\"}");
        return 0;
    }

    usado = (int)strlen(buffer);
    if (usado + 2 > tamanho)
    {
        snprintf(buffer, (size_t)tamanho,
                 "{\"erro\":\"falha ao montar relatorio de acessos\"}");
        return 0;
    }

    buffer[usado] = '}';
    buffer[usado + 1] = '\0';
    return 1;
}
