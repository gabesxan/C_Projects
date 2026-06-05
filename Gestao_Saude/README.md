# SIGEH-DF - Sistema Integrado de Gestao Hospitalar em C

O SIGEH-DF e um projeto academico desenvolvido em linguagem C para a disciplina de Algoritmos e Logica de Programacao.

O sistema roda no terminal e foi construido para praticar:

- `structs`
- vetores globais
- funcoes
- modularizacao em `.h` e `.c`
- menus com `switch case`
- regras de negocio
- compilacao com `Makefile`
- testes automatizados simples com `assert.h`

No estado atual, os dados ficam em memoria durante a execucao. O projeto nao possui banco de dados, persistencia em arquivo, interface grafica, API ou framework externo.

## Objetivo do Projeto

O objetivo do SIGEH-DF e simular a administracao de um ambiente hospitalar de forma simples, modular e explicavel em sala de aula.

O sistema permite gerenciar:

- pacientes
- medicos
- agendamentos
- alas
- leitos
- internacoes
- triagens
- relatorios

## Estrutura Geral

```text
src/main/c/app/main.c
src/main/c/model/hospital.h
src/main/c/headers/*.h
src/main/c/modules/*.c
src/test/c/modules/*.c
Makefile
README.md
```

## Arquitetura Adotada

### `src/main/c/app`

Contem o ponto de entrada da aplicacao.

### `src/main/c/model`

Contem a modelagem principal do sistema:
- constantes
- `structs`
- vetores globais
- contadores globais

### `src/main/c/headers`

Contem os arquivos `.h` dos modulos, com declaracoes de funcoes.

### `src/main/c/modules`

Contem os arquivos `.c`, com a implementacao de cada modulo.

### `src/test/c/modules`

Contem os testes automatizados simples com `assert.h`.

## Explicacao Arquivo por Arquivo

## `src/main/c/app/main.c`

Este e o ponto de entrada do sistema.

Responsabilidades:
- declarar os vetores globais do projeto
- declarar os contadores globais
- exibir o menu principal
- encaminhar o usuario para cada modulo

Estrutura atual:
- `exibirMenuPrincipal()`: imprime o menu principal
- `executarOpcaoMenuPrincipal(int opcao)`: recebe a opcao e chama o modulo correspondente
- `main()`: controla o loop principal do sistema

Importancia:
- centraliza a navegacao
- evita que a logica hospitalar fique concentrada no `main`
- deixa o programa mais organizado

## `src/main/c/model/hospital.h`

Este e o arquivo mais importante da modelagem do sistema.

Responsabilidades:
- incluir bibliotecas padrao
- definir constantes como `MAX_PACIENTES`, `MAX_MEDICOS`, `MAX_LEITOS`
- definir todas as `structs`
- declarar os vetores globais com `extern`
- declarar os contadores globais com `extern`

Structs presentes:
- `Paciente`
- `Medico`
- `Agendamento`
- `Ala`
- `Leito`
- `Internacao`
- `Triagem`

Tambem concentra regras de estado basicas:
- `ativo` para exclusao logica
- `status` para ciclos de vida como agendamento e internacao

Importancia:
- funciona como o nucleo compartilhado do projeto
- permite que todos os modulos usem a mesma modelagem

## `src/main/c/headers/paciente.h`

Declara as funcoes do modulo de pacientes.

Responsabilidades:
- expor o menu de pacientes
- expor funcoes auxiliares do modulo, como cadastro e exclusao logica, se existentes

Importancia:
- separa a interface do modulo da implementacao real em `paciente.c`

## `src/main/c/modules/paciente.c`

Implementa o gerenciamento de pacientes.

Responsabilidades:
- cadastrar paciente
- listar pacientes ativos
- editar paciente
- excluir paciente logicamente

Dados tratados:
- nome
- CPF
- idade
- telefone
- sexo
- regiao administrativa
- status `ativo`

Regras importantes:
- paciente novo entra como ativo
- paciente excluido nao e apagado fisicamente, apenas fica inativo
- listagem ignora pacientes inativos

Importancia:
- e a base de varias outras operacoes, porque triagem, agendamento e internacao dependem do paciente

## `src/main/c/headers/medico.h`

Declara as funcoes do modulo de medicos.

Responsabilidades:
- expor o menu de medicos
- expor funcoes auxiliares de cadastro e exclusao, se existentes

## `src/main/c/modules/medico.c`

Implementa o gerenciamento de medicos.

Responsabilidades:
- cadastrar medico
- listar medicos ativos
- editar medico
- excluir medico logicamente

Dados tratados:
- nome
- CRM
- especialidade
- status `ativo`

Regras importantes:
- medico novo entra como ativo
- exclusao e logica
- listagem ignora medicos inativos

