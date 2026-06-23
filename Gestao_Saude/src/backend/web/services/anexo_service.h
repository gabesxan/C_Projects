#ifndef ANEXO_SERVICE_H
#define ANEXO_SERVICE_H

/*
 * Service de anexos do SIGEH-DF. Orquestra o upload: decodifica o conteudo
 * base64, valida tamanho maximo e tipo (mime) permitido, grava o binario no
 * filesystem (src/backend/data/anexos/) e registra o metadado no repository.
 * Camada de regra acima do anexo_repository (que so toca no banco).
 */

/* Tamanho maximo do arquivo decodificado (5 MB). */
#define ANEXO_MAX_BYTES (5 * 1024 * 1024)

/* Cria um anexo a partir do conteudo em base64. Valida entidade/nome/mime e o
 * tamanho do binario decodificado, grava o arquivo e insere o metadado.
 * Escreve o resultado (JSON) em 'buffer' e preenche '*novo_id' em sucesso.
 * Retorna 1 se criou; 0 em parametro invalido, mime nao permitido, base64
 * invalido, arquivo acima do limite ou falha de escrita. */
int anexo_service_criar(const char *entidade, int entidade_id, const char *nome,
                        const char *mime, const char *conteudo_b64,
                        int autor_id, const char *autor_login,
                        char *buffer, int tamanho, int *novo_id);

/* Apaga o binario de um anexo do filesystem (caminho relativo a src/backend/data).
 * Best-effort: usado apos remover o metadado no repository. */
void anexo_service_apagar_arquivo(const char *caminho);

#endif
