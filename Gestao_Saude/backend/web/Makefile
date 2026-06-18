CC = gcc

# OpenSSL (hashing de senha). Sobrescreva OPENSSL_DIR se estiver noutro caminho.
OPENSSL_DIR ?= /opt/homebrew/opt/openssl@3

CFLAGS = -Wall -Wextra -pedantic \
	-Idatabase \
	-Irepositories \
	-Iservices \
	-Iutil \
	-I$(OPENSSL_DIR)/include

LDLIBS = -lsqlite3
MATH_LIB = -lm
CRYPTO_LIBS = -L$(OPENSSL_DIR)/lib -lcrypto
# Servidor concorrente (pool de threads).
THREAD_LIB = -pthread

# Diretorio de saida para binarios e bancos de teste (gerado, ignorado no git).
BUILD = build

DB_SRC = database/database.c
JSON_SRC = util/repo_json.c
SENHA_SRC = util/senha_util.c
CRED_SRC = util/credencial_util.c
REGIAO_SRC = util/regiao_df.c
USUARIO_SRC = repositories/usuario_repository.c
SESSAO_SRC = repositories/sessao_repository.c
AUDIT_SRC = repositories/auditoria_repository.c
ENF_SRC = repositories/enfermagem_repository.c
CHECKIN_SRC = repositories/checkin_repository.c
FIN_SRC = repositories/financeiro_repository.c

PAC_SRC = repositories/paciente_repository.c
MED_SRC = repositories/medico_repository.c
ALA_SRC = repositories/ala_repository.c
LEITO_SRC = repositories/leito_repository.c
TRIAGEM_SRC = repositories/triagem_repository.c
AGEND_SRC = repositories/agendamento_repository.c
PRONT_SRC = repositories/prontuario_repository.c
EXAME_SRC = repositories/exame_repository.c
INTERN_SRC = repositories/internacao_repository.c
PRESC_SRC = repositories/prescricao_repository.c

TRIAGEM_SVC_SRC = services/triagem_service.c
RELAT_SVC_SRC = services/relatorio_service.c

API_SRC = api/server.c
SEED_SRC = tools/seed.c

# Schema e banco oficiais (sobrescreviveis na linha de comando).
SCHEMA ?= ../data/schema_v3.sql

# Repositories que usam o utilitario de JSON precisam linkar JSON_SRC.
TEST_PAC = tests/test_paciente_repository.c $(PAC_SRC) $(DB_SRC)
TEST_MED = tests/test_medico_repository.c $(MED_SRC) $(DB_SRC)
TEST_ALA = tests/test_ala_repository.c $(ALA_SRC) $(JSON_SRC) $(DB_SRC)
TEST_LEITO = tests/test_leito_repository.c $(LEITO_SRC) $(ALA_SRC) $(JSON_SRC) $(DB_SRC)
TEST_TRIAGEM = tests/test_triagem_repository.c $(TRIAGEM_SRC) $(PAC_SRC) $(JSON_SRC) $(DB_SRC)
TEST_AGEND = tests/test_agendamento_repository.c $(AGEND_SRC) $(PAC_SRC) $(MED_SRC) $(JSON_SRC) $(DB_SRC)
TEST_PRONT = tests/test_prontuario_repository.c $(PRONT_SRC) $(PAC_SRC) $(MED_SRC) $(JSON_SRC) $(DB_SRC)
TEST_EXAME = tests/test_exame_repository.c $(EXAME_SRC) $(PAC_SRC) $(MED_SRC) $(PRONT_SRC) $(JSON_SRC) $(DB_SRC)
TEST_INTERN = tests/test_internacao_repository.c $(INTERN_SRC) $(PAC_SRC) $(ALA_SRC) $(LEITO_SRC) $(JSON_SRC) $(DB_SRC)
TEST_TRIAGEM_SVC = tests/test_triagem_service.c $(TRIAGEM_SVC_SRC) $(TRIAGEM_SRC) $(PAC_SRC) $(MED_SRC) $(PRONT_SRC) $(EXAME_SRC) $(AGEND_SRC) $(REGIAO_SRC) $(JSON_SRC) $(DB_SRC)
TEST_RELAT_SVC = tests/test_relatorio_service.c $(RELAT_SVC_SRC) $(PAC_SRC) $(MED_SRC) $(TRIAGEM_SRC) $(AGEND_SRC) $(PRONT_SRC) $(EXAME_SRC) $(JSON_SRC) $(DB_SRC)
TEST_USUARIO = tests/test_usuario_repository.c $(USUARIO_SRC) $(SENHA_SRC) $(JSON_SRC) $(DB_SRC)
TEST_SESSAO = tests/test_sessao_repository.c $(SESSAO_SRC) $(USUARIO_SRC) $(SENHA_SRC) $(JSON_SRC) $(DB_SRC)
TEST_AUDIT = tests/test_auditoria_repository.c $(AUDIT_SRC) $(JSON_SRC) $(DB_SRC)
TEST_CRED = tests/test_credencial_util.c $(CRED_SRC)
TEST_ENF = tests/test_enfermagem_repository.c $(ENF_SRC) $(PRESC_SRC) $(PAC_SRC) $(MED_SRC) $(JSON_SRC) $(DB_SRC)
TEST_CHECKIN = tests/test_checkin_repository.c $(CHECKIN_SRC) $(PAC_SRC) $(JSON_SRC) $(DB_SRC)
TEST_FIN = tests/test_financeiro_repository.c $(FIN_SRC) $(PAC_SRC) $(JSON_SRC) $(DB_SRC)
TEST_PRESC = tests/test_prescricao_repository.c $(PRESC_SRC) $(PAC_SRC) $(MED_SRC) $(JSON_SRC) $(DB_SRC)

