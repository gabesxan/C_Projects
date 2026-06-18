#ifndef TRIAGEM_SERVICE_H
#define TRIAGEM_SERVICE_H

/*
 * Service de triagem do backend web do SIGEH-DF.
 * Camada de decisao (somente-leitura): orquestra os repositories e produz
 * a recomendacao consolidada da triagem. Nao escreve no banco.
 */

/* Escreve (JSON) o checklist de triagem (chave, rotulo, nivel, classificacao)
 * — fonte unica usada pela UI e pela classificacao automatica. 1/0. */
int triagem_service_checklist_json(char *buffer, int tamanho);

/* Deriva a classificacao de risco a partir dos itens marcados (chaves
 * separadas por virgula), pela regra "discriminador mais grave vence".
 * Escreve a cor em 'classificacao' e o nivel (1-5) em *nivel_out. 1/0. */
int triagem_service_classificar(const char *itens, char *classificacao,
                                int classificacao_tam, int *nivel_out);

/* Avalia a triagem ativa mais recente do paciente e escreve em 'buffer' um
 * JSON com pacienteId, classificacao, prioridade e especialidadeProvavel.
 * Retorna 1 em sucesso; 0 se o paciente nao tem triagem ativa ou em erro. */
int triagem_service_avaliar_json(int paciente_id, char *buffer, int tamanho);

/* Lista (JSON) as triagens ativas cuja especialidade provavel (mapeada pelo
 * tipo) corresponde a 'especialidade' informada. Usado para escopar a fila de
 * triagem ao que o medico atende. Retorna 1 em sucesso, 0 em erro. */
int triagem_service_listar_por_especialidade_json(const char *especialidade,
                                                  char *buffer, int tamanho);

/* A partir da triagem do paciente, escreve em 'buffer' um JSON com a
 * especialidade provavel, a regiao do paciente e os medicos ativos sugeridos
 * (mesma especialidade e regiao). Retorna 1 em sucesso; 0 se nao ha triagem
 * ativa ou em erro. */
int triagem_service_sugerir_medicos_json(int paciente_id, char *buffer, int tamanho);

/* Historico clinico do paciente: prontuarios e exames anteriores (JSON).
 * Retorna 1 em sucesso (listas podem vir vazias); 0 em parametro/erro. */
int triagem_service_historico_json(int paciente_id, char *buffer, int tamanho);

/* Exames iniciais sugeridos conforme o tipo da triagem do paciente (JSON).
 * Retorna 1 em sucesso; 0 se nao ha triagem ativa ou em erro. */
int triagem_service_sugerir_exames_json(int paciente_id, char *buffer, int tamanho);

/* Tenta agendar o paciente com um medico disponivel da especialidade/regiao
 * indicada pela triagem, na data/horario informados. Escreve o resultado em
 * 'buffer' (JSON). Retorna 1 se agendou; 0 caso contrario. */
int triagem_service_agendar_json(int paciente_id, const char *data,
                                 const char *horario, char *buffer, int tamanho);

/* Encaminha o paciente para uma especialidade informada: agenda com um medico
 * disponivel dessa especialidade na regiao do paciente. Escreve o resultado em
 * 'buffer' (JSON). Retorna 1 se encaminhou; 0 caso contrario. */
int triagem_service_encaminhar_json(int paciente_id, const char *especialidade,
                                    const char *data, const char *horario,
                                    char *buffer, int tamanho);

#endif
