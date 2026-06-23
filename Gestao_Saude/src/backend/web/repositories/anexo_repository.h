#ifndef ANEXO_REPOSITORY_H
#define ANEXO_REPOSITORY_H

/*
 * Repository de anexos (SIGEH-DF): documentos vinculados a uma entidade
 * (ex.: exame, paciente) pelo par (entidade, entidade_id). O binario fica no
 * filesystem (src/backend/data/anexos/); aqui guardamos apenas os metadados e o
 * caminho relativo. Registros sao imutaveis: criar e remover, nunca editar.
 */

/* Insere o metadado de um anexo ja gravado no filesystem. Preenche
 * '*novo_id' com o id gerado quando informado. Retorna 1/0. */
int anexo_criar(const char *entidade, int entidade_id, const char *nome,
                const char *mime, long tamanho, const char *caminho,
                int autor_id, const char *autor_login, int *novo_id);

/* Finaliza o caminho de armazenamento de um anexo recem-criado (o caminho so e
 * conhecido depois do insert, pois inclui o id gerado). Retorna 1 se atualizou. */
int anexo_definir_caminho(int id, const char *caminho);

/* Lista (JSON) os anexos de uma entidade, sem expor o caminho interno. */
int anexo_listar_por_entidade_json(const char *entidade, int entidade_id,
                                   char *buffer, int tamanho);

/* Busca um anexo por id, devolvendo nome, mime e caminho interno para o
 * download. Retorna 1 se encontrou. */
int anexo_buscar(int id, char *nome, int tnome, char *mime, int tmime,
                 char *caminho, int tcaminho);

/* Remove (hard delete) o metadado de um anexo, devolvendo em 'caminho_out' o
 * caminho do arquivo para o chamador apagar do filesystem. Retorna 1 se uma
 * linha foi removida. */
int anexo_remover(int id, char *caminho_out, int tcaminho);

#endif