# Alvo padrao: compila a camada de banco e o servidor HTTP.
all: $(BUILD)/database.o api

# Cria o diretorio de saida sob demanda (usado como prerequisito de ordem).
$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/database.o: $(DB_SRC) | $(BUILD)
	$(CC) $(CFLAGS) -c $(DB_SRC) -o $(BUILD)/database.o

# Compila o servidor HTTP (health, cadastros, triagem inteligente e usuarios).
API_DEPS = $(API_SRC) $(PAC_SRC) $(MED_SRC) $(ALA_SRC) $(LEITO_SRC) \
	$(TRIAGEM_SRC) $(AGEND_SRC) $(PRONT_SRC) $(EXAME_SRC) $(INTERN_SRC) \
	$(USUARIO_SRC) $(SESSAO_SRC) $(AUDIT_SRC) $(ENF_SRC) $(CHECKIN_SRC) $(FIN_SRC) $(SENHA_SRC) \
	$(CRED_SRC) $(REGIAO_SRC) $(PRESC_SRC) \
	$(TRIAGEM_SVC_SRC) $(RELAT_SVC_SRC) $(JSON_SRC) $(DB_SRC)

api: $(API_DEPS) | $(BUILD)
	$(CC) $(CFLAGS) $(API_DEPS) -o $(BUILD)/sigeh_api $(LDLIBS) $(CRYPTO_LIBS) $(MATH_LIB) $(THREAD_LIB)

# Ferramenta de seed: recria o banco oficial e popula dados minimos.
SEED_DEPS = $(SEED_SRC) $(USUARIO_SRC) $(SENHA_SRC) $(PAC_SRC) $(MED_SRC) \
	$(ALA_SRC) $(LEITO_SRC) $(JSON_SRC) $(DB_SRC)

seed: $(SEED_DEPS) | $(BUILD)
	$(CC) $(CFLAGS) $(SEED_DEPS) -o $(BUILD)/seed $(LDLIBS) $(CRYPTO_LIBS)
	cd . && ./$(BUILD)/seed $(SCHEMA)

# Alias para recriar o banco do zero (equivale a seed).
db-reset: seed

# Builda o frontend (Vite) e publica em public/ para o servidor servir.
frontend:
	npm --prefix ../../frontend install
	npm --prefix ../../frontend run build
	rm -rf public
	cp -r ../../frontend/dist public

# Sobe o servidor (precisa rodar de dentro de backend/web/ para achar ../data/).
run: api
	./$(BUILD)/sigeh_api

# Alvo separado para o smoke test HTTP, sem misturar com o make test unitario.
api-smoke-test:
	# Garante permissao de execucao no script antes de rodar o teste.
	chmod +x tests/api_smoke_test.sh
	# Executa o smoke test que sobe a API, valida rotas e encerra ao final.
	./tests/api_smoke_test.sh

# Teste de integracao HTTP: fluxos ponta-a-ponta encadeados (alem do smoke).
api-integration-test:
	chmod +x tests/api_integration_test.sh
	./tests/api_integration_test.sh

test: test_paciente_repository test_medico_repository test_ala_repository \
	test_leito_repository test_triagem_repository test_agendamento_repository \
	test_prontuario_repository test_exame_repository test_internacao_repository \
	test_triagem_service test_relatorio_service test_usuario_repository \
	test_prescricao_repository test_auditoria_repository test_credencial_util \
	test_enfermagem_repository test_checkin_repository test_financeiro_repository \
	test_sessao_repository

test_paciente_repository: | $(BUILD)
	$(CC) $(CFLAGS) $(TEST_PAC) -o $(BUILD)/test_paciente_repository $(LDLIBS)
	./$(BUILD)/test_paciente_repository

