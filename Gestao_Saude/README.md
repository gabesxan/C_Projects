# SIGEH-DF — Sistema Integrado de Gestão Hospitalar do Distrito Federal

## Resumo do Projeto

O SIGEH-DF é um sistema hospitalar desenvolvido em linguagem C, executado no terminal, com o objetivo de simular um mini ERP hospitalar. O projeto foi construído para a disciplina de Algoritmos e Lógica de Programação e organiza informações relacionadas a pacientes, médicos, agendamentos, alas, leitos, internações, triagens e relatórios.

A proposta do sistema é reunir, em uma única aplicação, operações básicas de gestão hospitalar. Por meio de menus no terminal, o usuário pode cadastrar, consultar, editar e controlar dados essenciais para o funcionamento de uma unidade de saúde, utilizando estruturas e recursos fundamentais da linguagem C.

## Problema Real Abordado

Unidades de saúde lidam diariamente com grande volume de informações operacionais. Quando esses dados ficam espalhados, incompletos ou controlados manualmente, torna-se mais difícil acompanhar atendimentos, organizar equipes e tomar decisões rápidas.

Entre os principais problemas considerados pelo projeto estão:

- dados de pacientes e profissionais distribuídos em controles diferentes;
- formação de filas e dificuldade de acompanhamento dos atendimentos;
- conflitos de agenda entre médico, data e horário;
- controle manual de leitos e alas;
- dificuldade de priorizar casos graves;
- perda de informação entre etapas do atendimento;
- ausência de indicadores simples para apoiar decisões operacionais.

## Objetivo do Sistema

O objetivo do SIGEH-DF é centralizar informações hospitalares básicas e apoiar decisões operacionais iniciais. O sistema não substitui plataformas profissionais de saúde, mas representa, em escala acadêmica, como dados estruturados podem melhorar a organização do atendimento.

Com isso, o projeto busca demonstrar como conceitos de Algoritmos e Lógica de Programação podem ser aplicados em um problema próximo da realidade, envolvendo cadastro, busca, validação de regras, controle de estados e geração de informações para consulta.

## Diferenciais do Projeto

### Contexto do Distrito Federal

O SIGEH-DF considera o contexto territorial do Distrito Federal ao registrar a região administrativa dos pacientes. Essa informação permite que o sistema vá além do cadastro individual e abra espaço para análises sobre a origem da demanda hospitalar.

Em uma evolução futura, essa base pode contribuir para uma proposta de regionalização do atendimento, identificando quais regiões concentram mais pacientes, quais especialidades são mais procuradas e onde há maior incidência de casos graves.

### Painel Situacional do DF

O principal diferencial planejado para o projeto é o **Painel Situacional do DF**. A proposta é transformar cadastros hospitalares em informações úteis para gestão pública em saúde, reunindo indicadores simples e objetivos sobre atendimento, ocupação e demanda.

Entre os indicadores planejados para o painel estão:

- pacientes por região administrativa;
- triagens por classificação;
- taxa de ocupação por ala;
- leitos disponíveis;
- especialidade mais demandada;
- região com mais casos graves.

### Redução de Tempo de Espera

Ao organizar dados de pacientes, médicos, agendas, triagens e leitos, o sistema cria uma base para reduzir atrasos e retrabalho. A validação de conflitos de agendamento, por exemplo, evita que um mesmo médico seja associado a atendimentos simultâneos.

Da mesma forma, a triagem permite classificar atendimentos por gravidade, enquanto o controle de leitos e alas contribui para identificar disponibilidade de internação. Esses recursos ajudam a representar como a organização da informação pode apoiar a redução do tempo de espera em uma unidade de saúde.

## Funcionalidades Atuais

O projeto possui funcionalidades implementadas e estruturas iniciais para expansão:

- cadastro de pacientes;
- listagem de pacientes;
- edição de pacientes;
- exclusão lógica de pacientes;
- cadastro de médicos;
- listagem de médicos;
- edição de médicos;
- exclusão lógica de médicos;
- agendamento simples;
- validação de conflito por médico, data e horário;
- cancelamento de agendamentos;
- conclusão de agendamentos;
- estruturas base para alas;
- estruturas base para leitos;
- estruturas base para internações;
- estruturas base para triagem;
- estruturas base para relatórios.

## Tecnologias e Conceitos de ALP Utilizados

O SIGEH-DF utiliza conceitos fundamentais estudados em Algoritmos e Lógica de Programação, aplicados de forma progressiva no desenvolvimento do sistema:

- linguagem C;
- `structs`;
- vetores;
- constantes com `#define`;
- strings com `char[]`;
- laços de repetição;
- condicionais;
- `switch case`;
- busca linear;
- validação de regras de negócio;
- exclusão lógica;
- organização progressiva do código.

## Estrutura Geral do Sistema

O sistema é executado no terminal e organizado por menus. Cada módulo representa uma área de gestão hospitalar, permitindo que o usuário navegue entre cadastros, consultas e operações específicas.

A estrutura atual concentra o código no arquivo `main.c`, com dados armazenados em memória durante a execução. Essa escolha é compatível com a etapa inicial do projeto e com os objetivos da disciplina, permitindo focar na lógica de programação antes de avançar para persistência e modularização.

## Como Compilar

Para compilar o projeto, utilize:

```bash
gcc -Wall -Wextra -pedantic main.c -o sigeh
```

## Como Executar

Após a compilação, execute:

```bash
./sigeh
```

## Próximas Etapas

As próximas melhorias planejadas para o SIGEH-DF são:

- refatorar o código em arquivos separados;
- criar testes automatizados com `assert.h`;
- implementar persistência em arquivos `.txt`;
- implementar o Painel Situacional do DF;
- ampliar os indicadores por região administrativa;
- melhorar o fluxo de triagem e priorização;
- planejar migração futura para SQLite.

## Observação Acadêmica

O SIGEH-DF é um projeto acadêmico e didático. Seu objetivo principal é demonstrar a aplicação de estruturas de dados simples, validações e organização lógica em linguagem C, usando como tema a gestão hospitalar no contexto do Distrito Federal.
