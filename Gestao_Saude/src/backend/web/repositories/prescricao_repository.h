#ifndef PRESCRICAO_REPOSITORY_H
#define PRESCRICAO_REPOSITORY_H

/*
 * Repository de prescricoes (receitas/medicacao) do backend web do SIGEH-DF.
 * Usa database.h (SQLite) com prepared statements. Escrita: 1 = ok, 0 = falha.
 */

/* Cria a prescricao. Verifica alergia do paciente ao medicamento antes de
 * confirmar; via/duracao podem ser "". 1 = ok, 0 = falha. */
int prescricao_repo_criar(int paciente_id, int medico_id,
                          const char *medicamento, const char *dosagem,
                          const char *frequencia, const char *via,
                          const char *duracao, const char *observacoes);
int prescricao_repo_listar_json(char *buffer, int tamanho);
int prescricao_repo_listar_por_paciente_json(int paciente_id, char *buffer, int tamanho);
int prescricao_repo_listar_por_medico_json(int medico_id, char *buffer, int tamanho);

/* Suspende a prescricao; exige justificativa. 1 = ok, 0 = falha. */
int prescricao_repo_desativar(int id, const char *motivo);
int prescricao_repo_contar_ativos(void);

#endif
