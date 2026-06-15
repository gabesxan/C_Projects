<div align="center">

# 🏥 SIGEH-DF

### Sistema Integrado de Gestão Hospitalar

Projeto acadêmico em **C** que evoluiu de um sistema de terminal em memória para um **backend web full-stack** com banco SQLite, arquitetura em camadas, triagem inteligente, API HTTP e autenticação por papéis.

![C](https://img.shields.io/badge/Linguagem-C-00599C?logo=c&logoColor=white)
![SQLite](https://img.shields.io/badge/Banco-SQLite-003B57?logo=sqlite&logoColor=white)
![OpenSSL](https://img.shields.io/badge/Hash-OpenSSL%20SHA--256-721412?logo=openssl&logoColor=white)
![Build](https://img.shields.io/badge/build-passing-brightgreen)
![Testes](https://img.shields.io/badge/testes-12%2F12-brightgreen)
![Warnings](https://img.shields.io/badge/warnings-0-brightgreen)
![Licença](https://img.shields.io/badge/uso-acadêmico-blue)

</div>

---

## 📑 Sumário

- [Visão geral](#-visão-geral)
- [Destaques](#-destaques)
- [Arquitetura](#-arquitetura)
  - [Ciclo de vida de uma requisição](#ciclo-de-vida-de-uma-requisição)
  - [Convenções de projeto](#convenções-de-projeto)
- [Estrutura de pastas](#-estrutura-de-pastas)
- [Pré-requisitos](#-pré-requisitos)
- [Instalação e build](#-instalação-e-build)
  - [Alvos do Makefile](#alvos-do-makefile)
- [Testes](#-testes)
- [Autenticação](#-autenticação)
- [Autorização e papéis](#-autorização-e-papéis)
- [Referência da API](#-referência-da-api)
- [Walkthrough completo](#-walkthrough-completo)
- [Triagem inteligente](#-triagem-inteligente)
- [Modelo de domínio](#-modelo-de-domínio)
- [Códigos HTTP](#-códigos-http)
- [Como estender o projeto](#-como-estender-o-projeto)
- [Segurança](#-segurança)
- [Roadmap](#-roadmap)
- [Notas acadêmicas](#-notas-acadêmicas)

---

## 🎯 Visão geral

O **SIGEH-DF** (Sistema Integrado de Gestão Hospitalar) simula, de forma modular e didática, o funcionamento de um ambiente hospitalar. Ele cobre o ciclo de atendimento — do cadastro do paciente à triagem, agendamento, atendimento, exames, internação e indicadores gerenciais.

O projeto nasceu como uma aplicação de **terminal** (linha de comando, dados em memória) e foi reorganizado para um **backend web** em camadas, sem abandonar a simplicidade do C básico. A persistência é feita em **SQLite**; a comunicação, via **API HTTP** que responde em JSON.

O grande diferencial é a **triagem inteligente**: ela deixou de ser apenas uma classificação de risco e virou o **primeiro motor de decisão** do sistema. A partir de uma triagem, o sistema:

1. calcula **risco, prioridade e especialidade provável**;
2. consulta o **histórico clínico** (prontuários e exames anteriores);
3. **sugere exames iniciais** conforme o quadro;
4. encontra **médicos disponíveis** por especialidade e região;
5. **agenda ou encaminha** o paciente automaticamente.

---

## ✨ Destaques

- 🧠 **Triagem inteligente** como motor de decisão (avaliação, histórico, sugestão de exames, agendamento e encaminhamento automáticos).
- 🧱 **Arquitetura em camadas** clara: `database → repositories → services → api`.
- 🔒 **Segurança de dados**: prepared statements em toda entrada, integridade referencial no banco (FK), senhas com **hash SHA-256 + salt**.
- 👥 **Login por papéis** criado pelo administrador: `ADMIN`, `CADASTRO`, `MEDICO`, `ENFERMAGEM`, `PACIENTE`.
- 🌐 **API REST** em C puro com sockets POSIX (sem framework).
- ✅ **12 suítes de teste** automatizadas com `assert.h`, build sem warnings em `-Wall -Wextra -pedantic`.
- ♻️ **Banco reconstruível**: o schema é a fonte da verdade; o `.db` é descartável.

---

## 🧬 Origem do projeto

O SIGEH-DF nasceu como uma aplicação de **terminal** (CLI em C, dados em memória) e evoluiu para o **backend web em camadas** ([`backend/web/`](backend/web/)) que é o foco atual. A versão de terminal cumpriu seu papel como protótipo e foi descontinuada — seu histórico permanece preservado no repositório git, mas o código não faz mais parte da árvore do projeto.

A persistência ficou desde o início em **SQLite**, com o schema versionado em [`backend/data/schema_v2.sql`](backend/data/schema_v2.sql) como fonte única da verdade.

---

## 🏗️ Arquitetura

O backend web segue uma arquitetura em camadas, de baixo para cima. Cada camada só conhece a camada imediatamente inferior.

```text
┌───────────────────────────────────────────────────────────────┐
│  api/          Servidor HTTP (sockets POSIX), roteamento,       │
│                autenticação HTTP Basic e autorização por papel   │
├───────────────────────────────────────────────────────────────┤
│  services/     Regras de negócio: triagem inteligente e          │
│                relatórios. Orquestram vários repositories.       │
├───────────────────────────────────────────────────────────────┤
│  repositories/ Acesso a dados — 1 arquivo por entidade.          │
│                Só SQL, com prepared statements. "Burros".        │
├───────────────────────────────────────────────────────────────┤
│  database/     Camada fina sobre o SQLite: abrir/fechar,         │
│                executar SQL, resetar a partir do schema.         │
├───────────────────────────────────────────────────────────────┤
│  SQLite (backend/data/sigeh_v2.db)                               │
└───────────────────────────────────────────────────────────────┘
```

**Responsabilidades:**

- **`database/`** — encapsula o `sqlite3`: gerencia o caminho do banco, abre conexões (com `PRAGMA foreign_keys = ON`), executa scripts e recria o banco a partir do schema. Não conhece nenhuma entidade.
- **`repositories/`** — um por entidade. Convertem chamadas em SQL parametrizado (`prepare → bind → step → finalize`). Não contêm regra de negócio.
- **`services/`** — onde mora a inteligência. O `triagem_service` decide especialidade, sugere exames, agenda e encaminha; o `relatorio_service` agrega indicadores. Centralizam regras (ex.: mapa tipo→especialidade) num único lugar.
- **`api/`** — recebe a requisição HTTP, autentica, aplica a política de acesso e despacha para o handler, que chama um service ou repository e serializa a resposta em JSON.
- **`util/`** — utilitários transversais sem regra de negócio: `repo_json` (montagem/escape de JSON) e `senha_util` (hash de senha com SHA-256 + salt via OpenSSL).

### Ciclo de vida de uma requisição

```text
Cliente HTTP
   │  GET /triagem/1/avaliacao   (Authorization: Basic …)
   ▼
[ api ] aceita a conexão, lê a requisição
   │
   ├─▶ /health?  → responde direto (rota pública)
   │
   ├─▶ autentica (HTTP Basic) ──── falhou ─▶ 401 Unauthorized
   │
   ├─▶ autorizado(método, rota, papel)? ── não ─▶ 403 Forbidden
   │
   ▼
[ service ] triagem_service_avaliar_json(...)
   │
   ▼
[ repository ] triagem_repo_ultima_por_paciente(...)
   │
   ▼
[ database ] db_abrir() → SQLite → db_fechar()
   │
   ▼
JSON de volta ao cliente (200 / 4xx / 5xx)
```

### Convenções de projeto

| Convenção | Regra |
|---|---|
| **Retorno de escrita** | `1` = sucesso · `0` = falha |
| **Contagens** | retornam o número, ou `-1` em erro |
| **Listagens** | preenchem um buffer com **JSON** e retornam `1`/`0` |
| **SQL** | sempre `sqlite3_prepare_v2` + `bind` (nunca `sprintf` com dados do usuário) |
| **Exclusão** | lógica (`ativo = 0`) ou por status (`CANCELADO`/`ALTA`), nunca `DELETE` físico |
| **Nomes** | identificadores e mensagens em português, sem acento no código |
| **Estilo** | chaves em linha própria (Allman), 4 espaços, funções curtas |

---

## 📁 Estrutura de pastas

```text
Gestao_Saude/
├── README.md
├── .gitignore
└── backend/
    ├── data/                         # dados
    │   ├── schema_v2.sql             # schema único (fonte da verdade)
    │   └── sigeh_v2.db               # banco gerado (ignorado pelo git)
    └── web/                          # backend web
        ├── Makefile                  # build e testes do backend web
        ├── database/
        │   ├── database.h
        │   └── database.c            # camada fina sobre o SQLite
        ├── repositories/             # acesso a dados — 1 por entidade
        │   ├── paciente_repository.{h,c}
        │   ├── medico_repository.{h,c}
        │   ├── ala_repository.{h,c}
        │   ├── leito_repository.{h,c}
        │   ├── triagem_repository.{h,c}
        │   ├── agendamento_repository.{h,c}
        │   ├── prontuario_repository.{h,c}
        │   ├── exame_repository.{h,c}
        │   ├── internacao_repository.{h,c}
        │   └── usuario_repository.{h,c}
        ├── services/
        │   ├── triagem_service.{h,c}  # motor de decisão da triagem
        │   └── relatorio_service.{h,c}# indicadores gerenciais
        ├── util/
        │   ├── repo_json.{h,c}        # montagem/escape de JSON
        │   └── senha_util.{h,c}       # hashing SHA-256 + salt (OpenSSL)
        ├── api/
        │   └── server.c              # servidor HTTP, rotas, auth e guardas
        ├── tests/                    # testes assert.h + api_smoke_test.sh
        └── build/                    # binários e banco de teste (ignorado)
```

---

## ⚙️ Pré-requisitos

| Ferramenta | Uso |
|---|---|
| **GCC** ou **clang** | compilação (C padrão) |
| **make** | orquestração de build/test |
| **SQLite 3** (`-lsqlite3`) | banco de dados |
| **OpenSSL** (`libcrypto`) | hash de senhas (SHA-256) |
| **curl** | testar a API (opcional) |

> 💡 **OpenSSL no macOS (Homebrew)** costuma ficar em `/opt/homebrew/opt/openssl@3`, que **não** está no path padrão do compilador. O `Makefile` já aponta para lá via `OPENSSL_DIR`. Em outro caminho (ex.: Linux), basta sobrescrever:
> ```sh
> make OPENSSL_DIR=/usr
> ```

---

## 🚀 Instalação e build

### Backend Web (V2)

```sh
cd backend/web
make            # compila a camada de banco e o servidor da API
make run        # sobe o servidor HTTP na porta 8080
```

Ao subir, o servidor imprime `SIGEH-DF API ouvindo em http://localhost:8080` e **fica aguardando requisições** (é um servidor — não tem tela própria). Para interagir, use `curl` ou um navegador em outro terminal. Para encerrar, `Ctrl+C`.

```sh
curl -i http://localhost:8080/health          # rota pública
curl -u admin:secreta http://localhost:8080/me # rota autenticada
```

> ⚠️ O servidor abre o banco em `../data/sigeh_v2.db` por **caminho relativo** — execute sempre **de dentro de `backend/web/`**.

### Alvos do Makefile

| Alvo | O que faz |
|---|---|
| `make` / `make all` | Compila `build/database.o` e o servidor `build/sigeh_api` |
| `make api` | Compila apenas o servidor da API |
| `make run` | Compila e sobe o servidor na porta 8080 |
| `make test` | Compila e roda as 12 suítes de teste |
| `make test_<nome>` | Roda uma suíte específica (ex.: `make test_triagem_service`) |
| `make api-smoke-test` | Executa `tests/api_smoke_test.sh` (testes de ponta a ponta via `curl`) |
| `make clean` | Remove o diretório `build/` (binários, objetos e banco de teste) |

> Todos os artefatos compilados vão para `backend/web/build/` (binários, `database.o` e o banco de teste), mantido fora do versionamento.

---

## 🧪 Testes

```sh
cd backend/web
make test
```

São **12 suítes** com `assert.h`. Cada uma **recria um banco de teste isolado** (`build/test_sigeh_repository.db`) a partir do schema, de modo que **não dependem de dados antigos nem do banco de produção**.

| Camada | Suítes |
|---|---|
| **Repositories** | paciente · medico · ala · leito · triagem · agendamento · prontuario · exame · internacao · usuario |
| **Services** | triagem_service · relatorio_service |

**Detalhes relevantes:**

- Como o banco roda com **chaves estrangeiras ativas**, os testes **semeiam os registros-pai** antes dos filhos (ex.: criam a ala antes do leito, o paciente antes da triagem).
- O `triagem_service` é testado de ponta a ponta: avaliação, sugestão de médicos, histórico, sugestão de exames, agendamento (incluindo conflito de horário) e encaminhamento.
- O `usuario_repository` valida criação, login único, autenticação correta/incorreta e — importante — que a **listagem nunca expõe senha/hash/salt**.

Para validar a API em execução (rotas, auth, papéis), há o **smoke test**:

```sh
make api-smoke-test
```

---

## 🔐 Autenticação

A API usa **HTTP Basic**: o cliente envia, em cada requisição, o cabeçalho
`Authorization: Basic base64(login:senha)`. O servidor decodifica, separa `login:senha` e valida contra a tabela `usuarios` — **a cada requisição** (stateless, sem sessão em memória).

**Como as senhas são guardadas** (nunca em texto puro):

```text
cadastro:   senha ─▶ salt aleatório ─▶ SHA-256(salt + senha) ─▶ guarda (salt, hash)
login:      senha + salt guardado ─▶ SHA-256 ─▶ compara com o hash guardado
```

O hashing usa **OpenSSL (EVP/SHA-256)** com salt por usuário. Os **logins são criados pelo administrador** (`POST /usuarios`), nunca por auto-cadastro.

```sh
# uso do HTTP Basic com curl (-u login:senha)
curl -u admin:secreta http://localhost:8080/me
# -> {"papel":"ADMIN","pacienteId":0,"medicoId":0}
```

---

## 👥 Autorização e papéis

Toda rota exige autenticação, **exceto** `GET /health`. A política de acesso é **centralizada** numa única função (`autorizado(método, rota, papel)`) aplicada no topo do roteador — fácil de auditar e alterar.

| Papel | Permissões |
|---|---|
| **ADMIN** | Acesso total, incluindo o cadastro de usuários (`/usuarios`). |
| **CADASTRO** | CRUD completo dos cadastros: `pacientes`, `medicos`, `alas`, `leitos`. |
| **MEDICO** | Leitura dos cadastros **+** todo o clínico (triagens, agendamentos, prontuários, exames, internações, triagem inteligente, relatórios) **+** suas rotas `/me`. Nas listas amplas, vê **apenas os próprios dados** (escopo por identidade — veja abaixo). |
| **ENFERMAGEM** | Visão de enfermaria: **leitura** de internações, leitos e alas. |
| **PACIENTE** | Apenas os próprios dados, via `/me` (exames e prontuários). |

### Matriz de acesso (resumo)

| Grupo de rotas | ADMIN | CADASTRO | MEDICO | ENFERMAGEM | PACIENTE |
|---|:---:|:---:|:---:|:---:|:---:|
| `/usuarios` | ✅ | ❌ | ❌ | ❌ | ❌ |
| Cadastros — leitura (`GET`) | ✅ | ✅ | ✅ | ❌ | ❌ |
| Cadastros — escrita (`POST`/`DELETE`) | ✅ | ✅ | ❌ | ❌ | ❌ |
| Clínico (triagens, agend., pront., exames) | ✅ | ❌ | ✅ | ❌ | ❌ |
| Internações/leitos/alas — leitura | ✅ | ✅ | ✅ | ✅ | ❌ |
| Triagem inteligente + relatórios | ✅ | ❌ | ✅ | ❌ | ❌ |
| `/me` e `/me/...` | ✅ | ✅ | ✅ | ✅ | ✅ |

<sub>\* CADASTRO acessa alas/leitos como parte dos cadastros; internações são clínicas.</sub>

#### Escopo de dados por identidade (MEDICO)

Além da autorização por papel, as **listagens amplas são filtradas pela identidade** quando quem chama é um `MEDICO`: a mesma URL devolve só os dados dele.

| Rota | ADMIN / CADASTRO | MEDICO |
|---|---|---|
| `GET /pacientes` | todos | apenas pacientes com agendamento com o médico |
| `GET /agendamentos` | todos | apenas a própria agenda |

As contagens (`/...../contar`) permanecem **globais** (indicadores). As rotas `/me/...` seguem como o caminho explícito do "só o seu".

Respostas: **`401`** sem credencial válida · **`403`** papel sem permissão.

---

## 🌐 Referência da API

> Todas as rotas exigem autenticação, **exceto** `GET /health`. Os parâmetros de criação vão na **query string**.

### Saúde e sessão

| Método | Rota | Papel | Descrição |
|---|---|---|---|
| `GET` | `/health` | público | Status do serviço e do banco |
| `GET` | `/me` | autenticado | Papel e vínculos do usuário logado |
| `GET` | `/me/exames` | PACIENTE | Exames do próprio paciente |
| `GET` | `/me/prontuarios` | PACIENTE | Prontuários do próprio paciente |
| `GET` | `/me/agenda` | MEDICO | Agenda do próprio médico |
| `GET` | `/me/pacientes` | MEDICO | Pacientes do próprio médico |

### Usuários · ADMIN

| Método | Rota | Parâmetros |
|---|---|---|
| `GET` | `/usuarios` · `/usuarios/contar` | — |
| `POST` | `/usuarios` | `login`, `senha`, `papel`, `paciente_id`, `medico_id` |
| `DELETE` | `/usuarios/{id}` | — |

### Cadastros · ADMIN, CADASTRO (MEDICO só leitura)

Disponível para `pacientes`, `medicos`, `alas`, `leitos`:

| Método | Rota | Parâmetros de criação (exemplos) |
|---|---|---|
| `GET` | `/{entidade}` · `/{entidade}/contar` | — |
| `POST` | `/{entidade}` | `pacientes`: `nome,cpf,idade,telefone,sexo,regiao` · `medicos`: `nome,crm,especialidade,regiao` · `alas`: `nome,tipo,total_leitos` · `leitos`: `ala_id,numero` |
| `DELETE` | `/{entidade}/{id}` | exclusão lógica |

### Clínico · ADMIN, MEDICO (ENFERMAGEM lê internações/leitos/alas)

Disponível para `triagens`, `agendamentos`, `prontuarios`, `exames`, `internacoes`:

| Método | Rota | Observação |
|---|---|---|
| `GET` | `/{entidade}` · `/{entidade}/contar` | listar / contar |
| `POST` | `/{entidade}` | criar (parâmetros conforme a entidade) |
| `DELETE` | `/{entidade}/{id}` | exclusão lógica · em `agendamentos` = **cancelar** |
| `POST` | `/internacoes/{id}/alta` | `data` — dá alta (internações não têm `DELETE`) |

### Triagem inteligente · ADMIN, MEDICO

| Método | Rota | Descrição |
|---|---|---|
| `GET` | `/triagem/{pacienteId}/avaliacao` | Risco, prioridade e especialidade provável |
| `GET` | `/triagem/{pacienteId}/medicos` | Médicos sugeridos (especialidade + região) |
| `GET` | `/triagem/{pacienteId}/historico` | Prontuários e exames anteriores |
| `GET` | `/triagem/{pacienteId}/exames` | Exames iniciais sugeridos |
| `POST` | `/triagem/{pacienteId}/agendar` | `data`, `horario` — agenda com médico disponível |
| `POST` | `/triagem/{pacienteId}/encaminhar` | `especialidade`, `data`, `horario` — encaminha |

### Relatórios · ADMIN, MEDICO

| Método | Rota | Descrição |
|---|---|---|
| `GET` | `/relatorios/indicadores` | Totais, triagens por classificação e casos graves |
| `GET` | `/relatorios/distribuicao` | Pacientes por região administrativa e médicos por especialidade |
| `GET` | `/relatorios/agendamentos?inicio=AAAA-MM-DD&fim=AAAA-MM-DD` | Agendamentos não-cancelados no período: total e distribuição por dia (`400` se faltar `inicio`/`fim`) |

Exemplo de resposta de `/relatorios/distribuicao`:

```json
{
  "pacientesPorRegiao": [{ "regiao": 1, "total": 3 }, { "regiao": 7, "total": 1 }],
  "medicosPorEspecialidade": [{ "especialidade": "Cardiologia", "total": 2 }]
}
```

Exemplo de resposta de `/relatorios/agendamentos?inicio=2026-06-01&fim=2026-06-30`:

```json
{
  "inicio": "2026-06-01",
  "fim": "2026-06-30",
  "total": 2,
  "porDia": [{ "data": "2026-06-14", "total": 2 }]
}
```

---

## 🧭 Walkthrough completo

Um roteiro de ponta a ponta, do cadastro à decisão. (Assume um usuário `admin/secreta` já existente.)

```sh
B=http://localhost:8080

# 1) Admin cadastra um médico e um paciente
curl -u admin:secreta -X POST \
  "$B/medicos?nome=Dra+Helena&crm=CRM123&especialidade=Cardiologia&regiao=7"
curl -u admin:secreta -X POST \
  "$B/pacientes?nome=Maria&cpf=12345678900&idade=62&telefone=6199990000&sexo=F&regiao=7"

# 2) Admin cria os logins (médico vinculado ao médico #1, paciente ao paciente #1)
curl -u admin:secreta -X POST "$B/usuarios?login=helena&senha=h123&papel=MEDICO&medico_id=1"
curl -u admin:secreta -X POST "$B/usuarios?login=maria&senha=m123&papel=PACIENTE&paciente_id=1"

# 3) Registra a triagem da paciente (cardiologia, emergência)
curl -u helena:h123 -X POST \
  "$B/triagens?paciente_id=1&tipo=3&pontuacao=8&classificacao=Emergencia"

# 4) Avaliação inteligente da triagem
curl -u helena:h123 "$B/triagem/1/avaliacao"
# -> {"pacienteId":1,"classificacao":"Emergencia","prioridade":5,"especialidadeProvavel":"Cardiologia"}

# 5) Exames sugeridos e médicos disponíveis
curl -u helena:h123 "$B/triagem/1/exames"   # -> ["Eletrocardiograma","Hemograma"]
curl -u helena:h123 "$B/triagem/1/medicos"

# 6) Agendamento automático (slot válido na grade/expediente)
curl -u helena:h123 -X POST "$B/triagem/1/agendar?data=2026-07-01&horario=09:00"
# -> {"agendado":true,"pacienteId":1,"medicoId":1,"data":"2026-07-01","horario":"09:00"}

# 7) A paciente acessa só os próprios dados
curl -u maria:m123 "$B/me/exames"
curl -u maria:m123 "$B/pacientes"     # -> 403 Forbidden

# 8) Indicadores gerenciais
curl -u helena:h123 "$B/relatorios/indicadores"
```

---

## 🧠 Triagem inteligente

O fluxo coberto pelo motor de decisão:

```text
Paciente passa pela triagem
   └─▶ calcula risco, prioridade e especialidade provável      (/triagem/{id}/avaliacao)
       └─▶ consulta histórico: prontuários e exames anteriores  (/triagem/{id}/historico)
           └─▶ sugere exames iniciais conforme o tipo           (/triagem/{id}/exames)
               └─▶ encontra médicos por especialidade + região  (/triagem/{id}/medicos)
                   └─▶ agenda com médico disponível             (POST /triagem/{id}/agendar)
                       └─▶ ou encaminha para outra especialidade (POST /triagem/{id}/encaminhar)
```

**Mapa tipo de triagem → especialidade:**

| Tipo | Código | Especialidade provável |
|---|:---:|---|
| Geral | 1 | Clínico Geral |
| Ortopedia | 2 | Ortopedia |
| Cardiologia | 3 | Cardiologia |
| Pneumologia | 4 | Pneumologia |
| Pediatria | 5 | Pediatria |

**Mapa tipo → exames iniciais sugeridos:**

| Tipo | Exames |
|---|---|
| Geral | Hemograma, Urina |
| Ortopedia | Raio-X |
| Cardiologia | Eletrocardiograma, Hemograma |
| Pneumologia | Raio-X, Hemograma |
| Pediatria | Hemograma |

**Classificação → prioridade:** `Emergencia` (5) · `Muito prioritario` (4) · `Prioritario` (3) · `Comum` (2) · `Orientacao basica` (1).

**Regra de agenda:** expediente **08:00–18:00** e **grade de 30 min** (08:00, 08:30, …, 17:30). Horários fora da grade ou do expediente são **recusados em todos os caminhos de escrita** (agendar, encaminhar e `POST /agendamentos` direto). O conflito é por slot ocupado (mesmo médico, mesma data/horário, não cancelado). Esses valores são constantes no topo de `agendamento_repository.c`.

---

## 🗃️ Modelo de domínio

Dez tabelas em [`schema_v2.sql`](backend/data/schema_v2.sql). Campos por entidade:

<details>
<summary><b>Pacientes, Médicos e Usuários</b></summary>

**`pacientes`** — `id`, `nome`, `cpf`, `idade`, `telefone`, `sexo`, `regiao_administrativa`, `ativo`
**`medicos`** — `id`, `nome`, `crm`, `especialidade`, `regiao_administrativa`, `ativo`
**`usuarios`** — `id`, `login` (único), `senha_hash`, `salt`, `papel`, `paciente_id`, `medico_id`, `ativo`

</details>

<details>
<summary><b>Triagem, Agendamento, Prontuário e Exame</b></summary>

**`triagens`** — `id`, `paciente_id` (FK), `tipo_triagem`, `pontuacao`, `classificacao`, `ativo`
**`agendamentos`** — `id`, `paciente_id` (FK), `medico_id` (FK), `data`, `horario`, `status`
**`prontuarios`** — `id`, `paciente_id` (FK), `medico_id` (FK), `data`, `observacoes`, `diagnostico`, `conduta`, `alerta_importante`, `ativo`
**`exames`** — `id`, `paciente_id` (FK), `medico_id` (FK), `prontuario_id` (FK), `tipo_exame`, `data_solicitacao`, `data_resultado`, `resultado`, `status`, `urgente`, `ativo`

</details>

<details>
<summary><b>Ala, Leito e Internação</b></summary>

**`alas`** — `id`, `nome`, `tipo`, `total_leitos`, `leitos_ocupados`, `ativo`
**`leitos`** — `id`, `ala_id` (FK), `numero`, `ocupado`, `paciente_id`, `ativo`
**`internacoes`** — `id`, `paciente_id` (FK), `ala_id` (FK), `leito_id` (FK), `data_entrada`, `data_alta`, `status`

</details>

### Enumerações

| Conjunto | Valores |
|---|---|
| **Tipos de triagem** | 1 Geral · 2 Ortopedia · 3 Cardiologia · 4 Pneumologia · 5 Pediatria |
| **Tipos de exame** | 1 Hemograma · 2 Raio-X · 3 Tomografia · 4 Ressonância · 5 Eletrocardiograma · 6 Urina · 7 Ultrassonografia |
| **Tipos de ala** | 1 Internação · 2 UTI · 3 Observação · 4 Pediatria · 5 Cirúrgica |
| **Papéis de usuário** | ADMIN · CADASTRO · MEDICO · ENFERMAGEM · PACIENTE |

### Estados e máquinas de estado

```text
Agendamento:   AGENDADO ──cancelar──▶ CANCELADO
Internação:    INTERNADO ──dar alta──▶ ALTA
Exclusão lógica (ativo 1→0): paciente, médico, ala, leito, triagem, prontuário, exame, usuário
```

> A integridade entre entidades é garantida pelo banco (`PRAGMA foreign_keys = ON`): não é possível, por exemplo, criar um leito numa ala inexistente ou uma triagem para um paciente que não existe.

---

## 📟 Códigos HTTP

| Código | Quando |
|---|---|
| `200 OK` | Leitura/operação bem-sucedida |
| `201 Created` | Recurso criado |
| `400 Bad Request` | Dados inválidos / requisição malformada |
| `401 Unauthorized` | Sem credencial válida (HTTP Basic) |
| `403 Forbidden` | Autenticado, mas o papel não permite |
| `404 Not Found` | Rota ou recurso inexistente |
| `409 Conflict` | Conflito de regra (ex.: sem médico disponível, horário inválido) |
| `500 Internal Server Error` | Falha ao gerar resposta |

---

## 🧩 Como estender o projeto

Adicionar uma **nova entidade** com endpoint segue sempre o mesmo padrão:

1. **Schema** — adicione a tabela em `data/schema_v2.sql`.
2. **Repository** — crie `repositories/nova_repository.{h,c}` com `criar`, `listar_json`, `desativar`, `contar_ativos` (use `repo_json` para montar o JSON e prepared statements no SQL).
3. **Teste** — crie `tests/test_nova_repository.c` recriando o banco isolado e cobrindo criação, validações e contagem.
4. **Makefile** — adicione `NOVA_SRC`, o alvo `test_nova_repository`, inclua-o em `test` e no `.PHONY` (o `clean` já apaga todo o `build/`). Se a entidade for exposta, some `NOVA_SRC` ao `API_DEPS`.
5. **API** — em `api/server.c`, adicione o handler e as rotas (use os helpers `responderLista`, `responderContagem`, `responderCriacao`, `responderRemocao`) e ajuste a função `autorizado` se necessário.
6. **Verifique** — `make clean && make && make test` (sem warnings) e um `curl` na rota nova.

> Para **regra de negócio** que cruza entidades, crie um *service* em `services/` em vez de inflar o repository ou a API.

---

## 🔒 Segurança

- ✅ **Prepared statements** em toda entrada externa → imune a SQL injection.
- ✅ **Senhas com hash** SHA-256 + salt por usuário (OpenSSL). A listagem de usuários **nunca** devolve `senha_hash`/`salt`.
- ✅ **Integridade referencial** no banco (FK).
- ✅ **Autorização centralizada** por papel, negando por padrão.
- ⚠️ **Limitações acadêmicas conhecidas:** o servidor é HTTP (sem TLS) e single-thread; a criação via API usa query string (inclusive senha) — num cenário real, credenciais iriam no corpo da requisição sobre **HTTPS**, idealmente com um algoritmo de hashing com fator de trabalho (bcrypt/argon2).

---

## 🛣️ Roadmap

- 💊 **Módulo de prescrição/medicação** — base para a enfermagem ver "remédios a aplicar" e o paciente ver receitas (entidade ainda inexistente).
- 🖥️ **Frontend web** consumindo a API (`GET /me` já entrega o papel para a UI decidir o que exibir).
- 🔎 **Escopo de dados nas listas globais** — hoje quem tem permissão de leitura vê listas amplas; o escopo "só o seu" vive nas rotas `/me`.
- 📈 **Relatórios mais ricos** (por período, especialidade, região).
- 🧪 **Testes de integração da API** automatizados além do smoke test atual.
- 🧵 **Servidor concorrente** (hoje single-thread/bloqueante).

---

## 🎓 Notas acadêmicas

- Projeto construído para **clareza didática**, priorizando legibilidade e regras de negócio sobre performance.
- O backend web demonstra, em C básico, conceitos de **arquitetura em camadas, acesso a dados, regras de negócio, API HTTP e autenticação** — sem frameworks.
- A primeira versão era um app de **terminal** (CLI, dados em memória); a triagem já alimentava o agendamento por especialidade, região e disponibilidade — a semente da triagem inteligente que a V2 expandiu para a web. Esse protótipo foi descontinuado e seu histórico está preservado no git.
- Artefatos gerados (binários, `*.db`, `*.o`) ficam **fora** do versionamento; o **schema** é a fonte da verdade e o banco é sempre reconstruível a partir dele.
- Todo o código compila com `-Wall -Wextra -pedantic` **sem warnings**, e as 12 suítes de teste passam.

<div align="center">

---

Feito com `gcc -Wall -Wextra -pedantic` e muito cuidado com cada `free()`. 🩺

</div>
