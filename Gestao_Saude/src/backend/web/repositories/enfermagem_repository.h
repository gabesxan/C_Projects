#ifndef ENFERMAGEM_REPOSITORY_H
#define ENFERMAGEM_REPOSITORY_H

/*
 * Repository da enfermagem: administracao de medicamentos (MAR) e evolucao de
 * enfermagem. Append-only: registros nunca sao alterados nem apagados.
 */

/* Registra a administracao de uma prescricao ativa. Deriva o paciente da
 * prescricao. Retorna 1 em sucesso, 0 se a prescricao nao existe/ativa. */
int administracao_criar(int prescricao_id, int usuario_id,
                        const char *usuario_login, const char *observacao);

/* Lista (JSON) as administracoes de um paciente, mais recentes primeiro. */
int administracao_listar_por_paciente_json(int paciente_id, char *buffer, int tamanho);

/* Registra uma evolucao de enfermagem (texto + sinais vitais). Exige texto.
 * Retorna 1 em sucesso, 0 caso contrario. */
int evolucao_criar(int paciente_id, const char *autor_login, const char *texto,
                   const char *pressao, const char *temperatura,
                   const char *freq_cardiaca, const char *saturacao);

/* Lista (JSON) as evolucoes de enfermagem de um paciente, recentes primeiro. */
int evolucao_listar_por_paciente_json(int paciente_id, char *buffer, int tamanho);

#endif
