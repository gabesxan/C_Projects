CC = gcc

CFLAGS = -Wall -Wextra -pedantic \
	-Idatabase \
	-Irepositories

LDLIBS = -lsqlite3

DB_SRC = database/database.c

PAC_SRC = repositories/paciente_repository.c
MED_SRC = repositories/medico_repository.c

TEST_PAC = \
	tests/test_paciente_repository.c \
	$(PAC_SRC) \
	$(DB_SRC)

TEST_MED = \
	tests/test_medico_repository.c \
	$(MED_SRC) \
	$(DB_SRC)

# Alvo padrao: compila apenas a camada de banco (prova que ela esta sa).
all: database.o

database.o: $(DB_SRC)
	$(CC) $(CFLAGS) -c $(DB_SRC) -o database.o

test: test_paciente_repository test_medico_repository

test_paciente_repository:
	$(CC) $(CFLAGS) $(TEST_PAC) -o tests/test_paciente_repository $(LDLIBS)
	./tests/test_paciente_repository

test_medico_repository:
	$(CC) $(CFLAGS) $(TEST_MED) -o tests/test_medico_repository $(LDLIBS)
	./tests/test_medico_repository

clean:
	rm -f database.o \
		tests/test_paciente_repository \
		tests/test_medico_repository \
		tests/test_sigeh_repository.db

.PHONY: all test clean test_paciente_repository test_medico_repository
