<div align="center">

# 📦 Artefatos executáveis

### Distribuição pronta para rodar — binário do servidor + frontend buildado

![Conteúdo](https://img.shields.io/badge/conteúdo-binário%20%2B%20frontend-0b7285)
![Git](https://img.shields.io/badge/versionado-não%20(gerado)-lightgrey)

</div>

---

## 🎯 Para que serve

Esta pasta reúne os **artefatos prontos para distribuição** do SIGEH-DF: o
binário compilado do servidor e o frontend buildado. É o que você copia/entrega
para executar o sistema **sem precisar compilar** na máquina de destino.

---

## 🗂️ Estrutura

```text
executavel/
├── bin/
│   └── sigeh_api      # servidor HTTP/HTTPS compilado (gerado por `make api`)
└── frontend/          # frontend de produção (gerado por `npm run build`)
```

> [!NOTE]
> Os diretórios `bin/` e `frontend/` são **gerados** e ficam **fora do controle
> de versão** (ignorados pelo Git). O código-fonte vive em [`../src/`](../src).

---

## 🚀 Como gerar

O passo a passo completo está em
[manual.md → 6. Gerar distribuição](../manual.md#6-gerar-distribuição). Em resumo:

```sh
# binário
cd ../src/backend/web && make api
mkdir -p ../../../executavel/bin && cp build/sigeh_api ../../../executavel/bin/

# frontend
cd ../../frontend && npm run build
mkdir -p ../../executavel/frontend && cp -R dist/. ../../executavel/frontend/
```

---

## ▶️ Como executar a distribuição

```sh
cd executavel
./bin/sigeh_api
```

O servidor sobe na porta **8080**. Sirva a pasta `frontend/` por trás dele (ou
embuta o frontend no próprio servidor com `make frontend`, conforme o manual).
