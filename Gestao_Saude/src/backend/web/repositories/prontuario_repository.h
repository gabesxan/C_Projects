#ifndef PRONTUARIO_REPOSITORY_H
#define PRONTUARIO_REPOSITORY_H

/*
 * Repository de prontuarios do backend web do SIGEH-DF.
 * Usa database.h (SQLite) com prepared statements. Escrita: 1 = ok, 0 = falha.
 */

int prontuario_repo_criar(int paciente_id, int medico_id, const char *data,
                          const char *observacoes, const char *diagnostico,
                          const char *conduta, int alerta_importante);

/* Retifica um prontuario vigente: cria uma NOVA versao (vigente) e marca a
 * anterior como superada, preservando-a. Exige conduta e justificativa.
 * 1 = ok, 0 = falha. */
int prontuario_repo_retificar(int id, const char *data, const char *observacoes,
                              const char *diagnostico, const char *conduta,
                              int alerta_importante, const char *justificativa);

/* Lista (JSON) apenas as versoes vigentes. */
int prontuario_repo_listar_json(char *buffer, int tamanho);

/* Lista (JSON) TODAS as versoes do paciente (trilha completa, cronologica). */
int prontuario_repo_listar_por_paciente_json(int paciente_id, char *buffer, int tamanho);
int prontuario_repo_listar_por_medico_json(int medico_id, char *buffer, int tamanho);
int prontuario_repo_contar_ativos(void);

/* Conta prontuarios ativos assinados pelo medico. Retorna o total, ou -1 em erro. */
int prontuario_repo_contar_por_medico(int medico_id);

#endif
