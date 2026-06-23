#include "farmacia_service.h"
#include "medicamento_repository.h"
#include "estoque_repository.h"
#include "paciente_repository.h"
#include "financeiro_repository.h"

#include <stdio.h>

int farmacia_service_dispensar(int medicamento_id, int paciente_id,
                               int quantidade, const char *motivo,
                               int usuario_id, const char *usuario_login,
                               char *buffer, int tamanho)
{
    int preco;
    int saldo;
    int valor;
    int cobrancaGerada = 0;
    char detalhe[256];

    if (buffer == NULL || tamanho <= 0)
    {
        return 0;
    }

    if (medicamento_id <= 0 || paciente_id <= 0 || quantidade <= 0)
    {
        snprintf(buffer, (size_t)tamanho, "{\"erro\":\"dados invalidos\"}");
        return 0;
    }

    /* Medicamento ativo (e seu preco unitario do catalogo). */
    preco = medicamento_preco_centavos(medicamento_id);
    if (preco < 0)
    {
        snprintf(buffer, (size_t)tamanho, "{\"erro\":\"medicamento inexistente\"}");
        return 0;
    }

    /* Paciente ativo (regiao >= 0 indica paciente existente e ativo). */
    if (paciente_repo_regiao(paciente_id) < 0)
    {
        snprintf(buffer, (size_t)tamanho, "{\"erro\":\"paciente inexistente\"}");
        return 0;
    }

    saldo = estoque_saldo(medicamento_id);
    if (saldo < quantidade)
    {
        snprintf(buffer, (size_t)tamanho,
                 "{\"erro\":\"estoque insuficiente\",\"saldo\":%d}",
                 saldo < 0 ? 0 : saldo);
        return 0;
    }

    /* Movimentacao registra o paciente no motivo (sem coluna propria por ora). */
    if (motivo != NULL && motivo[0] != '\0')
    {
        snprintf(detalhe, sizeof(detalhe), "dispensacao paciente #%d: %s",
                 paciente_id, motivo);
    }
    else
    {
        snprintf(detalhe, sizeof(detalhe), "dispensacao paciente #%d", paciente_id);
    }

    if (estoque_baixar(medicamento_id, quantidade, "SAIDA", detalhe,
                       usuario_id, usuario_login) == 0)
    {
        snprintf(buffer, (size_t)tamanho, "{\"erro\":\"falha ao baixar estoque\"}");
        return 0;
    }

    /* Vinculo com o financeiro: preco 0 dispensa sem cobrar. */
    valor = quantidade * preco;
    if (valor > 0)
    {
        cobrancaGerada = cobranca_criar(paciente_id, 0, "PARTICULAR", "farmacia",
                                        "Dispensacao de medicamento", valor,
                                        "", "", "") == 1;
    }

    snprintf(buffer, (size_t)tamanho,
             "{\"dispensado\":true,\"medicamentoId\":%d,\"pacienteId\":%d,"
             "\"quantidade\":%d,\"valorCentavos\":%d,\"cobrancaGerada\":%s,"
             "\"saldo\":%d}",
             medicamento_id, paciente_id, quantidade, valor,
             cobrancaGerada ? "true" : "false",
             estoque_saldo(medicamento_id));
    return 1;
}
