#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

/*
 * Camada fina de acesso ao SQLite para o backend web do SIGEH-DF.
 * Independente da V1 terminal: nao inclui hospital.h nem o model.
 * Convencao de retorno: 1 = sucesso, 0 = falha.
 */

/* Define o caminho do arquivo .db usado por db_abrir. Retorna 1/0. */
int db_definir_caminho(const char *caminho);

/* Retorna o caminho atual do banco. */
const char *db_caminho(void);

/* Abre uma conexao SQLite no caminho atual. Retorna 1/0. */
int db_abrir(sqlite3 **db);

/* Fecha a conexao (seguro com NULL). */
void db_fechar(sqlite3 *db);

/* Executa um SQL sem leitura de linhas (DDL/script). Retorna 1/0. */
int db_executar(const char *sql);

/* Remove o .db atual e aplica o arquivo de schema informado. Retorna 1/0. */
int db_resetar_com_schema(const char *caminho_schema);

/* Aplica, em ordem, as migracoes de schema pendentes (versao > user_version
 * atual), preservando os dados. Idempotente e seguro em banco ja atualizado.
 * Bancos sem o schema base (nao semeados) sao ignorados. Retorna 1/0. */
int db_migrar(void);

#endif
