#ifndef CREDENCIAL_UTIL_H
#define CREDENCIAL_UTIL_H

/*
 * Geracao de credenciais do paciente criado no fluxo de triagem.
 * - normaliza nomes (minusculo, sem acentos, sem espacos);
 * - gera senha automatica a partir do nome + nascimento + digitos aleatorios.
 * O login unico (pac-<nome><sufixo>) e montado por quem chama, pois depende de
 * checar duplicidade no banco.
 */

/* Escreve em 'out' o 'nome' normalizado: minusculo, sem acentos (mapeando os
 * acentos portugueses em UTF-8), mantendo apenas letras e digitos. Retorna 1
 * em sucesso, 0 se faltar espaco. */
int credencial_normalizar(const char *nome, char *out, int tam);

/* Gera a senha automatica do paciente: 3 primeiras letras do nome (normalizado)
 * + data de nascimento sem separadores + 4 digitos aleatorios. Tudo minusculo,
 * sem espacos e sem acentos. 'tam' deve ser >= 20. Retorna 1/0. */
int credencial_gerar_senha(const char *nome, const char *nascimento,
                           char *out, int tam);

#endif
