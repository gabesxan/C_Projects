#include "paciente_repository.h"
#include "database.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

/* Colunas (e ordem) usadas em toda leitura de paciente, para montar o JSON
 * num so lugar (montarPacienteJson). */
#define PACIENTE_COLUNAS                                                \
    "id, nome, nascimento, documento, tipo_documento, telefone, sexo, " \
    "regiao_administrativa, responsavel, alergias, ativo, convenio_id"

/* Escreve em 'destino' a string 'origem' como literal JSON entre aspas,
 * escapando aspas e barras. Retorna 1 se coube, 0 se faltou espaco. */
static int escaparJson(char *destino, int tamanho, const char *origem)
{
    int pos = 0;
    int i;

    if (tamanho < 3)
    {
        return 0;
    }

    destino[pos++] = '"';

    for (i = 0; origem != NULL && origem[i] != '\0'; i++)
    {
        char c = origem[i];

        if (c == '"' || c == '\\')
        {
            if (pos + 2 > tamanho - 2)
            {
                return 0;
            }
            destino[pos++] = '\\';
            destino[pos++] = c;
        }
        else
        {
            if (pos + 1 > tamanho - 2)
            {
                return 0;
            }
            destino[pos++] = c;
        }
    }

    destino[pos++] = '"';
    destino[pos] = '\0';
    return 1;
}

/* Concatena 'texto' ao final de 'buffer', controlando o espaco usado.
 * Retorna 1 se coube, 0 caso contrario. */
static int anexarTexto(char *buffer, int tamanho, int *usado, const char *texto)
{
    int restante = tamanho - *usado;
    int escrito = snprintf(buffer + *usado, (size_t)restante, "%s", texto);

    if (escrito < 0 || escrito >= restante)
    {
        return 0;
    }

    *usado += escrito;
    return 1;
}

/* Idade em anos completos a partir de 'nascimento' (YYYY-MM-DD), relativa a
 * hoje. Retorna 0 se a data for invalida. */
static int idadeDe(const char *nascimento)
{
    int ay, am, ad;
    time_t agora;
    struct tm *hoje;
    int idade;

    if (nascimento == NULL || sscanf(nascimento, "%d-%d-%d", &ay, &am, &ad) != 3)
    {
        return 0;
    }

    agora = time(NULL);
    hoje = localtime(&agora);
    idade = (hoje->tm_year + 1900) - ay;

    if ((hoje->tm_mon + 1) < am ||
        ((hoje->tm_mon + 1) == am && hoje->tm_mday < ad))
    {
        idade--;
    }

    return idade < 0 ? 0 : idade;
}

/* 1 se 'nascimento' e uma data valida (YYYY-MM-DD) e nao esta no futuro. */
static int nascimentoValido(const char *nascimento)
{
    int ay, am, ad;
    int cy, cm, cd;
    time_t agora;
    struct tm *hoje;

    if (nascimento == NULL ||
        sscanf(nascimento, "%d-%d-%d", &ay, &am, &ad) != 3)
    {
        return 0;
    }

    if (ay < 1900 || am < 1 || am > 12 || ad < 1 || ad > 31)
    {
        return 0;
    }

    agora = time(NULL);
    hoje = localtime(&agora);
    cy = hoje->tm_year + 1900;
    cm = hoje->tm_mon + 1;
    cd = hoje->tm_mday;

    /* Recusa data no futuro. */
    if (ay > cy || (ay == cy && (am > cm || (am == cm && ad > cd))))
    {
        return 0;
    }

    return 1;
}

