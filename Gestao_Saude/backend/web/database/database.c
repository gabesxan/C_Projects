#include "database.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Caminho padrao relativo ao diretorio backend/web/ (sobrescrito nos testes). */
static char caminhoBanco[256] = "../data/sigeh_v2.db";

int db_definir_caminho(const char *caminho)
{
    if (caminho == NULL || caminho[0] == '\0')
    {
        return 0;
    }

    if (strlen(caminho) >= sizeof(caminhoBanco))
    {
        return 0;
    }

    strcpy(caminhoBanco, caminho);
    return 1;
}

const char *db_caminho(void)
{
    return caminhoBanco;
}

int db_abrir(sqlite3 **db)
{
    if (db == NULL)
    {
        return 0;
    }

    if (sqlite3_open(caminhoBanco, db) != SQLITE_OK)
    {
        if (*db != NULL)
        {
            sqlite3_close(*db);
            *db = NULL;
        }

        return 0;
    }

    /* Integridade referencial e por conexao no SQLite e precisa ser ligada
     * explicitamente a cada abertura. */
    sqlite3_exec(*db, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);

    return 1;
}

void db_fechar(sqlite3 *db)
{
    if (db != NULL)
    {
        sqlite3_close(db);
    }
}

int db_executar(const char *sql)
{
    sqlite3 *db = NULL;
    char *mensagemErro = NULL;
    int resultado;

    if (sql == NULL)
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    resultado = sqlite3_exec(db, sql, NULL, NULL, &mensagemErro);

    if (mensagemErro != NULL)
    {
        sqlite3_free(mensagemErro);
    }

    db_fechar(db);

    return resultado == SQLITE_OK;
}

int db_resetar_com_schema(const char *caminho_schema)
{
    FILE *arquivoSchema;
    long tamanhoArquivo;
    size_t bytesLidos;
    char *conteudoSchema;
    int resultadoExecucao;

    if (caminho_schema == NULL || caminho_schema[0] == '\0')
    {
        return 0;
    }

    if (remove(caminhoBanco) != 0 && errno != ENOENT)
    {
        return 0;
    }

    arquivoSchema = fopen(caminho_schema, "r");

    if (arquivoSchema == NULL)
    {
        return 0;
    }

    if (fseek(arquivoSchema, 0, SEEK_END) != 0)
    {
        fclose(arquivoSchema);
        return 0;
    }

    tamanhoArquivo = ftell(arquivoSchema);

    if (tamanhoArquivo < 0)
    {
        fclose(arquivoSchema);
        return 0;
    }

    if (fseek(arquivoSchema, 0, SEEK_SET) != 0)
    {
        fclose(arquivoSchema);
        return 0;
    }

    conteudoSchema = malloc((size_t)tamanhoArquivo + 1);

    if (conteudoSchema == NULL)
    {
        fclose(arquivoSchema);
        return 0;
    }

    bytesLidos = fread(conteudoSchema, 1, (size_t)tamanhoArquivo, arquivoSchema);
    conteudoSchema[bytesLidos] = '\0';
    fclose(arquivoSchema);

    if (bytesLidos != (size_t)tamanhoArquivo)
    {
        free(conteudoSchema);
        return 0;
    }

    resultadoExecucao = db_executar(conteudoSchema);
    free(conteudoSchema);

    return resultadoExecucao;
}
