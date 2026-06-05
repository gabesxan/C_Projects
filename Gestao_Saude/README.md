# SIGEH-DF - Sistema Integrado de Gestao Hospitalar em C

O SIGEH-DF e um projeto academico de sistema hospitalar desenvolvido em linguagem C para a disciplina de Algoritmos e Logica de Programacao.

A aplicacao roda no terminal e foi criada para praticar organizacao de codigo em C usando `structs`, vetores globais, funcoes, arquivos de cabecalho (`.h`), arquivos de implementacao (`.c`) e compilacao com `Makefile`.

No estado atual, os dados ficam apenas em memoria durante a execucao. O projeto ainda nao possui persistencia em arquivo, banco de dados, interface grafica, API, login ou framework externo de testes.

## Funcionalidades Atuais

O sistema possui modulos para:

- pacientes;
- medicos;
- agendamentos simples;
- alas;
- leitos;
- internacoes;
- triagem;
- relatorios gerais.

## Estrutura Do Projeto

A arquitetura oficial do projeto esta organizada assim:

```text
src/main/c/app/main.c
src/main/c/model/hospital.h
src/main/c/headers/*.h
src/main/c/modules/*.c
src/test/c/modules/*.c
```

Responsabilidades principais:

- `src/main/c/app`: ponto de entrada da aplicacao.
- `src/main/c/model`: `structs`, constantes e variaveis globais compartilhadas.
- `src/main/c/headers`: declaracoes das funcoes dos modulos.
- `src/main/c/modules`: implementacoes dos modulos.
- `src/test/c/modules`: testes automatizados simples.

Arquivos atuais:

```text
src/main/c/app/main.c
src/main/c/headers/agendamento.h
src/main/c/headers/ala.h
src/main/c/headers/internacao.h
src/main/c/headers/leito.h
src/main/c/headers/medico.h
src/main/c/headers/paciente.h
src/main/c/headers/relatorio.h
src/main/c/headers/triagem.h
src/main/c/model/hospital.h
src/main/c/modules/agendamento.c
src/main/c/modules/ala.c
src/main/c/modules/internacao.c
src/main/c/modules/leito.c
src/main/c/modules/medico.c
src/main/c/modules/paciente.c
src/main/c/modules/relatorio.c
src/main/c/modules/triagem.c
src/test/c/modules/test_agendamento.c
src/test/c/modules/test_ala.c
src/test/c/modules/test_internacao.c
src/test/c/modules/test_leito.c
src/test/c/modules/test_medico.c
src/test/c/modules/test_paciente.c
src/test/c/modules/test_relatorio.c
src/test/c/modules/test_triagem.c
```

## Como Compilar

Na raiz do projeto, execute:

```bash
make clean && make
```

Tambem e possivel executar os comandos separadamente:

```bash
make clean
make
```

## Como Executar

Para compilar e abrir o sistema no terminal:

```bash
make run
```

Ou, depois de compilar, execute diretamente:

```bash
./sigeh
```

## Testes

O projeto ja possui testes automatizados simples usando `assert.h`, sem framework externo.

O comando abaixo compila e executa todos os testes:

```bash
make test
```

Modulos com testes automatizados:

- agendamento;
- leito;
- triagem;
- paciente;
- medico;
- ala;
- internacao;
- relatorio.

Executaveis de teste gerados pelo `Makefile`:

- `test_agendamento`
- `test_leito`
- `test_triagem`
- `test_paciente`
- `test_medico`
- `test_ala`
- `test_internacao`
- `test_relatorio`

## Tecnologias E Conceitos

O projeto usa:

- linguagem C;
- terminal;
- `Makefile`;
- `structs`;
- vetores globais;
- funcoes;
- headers;
- modularizacao em arquivos `.c` e `.h`;
- testes simples com `assert.h`.

## Funcionalidades Futuras Planejadas

As proximas evolucoes planejadas para o SIGEH-DF incluem:

- Painel Situacional do DF;
- Prontuario Integrado;
- Exames Integrados;
- agendamento inteligente por especialidade;
- regionalizacao do atendimento;
- fila de prioridade por triagem;
- persistencia em arquivos `.txt`;
- SQLite futuramente.
