# SIGEH-DF — Sistema Integrado de Gestão Hospitalar do Distrito Federal

## 1. Visão Geral

O SIGEH-DF é um sistema hospitalar desenvolvido em linguagem C e executado no terminal. Ele foi criado como projeto acadêmico para a disciplina de Algoritmos e Lógica de Programação, com o objetivo de aplicar conceitos fundamentais da programação estruturada em um problema próximo da realidade: a organização de informações em uma unidade hospitalar.

O sistema simula um mini ERP hospitalar. Em vez de tratar apenas cadastros isolados, ele organiza módulos relacionados entre si, como pacientes, médicos, agendamentos, alas, leitos, internações, triagens e relatórios. Cada módulo representa uma parte do fluxo operacional de atendimento, desde o cadastro inicial até a ocupação de leitos e geração de informações para apoio à gestão.

A escolha da linguagem C e da execução em terminal é intencional. O projeto prioriza lógica, estruturas de dados simples, controle de fluxo, modularização e validações de regras de negócio antes de avançar para interfaces gráficas, banco de dados ou tecnologias mais complexas. Assim, o foco principal fica na construção do raciocínio algorítmico.

## 2. Problema Real

Hospitais e unidades de saúde lidam diariamente com grande volume de informações. Quando esses dados são mantidos em controles manuais, planilhas separadas ou registros sem integração, a gestão do atendimento se torna mais lenta, sujeita a falhas e difícil de acompanhar.

No contexto hospitalar, um dado isolado raramente é suficiente. Um paciente precisa estar associado a atendimentos, triagens, internações e possíveis agendamentos. Um médico precisa ter agenda organizada e especialidade definida. Um leito precisa estar vinculado a uma ala e precisa indicar se está livre ou ocupado. Quando essas relações não são bem controladas, surgem conflitos e perda de informação entre etapas do atendimento.

O SIGEH-DF simula esse problema em escala acadêmica, abordando pontos como:

- dados hospitalares espalhados entre diferentes controles;
- controle manual de pacientes, médicos, alas e leitos;
- conflitos de agenda entre médico, data e horário;
- dificuldade de identificar pacientes prioritários;
- perda de informação entre triagem, atendimento e internação;
- dificuldade de enxergar ocupação de leitos;
- dificuldade de analisar demanda por região administrativa.

## 3. Proposta de Solução

A proposta do SIGEH-DF é centralizar informações hospitalares em um único sistema em memória, usando vetores de `structs` para representar os principais dados operacionais. O sistema ainda não utiliza arquivos ou banco de dados, mas já organiza as entidades e regras de negócio de forma estruturada.

O usuário interage com o sistema por menus no terminal. Cada menu corresponde a um módulo do sistema, como pacientes, médicos, agendamentos ou internações. Essa divisão facilita a navegação e também reforça a modularização do código.

As entidades são relacionadas por IDs. Por exemplo, um agendamento liga um paciente a um médico; uma internação liga paciente, ala e leito; um leito pertence a uma ala. Esse modelo permite representar relacionamentos sem usar banco de dados, mantendo o projeto adequado à etapa atual da disciplina.

Além disso, o sistema valida regras de negócio antes de executar operações. Ele verifica, por exemplo, se o paciente está ativo, se o médico está ativo, se há conflito de agendamento e se um leito já está ocupado. Essas validações tornam o projeto mais próximo de um sistema real.

## 4. Contexto do Distrito Federal

Um diferencial importante do SIGEH-DF é considerar o território do Distrito Federal. O cadastro de pacientes inclui a região administrativa, permitindo registrar de onde vem a demanda hospitalar.

Atualmente, o sistema trabalha com exemplos de regiões administrativas como Plano Piloto, Ceilândia, Taguatinga, Samambaia, Gama, Sobradinho, Guará e Águas Claras. Essa informação amplia o valor do cadastro de pacientes, pois permite pensar em análises futuras sobre concentração de atendimentos, origem dos casos graves e demanda por região.

Essa característica abre caminho para o Painel Situacional do DF, uma evolução planejada que poderá transformar dados operacionais em indicadores para apoio à gestão. Em etapas futuras, médicos também poderão ser vinculados a regiões de atendimento, simulando regionalização do atendimento público e distribuição mais inteligente de recursos.

## 5. Diferenciais do Projeto

### 5.1 Painel Situacional do DF

O SIGEH-DF não foi pensado apenas como um CRUD. Embora possua cadastros, listagens, edições e exclusões lógicas, a proposta maior é criar uma base de dados organizada que possa ser usada para gerar indicadores.

O Painel Situacional do DF é uma funcionalidade planejada para consolidar informações úteis à gestão hospitalar. A ideia é transformar dados inseridos nos módulos em uma visão mais ampla da situação da unidade.

