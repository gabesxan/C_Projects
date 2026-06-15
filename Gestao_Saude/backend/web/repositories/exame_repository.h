#ifndef EXAME_REPOSITORY_H
#define EXAME_REPOSITORY_H

/*
 * Repository de exames do backend web do SIGEH-DF.
 * Cria o exame no momento da solicitacao (status 'SOLICITADO', sem resultado).
 * Usa database.h (SQLite) com prepared statements. Escrita: 1 = ok, 0 = falha.
 */

int exame_repo_criar(int paciente_id, int medico_id, int prontuario_id,
                     int tipo_exame, const char *data_solicitacao, int urgente);
int exame_repo_listar_json(char *buffer, int tamanho);
int exame_repo_listar_por_paciente_json(int paciente_id, char *buffer, int tamanho);
int exame_repo_listar_por_medico_json(int medico_id, char *buffer, int tamanho);
int exame_repo_desativar(int id);
int exame_repo_contar_ativos(void);

#endif
