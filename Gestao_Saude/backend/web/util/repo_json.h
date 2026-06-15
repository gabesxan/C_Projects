#ifndef REPO_JSON_H
#define REPO_JSON_H

/*
 * Utilitarios de montagem de JSON compartilhados pelos repositories.
 * Mantem o controle de espaco e o escape de strings num so lugar.
 */

/* Escreve 'origem' em 'destino' como literal JSON entre aspas, escapando
 * aspas e barras. Retorna 1 se coube, 0 caso contrario. */
int repo_json_escapar(char *destino, int tamanho, const char *origem);

/* Concatena 'texto' ao final de 'buffer', controlando o espaco usado.
 * Retorna 1 se coube, 0 caso contrario. */
int repo_json_anexar(char *buffer, int tamanho, int *usado, const char *texto);

#endif
