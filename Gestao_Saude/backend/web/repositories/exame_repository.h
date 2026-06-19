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

/* Avanca o status do exame um passo na linha do tempo
 * (SOLICITADO -> AUTORIZADO -> COLETADO -> EM_ANALISE). 1 = ok, 0 = invalido. */
int exame_repo_atualizar_status(int id, const char *novo_status);

/* Registra o resultado (apos a coleta), marca como CONCLUIDO e sinaliza se e
 * um resultado critico. 1 = ok, 0 = invalido. */
int exame_repo_registrar_resultado(int id, const char *resultado, int critico);

/* Registra ou atualiza o resultado estruturado de um analito do painel do
 * exame. So aceita exame em COLETADO/EM_ANALISE. 1 = ok, 0 = invalido. */
int exame_repo_registrar_resultado_analito(int exame_id, int analito_id,
                                           double valor_numerico,
                                           const char *valor_texto,
                                           const char *observacao);

/* Lista (JSON) os resultados estruturados do exame, com metadados do analito
 * e marcador de faixa de referencia. 1 = ok, 0 = exame inexistente/falha. */
int exame_repo_listar_resultados_analito_json(int exame_id, char *buffer,
                                              int tamanho);

/* Cancela o exame; exige motivo e nao cancela exame concluido. 1 = ok, 0 = falha. */
int exame_repo_cancelar(int id, const char *motivo);

/* Retifica o resultado de um exame concluido criando uma nova versao (vigente)
 * e preservando a anterior; exige justificativa. 1 = ok, 0 = falha. */
int exame_repo_retificar_resultado(int id, const char *resultado, int critico,
                                   const char *justificativa);

int exame_repo_desativar(int id);
int exame_repo_contar_ativos(void);

/* Conta exames ativos solicitados pelo medico. Retorna o total, ou -1 em erro. */
int exame_repo_contar_por_medico(int medico_id);

#endif
