#ifndef TRIAGEM_SERVICE_H
#define TRIAGEM_SERVICE_H

/*
 * Service de triagem do backend web do SIGEH-DF.
 * Camada de decisao (somente-leitura): orquestra os repositories e produz
 * a recomendacao consolidada da triagem. Nao escreve no banco.
 */

/* Avalia a triagem ativa mais recente do paciente e escreve em 'buffer' um
 * JSON com pacienteId, classificacao, prioridade e especialidadeProvavel.
 * Retorna 1 em sucesso; 0 se o paciente nao tem triagem ativa ou em erro. */
int triagem_service_avaliar_json(int paciente_id, char *buffer, int tamanho);

#endif
