#ifndef AUDITORIA_REPOSITORY_H
#define AUDITORIA_REPOSITORY_H

/*
 * Repository da trilha de auditoria do SIGEH-DF.
 * Registra acoes sensiveis (login, alta, prescricao, mudanca de leito, etc.)
 * de forma append-only: a tabela nunca e atualizada nem apagada.
 */

/* Registra uma acao na trilha de auditoria. Falhas de auditoria nunca devem
 * impedir a operacao principal, entao o retorno (1 ok / 0 falha) e informativo
 * e geralmente ignorado pelo chamador. usuario_login e detalhe podem ser "". */
int auditoria_registrar(int usuario_id, const char *usuario_login,
                        const char *acao, const char *entidade,
                        int entidade_id, const char *detalhe);

/* Lista (JSON) os ultimos registros de auditoria, mais recentes primeiro. */
int auditoria_listar_json(char *buffer, int tamanho);

/* Lista (JSON, array) os acessos registrados na trilha que dizem respeito aos
 * dados de um paciente: acoes sobre a propria ficha e sobre as entidades
 * clinicas/administrativas ligadas a ele (exames, prontuarios, prescricoes,
 * triagens, agendamentos, consentimentos, cobrancas, check-ins, internacoes e
 * solicitacoes). Mais recentes primeiro. Base do relatorio LGPD de acessos. */
int auditoria_acessos_paciente_json(int paciente_id, char *buffer, int tamanho);

int auditoria_contar(void);

#endif
