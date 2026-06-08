# SIGEH-DF

## Sistema Integrado de Gestao Hospitalar em C

O **SIGEH-DF** e um projeto academico em linguagem C, executado no terminal, desenvolvido para a disciplina de **Algoritmos e Logica de Programacao**. A proposta do sistema e simular, de forma modular e didatica, o funcionamento basico de um ambiente hospitalar, com foco em organizacao de codigo, regras de negocio e manipulacao de estruturas em memoria.

> Estado atual do projeto:
>
> - execucao em terminal
> - dados mantidos em memoria durante a execucao
> - modularizacao em `.h` e `.c`
> - compilacao com `Makefile`
> - testes simples com `assert.h`
> - sem banco de dados
> - sem persistencia em arquivo
> - sem interface grafica
> - sem API externa

---

## Indice

1. [Visao Geral](#visao-geral)
2. [Objetivos Didaticos](#objetivos-didaticos)
3. [Funcionalidades Atuais](#funcionalidades-atuais)
4. [Arquitetura do Projeto](#arquitetura-do-projeto)
5. [Estrutura de Pastas](#estrutura-de-pastas)
6. [Como Compilar e Executar](#como-compilar-e-executar)
7. [Como Rodar os Testes](#como-rodar-os-testes)
8. [Mapa de Arquivos](#mapa-de-arquivos)
9. [Explicacao Detalhada Arquivo por Arquivo](#explicacao-detalhada-arquivo-por-arquivo)
10. [Regras de Modelagem e Estados](#regras-de-modelagem-e-estados)
11. [Observacoes Importantes](#observacoes-importantes)
12. [Proximos Passos Naturais do Projeto](#proximos-passos-naturais-do-projeto)

---

## Visao Geral

O SIGEH-DF foi estruturado para representar rotinas centrais de um sistema hospitalar simplificado. O projeto foi dividido em modulos pequenos, cada um com responsabilidade bem definida, para facilitar manutencao, leitura e avaliacao academica.

A aplicacao permite trabalhar com:

- pacientes
- medicos
- agendamentos
- alas
- leitos
- internacoes
- triagens
- prontuarios
- relatorios gerenciais

A ideia central do projeto nao e reproduzir um hospital real com toda a sua complexidade, mas sim construir uma base didatica capaz de demonstrar:

- uso de `structs`
- vetores globais
- funcoes com responsabilidades separadas
- menus em terminal com `switch`
- estados logicos como `ativo`, `AGENDADO`, `INTERNADO` e `REMANEJADO`
- relacionamento entre modulos
- validacoes de regras de negocio
- testes automatizados simples

---

## Objetivos Didaticos

Este projeto foi construido para praticar, em C:

- modelagem de entidades com `struct`
- compartilhamento de dados via `extern`
- separacao entre declaracao e implementacao
- organizacao por modulos
- leitura e exibicao de dados em terminal
- validacoes de fluxo
- testes com `assert.h`
- evolucao incremental de funcionalidades sem trocar a arquitetura

---

## Funcionalidades Atuais

Atualmente, o sistema oferece:

- cadastro, listagem, edicao e exclusao logica de pacientes
- cadastro, listagem, edicao e exclusao logica de medicos
- agendamento orientado por triagem, especialidade, disponibilidade e regiao
- cancelamento, conclusao e remanejamento de agendamentos
- cadastro e gerenciamento de alas por tipo numerico
- cadastro e gerenciamento de leitos
- internacao e alta hospitalar
- triagem geral e especializada
- painel de relatorios com indicadores do sistema
- prontuario integrado vinculado a paciente e medico
- listagem de prontuarios por paciente, medico e especialidade

---

## Arquitetura do Projeto

A arquitetura atual do SIGEH-DF segue o padrao:

```text
app / model / headers / modules / test
```

### `src/main/c/app`
Contem o ponto de entrada do programa.

### `src/main/c/model`
Contem a modelagem central do sistema:

- `#define`
- `structs`
- vetores globais
- contadores globais
- constantes compartilhadas

### `src/main/c/headers`
Contem os arquivos `.h` dos modulos, com as assinaturas publicas das funcoes.

### `src/main/c/modules`
Contem a implementacao de cada modulo em arquivos `.c`.

### `src/test/c/modules`
Contem testes automatizados simples com `assert.h`, voltados para as regras principais de cada modulo.

---

## Estrutura de Pastas

```text
Gestao_Saude/
├── .gitignore
├── Makefile
├── README.md
├── data/
├── sigeh
├── test_agendamento
├── test_ala
├── test_internacao
├── test_leito
├── test_medico
├── test_paciente
├── test_prontuario
├── test_relatorio
├── test_triagem
└── src/
    ├── main/
    │   └── c/
    │       ├── app/
    │       │   └── main.c
    │       ├── headers/
    │       │   ├── agendamento.h
    │       │   ├── ala.h
    │       │   ├── internacao.h
    │       │   ├── leito.h
    │       │   ├── medico.h
    │       │   ├── paciente.h
    │       │   ├── prontuario.h
    │       │   ├── relatorio.h
    │       │   └── triagem.h
    │       ├── model/
    │       │   └── hospital.h
    │       └── modules/
    │           ├── agendamento.c
    │           ├── ala.c
    │           ├── internacao.c
    │           ├── leito.c
    │           ├── medico.c
    │           ├── paciente.c
    │           ├── prontuario.c
    │           ├── relatorio.c
    │           └── triagem.c
    └── test/
        └── c/
            └── modules/
                ├── test_agendamento.c
                ├── test_ala.c
                ├── test_internacao.c
                ├── test_leito.c
                ├── test_medico.c
                ├── test_paciente.c
                ├── test_prontuario.c
                ├── test_relatorio.c
                └── test_triagem.c
```

---

## Como Compilar e Executar

Na raiz do projeto:

```sh
make clean
make
make run
```

### O que cada comando faz

- `make clean`: remove executavel principal e executaveis de teste
- `make`: compila o sistema principal e gera o binario `sigeh`
- `make run`: compila e executa o sistema

---

## Como Rodar os Testes

```sh
make test
```

Esse alvo compila e executa os testes modulares:

- `test_agendamento`
- `test_ala`
- `test_internacao`
- `test_leito`
- `test_medico`
- `test_paciente`
- `test_prontuario`
- `test_relatorio`
- `test_triagem`

Observacao importante:

- os testes cobrem principalmente funcoes de regra de negocio
- menus interativos e mensagens impressas no terminal ainda dependem de validacao manual

---

## Mapa de Arquivos

### Arquivos centrais

- `Makefile`: compilacao, limpeza e execucao dos testes
- `README.md`: documentacao do projeto
- `src/main/c/app/main.c`: menu principal e definicoes globais
- `src/main/c/model/hospital.h`: modelagem compartilhada do sistema

### Headers publicos

- `agendamento.h`
- `ala.h`
- `internacao.h`
- `leito.h`
- `medico.h`
- `paciente.h`
- `prontuario.h`
- `relatorio.h`
- `triagem.h`

### Modulos de implementacao

- `agendamento.c`
- `ala.c`
- `internacao.c`
- `leito.c`
- `medico.c`
- `paciente.c`
- `prontuario.c`
- `relatorio.c`
- `triagem.c`

### Testes

- `test_agendamento.c`
- `test_ala.c`
- `test_internacao.c`
- `test_leito.c`
- `test_medico.c`
- `test_paciente.c`
- `test_prontuario.c`
- `test_relatorio.c`
- `test_triagem.c`

### Arquivos e pastas auxiliares

- `.gitignore`: define arquivos ignorados pelo Git
- `.vscode/settings.json`: preferencias locais do ambiente VS Code
- `data/`: pasta reservada para dados auxiliares do projeto
- `sigeh`: executavel principal gerado pelo `make`
- `test_*`: executaveis de teste gerados pelo `make test`

---

## Explicacao Detalhada Arquivo por Arquivo

### Arquivos da raiz

#### `Makefile`
Responsavel por toda a orquestracao de compilacao.

O `Makefile`:

- define o compilador (`gcc`)
- aplica `-Wall -Wextra -pedantic`
- configura os `-I` para `model`, `headers` e `modules`
- define os arquivos do executavel principal
- define os arquivos de cada teste modular
- possui os alvos `all`, `run`, `clean`, `test` e `tree`

Em termos praticos, ele e o arquivo que transforma a estrutura modular do projeto em um executavel utilizavel e em testes isolados.

#### `README.md`
Arquivo de documentacao principal do projeto.

Sua funcao e explicar:

- a proposta do sistema
- a arquitetura adotada
- a organizacao das pastas
- a funcao de cada arquivo
- os comandos de compilacao e teste
- as regras principais do dominio

#### `.gitignore`
Arquivo que informa ao Git quais arquivos nao devem ser versionados.

Normalmente ele serve para evitar que binarios e artefatos temporarios sejam enviados para o repositorio.

#### `.vscode/settings.json`
Arquivo de configuracao local do editor VS Code.

Ele nao faz parte da logica do sistema, mas ajuda a manter uma experiencia de edicao mais estavel no ambiente do desenvolvedor.

#### `data/`
Pasta de apoio para dados ou estruturas auxiliares do projeto.

Mesmo quando nao estiver sendo usada diretamente por todos os modulos, ela serve como espaco reservado para evolucoes futuras ou suporte ao projeto.

#### `sigeh`
Executavel principal gerado pelo `make`.

Nao e codigo-fonte, mas sim o resultado compilado do projeto.

#### `test_agendamento`, `test_ala`, `test_internacao`, `test_leito`, `test_medico`, `test_paciente`, `test_prontuario`, `test_relatorio`, `test_triagem`
Executaveis de teste gerados pelo alvo `make test`.

Eles representam binarios temporarios usados para validar cada modulo individualmente.

---

### `src/main/c/app/main.c`
Ponto de entrada do sistema.

Este arquivo cumpre dois papeis fundamentais:

1. definir os vetores globais e contadores globais do sistema
2. controlar o menu principal da aplicacao

Funcoes principais:

- `exibirMenu()`: imprime o menu principal
- `executarOpcao(int opcao)`: despacha a opcao escolhida para o modulo correspondente
- `main()`: controla o loop principal do sistema

Tambem e neste arquivo que ficam definidas as instancias globais de:

- `pacientes`
- `medicos`
- `agendamentos`
- `alas`
- `leitos`
- `internacoes`
- `triagens`
- `prontuarios`

Sem esse arquivo, os `extern` declarados em `hospital.h` nao teriam definicao real no programa principal.

---

### `src/main/c/model/hospital.h`
Nucleo de modelagem do SIGEH-DF.

Este e o arquivo mais importante do projeto em nivel estrutural, porque centraliza:

- bibliotecas padrao (`stdio.h`, `stdlib.h`, `string.h`)
- limites dos vetores (`MAX_*`)
- codigos de tipos de triagem
- codigos de tipos de ala
- definicao das structs principais
- declaracoes `extern` dos vetores globais
- declaracoes `extern` dos contadores globais

Constantes relevantes:

- `MAX_PACIENTES`
- `MAX_MEDICOS`
- `MAX_AGENDAMENTOS`
- `MAX_ALAS`
- `MAX_LEITOS`
- `MAX_INTERNACOES`
- `MAX_TRIAGENS`
- `MAX_PRONTUARIOS`

Tipos de triagem:

- `TRIAGEM_GERAL`
- `TRIAGEM_ORTOPEDIA`
- `TRIAGEM_CARDIOLOGIA`
- `TRIAGEM_PNEUMOLOGIA`
- `TRIAGEM_PEDIATRIA`

Tipos de ala:

- `ALA_INTERNACAO`
- `ALA_UTI`
- `ALA_OBSERVACAO`
- `ALA_PEDIATRIA`
- `ALA_CIRURGICA`

Structs definidas:

- `Paciente`
- `Medico`
- `Agendamento`
- `Ala`
- `Leito`
- `Internacao`
- `Triagem`
- `Prontuario`

Esse arquivo funciona como contrato central da aplicacao inteira.

---

### Headers de modulos

#### `src/main/c/headers/paciente.h`
Declara a interface publica do modulo de pacientes.

Funcoes expostas:

- `menuPacientes()`
- `cadastrarPaciente(...)`
- `excluirPaciente(int id)`

Ele permite que outros arquivos conhecam o modulo de pacientes sem acessar sua implementacao interna.

#### `src/main/c/headers/medico.h`
Declara a interface publica do modulo de medicos.

Funcoes expostas:

- `menuMedicos()`
- `cadastrarMedico(...)`
- `excluirMedico(int id)`

#### `src/main/c/headers/agendamento.h`
Declara a interface publica do modulo de agendamento.

Funcoes expostas:

- `menuAgendamentos()`
- `buscarAgenda(...)`
- `medicoOcupado(...)`
- `cancelarAgendamento(int id)`
- `concluirAgendamento(int id)`
- `trocaHorario(...)`
- `agendarMedico(...)`
- `obterEspecialidade(...)`
- `buscarMedicoRegiao(...)`
- `buscarMedico(...)`
- `agendarTriagem(...)`

Esse header concentra a parte mais rica de regras operacionais entre triagem, medicos, horario e remanejamento.

#### `src/main/c/headers/ala.h`
Declara a interface do modulo de alas.

Funcoes expostas:

- `menuAlas()`
- `cadastrarAla(...)`
- `excluirAla(int id)`
- `contarAlasPorTipo(int tipo)`
- `listarAlasPorTipo(int tipo)`

#### `src/main/c/headers/leito.h`
Declara a interface do modulo de leitos.

Funcoes expostas:

- `menuLeitos()`
- `cadastrarLeito(...)`
- `excluirLeito(int id)`

#### `src/main/c/headers/internacao.h`
Declara a interface do modulo de internacao.

Funcoes expostas:

- `menuInternacoes()`
- `internarPaciente(...)`
- `darAltaInternacao(...)`

#### `src/main/c/headers/triagem.h`
Declara a interface do modulo de triagem.

Funcoes expostas:

- `menuTriagem()`
- `classificarTriagem(...)`
- `nivelPrioridade(...)`
- `ehUrgente(...)`
- `triagemAtual(...)`
- `excluirTriagem(...)`
- `escolherTriagem()`
- `triagemGeral(...)`
- `triagemOrtopedia(...)`
- `triagemCardiologia(...)`
- `triagemPneumologia(...)`
- `triagemPediatria(...)`
- `exibirTipo(...)`

#### `src/main/c/headers/relatorio.h`
Declara a interface publica do modulo de relatorios.

Funcoes expostas:

- `menuRelatorios()`
- `contarLeitosOcupados()`
- `contarLivres()`
- `taxaAla(int alaId)`
- `contarTriagens(...)`
- `contarMedRegiao(...)`
- `relMedRegiao()`
- `contarPacRegiao(...)`
- `relPacRegiao()`
- `contarEsp(...)`
- `espDemandada(...)`
- `contarCasosGravesRegiao(...)`
- `regiaoMaisCasosGraves()`

#### `src/main/c/headers/prontuario.h`
Declara a interface do modulo de prontuario.

Funcoes expostas:

- `menuProntuarios()`
- `registrarProntuario(...)`
- `listarProntuarioPorPaciente(...)`
- `listarProntuarioPorMedico(...)`
- `listarProntuarioPorEspecialidade(...)`

---

### Modulos de implementacao

#### `src/main/c/modules/paciente.c`
Responsavel por toda a gestao de pacientes.

Operacoes principais:

- cadastro
- listagem
- edicao
- exclusao logica
- exibicao das regioes administrativas no fluxo do menu

Dados tratados pelo modulo:

- nome
- CPF
- idade
- telefone
- sexo
- regiao administrativa
- estado `ativo`

Importancia:

Esse modulo e base de varios outros. Sem pacientes validos e ativos, o sistema nao consegue realizar triagem, agendamento, internacao ou prontuario.

#### `src/main/c/modules/medico.c`
Responsavel pela gestao de medicos.

Operacoes principais:

- cadastro
- listagem
- edicao
- exclusao logica
- controle de especialidade
- controle de regiao administrativa

Importancia:

Esse modulo influencia diretamente agendamento, regionalizacao e prontuario.

#### `src/main/c/modules/agendamento.c`
Modulo central do fluxo ambulatorial.

Responsabilidades:

- localizar conflitos de horario
- verificar se um medico esta ocupado
- cancelar agendamento
- concluir agendamento
- descobrir a especialidade correta a partir da triagem
- buscar medico na mesma regiao
- buscar medico em outra regiao quando necessario
- decidir se um paciente mais urgente pode remanejar outro
- registrar agendamento novo
- marcar agendamento antigo como `REMANEJADO`

Regras importantes:

- o conflito relevante considera agendamentos com status `AGENDADO`
- a urgencia depende da classificacao da triagem
- `Emergencia` e `Muito prioritario` sao tratados como urgentes
- um paciente so remaneja outro se tiver prioridade maior
- em caso de empate, o agendamento ja existente e mantido

#### `src/main/c/modules/ala.c`
Modulo que representa a organizacao fisica das alas.

Responsabilidades:

- cadastrar alas
- excluir alas logicamente
- listar alas
- contar alas por tipo
- listar alas filtradas por tipo
- receber o tipo da ala via numeracao de menu

Tipos de ala suportados:

- Internacao
- UTI
- Observacao
- Pediatria
- Cirurgica

Esse modulo ajuda a estruturar o ambiente hospitalar em categorias padronizadas e evita erro de digitacao ao selecionar tipos.

#### `src/main/c/modules/leito.c`
Responsavel pelo gerenciamento de leitos.

Operacoes principais:

- cadastro de leito
- exclusao logica
- vinculacao a ala
- controle de ocupacao

Papel no sistema:

Ele funciona como ponte entre a estrutura fisica da ala e a internacao do paciente.

#### `src/main/c/modules/internacao.c`
Responsavel pelo fluxo de internacao e alta.

Operacoes principais:

- internar paciente em leito disponivel
- registrar data de entrada
- dar alta
- registrar data de alta
- liberar leito
- reduzir `leitosOcupados` da ala correspondente

Regras importantes:

- o paciente precisa existir e estar ativo
- o leito precisa existir e estar livre
- a internacao recebe status `INTERNADO`
- a alta altera o status da internacao e libera o leito

#### `src/main/c/modules/triagem.c`
Modulo responsavel por classificacao de risco.

Responsabilidades:

- escolher o tipo de triagem
- aplicar perguntas por especialidade
- calcular pontuacao
- converter pontuacao em classificacao
- informar prioridade numerica
- dizer se uma classificacao e urgente
- localizar a triagem ativa mais recente de um paciente

Tipos implementados:

- triagem geral
- triagem ortopedica
- triagem cardiologica
- triagem pneumologica
- triagem pediatrica

Esse modulo alimenta diretamente as decisoes de agendamento e prioridade.

#### `src/main/c/modules/relatorio.c`
Modulo de indicadores gerenciais.

Responsabilidades:

- contar leitos ocupados e livres
- calcular taxa de ocupacao por ala
- contar triagens por classificacao
- contar medicos por regiao
- contar pacientes por regiao
- identificar especialidade mais demandada
- identificar regiao com mais casos graves
- exibir painel consolidado de relatorios

Esse modulo transforma os dados operacionais em visao gerencial do sistema.

#### `src/main/c/modules/prontuario.c`
Modulo de prontuario integrado.

Responsabilidades:

- validar paciente e medico ativos antes do registro
- registrar atendimento clinico
- armazenar observacoes, diagnostico, conduta e alerta importante
- listar prontuarios por paciente
- listar prontuarios por medico
- listar prontuarios por especialidade
- exibir pacientes e medicos ativos para facilitar o cadastro no menu

Ponto forte do modulo:

Ele nao duplica nome de paciente ou medico dentro da struct. O prontuario guarda os ids e consulta os dados reais do sistema na hora de listar, preservando coerencia.

---

### Testes modulares

#### `src/test/c/modules/test_paciente.c`
Valida as operacoes principais do modulo de pacientes, como cadastro e exclusao.

#### `src/test/c/modules/test_medico.c`
Valida cadastro e exclusao logica de medicos.

#### `src/test/c/modules/test_agendamento.c`
Valida o modulo mais rico em regra de negocio.

Cenarios cobertos:

- busca de agenda
- ocupacao de medico
- cancelamento publico
- agendamento por triagem
- remanejamento
- prioridade
- empate de prioridade
- escolha de medico por disponibilidade e regiao

#### `src/test/c/modules/test_ala.c`
Valida cadastro de ala, tipo numerico, contagem por tipo e exclusao.

#### `src/test/c/modules/test_leito.c`
Valida cadastro e operacoes basicas do modulo de leitos.

#### `src/test/c/modules/test_internacao.c`
Valida internacao, alta, liberacao de leito e reducao de ocupacao.

#### `src/test/c/modules/test_triagem.c`
Valida classificacao, prioridade, urgencia e obtencao da triagem atual.

#### `src/test/c/modules/test_relatorio.c`
Valida indicadores do painel de relatorios.

Cenarios cobertos:

- ocupacao de leitos
- taxa por ala
- contagem por classificacao
- medicos por regiao
- pacientes por regiao
- especialidade mais demandada
- casos graves por regiao

#### `src/test/c/modules/test_prontuario.c`
Valida o modulo de prontuario.

Cenarios cobertos:

- registro valido
- associacao correta entre paciente e medico
- armazenamento correto dos campos
- falha com paciente invalido
- falha com medico invalido

---

## Regras de Modelagem e Estados

### Exclusao logica

O projeto usa exclusao logica em varias entidades.

Campos comuns:

- `Paciente.ativo`
- `Medico.ativo`
- `Ala.ativo`
- `Leito.ativo`
- `Triagem.ativo`
- `Prontuario.ativo`

Isso significa que o registro nao e apagado do vetor, apenas deixa de ser considerado ativo.

### Status usados no sistema

#### Agendamento

- `AGENDADO`
- `CANCELADO`
- `CONCLUIDO`
- `REMANEJADO`

#### Internacao

- `INTERNADO`
- `ALTA`

### Classificacoes de triagem

- `Emergencia`
- `Muito prioritario`
- `Prioritario`
- `Comum`
- `Orientacao basica`

### Tipos de ala

- `ALA_INTERNACAO`
- `ALA_UTI`
- `ALA_OBSERVACAO`
- `ALA_PEDIATRIA`
- `ALA_CIRURGICA`

---

## Observacoes Importantes

- Os dados continuam em memoria durante a execucao.
- O sistema ainda nao possui persistencia em arquivo.
- O sistema ainda nao utiliza banco relacional.
- Os testes automatizados cobrem bem regras centrais, mas nao substituem testes manuais dos menus.
- O projeto foi construido para clareza didatica, nao para alta performance ou concorrencia.
- O uso de vetores globais faz parte da proposta academica atual do sistema.

---

## Proximos Passos Naturais do Projeto

A base atual permite evolucoes como:

- fortalecer ainda mais o modulo de prontuario
- criar modulo de exames integrados
- ampliar filtros e consultas do prontuario
- adicionar persistencia em `.txt`
- depois avaliar migracao futura para SQLite
- ampliar cobertura de testes manuais e automatizados

---

## Fechamento

O SIGEH-DF e um projeto academico simples na superficie, mas com uma base modular bem rica para estudo. Ele ja demonstra integracao entre entidades, validacao de regras, menus operacionais, indicadores gerenciais e evolucao incremental de arquitetura sem abandonar a simplicidade do C basico.

A leitura do codigo fica muito mais proveitosa quando feita junto desta documentacao, porque cada arquivo tem uma responsabilidade clara dentro do sistema.
