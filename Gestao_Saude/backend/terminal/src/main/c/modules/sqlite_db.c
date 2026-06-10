#include "sqlite_db.h"
#include <errno.h>

static char caminhoBancoSQLite[256] = "../data/sigeh_v2.db";

const char *obterCaminhoBancoSQLite(void)
{
    return caminhoBancoSQLite;
}

const char *obterCaminhoSchemaSQLite(void)
{
    return "../data/schema_v2.sql";
}

int definirCaminhoBancoSQLite(const char *caminho)
{
    if (caminho == NULL || caminho[0] == '\0')
    {
        return 0;
    }

    if (strlen(caminho) >= sizeof(caminhoBancoSQLite))
    {
        return 0;
    }

    strcpy(caminhoBancoSQLite, caminho);
    return 1;
}

int abrirBancoSQLite(sqlite3 **db)
{
    if (db == NULL)
    {
        return 0;
    }

    if (sqlite3_open(obterCaminhoBancoSQLite(), db) != SQLITE_OK)
    {
        if (*db != NULL)
        {
            sqlite3_close(*db);
            *db = NULL;
        }

        return 0;
    }

    return 1;
}

void fecharBancoSQLite(sqlite3 *db)
{
    if (db != NULL)
    {
        sqlite3_close(db);
    }
}

int executarSQLSQLite(const char *sql)
{
    sqlite3 *db = NULL;
    char *mensagemErro = NULL;
    int resultado;

    if (sql == NULL)
    {
        return 0;
    }

    if (abrirBancoSQLite(&db) == 0)
    {
        return 0;
    }

    resultado = sqlite3_exec(db, sql, NULL, NULL, &mensagemErro);

    if (mensagemErro != NULL)
    {
        sqlite3_free(mensagemErro);
    }

    fecharBancoSQLite(db);

    return resultado == SQLITE_OK;
}

int reinicializarBancoSQLite(void)
{
    FILE *arquivoSchema;
    long tamanhoArquivo;
    size_t bytesLidos;
    char *conteudoSchema;
    int resultadoExecucao;

    if (remove(obterCaminhoBancoSQLite()) != 0 && errno != ENOENT)
    {
        return 0;
    }

    arquivoSchema = fopen(obterCaminhoSchemaSQLite(), "r");

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

    resultadoExecucao = executarSQLSQLite(conteudoSchema);
    free(conteudoSchema);

    return resultadoExecucao;
}