int paciente_repo_criar_retornando_id(const char *nome,
                                      const char *nascimento,
                                      const char *documento,
                                      const char *tipo_documento,
                                      const char *telefone,
                                      const char *sexo,
                                      int regiao_administrativa,
                                      const char *responsavel,
                                      const char *alergias,
                                      int *novo_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO pacientes "
        "(nome, nascimento, documento, tipo_documento, telefone, sexo, "
        "regiao_administrativa, responsavel, alergias, ativo) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, 1);";
    const char *tipo = (tipo_documento != NULL && tipo_documento[0] != '\0')
                           ? tipo_documento
                           : "CPF";
    int ok;

    if (novo_id != NULL)
    {
        *novo_id = 0;
    }

    /* Cadastro minimo: nome, nascimento, documento, telefone. */
    if (nome == NULL || nome[0] == '\0' ||
        documento == NULL || documento[0] == '\0' ||
        telefone == NULL || telefone[0] == '\0' ||
        sexo == NULL)
    {
        return 0;
    }

    /* Data de nascimento valida e nao futura (idade derivada dela). */
    if (nascimentoValido(nascimento) == 0)
    {
        return 0;
    }

    /* Menor de idade exige responsavel. */
    if (idadeDe(nascimento) < 18 &&
        (responsavel == NULL || responsavel[0] == '\0'))
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_text(stmt, 1, nome, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, nascimento, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, documento, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, tipo, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, telefone, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, sexo, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 7, regiao_administrativa);
    sqlite3_bind_text(stmt, 8, responsavel != NULL ? responsavel : "",
                      -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 9, alergias != NULL ? alergias : "",
                      -1, SQLITE_STATIC);

    /* Falha aqui inclui a violacao do indice de CPF unico entre ativos. */
    ok = sqlite3_step(stmt) == SQLITE_DONE;

    if (ok && novo_id != NULL)
    {
        *novo_id = (int)sqlite3_last_insert_rowid(db);
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return ok ? 1 : 0;
}

int paciente_repo_criar(const char *nome,
                        const char *nascimento,
                        const char *documento,
                        const char *tipo_documento,
                        const char *telefone,
                        const char *sexo,
                        int regiao_administrativa,
                        const char *responsavel,
                        const char *alergias)
{
    return paciente_repo_criar_retornando_id(nome, nascimento, documento,
                                             tipo_documento, telefone, sexo,
                                             regiao_administrativa, responsavel,
                                             alergias, NULL);
}

/* Monta o objeto JSON de um paciente a partir de uma linha posicionada em
 * PACIENTE_COLUNAS. Escreve em 'objeto' (com 'prefixo' antes). 1 ok / 0 erro. */
static int montarPacienteJson(sqlite3_stmt *stmt, const char *prefixo,
                              char *objeto, int tam)
{
    char nomeJson[256];
    char docJson[64];
    char tipoJson[16];
    char nascJson[16];
    char telefoneJson[64];
    char sexoJson[16];
    char responsavelJson[256];
    char alergiasJson[512];
    int id = sqlite3_column_int(stmt, 0);
    const char *nome = (const char *)sqlite3_column_text(stmt, 1);
    const char *nascimento = (const char *)sqlite3_column_text(stmt, 2);
    const char *documento = (const char *)sqlite3_column_text(stmt, 3);
    const char *tipo = (const char *)sqlite3_column_text(stmt, 4);
    const char *telefone = (const char *)sqlite3_column_text(stmt, 5);
    const char *sexo = (const char *)sqlite3_column_text(stmt, 6);
    int regiao = sqlite3_column_int(stmt, 7);
    const char *responsavel = (const char *)sqlite3_column_text(stmt, 8);
    const char *alergias = (const char *)sqlite3_column_text(stmt, 9);
    int ativo = sqlite3_column_int(stmt, 10);
    int convenioId = sqlite3_column_int(stmt, 11);
    int escrito;

    if (escaparJson(nomeJson, sizeof(nomeJson), nome) == 0 ||
        escaparJson(nascJson, sizeof(nascJson), nascimento) == 0 ||
        escaparJson(docJson, sizeof(docJson), documento) == 0 ||
        escaparJson(tipoJson, sizeof(tipoJson), tipo) == 0 ||
        escaparJson(telefoneJson, sizeof(telefoneJson), telefone) == 0 ||
        escaparJson(sexoJson, sizeof(sexoJson), sexo) == 0 ||
        escaparJson(responsavelJson, sizeof(responsavelJson), responsavel) == 0 ||
        escaparJson(alergiasJson, sizeof(alergiasJson), alergias) == 0)
    {
        return 0;
    }

    escrito = snprintf(objeto, (size_t)tam,
                       "%s{\"id\":%d,\"nome\":%s,\"nascimento\":%s,\"idade\":%d,"
                       "\"documento\":%s,\"tipoDocumento\":%s,\"telefone\":%s,\"sexo\":%s,"
                       "\"regiaoAdministrativa\":%d,\"responsavel\":%s,\"alergias\":%s,"
                       "\"ativo\":%d,\"convenioId\":%d}",
                       prefixo, id, nomeJson, nascJson, idadeDe(nascimento), docJson, tipoJson,
                       telefoneJson, sexoJson, regiao, responsavelJson, alergiasJson, ativo,
                       convenioId);

    return (escrito > 0 && escrito < tam) ? 1 : 0;
}

/* Executa 'sql' (com um possivel bind de medico_id) e serializa as linhas
 * como um array JSON de pacientes. Se medico_id > 0, faz o bind no parametro 1. */
static int listarComSql(const char *sql, int medico_id, char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    int usado = 0;
    int primeiro = 1;

    if (buffer == NULL || tamanho <= 0)
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    if (medico_id > 0)
    {
        sqlite3_bind_int(stmt, 1, medico_id);
    }

    buffer[0] = '\0';

    if (anexarTexto(buffer, tamanho, &usado, "[") == 0)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char objeto[1600];

        if (montarPacienteJson(stmt, primeiro ? "" : ",",
                               objeto, sizeof(objeto)) == 0 ||
            anexarTexto(buffer, tamanho, &usado, objeto) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        primeiro = 0;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);

    return anexarTexto(buffer, tamanho, &usado, "]");
}

int paciente_repo_listar_json(char *buffer, int tamanho)
{
    return listarComSql(
        "SELECT " PACIENTE_COLUNAS " FROM pacientes WHERE ativo = 1 ORDER BY id;",
        0, buffer, tamanho);
}

int paciente_repo_listar_por_medico_json(int medico_id, char *buffer, int tamanho)
{
    /* Mesmas 11 colunas/ordem de PACIENTE_COLUNAS, qualificadas por 'p' por
     * causa do JOIN (montarPacienteJson le por posicao). */
    const char *sql =
        "SELECT DISTINCT p.id, p.nome, p.nascimento, p.documento, "
        "p.tipo_documento, p.telefone, p.sexo, p.regiao_administrativa, "
        "p.responsavel, p.alergias, p.ativo, p.convenio_id "
        "FROM pacientes p JOIN agendamentos a ON a.paciente_id = p.id "
        "WHERE p.ativo = 1 AND a.medico_id = ? AND a.status != 'CANCELADO' "
        "ORDER BY p.id;";

    if (medico_id <= 0)
    {
        return 0;
    }

    return listarComSql(sql, medico_id, buffer, tamanho);
}

int paciente_repo_detalhe_json(int id, char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT " PACIENTE_COLUNAS " FROM pacientes WHERE id = ?;";
    int encontrado = 0;

    if (buffer == NULL || tamanho <= 0 || id <= 0)
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_int(stmt, 1, id);
    buffer[0] = '\0';

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        encontrado = montarPacienteJson(stmt, "", buffer, tamanho);
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return encontrado;
}

int paciente_repo_buscar_json(const char *termo, char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT " PACIENTE_COLUNAS " FROM pacientes "
        "WHERE ativo = 1 AND (nome LIKE ?1 OR documento LIKE ?1) "
        "ORDER BY nome LIMIT 50;";
    char like[128];
    int usado = 0;
    int primeiro = 1;

    if (buffer == NULL || tamanho <= 0 || termo == NULL)
    {
        return 0;
    }

    snprintf(like, sizeof(like), "%%%s%%", termo);

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_text(stmt, 1, like, -1, SQLITE_TRANSIENT);
    buffer[0] = '\0';

    if (anexarTexto(buffer, tamanho, &usado, "[") == 0)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char objeto[1600];

        if (montarPacienteJson(stmt, primeiro ? "" : ",",
                               objeto, sizeof(objeto)) == 0 ||
            anexarTexto(buffer, tamanho, &usado, objeto) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        primeiro = 0;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);

    return anexarTexto(buffer, tamanho, &usado, "]");
}

int paciente_repo_desativar(int id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE pacientes SET ativo = 0 WHERE id = ? AND ativo = 1;";
    int alteradas;

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    sqlite3_finalize(stmt);
    alteradas = sqlite3_changes(db);
    db_fechar(db);

    return alteradas > 0 ? 1 : 0;
}

int paciente_repo_atualizar_contato(int id, const char *telefone)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE pacientes SET telefone = ? WHERE id = ? AND ativo = 1;";
    int ok = 0;

    if (id <= 0 || telefone == NULL || telefone[0] == '\0')
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_text(stmt, 1, telefone, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, id);
    ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;

    sqlite3_finalize(stmt);
    db_fechar(db);
    return ok ? 1 : 0;
}

