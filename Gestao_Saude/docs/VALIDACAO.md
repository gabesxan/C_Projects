# Validação do projeto

Backend:

```sh
cd src/backend/web
make api
make test
make api-smoke-test
make api-integration-test
make api-tls-smoke-test
```

Frontend:

```sh
cd src/frontend
npm test
npm run lint
npm run build
```

O projeto possui 28 suítes C, 29 testes frontend, smoke HTTP, integração da API
e smoke HTTPS.
