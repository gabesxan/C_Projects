<div align="center">

# 📘 Manual do SIGEH-DF

### Instalar · Validar · Executar · Distribuir

![Setup](https://img.shields.io/badge/guia-passo%20a%20passo-0b7285)
![Plataformas](https://img.shields.io/badge/SO-macOS%20%7C%20Linux-555)
![Frontend](https://img.shields.io/badge/frontend-React%20%2B%20Vite-61DAFB?logo=react&logoColor=black)

</div>

> Guia objetivo para colocar o **SIGEH-DF** de pé do zero. Para a visão completa
> do sistema e a referência da API, veja o [README.md](README.md);
> para a arquitetura, [docs/ARQUITETURA.md](docs/ARQUITETURA.md).

---

## 📑 Conteúdo

1. [Estrutura principal](#1-estrutura-principal)
2. [Pré-requisitos](#2-pré-requisitos)
3. [Instalação](#3-instalação)
4. [Executar em desenvolvimento](#4-executar-em-desenvolvimento)
5. [Validação](#5-validação)
6. [Gerar distribuição](#6-gerar-distribuição)
7. [Credenciais do seed](#7-credenciais-do-seed)
8. [Limpeza](#8-limpeza)

---

## 1. Estrutura principal

```text
Gestao_Saude/
├── src/
│   ├── backend/        # servidor C + banco SQLite
│   └── frontend/       # aplicação React/Vite
├── docs/               # 📚 documentação técnica
├── imagens/            # capturas de tela e diagramas
├── executavel/         # 📦 distribuição (binário + frontend buildado)
├── README.md           # visão geral e referência da API
└── manual.md           # este guia
```

> [!NOTE]
> O código-fonte fica **exclusivamente** em `src/`. A pasta `executavel/`
> recebe somente o binário e o frontend prontos para distribuição.

---

## 2. Pré-requisitos

| Ferramenta | Para quê |
|---|---|
| **GCC ou Clang** | Compilar o backend em C |
| **Make** | Orquestrar build, testes e seed |
| **SQLite 3** | Banco de dados |
| **OpenSSL** | Hash de senha e TLS |
| **Node.js + npm** | Build e testes do frontend |
| **curl** | Smoke/integração da API |

No **macOS** com Homebrew, o backend usa por padrão `/opt/homebrew/opt/openssl@3`.
No **Linux**, aponte o OpenSSL do sistema:

```sh
make OPENSSL_DIR=/usr
```

---

## 3. Instalação

**1.** Dependências do frontend:

```sh
cd src/frontend
npm install
```

**2.** Criar o banco com dados de exemplo (seed):

```sh
cd ../backend/web
make seed
```

> [!TIP]
> O banco (`.db`) é descartável. Rode `make seed` sempre que quiser um estado
> limpo e populado, ou `make db-reset` para recriar do zero.

---

## 4. Executar em desenvolvimento

**Backend** (terminal 1):

```sh
cd src/backend/web
make run
```

**Frontend** (terminal 2):

```sh
cd src/frontend
npm run dev
```

Abra **`http://localhost:5173`**. O Vite encaminha as chamadas `/api` para o
backend em `http://localhost:8080`.

---

## 5. Validação

```sh
# Backend
cd src/backend/web
make api                    # compila sem warnings
make test                   # 28 suítes unitárias C
make api-smoke-test         # smoke HTTP
make api-integration-test   # integração ponta a ponta
make api-tls-smoke-test     # smoke HTTPS

# Frontend
cd ../../frontend
npm test                    # 34 testes (7 arquivos) com Vitest
npm run lint                # ESLint
npm run build               # build de produção
```

📖 Detalhes de o que cada teste cobre em [docs/VALIDACAO.md](docs/VALIDACAO.md).

---

## 6. Gerar distribuição

A pasta de distribuição tem este formato:

```text
executavel/
├── bin/sigeh_api        # binário do servidor
└── frontend/            # frontend buildado
```

Para gerá-la:

```sh
# Binário do servidor
cd src/backend/web
make api
mkdir -p ../../../executavel/bin
cp build/sigeh_api ../../../executavel/bin/

# Frontend buildado
cd ../../frontend
npm run build
mkdir -p ../../executavel/frontend
cp -R dist/. ../../executavel/frontend/
```

**Alternativa — frontend embutido no servidor C:**

```sh
cd src/backend/web
make frontend     # copia o build do frontend para public/
make run
```

Nesse modo, tudo é servido pela mesma origem: abra **`http://localhost:8080`**.

---

## 7. Credenciais do seed

| Papel | Login | Senha |
|---|---|---|
| 🟣 ADMIN | `admin` | `admin123` |
| 🔵 CADASTRO | `cadastro` | `cadastro123` |
| 🟢 MEDICO | `medico` | `medico123` |
| 🟩 ENFERMAGEM | `enfermagem` | `enfermagem123` |
| 🟠 PACIENTE | `paciente` | `paciente123` |
| 🟠 PACIENTE (2º) | `paciente2` | `paciente123` |

> [!WARNING]
> Estas credenciais são **apenas para desenvolvimento/demonstração**. No
> primeiro acesso o sistema pode exigir troca de senha. Nunca use senhas de
> seed em ambientes reais.

---

## 8. Limpeza

```sh
# Backend (remove build/, binários e artefatos)
cd src/backend/web
make clean

# Frontend (remove o build de produção)
cd ../../frontend
rm -rf dist
```

---

<div align="center">

📚 [README.md](README.md) · [docs/ARQUITETURA.md](docs/ARQUITETURA.md) · [docs/VALIDACAO.md](docs/VALIDACAO.md)

</div>