int paciente_repo_definir_convenio(int id, int convenio_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "UPDATE pacientes SET convenio_id = ? WHERE id = ? AND ativo = 1;";
    int ok = 0;

    if (id <= 0 || convenio_id < 0)
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    sqlite3_bind_int(stmt, 1, convenio_id);
    sqlite3_bind_int(stmt, 2, id);
    ok = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db) > 0;

    sqlite3_finalize(stmt);
    db_fechar(db);
    return ok ? 1 : 0;
}

int paciente_repo_regiao(int id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT regiao_administrativa FROM pacientes "
        "WHERE id = ? AND ativo = 1;";
    int regiao = -1;

    if (id <= 0)
    {
        return -1;
    }

    if (db_abrir(&db) == 0)
    {
        return -1;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return -1;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        regiao = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return regiao;
}

int paciente_repo_distribuicao_por_regiao_json(char *buffer, int tamanho)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT regiao_administrativa, COUNT(*) FROM pacientes "
        "WHERE ativo = 1 GROUP BY regiao_administrativa "
        "ORDER BY regiao_administrativa;";
    int usado = 0;
    int primeiro = 1;

    if (buffer == NULL || tamanho <= 0)
    {
        return 0;
    }

    if (db_abrir(&db) == 0)
    {
        return 0;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return 0;
    }

    buffer[0] = '\0';

    if (anexarTexto(buffer, tamanho, &usado, "[") == 0)
    {
        sqlite3_finalize(stmt);
        db_fechar(db);
        return 0;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        char objeto[64];
        int regiao = sqlite3_column_int(stmt, 0);
        int total = sqlite3_column_int(stmt, 1);
        int escrito;

        escrito = snprintf(objeto, sizeof(objeto),
                           "%s{\"regiao\":%d,\"total\":%d}",
                           primeiro ? "" : ",", regiao, total);

        if (escrito < 0 || escrito >= (int)sizeof(objeto))
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        if (anexarTexto(buffer, tamanho, &usado, objeto) == 0)
        {
            sqlite3_finalize(stmt);
            db_fechar(db);
            return 0;
        }

        primeiro = 0;
    }

    sqlite3_finalize(stmt);
    db_fechar(db);

    if (anexarTexto(buffer, tamanho, &usado, "]") == 0)
    {
        return 0;
    }

    return 1;
}

int paciente_repo_contar_por_medico(int medico_id)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "SELECT COUNT(DISTINCT p.id) FROM pacientes p "
        "JOIN agendamentos a ON a.paciente_id = p.id "
        "WHERE p.ativo = 1 AND a.medico_id = ? AND a.status != 'CANCELADO';";
    int total = -1;

    if (medico_id <= 0)
    {
        return -1;
    }

    if (db_abrir(&db) == 0)
    {
        return -1;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return -1;
    }

    sqlite3_bind_int(stmt, 1, medico_id);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        total = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return total;
}

int paciente_repo_contar_ativos(void)
{
    sqlite3 *db = NULL;
    sqlite3_stmt *stmt = NULL;
    const char *sql = "SELECT COUNT(*) FROM pacientes WHERE ativo = 1;";
    int total = -1;

    if (db_abrir(&db) == 0)
    {
        return -1;
    }

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        db_fechar(db);
        return -1;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        total = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    db_fechar(db);
    return total;
}
