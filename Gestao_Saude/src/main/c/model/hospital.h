#ifndef HOSPITAL_H
#define HOSPITAL_H
#define TRIAGEM_GERAL 1
#define TRIAGEM_ORTOPEDIA 2
#define TRIAGEM_CARDIOLOGIA 3
#define TRIAGEM_PNEUMOLOGIA 4
#define TRIAGEM_PEDIATRIA 5
#define MAX_EXAMES 300

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline void limparEntrada(void)
{
    int caractere;

    while ((caractere = getchar()) != '\n' && caractere != EOF)
    {
    }
}

static inline int lerInteiro(int *valor)
{
    if (scanf("%d", valor) != 1)
    {
        limparEntrada();
        return 0;
    }

    return 1;
}

#define MAX_PACIENTES 100
#define MAX_MEDICOS 50
#define MAX_AGENDAMENTOS 200
#define MAX_ALAS 20
#define MAX_LEITOS 300
#define MAX_INTERNACOES 200
#define MAX_TRIAGENS 300
#define MAX_PRONTUARIOS 300

#define ALA_INTERNACAO 1
#define ALA_UTI 2
#define ALA_OBSERVACAO 3
#define ALA_PEDIATRIA 4
#define ALA_CIRURGICA 5

#define EXAME_HEMOGRAMA 1
#define EXAME_RAIO_X 2
#define EXAME_TOMOGRAFIA 3
#define EXAME_RESSONANCIA 4
#define EXAME_ELETROCARDIOGRAMA 5
#define EXAME_URINA 6
#define EXAME_ULTRASSONOGRAFIA 7

typedef struct
{
    int id;
    char nome[100];
    char cpf[15];
    int idade;
    char telefone[20];
    char sexo;
    int regiaoAdministrativa;
    int ativo;
} Paciente;

typedef struct
{
    int id;
    char nome[100];
    char crm[20];
    char especialidade[50];
    int regiaoAdministrativa;
    int ativo;
} Medico;

typedef struct
{
    int id;
    int pacienteId;
    int medicoId;
    char data[11];
    char horario[6];
    char status[20];
} Agendamento;

typedef struct
{
    int id;
    char nome[100];
    int tipo;
    int totalLeitos;
    int leitosOcupados;
    int ativo;
} Ala;

typedef struct
{
    int id;
    int alaId;
    int numero;
    int ocupado;
    int pacienteId;
    int ativo;
} Leito;

typedef struct
{
    int id;
    int pacienteId;
    int alaId;
    int leitoId;
    char dataEntrada[11];
    char dataAlta[11];
    char status[20];
} Internacao;

typedef struct
{
    int id;
    int pacienteId;
    int tipoTriagem;
    int pontuacao;
    char classificacao[30];
    int ativo;
} Triagem;

typedef struct
{
    int id;
    int pacienteId;
    int medicoId;
    char data[11];
    char observacoes[300];
    char diagnostico[200];
    char conduta[200];
    int alertaImportante;
    int ativo;
} Prontuario;

typedef struct
{
    int id;
    int pacienteId;
    int medicoId;
    int prontuarioId;
    int tipoExame;
    char dataSolicitacao[11];
    char dataResultado[11];
    char resultado[300];
    char status[20];
    int urgente;
    int ativo;
} Exame;

extern Paciente pacientes[MAX_PACIENTES];
extern Medico medicos[MAX_MEDICOS];
extern Agendamento agendamentos[MAX_AGENDAMENTOS];
extern Ala alas[MAX_ALAS];
extern Leito leitos[MAX_LEITOS];
extern Internacao internacoes[MAX_INTERNACOES];
extern Triagem triagens[MAX_TRIAGENS];
extern Prontuario prontuarios[MAX_PRONTUARIOS];
extern Exame exames[MAX_EXAMES];

extern int totalPacientes;
extern int totalMedicos;
extern int totalAgendamentos;
extern int totalAlas;
extern int totalLeitos;
extern int totalInternacoes;
extern int totalTriagens;
extern int totalProntuarios;
extern int totalExames;

#endif
