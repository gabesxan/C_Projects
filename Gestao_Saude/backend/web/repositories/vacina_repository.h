#ifndef VACINA_REPOSITORY_H
#define VACINA_REPOSITORY_H

/*
 * Repository do catalogo de vacinas (SIGEH-DF).
 * Sub-etapa 6a: somente catalogo e esquema basico de doses. Aplicacoes e
 * carteira do paciente ficam nas proximas sub-etapas.
 */

int vacina_criar(const char *nome, const char *fabricante,
                 const char *doencas_alvo, int doses_previstas,
                 int intervalo_dias, int reforco_dias);

int vacina_listar_json(char *buffer, int tamanho);

int vacina_desativar(int id);

int vacina_contar_ativas(void);

int vacina_ativa(int id);

#endif
