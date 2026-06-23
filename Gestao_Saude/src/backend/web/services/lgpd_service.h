#ifndef LGPD_SERVICE_H
#define LGPD_SERVICE_H

/*
 * Service LGPD do SIGEH-DF. Camada somente-leitura que monta o relatorio de
 * acessos aos dados de um paciente a partir da trilha de auditoria
 * (append-only). O relatorio responde "quem acessou, quando, qual acao, sobre
 * qual entidade/id e com qual detalhe", sempre filtrado por um paciente.
 *
 * A SQL do recorte vive no auditoria_repository; aqui ficam a validacao e a
 * composicao da resposta (envelope com o filtro de paciente). O controle de
 * acesso e a auditoria do proprio acesso ao relatorio sao responsabilidade da
 * camada de API.
 */

/* Monta em 'buffer' o relatorio de acessos do paciente:
 * {"pacienteId":<id>,"acessos":[<registros de auditoria, mais recentes antes>]}.
 * Cada registro traz usuarioId, usuarioLogin, acao, entidade, entidadeId,
 * detalhe e criadoEm. Retorna 1 em sucesso; 0 em paciente invalido ou falha
 * (gravando um JSON de erro em 'buffer'). */
int lgpd_service_relatorio_acessos_json(int paciente_id, char *buffer,
                                        int tamanho);

#endif