Indicadores planejados incluem:

- pacientes por região administrativa;
- triagens por classificação;
- leitos disponíveis;
- taxa de ocupação por ala;
- especialidade mais demandada;
- região com mais casos graves.

### 5.2 Redução do Tempo de Espera

Um sistema hospitalar organizado pode contribuir para reduzir tempo de espera ao melhorar o fluxo de informações. No SIGEH-DF, essa ideia aparece principalmente na triagem, nos agendamentos e no controle de leitos.

A triagem calcula uma pontuação com base em sintomas e idade, permitindo classificar pacientes por prioridade. Em uma evolução futura, essa pontuação poderá alimentar uma fila de prioridade.

O agendamento evita conflito de horário para o mesmo médico, e futuras melhorias poderão buscar médico disponível por especialidade. O controle de leitos também contribui para identificar leitos livres e ocupados, além de permitir alertas de lotação em etapas futuras.

### 5.3 Continuidade do Cuidado

Outra evolução planejada é o prontuário integrado. A ideia é que informações clínicas fiquem associadas ao paciente, permitindo que outro médico acesse o histórico anterior sem depender apenas de relatos repetidos ou registros em papel.

Esse recurso ainda não está implementado, mas representa uma evolução natural do sistema. Ele ajudaria a reduzir perda de informação entre atendimentos, melhorar a continuidade do cuidado e organizar melhor o histórico do paciente.

### 5.4 Exames Integrados

Os exames integrados também fazem parte das funcionalidades planejadas. A proposta é permitir que um médico solicite um exame, que esse exame fique pendente no sistema, que um responsável registre o resultado e que o médico consiga consultar esse resultado posteriormente.

Nessa evolução, o resultado ficaria vinculado ao paciente, contribuindo para um fluxo mais completo de atendimento e para a integração entre módulos clínicos.

## 6. Entidades do Sistema

### Paciente

Representa a pessoa atendida pelo sistema. Possui ID, nome, CPF, idade, telefone, sexo, região administrativa e campo `ativo`. O campo `ativo` permite aplicar exclusão lógica, mantendo o registro no vetor, mas impedindo seu uso como paciente válido em operações futuras.

### Médico

Representa o profissional de atendimento. Possui ID, nome, CRM, especialidade e campo `ativo`. A especialidade é registrada como texto nesta etapa, mas poderá ser transformada em código numérico no futuro para facilitar busca, validação e relatórios.

### Agendamento

Representa a ligação entre um paciente e um médico em uma data e horário. Possui ID, `pacienteId`, `medicoId`, data, horário e status. O sistema valida conflito considerando médico, data, horário e status do agendamento.

### Ala

Representa um setor hospitalar. Possui ID, nome, tipo, total de leitos, quantidade de leitos ocupados e campo `ativo`. Esse módulo permite acompanhar a capacidade de uma área do hospital.

### Leito

Representa uma cama hospitalar. Possui ID, ID da ala, número, indicador de ocupação e ID do paciente associado quando estiver ocupado. O leito pode estar livre ou ocupado, e essa informação é usada nas internações.

### Internação

Representa a ligação entre paciente, ala e leito. Controla data de entrada, data de alta e status. Ao internar um paciente, o leito passa a ficar ocupado; ao registrar alta, o leito é liberado.

### Triagem

Representa a avaliação inicial de risco do paciente. Armazena sintomas, pontuação e classificação. A pontuação é calculada com base em sinais informados e idade do paciente.

## 7. Regras de Negócio

O sistema possui regras de negócio simples, mas importantes para simular uma operação hospitalar coerente.

Um mesmo médico não pode ter dois agendamentos ativos no mesmo dia e horário. A validação compara o ID do médico, a data e o horário. Médicos diferentes podem atender no mesmo dia e horário, pois o conflito é individual por profissional.

Agendamentos cancelados não geram conflito. Isso permite reutilizar um horário quando o agendamento anterior foi marcado como `CANCELADO`.

Pacientes inativos não podem ser usados como pacientes válidos em operações que exigem um paciente ativo. Isso impede editar um paciente já removido logicamente, excluí-lo novamente ou utilizá-lo em novos processos.

Médicos inativos não devem ser usados em agendamentos. Antes de criar um agendamento, o sistema verifica se o médico existe e se está ativo.

Um leito ocupado não pode receber outro paciente. Durante uma internação, o sistema verifica se o leito existe e se ainda está livre. Ao registrar alta, o leito é liberado e o paciente associado ao leito volta a ser zero.

A triagem soma pontos com base em sintomas e idade. Falta de ar, dor intensa, febre, pressão alta e idade de risco contribuem para a pontuação final, que define a classificação do atendimento.

