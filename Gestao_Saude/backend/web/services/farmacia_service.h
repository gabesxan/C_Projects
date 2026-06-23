#ifndef FARMACIA_SERVICE_H
#define FARMACIA_SERVICE_H

/*
 * Service da farmacia do SIGEH-DF. Orquestra a dispensacao de medicamento a um
 * paciente, que cruza tres entidades: debita o estoque (FIFO, transacao no
 * estoque_repository) e gera uma cobranca PARTICULAR no financeiro
 * (quantidade x preco do catalogo). Camada de regra acima dos repositories.
 */

/* Dispensa 'quantidade' do medicamento ao paciente: valida medicamento/paciente
 * ativos e saldo suficiente, baixa o estoque e, se o preco unitario > 0, lanca
 * a cobranca PARTICULAR. Escreve o resultado (JSON) em 'buffer'. Retorna 1 se
 * dispensou; 0 em parametro invalido, paciente/medicamento inexistente ou saldo
 * insuficiente (nesse caso o estoque nao e alterado). */
int farmacia_service_dispensar(int medicamento_id, int paciente_id,
                               int quantidade, const char *motivo,
                               int usuario_id, const char *usuario_login,
                               char *buffer, int tamanho);

#endif
