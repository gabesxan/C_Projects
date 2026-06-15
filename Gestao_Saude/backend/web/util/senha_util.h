#ifndef SENHA_UTIL_H
#define SENHA_UTIL_H

/*
 * Utilitario de hashing de senhas (SHA-256 + salt, via OpenSSL).
 * Nunca guarda/transporta a senha em texto: armazena salt + hash.
 */

/* Gera um salt aleatorio em hex (NUL-terminado). 'tam' deve ser >= 17.
 * Retorna 1 em sucesso, 0 caso contrario. */
int senha_gerar_salt(char *salt, int tam);

/* Calcula o hash hex de (salt + senha) em 'hash'. 'tam' deve ser >= 65.
 * Retorna 1 em sucesso, 0 caso contrario. */
int senha_hash(const char *salt, const char *senha, char *hash, int tam);

#endif