A exclusão lógica usa `ativo = 0`. Essa decisão evita deslocar elementos no vetor e mantém o projeto adequado ao nível atual de ALP.

## 8. Módulos Implementados

### Pacientes

Permite cadastrar, listar, editar e remover logicamente pacientes. Também registra a região administrativa do paciente, o que cria base para análises territoriais futuras.

### Médicos

Permite cadastrar, listar, editar e remover logicamente médicos. Registra CRM e especialidade, dados importantes para controle profissional e futuras buscas por especialidade.

### Agendamentos

Permite criar, listar, cancelar e concluir agendamentos. O módulo valida conflito para impedir que o mesmo médico tenha dois atendimentos ativos no mesmo dia e horário.

### Alas

Permite cadastrar, listar e remover logicamente alas. Cada ala possui total de leitos e quantidade de leitos ocupados, servindo como base para controle de capacidade.

### Leitos

Permite cadastrar e listar leitos. Cada leito pertence a uma ala e pode estar livre ou ocupado. A ocupação e liberação são controladas pelo módulo de internações.

### Internações

Permite internar paciente, dar alta e listar internações. Ao internar, o sistema ocupa o leito; ao dar alta, libera o leito e atualiza a ocupação da ala.

### Triagem

Permite realizar triagem e listar triagens. O módulo calcula pontuação de risco e classifica o paciente conforme os critérios definidos no código.

### Relatórios

Mostra uma visão geral dos totais cadastrados e da ocupação por ala. Esse módulo é a base inicial para o futuro Painel Situacional do DF.

## 9. Arquitetura e Organização

O projeto foi modularizado para separar responsabilidades e organizado em camadas de diretórios. A pasta `src/` guarda as implementações em arquivos `.c`, separando o ponto de entrada da aplicação dos módulos do sistema. A pasta `include/` guarda os cabeçalhos `.h`, separando definições centrais dos cabeçalhos específicos de cada módulo.

O arquivo `include/core/hospital.h` centraliza as `structs`, constantes com `#define` e declarações `extern` dos vetores e contadores globais. Cada módulo possui seu próprio par `.c` e `.h`, como `src/modules/paciente.c` com `include/modules/paciente.h` e `src/modules/agendamento.c` com `include/modules/agendamento.h`.

O arquivo `src/app/main.c` controla apenas o menu principal, define os vetores globais e chama os menus dos módulos. Os dados ainda ficam em memória por meio de vetores globais. Persistência em arquivos e banco de dados são etapas futuras.

## 10. Estrutura de Pastas

```text
Gestao_Saude/
  src/
    app/
      main.c

    modules/
      paciente.c
      medico.c
      agendamento.c
      ala.c
      leito.c
      internacao.c
      triagem.c
      relatorio.c

  include/
    core/
      hospital.h

    modules/
      paciente.h
      medico.h
      agendamento.h
      ala.h
      leito.h
      internacao.h
      triagem.h
      relatorio.h

  tests/
    agendamento/
    paciente/
    medico/
    triagem/
    leito/

  data/
  docs/

  README.md
  .gitignore
  Makefile
```

- `src/app/`: contém o arquivo `main.c`, responsável pelo menu principal e pela definição dos vetores e contadores globais.
- `src/modules/`: contém os arquivos `.c` dos módulos funcionais do sistema.
- `include/core/`: contém o header central `hospital.h`, com structs, constantes e declarações `extern`.
- `include/modules/`: contém os headers específicos de cada módulo.
- `tests/`: contém subpastas organizadas por área de teste planejada, como `agendamento/`, `paciente/`, `medico/`, `triagem/` e `leito/`.
- `data/`: será usada futuramente para arquivos `.txt` de persistência.
- `docs/`: será usada para documentação complementar.
- `Makefile`: centraliza os comandos de compilação, execução, limpeza e teste.

## 11. Como Compilar e Executar

Para compilar com o `Makefile`:

```bash
make
```

Para compilar e executar:

```bash
make run
```

Para limpar binários e arquivos objeto:

```bash
make clean
```

Para executar a etapa de testes planejada:

```bash
make test
```

Atualmente, `make test` apenas informa que os testes serão implementados depois.

Comando manual equivalente de compilação:

```bash
gcc -Wall -Wextra -pedantic -Iinclude src/app/main.c src/modules/paciente.c src/modules/medico.c src/modules/agendamento.c src/modules/ala.c src/modules/leito.c src/modules/internacao.c src/modules/triagem.c src/modules/relatorio.c -o sigeh
```

Após compilar, também é possível executar diretamente:

```bash
./sigeh
```

## 12. Conceitos de ALP Aplicados

