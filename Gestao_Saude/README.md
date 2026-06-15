<div align="center">

# 🏥 SIGEH-DF

### Sistema Integrado de Gestão Hospitalar

Projeto acadêmico em **C** que evoluiu de um sistema de terminal em memória para um **backend web full-stack** com banco SQLite, camadas bem definidas, triagem inteligente, API HTTP e autenticação por papéis.

![C](https://img.shields.io/badge/Linguagem-C-00599C?logo=c&logoColor=white)
![SQLite](https://img.shields.io/badge/Banco-SQLite-003B57?logo=sqlite&logoColor=white)
![OpenSSL](https://img.shields.io/badge/Hash-OpenSSL%20SHA--256-721412?logo=openssl&logoColor=white)
![Build](https://img.shields.io/badge/build-passing-brightgreen)
![Testes](https://img.shields.io/badge/testes-12%2F12-brightgreen)
![Licença](https://img.shields.io/badge/uso-acadêmico-blue)

</div>

---

## 📑 Índice

- [Visão geral](#-visão-geral)
- [As duas versões](#-as-duas-versões)
- [Arquitetura](#-arquitetura)
- [Estrutura de pastas](#-estrutura-de-pastas)
- [Pré-requisitos](#-pré-requisitos)
- [Como compilar e executar](#-como-compilar-e-executar)
- [Como rodar os testes](#-como-rodar-os-testes)
- [Autenticação e papéis](#-autenticação-e-papéis)
- [Referência da API](#-referência-da-api)
- [Triagem inteligente](#-triagem-inteligente)
- [Modelo de domínio](#-modelo-de-domínio)
- [Próximos passos](#-próximos-passos)
- [Notas acadêmicas](#-notas-acadêmicas)

---

## 🎯 Visão geral

O **SIGEH-DF** simula, de forma modular e didática, o funcionamento de um ambiente hospitalar: pacientes, médicos, agendamentos, alas, leitos, internações, triagens, prontuários, exames e relatórios.

O grande diferencial é a **triagem inteligente**: ela deixou de ser apenas uma classificação de risco e virou o **primeiro motor de decisão** do sistema — a partir de uma triagem, o sistema calcula prioridade e especialidade provável, consulta o histórico clínico, sugere exames iniciais, encontra médicos disponíveis por especialidade e região, e agenda ou encaminha o paciente.

---

## 🧬 As duas versões

| Versão | Onde | O que é |
|---|---|---|
| **V1 — Terminal** | [`backend/terminal/`](backend/terminal/) | Aplicação CLI original, dados em memória + persistência SQLite, menus interativos. Preservada e funcional. |
| **V2 — Backend Web** | [`backend/`](backend/) (`database`, `repositories`, `services`, `api`) | Backend HTTP em camadas sobre SQLite, com triagem inteligente, API REST e login por papéis. Foco do desenvolvimento atual. |

As duas versões compartilham o mesmo schema ([`backend/data/schema_v2.sql`](backend/data/schema_v2.sql)) mas são **independentes**: a V2 não depende do código da V1.

---

## 🏗️ Arquitetura

O backend web segue uma arquitetura em camadas, de baixo para cima:

```text
┌─────────────────────────────────────────────────────────┐
│  api/         Servidor HTTP (sockets POSIX) + roteamento  │
│               + autenticação HTTP Basic + autorização      │
├─────────────────────────────────────────────────────────┤
│  services/    Regras de negócio (triagem inteligente,      │
│               relatórios) — orquestram os repositories     │
├─────────────────────────────────────────────────────────┤
│  repositories/  Acesso a dados (1 por entidade), apenas    │
│                 SQL com prepared statements                │
├─────────────────────────────────────────────────────────┤
│  database/    Camada fina sobre o SQLite (abrir/fechar,    │
│               executar, resetar com schema)                │
└─────────────────────────────────────────────────────────┘
```

**Princípios aplicados:**

- 🔒 **Prepared statements** em toda entrada externa (sem `sprintf` em SQL) → sem SQL injection.
- 🧱 **Separação de responsabilidades**: a API roteia, os services decidem, os repositories persistem.
- 🔗 **Integridade referencial** garantida pelo banco (`PRAGMA foreign_keys = ON`).
- 🔑 **Senhas com hash** SHA-256 + salt (OpenSSL) — nunca em texto puro.
- ✅ **Convenção de retorno**: escrita devolve `1` (sucesso) / `0` (falha); listagens devolvem JSON.

---

## 📁 Estrutura de pastas

```text
Gestao_Saude/
├── README.md
├── .gitignore
└── backend/
    ├── Makefile                  # build/test do backend web
    ├── data/
    │   ├── schema_v2.sql         # schema único (fonte da verdade)
    │   └── sigeh_v2.db           # banco (gerado, ignorado pelo git)
    ├── database/
    │   ├── database.h / .c        # camada fina SQLite
    ├── repositories/             # 1 repository por entidade
    │   ├── repo_json.*            # util de montagem de JSON
    │   ├── senha_util.*           # hashing SHA-256 + salt (OpenSSL)
    │   ├── paciente_repository.*
    │   ├── medico_repository.*
    │   ├── ala_repository.*
    │   ├── leito_repository.*
    │   ├── triagem_repository.*
    │   ├── agendamento_repository.*
    │   ├── prontuario_repository.*
    │   ├── exame_repository.*
    │   ├── internacao_repository.*
    │   └── usuario_repository.*
    ├── services/
    │   ├── triagem_service.*      # motor de decisão da triagem
    │   └── relatorio_service.*    # indicadores
    ├── api/
    │   └── health.c              # servidor HTTP + rotas + auth
    ├── tests/                    # testes com assert.h (banco isolado)
    └── terminal/                 # V1 CLI preservada (app/model/headers/modules/test)
```

---

## ⚙️ Pré-requisitos

- **GCC** (ou clang) com suporte a C padrão
- **make**
- **SQLite 3** (`-lsqlite3`)
- **OpenSSL** (`libcrypto`) — usado no hash de senhas

> 💡 No macOS via Homebrew, o OpenSSL costuma ficar em `/opt/homebrew/opt/openssl@3`. O `Makefile` já aponta para lá por padrão. Em outro caminho, sobrescreva: `make OPENSSL_DIR=/seu/openssl`.

---

## 🚀 Como compilar e executar

### Backend Web (V2)

```sh
cd backend
make            # compila a camada de banco e o servidor da API
make run        # sobe o servidor HTTP na porta 8080
```

Com o servidor no ar, em outro terminal:

```sh
# rota pública de saúde
curl -i http://localhost:8080/health

# rota autenticada (HTTP Basic)
curl -u admin:secreta http://localhost:8080/me
```

> O servidor lê o banco em `backend/data/sigeh_v2.db` (caminho relativo) — **rode sempre de dentro de `backend/`**.
> Para recriar o banco do zero a partir do schema, use a função `db_resetar_com_schema` (chamada nos testes e em scripts de seed).

### Terminal (V1)

```sh
cd backend/terminal
make
make run
```

---

## 🧪 Como rodar os testes

```sh
cd backend
make test
```

São **12 suítes** com `assert.h`, cada uma recriando um **banco de teste isolado** (`tests/test_sigeh_repository.db`) a partir do schema — sem depender de dados antigos:

| Repositories | Services |
|---|---|
| paciente, medico, ala, leito | triagem_service |
| triagem, agendamento, prontuario | relatorio_service |
| exame, internacao, usuario | |

O build usa `-Wall -Wextra -pedantic` e fecha **sem warnings**.

---

## 🔐 Autenticação e papéis

A API usa **HTTP Basic** (`Authorization: Basic base64(login:senha)`), validado **a cada requisição** contra a tabela `usuarios` (stateless). Os **logins são criados pelo administrador**.

| Papel | Pode acessar |
|---|---|
| **ADMIN** | Tudo, incluindo cadastro de usuários (`/usuarios`). |
| **CADASTRO** | CRUD dos cadastros: pacientes, médicos, alas, leitos. |
| **MEDICO** | Leitura dos cadastros + todo o clínico (triagens, agendamentos, prontuários, exames, internações, triagem inteligente, relatórios) + suas rotas `/me`. |
| **ENFERMAGEM** | Visão de enfermaria: leitura de internações, leitos e alas. |
| **PACIENTE** | Apenas as próprias informações via `/me` (exames, prontuários). |

Respostas de acesso: **`401`** sem credencial válida · **`403`** papel sem permissão.

> A política de autorização é **centralizada** em uma única função (`autorizado()`) aplicada no topo do roteador — fácil de auditar e manter.

---

## 🌐 Referência da API

> Todas as rotas exigem autenticação, **exceto** `GET /health`. Parâmetros de criação vão na query string.

### Saúde e sessão

| Método | Rota | Papel | Descrição |
|---|---|---|---|
| `GET` | `/health` | público | Status do serviço + banco |
| `GET` | `/me` | autenticado | Papel e vínculos do usuário logado |
| `GET` | `/me/exames` | PACIENTE | Exames do próprio paciente |
| `GET` | `/me/prontuarios` | PACIENTE | Prontuários do próprio paciente |
| `GET` | `/me/agenda` | MEDICO | Agenda do próprio médico |
| `GET` | `/me/pacientes` | MEDICO | Pacientes do próprio médico |

### Usuários (ADMIN)

| Método | Rota | Descrição |
|---|---|---|
| `GET` | `/usuarios` · `/usuarios/contar` | Listar / contar |
| `POST` | `/usuarios?login=&senha=&papel=&paciente_id=&medico_id=` | Criar usuário |
| `DELETE` | `/usuarios/{id}` | Desativar |

### Cadastros (ADMIN, CADASTRO · MEDICO só leitura)

Para `pacientes`, `medicos`, `alas`, `leitos`:

| Método | Rota | Descrição |
|---|---|---|
| `GET` | `/{entidade}` · `/{entidade}/contar` | Listar / contar |
| `POST` | `/{entidade}?...` | Criar |
| `DELETE` | `/{entidade}/{id}` | Exclusão lógica |

### Clínico (ADMIN, MEDICO · ENFERMAGEM lê internações/leitos/alas)

Para `triagens`, `agendamentos`, `prontuarios`, `exames`, `internacoes`:

| Método | Rota | Observação |
|---|---|---|
| `GET` | `/{entidade}` · `/{entidade}/contar` | Listar / contar |
| `POST` | `/{entidade}?...` | Criar |
| `DELETE` | `/{entidade}/{id}` | Exclusão lógica · em `agendamentos` = **cancelar** |
| `POST` | `/internacoes/{id}/alta?data=` | Dar alta (internações não têm DELETE) |

### Triagem inteligente (ADMIN, MEDICO)

| Método | Rota | Descrição |
|---|---|---|
| `GET` | `/triagem/{pacienteId}/avaliacao` | Risco, prioridade e especialidade provável |
| `GET` | `/triagem/{pacienteId}/medicos` | Médicos sugeridos (especialidade + região) |
| `GET` | `/triagem/{pacienteId}/historico` | Prontuários + exames anteriores |
| `GET` | `/triagem/{pacienteId}/exames` | Exames iniciais sugeridos |
| `POST` | `/triagem/{pacienteId}/agendar?data=&horario=` | Agenda com médico disponível |
| `POST` | `/triagem/{pacienteId}/encaminhar?especialidade=&data=&horario=` | Encaminha para outra especialidade |

### Relatórios (ADMIN, MEDICO)

| Método | Rota | Descrição |
|---|---|---|
| `GET` | `/relatorios/indicadores` | Totais + triagens por classificação + casos graves |

#### Exemplos

```sh
# Admin cria um médico e um usuário médico vinculado a ele
curl -u admin:secreta -X POST \
  "http://localhost:8080/medicos?nome=Dra+Helena&crm=CRM123&especialidade=Cardiologia&regiao=7"
curl -u admin:secreta -X POST \
  "http://localhost:8080/usuarios?login=helena&senha=h123&papel=MEDICO&medico_id=1"

# Avaliação inteligente da triagem de um paciente
curl -u helena:h123 http://localhost:8080/triagem/1/avaliacao
# -> {"pacienteId":1,"classificacao":"Emergencia","prioridade":5,"especialidadeProvavel":"Cardiologia"}

# Agendamento automático
curl -u helena:h123 -X POST \
  "http://localhost:8080/triagem/1/agendar?data=2026-07-01&horario=09:00"
```

---

## 🧠 Triagem inteligente

O fluxo que o motor de decisão cobre hoje:

```text
Paciente passa pela triagem
   └─▶ calcula risco, prioridade e especialidade provável     (/triagem/{id}/avaliacao)
       └─▶ consulta histórico: prontuários e exames anteriores (/triagem/{id}/historico)
           └─▶ sugere exames iniciais conforme o tipo          (/triagem/{id}/exames)
               └─▶ encontra médicos por especialidade + região (/triagem/{id}/medicos)
                   └─▶ agenda com médico disponível            (POST /triagem/{id}/agendar)
                       └─▶ ou encaminha para outra especialidade (POST /triagem/{id}/encaminhar)
```

**Regra de agenda:** janela de expediente **08:00–18:00** e **grade de 30 min** (08:00, 08:30, …, 17:30). Horários fora da grade ou do expediente são recusados, em todos os caminhos de escrita.

---

## 🗃️ Modelo de domínio

Entidades persistidas (tabelas em [`schema_v2.sql`](backend/data/schema_v2.sql)):

`pacientes` · `medicos` · `alas` · `leitos` · `triagens` · `agendamentos` · `prontuarios` · `exames` · `internacoes` · `usuarios`

**Estados e exclusão lógica:**

- Entidades com `ativo` (0/1): paciente, médico, ala, leito, triagem, prontuário, exame, usuário → exclusão lógica.
- Baseadas em `status`: `agendamentos` (`AGENDADO` → `CANCELADO`) e `internacoes` (`INTERNADO` → `ALTA`).

**Classificações de triagem:** `Emergencia` (5) · `Muito prioritario` (4) · `Prioritario` (3) · `Comum` (2) · `Orientacao basica` (1).

---

## 🧭 Próximos passos

- 💊 **Módulo de prescrição/medicação** — necessário para a enfermagem ver "remédios a aplicar" e o paciente ver receitas (entidade ainda inexistente).
- 🖥️ **Frontend web** consumindo a API (o `GET /me` já entrega o papel para a UI decidir o que exibir).
- 🔎 **Escopo de dados nas listas globais** — hoje quem tem permissão de leitura vê listas amplas; o escopo "só o seu" vive nas rotas `/me`.
- 📈 **Relatórios mais ricos** (por período, por especialidade, por região).
- 🧪 **Testes de integração da API** (hoje validada por smoke tests com `curl`).

---

## 🎓 Notas acadêmicas

- Projeto construído para **clareza didática**, priorizando legibilidade e regras de negócio sobre performance.
- O servidor HTTP é **single-thread e bloqueante** — adequado para fins acadêmicos, não para produção.
- A criação de registros pela API usa **query string** (inclusive senha) por simplicidade; num cenário real, dados sensíveis iriam no corpo da requisição sobre HTTPS.
- Artefatos gerados (binários, `*.db`, `*.o`) ficam fora do versionamento (`.gitignore`); o **schema** é a fonte da verdade e o banco é sempre reconstruível.

<div align="center">

---

Feito com `gcc -Wall -Wextra -pedantic` e muito cuidado com cada `free()`. 🩺

</div>
