# Arquitetura do SIGEH-DF

## Organização

- `src/backend/data`: schema SQLite e banco local. Dados de runtime não fazem
  parte da estrutura versionada.
- `src/backend/web`: servidor HTTP/HTTPS, módulos C, testes e Makefile.
- `src/frontend`: aplicação React/Vite e testes de componentes.
- `docs`: documentação técnica complementar.
- `imagens`: diagramas, capturas de tela e imagens da documentação.
- `executavel`: binário e frontend preparados para distribuição.

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

Diretórios `build`, `dist`, `public` gerado, bancos locais, arquivos enviados e
certificados não fazem parte do código-fonte versionado. A distribuição final
fica separada em `executavel/`.
