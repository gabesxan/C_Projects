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
int agendamento_repo_cancelar(int id);
int agendamento_repo_contar_ativos(void);

/* 1 se o medico tem agendamento nao-cancelado na data/horario; 0 se livre;
 * -1 em erro de banco. */
int agendamento_repo_medico_ocupado(int medico_id, const char *data,
                                    const char *horario);

/* 1 se o horario "HH:MM" cai num slot valido da agenda: dentro do expediente
 * e alinhado a grade de slots. 0 caso contrario. */
int agendamento_repo_horario_valido(const char *horario);

#endif
