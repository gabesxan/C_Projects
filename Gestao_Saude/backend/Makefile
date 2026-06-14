CC = gcc

CFLAGS = -Wall -Wextra -pedantic \
	-Idatabase \
	-Irepositories

LDLIBS = -lsqlite3

DB_SRC = database/database.c

PAC_SRC = repositories/paciente_repository.c
MED_SRC = repositories/medico_repository.c

API_SRC = api/health.c

TEST_PAC = \
	tests/test_paciente_repository.c \
	$(PAC_SRC) \
	$(DB_SRC)

TEST_MED = \
	tests/test_medico_repository.c \
	$(MED_SRC) \
	$(DB_SRC)

# Alvo padrao: compila a camada de banco e a API de health.
all: database.o api

database.o: $(DB_SRC)
	$(CC) $(CFLAGS) -c $(DB_SRC) -o database.o

# Compila o servidor HTTP minimo (endpoint GET /health).
api: $(API_SRC) $(PAC_SRC) $(DB_SRC)
	$(CC) $(CFLAGS) $(API_SRC) $(PAC_SRC) $(DB_SRC) -o api/sigeh_api $(LDLIBS)

# Sobe o servidor (precisa rodar de dentro de backend/ para achar data/).
run: api
	./api/sigeh_api

test: test_paciente_repository test_medico_repository

test_paciente_repository:
	$(CC) $(CFLAGS) $(TEST_PAC) -o tests/test_paciente_repository $(LDLIBS)
	./tests/test_paciente_repository

test_medico_repository:
	$(CC) $(CFLAGS) $(TEST_MED) -o tests/test_medico_repository $(LDLIBS)
	./tests/test_medico_repository

clean:
	rm -f database.o \
		api/sigeh_api \
		tests/test_paciente_repository \
		tests/test_medico_repository \
		tests/test_sigeh_repository.db

.PHONY: all api run test clean test_paciente_repository test_medico_repository