Importancia:
- influencia diretamente o modulo de agendamento
- e a base para futuras melhorias como regionalizacao e especialidade

## `src/main/c/headers/agendamento.h`

Declara as funcoes do modulo de agendamento.

Responsabilidades:
- expor o menu de agendamentos
- expor verificacao de conflito medico
- expor operacoes como cancelamento e conclusao, se existentes

## `src/main/c/modules/agendamento.c`

Implementa o gerenciamento de agendamentos.

Responsabilidades:
- criar agendamento
- listar agendamentos
- cancelar agendamento
- concluir agendamento
- verificar conflito de horario do medico

Dados tratados:
- `pacienteId`
- `medicoId`
- `data`
- `horario`
- `status`

Status usados:
- `AGENDADO`
- `CANCELADO`
- `CONCLUIDO`

Regras importantes:
- nao agenda paciente inativo
- nao agenda medico inativo
- nao permite conflito de horario para medico em agendamento ativo
- cancelamento preserva historico

Importancia:
- e um dos modulos com mais regra de negocio
- ja possui teste automatizado especifico

## `src/main/c/headers/ala.h`

Declara as funcoes do modulo de alas.

Responsabilidades:
- expor o menu de alas
- expor funcoes auxiliares de cadastro e exclusao logica, se existentes

## `src/main/c/modules/ala.c`

Implementa o gerenciamento de alas hospitalares.

Responsabilidades:
- cadastrar ala
- listar alas ativas
- excluir ala logicamente

Dados tratados:
- nome da ala
- tipo
- total de leitos
- leitos ocupados
- status `ativo`

Regras importantes:
- ala nova comeca ativa
- exclusao e logica
- listagem ignora alas inativas

Importancia:
- estrutura a distribuicao fisica do hospital
- serve de base para leitos e internacoes

## `src/main/c/headers/leito.h`

Declara as funcoes do modulo de leitos.

Responsabilidades:
- expor o menu de leitos
- expor `cadastrarLeito(...)`
- expor `excluirLeito(...)`

## `src/main/c/modules/leito.c`

Implementa o gerenciamento de leitos.

Responsabilidades:
- cadastrar leito
- listar leitos ativos
- excluir leito logicamente

Dados tratados:
- `alaId`
- numero do leito
- ocupacao
- `pacienteId`
- status `ativo`

Regras importantes:
- leito novo entra ativo
- leito precisa pertencer a uma ala ativa
- nao pode excluir leito ocupado
- listagem ignora leitos inativos

Importancia:
- conecta a estrutura das alas com a internacao do paciente

## `src/main/c/headers/internacao.h`

Declara as funcoes do modulo de internacao.

Responsabilidades:
- expor o menu de internacoes
- expor `internarPaciente(...)`
- expor `darAltaInternacao(...)`

## `src/main/c/modules/internacao.c`

Implementa o ciclo de vida da internacao.

Responsabilidades:
- internar paciente em leito disponivel
- registrar alta
- listar internacoes

Dados tratados:
- `pacienteId`
- `alaId`
- `leitoId`
- `dataEntrada`
- `dataAlta`
- `status`

Status usados:
- `INTERNADO`
- `ALTA`

Regras importantes:
- paciente precisa existir e estar ativo
- leito precisa existir e estar livre
- ao internar:
  - leito fica ocupado
  - `pacienteId` do leito e preenchido
  - `leitosOcupados` da ala aumenta
- ao dar alta:
  - `status` muda para `ALTA`
  - leito volta a livre
  - `pacienteId` do leito volta para `0`
  - `leitosOcupados` da ala diminui
- historico e preservado; nao ha exclusao fisica

Importancia:
- e um modulo central do fluxo hospitalar
- trata ocupacao real de recursos

## `src/main/c/headers/triagem.h`

Declara as funcoes do modulo de triagem.

Responsabilidades:
- expor o menu de triagem
- expor funcoes de classificacao
- expor exclusao logica
- suportar a evolucao futura para tipos de triagem e submenus especializados

## `src/main/c/modules/triagem.c`

Implementa o gerenciamento de triagem.

Responsabilidades:
- registrar triagem
- calcular pontuacao conforme regras do tipo de triagem
- classificar a gravidade
- listar triagens ativas
- excluir triagem logicamente

Dados tratados:
- `pacienteId`
- tipo da triagem
- pontuacao
- classificacao
- status `ativo`

Classificacoes usadas:
- `Emergencia`
- `Muito prioritario`
- `Prioritario`
- `Comum`
- `Orientacao basica`

Regras importantes:
- triagem depende de paciente ativo
- pontuacao define a classificacao
- exclusao e logica
- listagem ignora triagens inativas

