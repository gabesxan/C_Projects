# Arquitetura do SIGEH-DF

## Organização

- `src/backend/data`: schema SQLite, banco local e anexos de runtime.
- `src/backend/web`: servidor HTTP/HTTPS, módulos C, testes e Makefile.
- `src/frontend`: aplicação React/Vite e testes de componentes.
- `docs`: documentação técnica complementar.
- `imagens`: diagramas, capturas de tela e imagens da documentação.
- `executavel`: distribuição gerada por `make release`.

## Backend

O backend mantém suas camadas atuais:

```text
api → services → repositories → database → SQLite
```

Utilitários transversais permanecem em `util/`. O `Makefile` do backend deve ser
executado dentro de `src/backend/web` ou por meio do Makefile da raiz.

## Frontend

O frontend usa React, Vite e Tailwind. Em desenvolvimento, o proxy `/api`
encaminha requisições para a porta 8080. Em produção, o servidor C pode servir
o build copiado para `src/backend/web/public`.

## Artefatos

Diretórios `build`, `dist`, `public` gerado, bancos locais, anexos e
certificados não fazem parte do código-fonte versionado. O alvo `make release`
reúne o binário e o frontend em `executavel/`.