`structs` são usadas para representar entidades do domínio hospitalar, como paciente, médico, agendamento, ala, leito, internação e triagem. Isso permite agrupar dados relacionados em um único tipo.

Vetores armazenam múltiplos registros em memória. Cada entidade possui um vetor global com tamanho máximo definido, como `pacientes`, `medicos` e `leitos`.

Strings com `char[]` representam textos como nome, CPF, telefone, especialidade, data, horário, status e classificação. Como C não possui tipo `string` nativo, o projeto usa vetores de caracteres.

`#define` define constantes de limite, como `MAX_PACIENTES`, `MAX_MEDICOS` e `MAX_LEITOS`. Isso evita números soltos no código e facilita ajustes futuros.

Funções organizam o sistema em blocos reutilizáveis. Cada módulo possui uma função principal de menu, como `menuPacientes`, `menuMedicos` e `menuInternacoes`.

Headers `.h` compartilham declarações entre arquivos. Eles permitem que um módulo conheça as structs, constantes e protótipos necessários sem duplicar código.

Modularização divide o projeto em partes menores e mais fáceis de manter. Em vez de concentrar toda a lógica em `main.c`, cada área do sistema possui seu próprio arquivo.

`do while` mantém os menus ativos até o usuário escolher voltar ao menu principal ou sair do sistema.

`switch case` trata as opções dos menus de forma organizada, separando cada operação em um caso específico.

`for` percorre vetores para listar registros, buscar IDs e atualizar dados.

`if/else` valida condições e controla decisões, como verificar se um registro existe, se está ativo ou se um leito está ocupado.

`strcmp` compara strings, sendo usado em validações como status de agendamento e status de internação. `strcpy` copia valores de texto, como status e classificações.

Busca linear é usada para localizar registros por ID dentro dos vetores. Essa abordagem é simples e adequada ao nível atual do projeto.

Exclusão lógica permite marcar registros como inativos sem removê-los fisicamente do vetor. Isso simplifica o controle dos dados em memória.

Validação de regras de negócio garante que operações inválidas sejam bloqueadas, como conflito de agenda, uso de médico inativo e internação em leito ocupado.

## 13. Etapas Futuras

As evoluções planejadas seguem uma ordem progressiva:

1. Implementar testes automatizados com `assert.h`.
2. Implementar persistência em arquivos `.txt`.
3. Transformar especialidade em código numérico.
4. Ampliar a regionalização do atendimento.
5. Criar agendamento inteligente por especialidade e disponibilidade.
6. Desenvolver o Painel Situacional do DF.
7. Implementar Prontuário Integrado.
8. Implementar Exames Integrados.
9. Migrar futuramente para SQLite.

## 14. Como Apresentar ao Professor

Uma forma clara de defender o projeto é começar pelo problema: hospitais precisam organizar pacientes, médicos, agendas, leitos, internações e triagens. Em seguida, explique que o SIGEH-DF simula esse fluxo em C usando estruturas, vetores, menus e validações.

Depois, apresente a solução: o sistema centraliza dados em memória, separa operações por módulos e relaciona entidades por ID. Mostre que não é apenas uma sequência de cadastros, pois existem regras de negócio, como conflito de agendamento, exclusão lógica, controle de leito ocupado e liberação de leito após alta.

O diferencial do Distrito Federal deve ser destacado com o uso da região administrativa no cadastro de pacientes. Esse dado permite planejar indicadores territoriais e o Painel Situacional do DF.

Também é importante relacionar o projeto aos conceitos de ALP: `structs`, vetores, `char[]`, funções, headers, modularização, laços, condicionais, busca linear e validação de regras.

Por fim, explique a evolução progressiva: primeiro o projeto consolida lógica e organização; depois avança para testes, persistência, painel, prontuário, exames e SQLite.

## 15. Limitações Atuais

O SIGEH-DF ainda possui limitações importantes:

- os dados ficam apenas em memória e são perdidos ao encerrar o programa;
- ainda não há persistência em arquivos;
- ainda não há SQLite;
- não há sistema de login ou controle de usuários;
- as validações ainda são simples;
- o sistema é acadêmico e executado em terminal;
- não há interface gráfica;
- não há integração real com serviços externos.

Essas limitações fazem parte da etapa atual do projeto e ajudam a delimitar o escopo para a disciplina.

## 16. Observação Acadêmica

O SIGEH-DF foi construído progressivamente para demonstrar lógica de programação, organização de dados, modularização e regras de negócio antes de avançar para banco de dados ou recursos mais complexos.

Essa progressão é importante no contexto de Algoritmos e Lógica de Programação, pois permite compreender primeiro como os dados são representados, percorridos, validados e relacionados em memória. Depois dessa base, o projeto pode evoluir com mais segurança para persistência em arquivos, testes automatizados e SQLite.