test_medico_repository: | $(BUILD)
	$(CC) $(CFLAGS) $(TEST_MED) -o $(BUILD)/test_medico_repository $(LDLIBS)
	./$(BUILD)/test_medico_repository

test_ala_repository: | $(BUILD)
	$(CC) $(CFLAGS) $(TEST_ALA) -o $(BUILD)/test_ala_repository $(LDLIBS)
	./$(BUILD)/test_ala_repository

test_leito_repository: | $(BUILD)
	$(CC) $(CFLAGS) $(TEST_LEITO) -o $(BUILD)/test_leito_repository $(LDLIBS)
	./$(BUILD)/test_leito_repository

test_triagem_repository: | $(BUILD)
	$(CC) $(CFLAGS) $(TEST_TRIAGEM) -o $(BUILD)/test_triagem_repository $(LDLIBS)
	./$(BUILD)/test_triagem_repository

test_agendamento_repository: | $(BUILD)
	$(CC) $(CFLAGS) $(TEST_AGEND) -o $(BUILD)/test_agendamento_repository $(LDLIBS)
	./$(BUILD)/test_agendamento_repository

test_prontuario_repository: | $(BUILD)
	$(CC) $(CFLAGS) $(TEST_PRONT) -o $(BUILD)/test_prontuario_repository $(LDLIBS)
	./$(BUILD)/test_prontuario_repository

test_exame_repository: | $(BUILD)
	$(CC) $(CFLAGS) $(TEST_EXAME) -o $(BUILD)/test_exame_repository $(LDLIBS)
	./$(BUILD)/test_exame_repository

test_internacao_repository: | $(BUILD)
	$(CC) $(CFLAGS) $(TEST_INTERN) -o $(BUILD)/test_internacao_repository $(LDLIBS)
	./$(BUILD)/test_internacao_repository

test_triagem_service: | $(BUILD)
	$(CC) $(CFLAGS) $(TEST_TRIAGEM_SVC) -o $(BUILD)/test_triagem_service $(LDLIBS) $(MATH_LIB)
	./$(BUILD)/test_triagem_service

test_relatorio_service: | $(BUILD)
	$(CC) $(CFLAGS) $(TEST_RELAT_SVC) -o $(BUILD)/test_relatorio_service $(LDLIBS)
	./$(BUILD)/test_relatorio_service

test_usuario_repository: | $(BUILD)
	$(CC) $(CFLAGS) $(TEST_USUARIO) -o $(BUILD)/test_usuario_repository $(LDLIBS) $(CRYPTO_LIBS)
	./$(BUILD)/test_usuario_repository

test_prescricao_repository: | $(BUILD)
	$(CC) $(CFLAGS) $(TEST_PRESC) -o $(BUILD)/test_prescricao_repository $(LDLIBS)
	./$(BUILD)/test_prescricao_repository

test_auditoria_repository: | $(BUILD)
	$(CC) $(CFLAGS) $(TEST_AUDIT) -o $(BUILD)/test_auditoria_repository $(LDLIBS)
	./$(BUILD)/test_auditoria_repository

test_credencial_util: | $(BUILD)
	$(CC) $(CFLAGS) $(TEST_CRED) -o $(BUILD)/test_credencial_util
	./$(BUILD)/test_credencial_util

test_enfermagem_repository: | $(BUILD)
	$(CC) $(CFLAGS) $(TEST_ENF) -o $(BUILD)/test_enfermagem_repository $(LDLIBS)
	./$(BUILD)/test_enfermagem_repository

test_checkin_repository: | $(BUILD)
	$(CC) $(CFLAGS) $(TEST_CHECKIN) -o $(BUILD)/test_checkin_repository $(LDLIBS)
	./$(BUILD)/test_checkin_repository

test_financeiro_repository: | $(BUILD)
	$(CC) $(CFLAGS) $(TEST_FIN) -o $(BUILD)/test_financeiro_repository $(LDLIBS)
	./$(BUILD)/test_financeiro_repository

test_sessao_repository: | $(BUILD)
	$(CC) $(CFLAGS) $(TEST_SESSAO) -o $(BUILD)/test_sessao_repository $(LDLIBS) $(CRYPTO_LIBS)
	./$(BUILD)/test_sessao_repository

clean:
	rm -rf $(BUILD)

.PHONY: all api frontend run api-smoke-test api-integration-test seed db-reset test clean \
	test_paciente_repository test_medico_repository test_ala_repository \
	test_leito_repository test_triagem_repository test_agendamento_repository \
	test_prontuario_repository test_exame_repository test_internacao_repository \
	test_triagem_service test_relatorio_service test_usuario_repository \
	test_prescricao_repository test_auditoria_repository test_credencial_util \
	test_enfermagem_repository test_checkin_repository test_financeiro_repository \
	test_sessao_repository
