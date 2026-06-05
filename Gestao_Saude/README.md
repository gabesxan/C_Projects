# SIGEH-DF — Sistema Integrado de Gestão Hospitalar em C

Projeto educacional de um sistema de gestão hospitalar desenvolvido em linguagem C, executado pelo terminal e organizado em múltiplos arquivos.

A aplicação foi criada para praticar Algoritmos e Lógica de Programação em C, com foco em modularização, separação entre arquivos `.c` e `.h`, uso de `structs`, funções, menus no terminal, organização de código-fonte e compilação com `Makefile`.

O projeto não usa interface gráfica, API web, framework externo ou banco de dados externo. Toda a interação acontece pelo terminal, e os dados ainda ficam apenas em memória durante a execução.

---

## Índice

- [Visão geral](#visão-geral)
- [Estado atual](#estado-atual)
- [Módulos](#módulos)
- [Tecnologias](#tecnologias)
- [Como executar](#como-executar)
- [Estrutura do projeto](#estrutura-do-projeto)
- [Mapa dos arquivos](#mapa-dos-arquivos)
- [Guia arquivo por arquivo](#guia-arquivo-por-arquivo)
- [Organização da aplicação](#organização-da-aplicação)
- [Fluxo principal da aplicação](#fluxo-principal-da-aplicação)
- [Organização dos módulos](#organização-dos-módulos)
- [Arquivos de cabeçalho](#arquivos-de-cabeçalho)
- [Arquivos de implementação](#arquivos-de-implementação)
- [Testes](#testes)
- [Conceitos praticados](#conceitos-praticados)
- [Linha do tempo dos commits](#linha-do-tempo-dos-commits)

---

## Visão geral

O SIGEH-DF representa um sistema hospitalar simples feito em linguagem C.

O projeto é dividido em módulos relacionados a:

- pacientes;
- médicos;
- agendamentos;
- alas;
- leitos;
- internações;
- triagem;
- relatórios.

A ideia principal do projeto é separar bem os arquivos por responsabilidade:

- a pasta `app` guarda o ponto de entrada da aplicação;
- a pasta `model` guarda a estrutura principal compartilhada do sistema;
- a pasta `headers` guarda os arquivos `.h` dos módulos;
- a pasta `modules` guarda os arquivos `.c` com as implementações dos módulos;
- a pasta `src/test/c` guarda os arquivos de teste.

Essa separação ajuda a manter o código organizado. O `main.c` não precisa concentrar todos os módulos, e cada parte do sistema possui seus próprios arquivos de declaração e implementação.

---

## Estado atual

A aplicação está organizada em C com separação entre ponto de entrada, modelo principal, headers, implementações e testes.

Pontos principais do estado atual:

- `main.c` fica em `src/main/c/app`.
- `hospital.h` fica em `src/main/c/model`.
- Os arquivos `.h` dos módulos ficam em `src/main/c/headers`.
- Os arquivos `.c` dos módulos ficam em `src/main/c/modules`.
- Os testes ficam em `src/test/c/modules`.
- A compilação é feita com `Makefile`.
- O projeto possui módulos separados para pacientes, médicos, agendamentos, alas, leitos, internações, triagem e relatórios.
- Atualmente não há camada de persistência implementada; se for necessário no futuro, uma pasta `persistence` deve ser criada apenas para persistência em `.txt` ou SQLite.

---

## Módulos

O projeto possui os seguintes módulos principais:

- Paciente.
- Médico.
- Agendamento.
- Ala.
- Leito.
- Internação.
- Triagem.
- Relatório.

Cada módulo possui:

- um arquivo `.h` na pasta `headers`;
- um arquivo `.c` na pasta `modules`.

Exemplo:

```text
src/main/c/headers/paciente.h
src/main/c/modules/paciente.c
```

---

## Tecnologias

- Linguagem C.
- GCC ou Clang.
- Makefile.
- Terminal.

Bibliotecas padrão de C usadas conforme necessidade do projeto:

- `stdio.h`
- `stdlib.h`
- `string.h`

---

## Como executar

Na raiz do projeto, compile com:

```bash
make clean
make
```

Depois execute com:

```bash
make run
```

Também é possível executar diretamente o binário gerado:

```bash
./sigeh
```

Para limpar o executável gerado:

```bash
make clean
```

---

## Estrutura do projeto

```text
.
├── .gitignore
├── Makefile
├── README.md
├── data
└── src
    ├── main
    │   └── c
    │       ├── app
    │       │   └── main.c
    │       ├── model
    │       │   └── hospital.h
    │       ├── headers
    │       │   ├── agendamento.h
    │       │   ├── ala.h
    │       │   ├── internacao.h
    │       │   ├── leito.h
    │       │   ├── medico.h
    │       │   ├── paciente.h
    │       │   ├── relatorio.h
    │       │   └── triagem.h
    │       └── modules
    │           ├── agendamento.c
    │           ├── ala.c
    │           ├── internacao.c
    │           ├── leito.c
    │           ├── medico.c
    │           ├── paciente.c
    │           ├── relatorio.c
    │           └── triagem.c
    └── test
        └── c
            ├── model
            └── modules
                ├── test_agendamento.c
                ├── test_leito.c
                ├── test_medico.c
                ├── test_paciente.c
                └── test_triagem.c
```

A pasta `data/` é reservada para arquivos de dados caso o projeto venha a usar persistência em `.txt` ou SQLite. No estado atual, os dados permanecem em memória.

---

## Mapa dos arquivos

Esta parte mostra onde cada arquivo entra no projeto.

```text
Makefile
```

Arquivo usado para automatizar a compilação do projeto.

Ele define:

- o compilador;
- as flags de compilação;
- os caminhos dos headers;
- os arquivos `.c` usados na compilação;
- o nome do executável;
- comandos auxiliares como `make`, `make run` e `make clean`.

```text
.gitignore
```

Arquivo usado para evitar que arquivos gerados localmente sejam versionados no Git.

Pode ser usado para ignorar:

- executáveis;
- arquivos temporários;
- arquivos gerados pelo sistema operacional;
- arquivos locais do editor.

```text
README.md
```

Arquivo central de documentação do projeto.

Reúne:

- visão geral;
- estrutura de pastas;
- explicação dos módulos;
- comandos de compilação;
- organização da aplicação;
- conceitos de C praticados.

```text
data
```

Pasta reservada para arquivos de dados do projeto.

```text
src/main/c/app
```

Pasta do ponto de entrada da aplicação.

Contém o arquivo `main.c`.

```text
src/main/c/model
```

Pasta da estrutura principal compartilhada do sistema.

Contém o arquivo `hospital.h`.

```text
src/main/c/headers
```

Pasta dos arquivos de cabeçalho dos módulos.

Os arquivos `.h` declaram funções, tipos e estruturas usados pelos arquivos `.c`.

```text
src/main/c/modules
```

Pasta dos arquivos de implementação dos módulos.

Os arquivos `.c` implementam as funções declaradas nos headers.

```text
src/test/c/model
```

Pasta reservada para testes relacionados ao modelo principal do sistema.

```text
src/test/c/modules
```

Pasta com arquivos de teste relacionados aos módulos do projeto.

---

## Guia arquivo por arquivo

O objetivo desta parte é explicar a função de cada arquivo dentro do projeto.

---

### `Makefile`

O `Makefile` automatiza o processo de compilação.

Ele evita que seja necessário digitar manualmente todos os arquivos `.c` no terminal.

Estrutura principal:

```makefile
CC = gcc
```

Define o compilador usado no projeto.

```makefile
CFLAGS = -Wall -Wextra -pedantic \
	-Isrc/main/c/model \
	-Isrc/main/c/headers \
	-Isrc/main/c/modules
```

Define as flags de compilação e os caminhos de busca para arquivos `.h`.

As flags usadas são:

| Flag | Função |
|---|---|
| `-Wall` | Ativa avisos importantes do compilador |
| `-Wextra` | Ativa avisos adicionais |
| `-pedantic` | Torna a compilação mais rigorosa em relação ao padrão da linguagem C |
| `-I...` | Informa ao compilador onde procurar arquivos `.h` |

```makefile
SRC = \
	src/main/c/app/main.c \
	src/main/c/modules/paciente.c \
	src/main/c/modules/medico.c \
	src/main/c/modules/agendamento.c \
	src/main/c/modules/ala.c \
	src/main/c/modules/leito.c \
	src/main/c/modules/internacao.c \
	src/main/c/modules/triagem.c \
	src/main/c/modules/relatorio.c
```

Lista os arquivos `.c` que fazem parte da compilação do sistema.

```makefile
OUT = sigeh
```

Define o nome do executável gerado.

```makefile
all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)
```

Compila o projeto.

```makefile
run: all
	./$(OUT)
```

Compila e executa o programa.

```makefile
clean:
	rm -f $(OUT)
```

Remove o executável gerado.

O que foi estudado aqui:

- compilação de múltiplos arquivos `.c`;
- uso de flags de compilação;
- inclusão de diretórios com `-I`;
- automação de comandos com `Makefile`.

---

### `.gitignore`

Arquivo usado para evitar que arquivos locais ou gerados automaticamente entrem no repositório.

O que foi estudado aqui:

- diferença entre código-fonte e arquivo gerado;
- organização de repositório;
- prevenção contra versionamento de arquivos desnecessários.

---

### `README.md`

Este arquivo funciona como documentação principal do projeto.

Ele explica:

- o objetivo do sistema;
- a estrutura de pastas;
- os módulos existentes;
- como compilar;
- como executar;
- como os arquivos estão organizados;
- quais conceitos de C foram praticados.

O que foi estudado aqui:

- documentação técnica;
- Markdown;
- organização de projeto;
- clareza na apresentação de código.

---

### `src/main/c/app/main.c`

Arquivo principal do sistema.

É o ponto de entrada da aplicação em C.

Esse arquivo deve conter a função:

```c
int main(void)
```

ou uma variação equivalente da função `main`.

Responsabilidades do arquivo:

- iniciar o programa;
- chamar os menus ou funções principais;
- controlar o fluxo inicial da aplicação;
- integrar os módulos do sistema.

O que foi estudado aqui:

- ponto de entrada em C;
- função `main`;
- organização do fluxo principal;
- separação entre inicialização do programa e implementação dos módulos.

---

### `src/main/c/model/hospital.h`

Arquivo principal de modelo do sistema.

Esse arquivo concentra definições compartilhadas entre os módulos.

Pode conter:

- constantes globais;
- `structs` principais;
- definições comuns;
- tipos compartilhados;
- declarações gerais usadas por mais de um módulo.

Responsabilidade principal:

- centralizar a estrutura comum do sistema hospitalar.

O que foi estudado aqui:

- arquivo de cabeçalho compartilhado;
- uso de `struct`;
- constantes;
- reutilização de definições em múltiplos arquivos.

---

### `src/main/c/headers/paciente.h`

Header do módulo de pacientes.

Responsabilidades:

- declarar as funções relacionadas ao módulo de pacientes;
- expor para outros arquivos quais operações do módulo podem ser chamadas;
- incluir as dependências necessárias para o módulo.

Arquivo de implementação correspondente:

```text
src/main/c/modules/paciente.c
```

O que foi estudado aqui:

- separação entre declaração e implementação;
- modularização;
- uso de header próprio por módulo.

---

### `src/main/c/modules/paciente.c`

Implementação do módulo de pacientes.

Responsabilidades:

- implementar as funções declaradas em `paciente.h`;
- concentrar a lógica relacionada ao módulo de pacientes.

Header correspondente:

```text
src/main/c/headers/paciente.h
```

O que foi estudado aqui:

- implementação de funções em arquivo `.c`;
- separação entre código principal e módulo;
- organização por responsabilidade.

---

### `src/main/c/headers/medico.h`

Header do módulo de médicos.

Responsabilidades:

- declarar as funções relacionadas ao módulo de médicos;
- permitir que outros arquivos chamem as operações do módulo.

Arquivo de implementação correspondente:

```text
src/main/c/modules/medico.c
```

O que foi estudado aqui:

- arquivo `.h` por módulo;
- declaração de funções;
- organização modular.

---

### `src/main/c/modules/medico.c`

Implementação do módulo de médicos.

Responsabilidades:

- implementar as funções declaradas em `medico.h`;
- concentrar a lógica relacionada ao módulo de médicos.

Header correspondente:

```text
src/main/c/headers/medico.h
```

O que foi estudado aqui:

- arquivo `.c` por módulo;
- implementação de funções;
- separação de responsabilidades.

---

### `src/main/c/headers/agendamento.h`

Header do módulo de agendamentos.

Responsabilidades:

- declarar as funções relacionadas ao módulo de agendamentos;
- permitir que o restante do sistema acesse as operações desse módulo.

Arquivo de implementação correspondente:

```text
src/main/c/modules/agendamento.c
```

O que foi estudado aqui:

- modularização;
- uso de headers;
- organização de funções por domínio do sistema.

---

### `src/main/c/modules/agendamento.c`

Implementação do módulo de agendamentos.

Responsabilidades:

- implementar as funções declaradas em `agendamento.h`;
- concentrar a lógica relacionada aos agendamentos.

Header correspondente:

```text
src/main/c/headers/agendamento.h
```

O que foi estudado aqui:

- implementação modular;
- separação entre declaração e implementação;
- organização de código por assunto.

---

### `src/main/c/headers/ala.h`

Header do módulo de alas.

Responsabilidades:

- declarar as funções relacionadas ao módulo de alas;
- expor as operações desse módulo para o restante do sistema.

Arquivo de implementação correspondente:

```text
src/main/c/modules/ala.c
```

O que foi estudado aqui:

- uso de arquivo `.h` para declaração;
- separação por módulo;
- estruturação do projeto.

---

### `src/main/c/modules/ala.c`

Implementação do módulo de alas.

Responsabilidades:

- implementar as funções declaradas em `ala.h`;
- concentrar a lógica relacionada às alas.

Header correspondente:

```text
src/main/c/headers/ala.h
```

O que foi estudado aqui:

- arquivo `.c` de implementação;
- organização por módulo;
- modularização em C.

---

### `src/main/c/headers/leito.h`

Header do módulo de leitos.

Responsabilidades:

- declarar as funções relacionadas ao módulo de leitos;
- permitir integração com outros módulos do sistema.

Arquivo de implementação correspondente:

```text
src/main/c/modules/leito.c
```

O que foi estudado aqui:

- declaração de funções;
- uso de header por módulo;
- separação entre interface e implementação.

---

### `src/main/c/modules/leito.c`

Implementação do módulo de leitos.

Responsabilidades:

- implementar as funções declaradas em `leito.h`;
- concentrar a lógica relacionada aos leitos.

Header correspondente:

```text
src/main/c/headers/leito.h
```

O que foi estudado aqui:

- implementação em C;
- organização modular;
- separação entre `.h` e `.c`.

---

### `src/main/c/headers/internacao.h`

Header do módulo de internações.

Responsabilidades:

- declarar as funções relacionadas ao módulo de internações;
- expor operações de internação para o restante do sistema.

Arquivo de implementação correspondente:

```text
src/main/c/modules/internacao.c
```

O que foi estudado aqui:

- arquivo de cabeçalho;
- declaração de funções;
- modularização do sistema.

---

### `src/main/c/modules/internacao.c`

Implementação do módulo de internações.

Responsabilidades:

- implementar as funções declaradas em `internacao.h`;
- concentrar a lógica relacionada às internações.

Header correspondente:

```text
src/main/c/headers/internacao.h
```

O que foi estudado aqui:

- implementação em arquivo `.c`;
- organização por módulo;
- divisão do projeto em partes menores.

---

### `src/main/c/headers/triagem.h`

Header do módulo de triagem.

Responsabilidades:

- declarar as funções relacionadas ao módulo de triagem;
- permitir que outros arquivos chamem operações de triagem.

Arquivo de implementação correspondente:

```text
src/main/c/modules/triagem.c
```

O que foi estudado aqui:

- header de módulo;
- organização de funções;
- separação entre declaração e implementação.

---

### `src/main/c/modules/triagem.c`

Implementação do módulo de triagem.

Responsabilidades:

- implementar as funções declaradas em `triagem.h`;
- concentrar a lógica relacionada à triagem.

Header correspondente:

```text
src/main/c/headers/triagem.h
```

O que foi estudado aqui:

- implementação modular;
- separação de código;
- organização por responsabilidade.

---

### `src/main/c/headers/relatorio.h`

Header do módulo de relatórios.

Responsabilidades:

- declarar as funções relacionadas ao módulo de relatórios;
- permitir que o sistema acesse as operações de relatório.

Arquivo de implementação correspondente:

```text
src/main/c/modules/relatorio.c
```

O que foi estudado aqui:

- header de módulo;
- declaração de funções;
- divisão do sistema em arquivos.

---

### `src/main/c/modules/relatorio.c`

Implementação do módulo de relatórios.

Responsabilidades:

- implementar as funções declaradas em `relatorio.h`;
- concentrar a lógica relacionada aos relatórios.

Header correspondente:

```text
src/main/c/headers/relatorio.h
```

O que foi estudado aqui:

- implementação em `.c`;
- modularização;
- integração entre módulos.

---

### `src/test/c/modules/test_agendamento.c`

Arquivo de teste relacionado ao módulo de agendamentos.

O que foi estudado aqui:

- separação entre código principal e código de teste;
- testes por módulo.

---

### `src/test/c/modules/test_leito.c`

Arquivo de teste relacionado ao módulo de leitos.

O que foi estudado aqui:

- organização de testes;
- validação isolada por módulo.

---

### `src/test/c/modules/test_medico.c`

Arquivo de teste relacionado ao módulo de médicos.

O que foi estudado aqui:

- teste separado do código principal;
- organização por assunto.

---

### `src/test/c/modules/test_paciente.c`

Arquivo de teste relacionado ao módulo de pacientes.

O que foi estudado aqui:

- teste por módulo;
- estrutura de teste separada da aplicação principal.

---

### `src/test/c/modules/test_triagem.c`

Arquivo de teste relacionado ao módulo de triagem.

O que foi estudado aqui:

- organização dos testes;
- separação entre implementação e validação.

---

## Organização da aplicação

A aplicação está organizada em quatro blocos principais dentro de `src/main/c`.

---

### `app`

Contém o ponto de entrada da aplicação.

Arquivo:

```text
src/main/c/app/main.c
```

Responsabilidade:

- iniciar o programa;
- chamar os módulos necessários;
- controlar o fluxo principal.

---

### `model`

Contém a estrutura principal compartilhada.

Arquivo:

```text
src/main/c/model/hospital.h
```

Responsabilidade:

- definir estruturas compartilhadas;
- concentrar definições comuns;
- servir como base para os módulos.

---

### `headers`

Contém as declarações dos módulos.

Arquivos:

```text
src/main/c/headers/agendamento.h
src/main/c/headers/ala.h
src/main/c/headers/internacao.h
src/main/c/headers/leito.h
src/main/c/headers/medico.h
src/main/c/headers/paciente.h
src/main/c/headers/relatorio.h
src/main/c/headers/triagem.h
```

Responsabilidade:

- declarar funções;
- expor a interface de cada módulo;
- permitir que os arquivos `.c` se comuniquem.

---

### `modules`

Contém as implementações dos módulos.

Arquivos:

```text
src/main/c/modules/agendamento.c
src/main/c/modules/ala.c
src/main/c/modules/internacao.c
src/main/c/modules/leito.c
src/main/c/modules/medico.c
src/main/c/modules/paciente.c
src/main/c/modules/relatorio.c
src/main/c/modules/triagem.c
```

Responsabilidade:

- implementar as funções declaradas nos headers;
- concentrar a lógica dos módulos;
- manter o código dividido por área do sistema.

---

### `test`

Contém os arquivos de teste.

Arquivos atuais:

```text
src/test/c/modules/test_agendamento.c
src/test/c/modules/test_leito.c
src/test/c/modules/test_medico.c
src/test/c/modules/test_paciente.c
src/test/c/modules/test_triagem.c
```

Responsabilidade:

- separar os testes do código principal;
- permitir validações por módulo.

---

## Fluxo principal da aplicação

O fluxo principal começa em:

```text
src/main/c/app/main.c
```

A partir desse arquivo, o sistema utiliza os módulos definidos nos headers e implementados na pasta `modules`.

O fluxo geral pode ser representado assim:

```text
main.c
  ↓
headers dos módulos
  ↓
implementações em modules
  ↓
retorno ao fluxo principal
```

O `main.c` deve concentrar o início da execução e a chamada das funções principais, enquanto os arquivos `.c` dos módulos concentram suas próprias implementações.

---

## Organização dos módulos

Cada módulo segue o mesmo padrão:

```text
headers/nome_do_modulo.h
modules/nome_do_modulo.c
```

Exemplo:

```text
headers/paciente.h
modules/paciente.c
```

Isso significa que:

- o arquivo `.h` declara o que o módulo oferece;
- o arquivo `.c` implementa como o módulo funciona.

Esse padrão se repete em:

| Módulo | Header | Implementação |
|---|---|---|
| Paciente | `headers/paciente.h` | `modules/paciente.c` |
| Médico | `headers/medico.h` | `modules/medico.c` |
| Agendamento | `headers/agendamento.h` | `modules/agendamento.c` |
| Ala | `headers/ala.h` | `modules/ala.c` |
| Leito | `headers/leito.h` | `modules/leito.c` |
| Internação | `headers/internacao.h` | `modules/internacao.c` |
| Triagem | `headers/triagem.h` | `modules/triagem.c` |
| Relatório | `headers/relatorio.h` | `modules/relatorio.c` |

---

## Arquivos de cabeçalho

Os arquivos `.h` ficam em:

```text
src/main/c/headers
```

Eles servem para declarar funções, tipos e estruturas que serão usadas por outros arquivos.

Exemplo de uso:

```c
#include "paciente.h"
```

Como o `Makefile` inclui a pasta `headers` com `-I`, os arquivos podem ser incluídos diretamente pelo nome.

Exemplo:

```makefile
-Isrc/main/c/headers
```

Isso permite usar:

```c
#include "paciente.h"
#include "medico.h"
#include "agendamento.h"
```

sem precisar escrever o caminho completo.

---

## Arquivos de implementação

Os arquivos `.c` ficam em:

```text
src/main/c/modules
```

Eles implementam as funções declaradas nos headers.

Exemplo:

```text
src/main/c/headers/paciente.h
src/main/c/modules/paciente.c
```

O arquivo `.h` informa quais funções existem.

O arquivo `.c` contém o código dessas funções.

---

## Testes

A pasta de testes atual é:

```text
src/test/c
```

Dentro dela, existem arquivos de teste em:

```text
src/test/c/modules
```

Arquivos existentes:

```text
test_agendamento.c
test_leito.c
test_medico.c
test_paciente.c
test_triagem.c
```

A execução automatizada dos testes depende da configuração do `Makefile`.

---

## Conceitos praticados

O projeto permite praticar os seguintes conceitos de C e de Algoritmos e Lógica de Programação:

- função `main`;
- modularização;
- múltiplos arquivos `.c`;
- múltiplos arquivos `.h`;
- separação entre declaração e implementação;
- diretiva `#include`;
- include guards;
- compilação com múltiplos arquivos;
- flags de compilação;
- uso de `Makefile`;
- organização de pastas;
- funções;
- `structs`;
- constantes;
- menus no terminal;
- condicionais;
- laços de repetição;
- separação entre código principal e testes.

---
