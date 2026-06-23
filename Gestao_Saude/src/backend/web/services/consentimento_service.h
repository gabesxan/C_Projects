#ifndef CONSENTIMENTO_SERVICE_H
#define CONSENTIMENTO_SERVICE_H

/*
 * Service de consentimentos LGPD do SIGEH-DF. Camada de regra acima do
 * consentimento_repository: valida os campos obrigatorios, garante as
 * transicoes de estado (CONCEDIDO -> REVOGADO) e devolve a resposta em JSON
 * pronta para a API. Mantem o historico imutavel (nunca apaga linhas).
 */

/* Cria um consentimento a partir dos dados administrativos. Valida paciente,
 * finalidade e versao do termo. Escreve o resultado (JSON) em 'buffer' e
 * preenche '*novo_id' em sucesso. Retorna 1 se criou; 0 em dado invalido. */
int consentimento_service_criar(int paciente_id, const char *finalidade,
                                const char *versao_termo,
                                char *buffer, int tamanho, int *novo_id);

/* Revoga um consentimento, exigindo motivo (acao sensivel). So permite a
 * transicao a partir de CONCEDIDO; preserva a linha original. Escreve o
 * resultado (JSON) em 'buffer'. Preenche '*nao_encontrado' com 1 quando o id
 * nao existe (para a API responder 404). Retorna 1 se revogou; 0 caso
 * contrario (motivo ausente, inexistente ou ja revogado). */
int consentimento_service_revogar(int id, const char *motivo,
                                  char *buffer, int tamanho,
                                  int *nao_encontrado);

#endif