Importancia:
- e a base para futuras regras de prioridade e encaminhamento

## `src/main/c/headers/relatorio.h`

Declara as funcoes do modulo de relatorios.

Responsabilidades:
- expor o menu de relatorios
- expor funcoes auxiliares de contagem, taxa de ocupacao e indicadores, se existentes

## `src/main/c/modules/relatorio.c`

Implementa os relatorios do sistema.

Responsabilidades:
- mostrar totais gerais
- exibir ocupacao por ala
- calcular indicadores simples com os vetores atuais

Indicadores atuais podem incluir:
- total de pacientes
- total de medicos
- total de agendamentos
- total de alas
- total de leitos
- total de internacoes
- total de triagens
- taxa de ocupacao por ala

Importancia:
- transforma os dados cadastrados em informacao gerencial

## Testes Automatizados

Os testes ficam em:

```text
src/test/c/modules/
```

Arquivos atuais:

- `test_agendamento.c`
- `test_ala.c`
- `test_internacao.c`
- `test_leito.c`
- `test_medico.c`
- `test_paciente.c`
- `test_relatorio.c`
- `test_triagem.c`

Cada arquivo de teste valida regras especificas do seu modulo com `assert.h`.

### `test_paciente.c`

Valida:
- cadastro de paciente
- preenchimento correto dos campos
- exclusao logica
- comportamento de paciente ativo e inativo

### `test_medico.c`

Valida:
- cadastro de medico
- preenchimento correto dos campos
- exclusao logica
- comportamento de medico ativo e inativo

### `test_agendamento.c`

Valida:
- conflito de horario do medico
- cancelamento
- conclusao
- efeito do status sobre a disponibilidade do horario

### `test_ala.c`

Valida:
- cadastro de ala
- campos iniciais
- exclusao logica

### `test_leito.c`

Valida:
- cadastro de leito
- vinculacao com ala
- inicio como leito livre e ativo
- exclusao logica
- bloqueio de exclusao de leito ocupado

### `test_internacao.c`

Valida:
- internacao correta
- ocupacao do leito
- atualizacao da ala
- alta
- liberacao do leito
- preservacao do historico
- bloqueio de alta duplicada

### `test_triagem.c`

Valida:
- classificacao da triagem
- pontuacao final conforme o modelo atual
- exclusao logica
- status ativo/inativo

### `test_relatorio.c`

Valida:
- contagens
- ocupacao
- indicadores principais do modulo de relatorio

## Makefile

O projeto usa `Makefile` para compilar e testar.

Comandos principais:

```bash
make clean
make
make run
make test
```

### `make clean`

Remove:
- executavel principal `sigeh`
- executaveis de teste

### `make`

Compila o sistema principal.

### `make run`

Compila e executa o sistema.

### `make test`

Compila e executa os testes automatizados.

## Como Compilar

```bash
make clean
make
```

## Como Executar

```bash
make run
```

Ou, se ja estiver compilado:

```bash
./sigeh
```

## Como Rodar Os Testes

```bash
make test
```

## Regras de Persistencia e Estado

No estado atual:

- os dados ficam em memoria
- nao ha salvamento em arquivo
- nao ha banco de dados
- nao ha SQLite
- nao ha API
- nao ha interface grafica

Isso significa que, ao encerrar o programa, os dados sao perdidos.

## Padroes de Exclusao no Projeto

O projeto usa dois modelos de “remoção”, dependendo do modulo.

### Exclusao logica com `ativo`

Usada em:
- paciente
- medico
- ala
- leito
- triagem

Exemplo:
```c
objeto.ativo = 0;
```

### Ciclo de vida por `status`

Usado em:
- agendamento
- internacao

Exemplos:
- agendamento: `AGENDADO`, `CANCELADO`, `CONCLUIDO`
- internacao: `INTERNADO`, `ALTA`

Esse modelo preserva historico e evita apagar registros relevantes.

## Proximas Evolucoes Planejadas

As proximas evolucoes previstas para o projeto incluem:

- regionalizacao do atendimento
- medicos por regiao
- controle mais forte de especialidade
- painel situacional do DF
- prontuario integrado
- exames integrados
- fila de prioridade por triagem
- persistencia em arquivos `.txt`
- SQLite futuramente

## Limites Atuais do Projeto

Alguns limites importantes da versao atual:

- os dados sao mantidos apenas em memoria
- o sistema ainda depende de `scanf`
- nao ha validacao robusta de entrada
- nao ha autenticacao ou perfil de acesso
- nao ha banco de dados
- nao ha agenda inteligente completa

Mesmo assim, o projeto ja cumpre bem o objetivo academico de praticar organizacao, modelagem e regras de negocio em C.