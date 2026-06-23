# Manual do SIGEH-DF

Este manual apresenta os comandos essenciais para instalar, validar e executar
o SIGEH-DF após a organização do repositório.

## 1. Estrutura principal

```text
Gestao_Saude/
├── src/
│   ├── backend/
│   └── frontend/
├── docs/
├── imagens/
├── executavel/
├── README.md
└── manual.md
```

O código-fonte fica exclusivamente em `src/`. A pasta `executavel/` recebe
somente o binário e o frontend preparados para distribuição.

## 2. Pré-requisitos

- GCC ou Clang
- Make
- SQLite 3
- OpenSSL
- Node.js e npm
- curl

No macOS com Homebrew, o backend usa por padrão
`/opt/homebrew/opt/openssl@3`. Em Linux, use:

```sh
make OPENSSL_DIR=/usr
```

## 3. Instalação

Instale as dependências do frontend:

```sh
cd src/frontend
npm install
```

Crie o banco com os dados de exemplo:

```sh
cd ../backend/web
make seed
```

## 4. Executar em desenvolvimento

Backend, a partir da raiz:

```sh
cd src/backend/web
make run
```

Frontend, em outro terminal:

```sh
cd src/frontend
npm run dev
```

Abra `http://localhost:5173`. O Vite encaminha `/api` para o backend em
`http://localhost:8080`.

## 5. Validação

```sh
cd src/backend/web
make api
make test
make api-smoke-test
make api-integration-test
make api-tls-smoke-test

cd ../../frontend
npm test
npm run lint
npm run build
```

## 6. Gerar distribuição

Copie o binário e o frontend compilado para:

```text
executavel/
├── bin/sigeh_api
└── frontend/
```

Para gerar os arquivos:

```sh
cd src/backend/web
make api
mkdir -p ../../../executavel/bin
cp build/sigeh_api ../../../executavel/bin/

cd ../../frontend
npm run build
mkdir -p ../../executavel/frontend
cp -R dist/. ../../executavel/frontend/
```

O frontend também pode ser incorporado ao servidor em C:

```sh
cd src/backend/web
make frontend
make run
```

Nesse modo, abra `http://localhost:8080`.

## 7. Credenciais do seed

| Papel | Login | Senha |
|---|---|---|
| ADMIN | `admin` | `admin123` |
| CADASTRO | `cadastro` | `cadastro123` |
| MEDICO | `medico` | `medico123` |
| ENFERMAGEM | `enfermagem` | `enfermagem123` |
| PACIENTE | `paciente` | `paciente123` |

## 8. Limpeza

```sh
cd src/backend/web
make clean

cd ../../frontend
rm -rf dist
```
