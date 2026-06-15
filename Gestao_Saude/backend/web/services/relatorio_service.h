#ifndef RELATORIO_SERVICE_H
#define RELATORIO_SERVICE_H

/*
 * Service de relatorios do backend web do SIGEH-DF.
 * Camada somente-leitura: agrega indicadores a partir dos repositories.
 */

/* Escreve em 'buffer' um JSON com os indicadores gerais do sistema.
 * Retorna 1 em sucesso, 0 em erro. */
int relatorio_service_indicadores_json(char *buffer, int tamanho);

/* Escreve em 'buffer' um JSON com a distribuicao de pacientes por regiao e
 * de medicos por especialidade. Retorna 1 em sucesso, 0 em erro. */
int relatorio_service_distribuicao_json(char *buffer, int tamanho);

/* Escreve em 'buffer' um JSON com o total e a distribuicao por dia dos
 * agendamentos nao-cancelados no intervalo [inicio, fim] ("YYYY-MM-DD").
 * Retorna 1 em sucesso, 0 em erro (inclusive datas vazias). */
int relatorio_service_agendamentos_periodo_json(const char *inicio,
                                                const char *fim,
                                                char *buffer, int tamanho);

#endif
