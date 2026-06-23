#include "consentimento_service.h"
#include "consentimento_repository.h"

#include <stdio.h>
#include <string.h>

int consentimento_service_criar(int paciente_id, const char *finalidade,
                                const char *versao_termo,
                                char *buffer, int tamanho, int *novo_id)
{
    int id = 0;

    if (novo_id != NULL) *novo_id = 0;
    if (buffer == NULL || tamanho <= 0) return 0;

    if (paciente_id <= 0 ||
        finalidade == NULL || finalidade[0] == '\0' ||
        versao_termo == NULL || versao_termo[0] == '\0')
    {
        snprintf(buffer, (size_t)tamanho, "{\"erro\":\"dados invalidos\"}");
        return 0;
    }

    if (consentimento_criar(paciente_id, finalidade, versao_termo, &id) == 0)
    {
        snprintf(buffer, (size_t)tamanho,
                 "{\"erro\":\"falha ao registrar consentimento\"}");
        return 0;
    }

    if (novo_id != NULL) *novo_id = id;
    snprintf(buffer, (size_t)tamanho,
             "{\"id\":%d,\"status\":\"CONCEDIDO\"}", id);
    return 1;
}

int consentimento_service_revogar(int id, const char *motivo,
                                  char *buffer, int tamanho,
                                  int *nao_encontrado)
{
    char status[32];
    int paciente_id = 0;

    if (nao_encontrado != NULL) *nao_encontrado = 0;
    if (buffer == NULL || tamanho <= 0) return 0;

    if (id <= 0 || motivo == NULL || motivo[0] == '\0')
    {
        snprintf(buffer, (size_t)tamanho,
                 "{\"erro\":\"motivo obrigatorio para revogar consentimento\"}");
        return 0;
    }

    if (consentimento_buscar(id, &paciente_id, status, sizeof(status)) == 0)
    {
        if (nao_encontrado != NULL) *nao_encontrado = 1;
        snprintf(buffer, (size_t)tamanho,
                 "{\"erro\":\"consentimento nao encontrado\"}");
        return 0;
    }

    if (strcmp(status, "CONCEDIDO") != 0)
    {
        snprintf(buffer, (size_t)tamanho,
                 "{\"erro\":\"consentimento ja revogado\"}");
        return 0;
    }

    if (consentimento_revogar(id, motivo) == 0)
    {
        snprintf(buffer, (size_t)tamanho,
                 "{\"erro\":\"falha ao revogar consentimento\"}");
        return 0;
    }

    snprintf(buffer, (size_t)tamanho, "{\"id\":%d,\"status\":\"REVOGADO\"}", id);
    return 1;
}
