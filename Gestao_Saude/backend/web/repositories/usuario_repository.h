#ifndef USUARIO_REPOSITORY_H
#define USUARIO_REPOSITORY_H

/*
 * Repository de usuarios do backend web do SIGEH-DF.
 * Senhas sao guardadas como salt + hash SHA-256 (nunca em texto).
 * papel: ADMIN, CADASTRO, MEDICO, ENFERMAGEM, PACIENTE.
 */

int usuario_repo_criar(const char *nome, const char *login, const char *senha,
                       const char *papel, int paciente_id, int medico_id);

/* Valida credenciais; em sucesso preenche papel, vinculos e o id do usuario
 * (todos os ponteiros de saida podem ser NULL) e retorna 1. Login
 * inexistente/inativo ou senha errada -> 0. */
int usuario_repo_autenticar(const char *login, const char *senha,
                            char *papel, int papel_tam,
                            int *paciente_id, int *medico_id,
                            int *usuario_id);

/* Como usuario_repo_autenticar, mas com bloqueio por tentativas: cada senha
 * errada incrementa o contador e, ao atingir o limite, bloqueia o login por
 * um intervalo; um acerto zera o contador. Se o login estiver bloqueado no
 * momento, *bloqueado recebe 1 (pode ser NULL) e retorna 0. */
int usuario_repo_autenticar_com_bloqueio(const char *login, const char *senha,
                                         char *papel, int papel_tam,
                                         int *paciente_id, int *medico_id,
                                         int *usuario_id, int *bloqueado);

/* Reativa um usuario previamente desativado. Retorna 1 se reativou. */
int usuario_repo_reativar(int id);

/* 1 se ja existe um usuario (ativo ou nao) com o login informado; 0 se livre.
 * Usado para garantir a unicidade do login gerado no fluxo de triagem. */
int usuario_repo_login_existe(const char *login);

/* Lista (JSON) os usuarios ativos SEM expor senha/hash/salt. */
int usuario_repo_listar_json(char *buffer, int tamanho);

int usuario_repo_desativar(int id);
int usuario_repo_contar_ativos(void);

#endif
