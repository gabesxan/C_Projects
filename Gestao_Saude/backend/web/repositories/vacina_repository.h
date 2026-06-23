#ifndef VACINA_REPOSITORY_H
#define VACINA_REPOSITORY_H

/*
 * Repository de vacinacao (SIGEH-DF): catalogo de vacinas e aplicacoes com
 * rastreabilidade por paciente, lote e estoque.
 */

int vacina_criar(const char *nome, const char *fabricante,
                 const char *doencas_alvo, int doses_previstas,
                 int intervalo_dias, int reforco_dias, int medicamento_id);

int vacina_listar_json(char *buffer, int tamanho);

int vacina_desativar(int id);

int vacina_contar_ativas(void);

int vacina_ativa(int id);

int vacina_aplicar(int paciente_id, int vacina_id, int dose_numero,
                   const char *lote, const char *validade,
                   int aplicador_usuario_id, const char *aplicador_login,
                   const char *observacao, int *aplicacao_id);

int vacina_aplicacoes_listar_json(char *buffer, int tamanho);

int vacina_aplicacoes_listar_por_paciente_json(int paciente_id,
                                               char *buffer, int tamanho);

#endif
