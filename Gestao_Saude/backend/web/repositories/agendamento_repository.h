#ifndef AGENDAMENTO_REPOSITORY_H
#define AGENDAMENTO_REPOSITORY_H

/*
 * Repository de agendamentos do backend web do SIGEH-DF.
 * A tabela usa 'status' (nao 'ativo'); cancelar = status 'CANCELADO'.
 * Usa database.h (SQLite) com prepared statements. Escrita: 1 = ok, 0 = falha.
 */

int agendamento_repo_criar(int paciente_id, int medico_id,
                           const char *data, const char *horario);
int agendamento_repo_listar_json(char *buffer, int tamanho);
int agendamento_repo_listar_por_medico_json(int medico_id, char *buffer, int tamanho);

/* Lista (JSON) os agendamentos de um paciente (todos os status). */
int agendamento_repo_listar_por_paciente_json(int paciente_id, char *buffer, int tamanho);

/* Cancela um agendamento; exige motivo (a consulta permanece no historico).
 * Retorna 1 se cancelou, 0 caso contrario. */
int agendamento_repo_cancelar(int id, const char *motivo);

/* Reagenda (data/horario) um agendamento ainda nao realizado (status AGENDADO),
 * respeitando grade e conflitos de medico/paciente. 1 = ok, 0 = falha. */
int agendamento_repo_reagendar(int id, const char *data, const char *horario);

/* 1 se o paciente ja tem atendimento nao-cancelado nesse data/horario; 0 se
 * livre; -1 em erro. */
int agendamento_repo_paciente_ocupado(int paciente_id, const char *data,
                                      const char *horario);
int agendamento_repo_contar_ativos(void);

/* Conta agendamentos nao-cancelados do medico. Retorna o total, ou -1 em erro. */
int agendamento_repo_contar_por_medico(int medico_id);

/* 1 se o medico tem agendamento nao-cancelado na data/horario; 0 se livre;
 * -1 em erro de banco. */
int agendamento_repo_medico_ocupado(int medico_id, const char *data,
                                    const char *horario);

/* 1 se o horario "HH:MM" cai num slot valido da agenda: dentro do expediente
 * e alinhado a grade de slots. 0 caso contrario. */
int agendamento_repo_horario_valido(const char *horario);

/* Conta agendamentos nao-cancelados com data no intervalo [inicio, fim]
 * (datas "YYYY-MM-DD", limites inclusivos). Retorna o total, ou -1 em erro. */
int agendamento_repo_contar_por_periodo(const char *inicio, const char *fim);

/* Escreve (JSON) a distribuicao de agendamentos nao-cancelados por dia no
 * intervalo [inicio, fim], no formato [{"data":"2026-06-14","total":2},...].
 * Retorna 1 em sucesso, 0 em erro. */
int agendamento_repo_distribuicao_por_dia_json(const char *inicio,
                                               const char *fim,
                                               char *buffer, int tamanho);

#endif
