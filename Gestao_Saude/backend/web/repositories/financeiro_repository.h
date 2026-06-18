#ifndef FINANCEIRO_REPOSITORY_H
#define FINANCEIRO_REPOSITORY_H

/*
 * Repository financeiro do SIGEH-DF: convenios e cobrancas.
 * Valores em centavos (inteiro) para evitar imprecisao de ponto flutuante.
 * Cobrancas nunca sao apagadas; mudam de status (auditoria financeira).
 */

/* --- Convenios --- */
int convenio_criar(const char *nome);
int convenio_listar_json(char *buffer, int tamanho);
int convenio_desativar(int id);
int convenio_contar_ativos(void);

/* --- Cobrancas --- */
/* Cria uma cobranca PENDENTE. forma: 'PARTICULAR' ou 'CONVENIO' (exige
 * convenio_id > 0). valor_centavos > 0. Retorna 1 em sucesso. */
int cobranca_criar(int paciente_id, int convenio_id, const char *forma,
                   const char *origem, const char *descricao,
                   int valor_centavos);

/* Muda o status da cobranca (PENDENTE/AUTORIZADA -> PAGA/AUTORIZADA/GLOSADA/
 * CANCELADA). GLOSADA e CANCELADA exigem motivo. 1 = ok, 0 = invalido. */
int cobranca_atualizar_status(int id, const char *novo_status, const char *motivo);

int cobranca_listar_json(char *buffer, int tamanho);
int cobranca_listar_por_paciente_json(int paciente_id, char *buffer, int tamanho);
int cobranca_contar_pendentes(void);

/* Demonstrativo financeiro (JSON): totais por status e valores consolidados. */
int cobranca_demonstrativo_json(char *buffer, int tamanho);

#endif
