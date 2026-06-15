#ifndef TRIAGEM_SERVICE_H
#define TRIAGEM_SERVICE_H

/*
 * Service de triagem do backend web do SIGEH-DF.
 * Camada de decisao (somente-leitura): orquestra os repositories e produz
 * a recomendacao consolidada da triagem. Nao escreve no banco.
 */

/* Avalia a triagem ativa mais recente do paciente e escreve em 'buffer' um
 * JSON com pacienteId, classificacao, prioridade e especialidadeProvavel.
 * Retorna 1 em sucesso; 0 se o paciente nao tem triagem ativa ou em erro. */
int triagem_service_avaliar_json(int paciente_id, char *buffer, int tamanho);

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

#endif
