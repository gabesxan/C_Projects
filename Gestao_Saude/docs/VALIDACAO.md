<div align="center">

# ✅ Validação do SIGEH-DF

### Como provar que o sistema está saudável — backend e frontend

![Testes C](https://img.shields.io/badge/suítes%20C-29%2F29-brightgreen)
![Testes Frontend](https://img.shields.io/badge/testes%20frontend-39%2F39-brightgreen)
![Warnings](https://img.shields.io/badge/warnings-0-brightgreen)
![Smoke](https://img.shields.io/badge/smoke-HTTP%20%2B%20HTTPS-0b7285)

</div>

---

## 📑 Conteúdo

- [Resumo da cobertura](#-resumo-da-cobertura)
- [Validação do backend](#-validação-do-backend)
- [Validação do frontend](#-validação-do-frontend)
- [O que cada teste cobre](#-o-que-cada-teste-cobre)
- [Saída esperada](#-saída-esperada)
- [Solução de problemas](#-solução-de-problemas)

---

## 📊 Resumo da cobertura

| Camada | Tipo | Quantidade | Ferramenta |
|---|---|---|---|
| Backend | Suítes unitárias C | **29** | `assert.h` |
| Backend | Smoke HTTP | 1 | `curl` + shell |
| Backend | Integração ponta a ponta | 1 | `curl` + shell |
| Backend | Smoke HTTPS (TLS) | 1 | `curl` + OpenSSL |
| Frontend | Testes de componente | **39** (8 arquivos) | Vitest + Testing Library |
| Frontend | Lint | — | ESLint |
| Frontend | Build | — | Vite |

> [!NOTE]
> Todo o código C compila com `-Wall -Wextra -pedantic` **sem warnings**.

---

## ⚙️ Validação do backend

A partir de [`src/backend/web`](../src/backend/web):

```sh
cd src/backend/web

make api                    # 🔨 compila o servidor (deve terminar sem warnings)
make test                   # 🧪 roda as 29 suítes unitárias em C
make api-smoke-test         # 💨 smoke HTTP: liveness + escopo por papel
make api-integration-test   # 🔗 fluxos ponta a ponta encadeados
make api-tls-smoke-test     # 🔐 smoke HTTPS: TLS no ar, HTTP puro rejeitado
```

| Alvo | O que valida |
|---|---|
| `make api` | Compilação limpa de toda a árvore C. |
| `make test` | Repositories, services e utilitários isoladamente, contra um banco temporário. |
| `make api-smoke-test` | Servidor sobe, `/health` responde, autenticação e **autorização por papel** funcionam. |
| `make api-integration-test` | Fluxos reais encadeados: recepção → triagem → agendamento → exames → financeiro → farmácia → vacinação → anexos → **consentimentos e relatório LGPD**. |
| `make api-tls-smoke-test` | Servidor TLS responde por HTTPS e **rejeita HTTP puro** na porta segura. |

---

## 🎨 Validação do frontend

A partir de [`src/frontend`](../src/frontend):

```sh
cd src/frontend

npm test          # 🧪 39 testes (8 arquivos) com Vitest + Testing Library
npm run lint      # 🧹 ESLint (zero problemas)
npm run build     # 📦 build de produção com Vite
```

---

## 🔍 O que cada teste cobre

<details>
<summary><b>29 suítes unitárias em C</b> (clique para expandir)</summary>

| # | Suíte | Foco |
|---|---|---|
| 1 | `test_paciente_repository` | CRUD e busca de pacientes |
| 2 | `test_medico_repository` | Médicos, especialidade e região |
| 3 | `test_ala_repository` | Alas hospitalares |
| 4 | `test_leito_repository` | Leitos e status |
| 5 | `test_triagem_repository` | Triagens e versionamento |
| 6 | `test_agendamento_repository` | Agendamentos por paciente/médico |
| 7 | `test_prontuario_repository` | Prontuários clínicos |
| 8 | `test_exame_repository` | Exames, analitos e resultados |
| 9 | `test_internacao_repository` | Internações e transferências |
| 10 | `test_triagem_service` | Motor de triagem inteligente |
| 11 | `test_relatorio_service` | Indicadores e distribuições |
| 12 | `test_usuario_repository` | Usuários e papéis |
| 13 | `test_prescricao_repository` | Prescrições e administração |
| 14 | `test_auditoria_repository` | Trilha de auditoria (append-only) |
| 15 | `test_credencial_util` | Credenciais e hash de senha |
| 16 | `test_enfermagem_repository` | Evoluções de enfermagem |
| 17 | `test_checkin_repository` | Fila de recepção |
| 18 | `test_financeiro_repository` | Convênios e cobranças |
| 19 | `test_sessao_repository` | Sessões e bloqueio por tentativas |
| 20 | `test_migracoes` | Migrações de schema por `user_version` |
| 21 | `test_lote_repository` | Lotes de faturamento |
| 22 | `test_analito_repository` | Catálogo de analitos |
| 23 | `test_solicitacao_repository` | Solicitações do paciente |
| 24 | `test_medicamento_repository` | Catálogo de medicamentos |
| 25 | `test_estoque_repository` | Estoque e movimentações |
| 26 | `test_vacina_repository` | Vacinas e carteira |
| 27 | `test_anexo_repository` | Anexos/documentos |
| 28 | `test_consentimento_repository` | Consentimentos LGPD (histórico imutável) |
| 29 | `test_notificacao_repository` | Notificações in-app (fan-out por papel, leitura) |

</details>

<details>
<summary><b>Testes de frontend (Vitest)</b></summary>

Cobrem os componentes e fluxos críticos do portal, incluindo:

- Render da aplicação e navegação base.
- `ResourceForm` e `FieldSelect` (formulários e seleção de entidades).
- `AnexoPanel` (upload, validação de MIME/tamanho, download, remoção com motivo).
- `ConsentimentoWallet` (carteira LGPD: consentimentos concedidos/revogados,
  revogação com motivo obrigatório, relatório de acessos, estados de
  carregamento/erro/vazio).
- `NotificationBell` (sino in-app: contador de não lidas, abertura do painel,
  marcar uma/todas como lidas, estado vazio).

</details>

---

## 🖥️ Saída esperada

```text
$ make test
test_paciente_repository: OK
test_medico_repository: OK
...
test_notificacao_repository: OK          # 29 linhas "OK", nenhuma falha

$ npm test
 Test Files  8 passed (8)
      Tests  39 passed (39)
```

> [!TIP]
> Qualquer linha diferente de `OK` (ou qualquer `FAIL`/`assert`) interrompe a
> suíte com código de saída diferente de zero — ótimo para CI.

---

## 🧯 Solução de problemas

| Sintoma | Causa provável | Como resolver |
|---|---|---|
| `openssl/...: No such file` ao compilar | OpenSSL fora do caminho padrão | `make OPENSSL_DIR=/usr` (Linux) ou ajuste o prefixo do Homebrew |
| Smoke/integração falham ao subir | Porta `8080` ocupada | Encerre o processo na porta ou aguarde o teste liberar |
| `npm test`/`lint` não rodam | Dependências ausentes | `cd src/frontend && npm install` |
| Banco "sujo" entre execuções | `.db` antigo | `make seed` (recria do schema) ou `make clean` |

---

<div align="center">

📚 Veja também: [README.md](../README.md) · [ARQUITETURA.md](ARQUITETURA.md) · [manual.md](../manual.md)

</div>
