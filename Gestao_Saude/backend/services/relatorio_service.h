#ifndef RELATORIO_SERVICE_H
#define RELATORIO_SERVICE_H

/*
 * Service de relatorios do backend web do SIGEH-DF.
 * Camada somente-leitura: agrega indicadores a partir dos repositories.
 */

/* Escreve em 'buffer' um JSON com os indicadores gerais do sistema.
 * Retorna 1 em sucesso, 0 em erro. */
int relatorio_service_indicadores_json(char *buffer, int tamanho);

#endif
