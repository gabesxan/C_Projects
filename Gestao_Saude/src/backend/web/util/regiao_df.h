#ifndef REGIAO_DF_H
#define REGIAO_DF_H

/*
 * Regioes Administrativas (RAs) do Distrito Federal com coordenadas reais.
 * O campo regiao_administrativa de pacientes/medicos usa estes ids.
 * Usado pelo agendamento inteligente para achar a RA mais proxima quando nao
 * ha medico disponivel na RA de origem.
 */

/* Nome da RA pelo id (1..N). "" se id desconhecido. */
const char *regiao_df_nome(int id);

/* Distancia em km (haversine) entre duas RAs. Retorna um valor grande
 * (>= 1e6) se algum id for desconhecido. */
double regiao_df_distancia(int a, int b);

/* Preenche 'ids_out' com os ids de TODAS as RAs conhecidas ordenadas pela
 * distancia crescente a partir de 'origem' (a propria origem vem primeiro).
 * Retorna a quantidade escrita (limitada a 'max'). */
int regiao_df_ordenar_por_distancia(int origem, int *ids_out, int max);

#endif
