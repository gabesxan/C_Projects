#ifndef HOSPITAL_H
#define HOSPITAL_H
#define TRIAGEM_GERAL 1
#define TRIAGEM_ORTOPEDIA 2
#define TRIAGEM_CARDIOLOGIA 3
#define TRIAGEM_PNEUMOLOGIA 4
#define TRIAGEM_PEDIATRIA 5

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PACIENTES 100
#define MAX_MEDICOS 50
#define MAX_AGENDAMENTOS 200
#define MAX_ALAS 20
#define MAX_LEITOS 300
#define MAX_INTERNACOES 200
#define MAX_TRIAGENS 300

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
    char tipo[50];
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

extern Paciente pacientes[MAX_PACIENTES];
extern Medico medicos[MAX_MEDICOS];
extern Agendamento agendamentos[MAX_AGENDAMENTOS];
extern Ala alas[MAX_ALAS];
extern Leito leitos[MAX_LEITOS];
extern Internacao internacoes[MAX_INTERNACOES];
extern Triagem triagens[MAX_TRIAGENS];

extern int totalPacientes;
extern int totalMedicos;
extern int totalAgendamentos;
extern int totalAlas;
extern int totalLeitos;
extern int totalInternacoes;
extern int totalTriagens;

#endif
