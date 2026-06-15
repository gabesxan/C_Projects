CC = gcc

CFLAGS = -Wall -Wextra -pedantic \
	-Idatabase \
	-Irepositories \
	-Iservices

LDLIBS = -lsqlite3

DB_SRC = database/database.c
JSON_SRC = repositories/repo_json.c

PAC_SRC = repositories/paciente_repository.c
MED_SRC = repositories/medico_repository.c
ALA_SRC = repositories/ala_repository.c
LEITO_SRC = repositories/leito_repository.c
TRIAGEM_SRC = repositories/triagem_repository.c
AGEND_SRC = repositories/agendamento_repository.c
PRONT_SRC = repositories/prontuario_repository.c
EXAME_SRC = repositories/exame_repository.c
INTERN_SRC = repositories/internacao_repository.c

TRIAGEM_SVC_SRC = services/triagem_service.c
RELAT_SVC_SRC = services/relatorio_service.c

API_SRC = api/health.c

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
TEST_TRIAGEM_SVC = tests/test_triagem_service.c $(TRIAGEM_SVC_SRC) $(TRIAGEM_SRC) $(PAC_SRC) $(MED_SRC) $(PRONT_SRC) $(EXAME_SRC) $(AGEND_SRC) $(JSON_SRC) $(DB_SRC)
TEST_RELAT_SVC = tests/test_relatorio_service.c $(RELAT_SVC_SRC) $(PAC_SRC) $(MED_SRC) $(TRIAGEM_SRC) $(AGEND_SRC) $(JSON_SRC) $(DB_SRC)

# Alvo padrao: compila a camada de banco e a API de health.
all: database.o api

database.o: $(DB_SRC)
	$(CC) $(CFLAGS) -c $(DB_SRC) -o database.o

# Compila o servidor HTTP (health, pacientes, medicos e triagem inteligente).
API_DEPS = $(API_SRC) $(PAC_SRC) $(MED_SRC) $(ALA_SRC) $(LEITO_SRC) \
	$(TRIAGEM_SRC) $(AGEND_SRC) $(PRONT_SRC) $(EXAME_SRC) $(INTERN_SRC) \
	$(TRIAGEM_SVC_SRC) $(RELAT_SVC_SRC) $(JSON_SRC) $(DB_SRC)

api: $(API_DEPS)
	$(CC) $(CFLAGS) $(API_DEPS) -o api/sigeh_api $(LDLIBS)

# Sobe o servidor (precisa rodar de dentro de backend/ para achar data/).
run: api
	./api/sigeh_api

test: test_paciente_repository test_medico_repository test_ala_repository \
	test_leito_repository test_triagem_repository test_agendamento_repository \
	test_prontuario_repository test_exame_repository test_internacao_repository \
	test_triagem_service test_relatorio_service

test_paciente_repository:
	$(CC) $(CFLAGS) $(TEST_PAC) -o tests/test_paciente_repository $(LDLIBS)
	./tests/test_paciente_repository

test_medico_repository:
	$(CC) $(CFLAGS) $(TEST_MED) -o tests/test_medico_repository $(LDLIBS)
	./tests/test_medico_repository

test_ala_repository:
	$(CC) $(CFLAGS) $(TEST_ALA) -o tests/test_ala_repository $(LDLIBS)
	./tests/test_ala_repository

test_leito_repository:
	$(CC) $(CFLAGS) $(TEST_LEITO) -o tests/test_leito_repository $(LDLIBS)
	./tests/test_leito_repository

test_triagem_repository:
	$(CC) $(CFLAGS) $(TEST_TRIAGEM) -o tests/test_triagem_repository $(LDLIBS)
	./tests/test_triagem_repository

test_agendamento_repository:
	$(CC) $(CFLAGS) $(TEST_AGEND) -o tests/test_agendamento_repository $(LDLIBS)
	./tests/test_agendamento_repository

test_prontuario_repository:
	$(CC) $(CFLAGS) $(TEST_PRONT) -o tests/test_prontuario_repository $(LDLIBS)
	./tests/test_prontuario_repository

test_exame_repository:
	$(CC) $(CFLAGS) $(TEST_EXAME) -o tests/test_exame_repository $(LDLIBS)
	./tests/test_exame_repository

test_internacao_repository:
	$(CC) $(CFLAGS) $(TEST_INTERN) -o tests/test_internacao_repository $(LDLIBS)
	./tests/test_internacao_repository

test_triagem_service:
	$(CC) $(CFLAGS) $(TEST_TRIAGEM_SVC) -o tests/test_triagem_service $(LDLIBS)
	./tests/test_triagem_service

test_relatorio_service:
	$(CC) $(CFLAGS) $(TEST_RELAT_SVC) -o tests/test_relatorio_service $(LDLIBS)
	./tests/test_relatorio_service

clean:
	rm -f database.o \
		api/sigeh_api \
		tests/test_paciente_repository \
		tests/test_medico_repository \
		tests/test_ala_repository \
		tests/test_leito_repository \
		tests/test_triagem_repository \
		tests/test_agendamento_repository \
		tests/test_prontuario_repository \
		tests/test_exame_repository \
		tests/test_internacao_repository \
		tests/test_triagem_service \
		tests/test_relatorio_service \
		tests/test_sigeh_repository.db

.PHONY: all api run test clean \
	test_paciente_repository test_medico_repository test_ala_repository \
	test_leito_repository test_triagem_repository test_agendamento_repository \
	test_prontuario_repository test_exame_repository test_internacao_repository \
	test_triagem_service test_relatorio_service
