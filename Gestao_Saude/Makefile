CC = gcc

CFLAGS = -Wall -Wextra -pedantic \
	-Isrc/main/c/model \
	-Isrc/main/c/headers \
	-Isrc/main/c/modules

SRC = \
	src/main/c/app/main.c \
	src/main/c/modules/paciente.c \
	src/main/c/modules/medico.c \
	src/main/c/modules/agendamento.c \
	src/main/c/modules/ala.c \
	src/main/c/modules/leito.c \
	src/main/c/modules/internacao.c \
	src/main/c/modules/triagem.c \
	src/main/c/modules/relatorio.c \
	src/main/c/modules/prontuario.c \
	src/main/c/modules/exame.c

OUT = sigeh

TEST_AGENDAMENTO = \
        src/test/c/modules/test_agendamento.c \
        src/main/c/modules/agendamento.c \
        src/main/c/modules/triagem.c \
        src/main/c/modules/prontuario.c

TEST_LEITO = \
		src/test/c/modules/test_leito.c \
		src/main/c/modules/leito.c

TEST_TRIAGEM = \
		src/test/c/modules/test_triagem.c \
		src/main/c/modules/triagem.c

TEST_PACIENTE = \
		src/test/c/modules/test_paciente.c \
		src/main/c/modules/paciente.c

TEST_MEDICO = \
		src/test/c/modules/test_medico.c \
		src/main/c/modules/medico.c
	
TEST_ALA = \
		src/test/c/modules/test_ala.c \
		src/main/c/modules/ala.c

TEST_INTERNACAO = \
		src/test/c/modules/test_internacao.c \
	src/main/c/modules/internacao.c

TEST_RELATORIO = \
		src/test/c/modules/test_relatorio.c \
		src/main/c/modules/relatorio.c

TEST_PRONTUARIO = \
        src/test/c/modules/test_prontuario.c \
        src/main/c/modules/prontuario.c \
        src/main/c/modules/agendamento.c \
        src/main/c/modules/triagem.c

TEST_EXAME = \
        src/test/c/modules/test_exame.c \
        src/main/c/modules/exame.c \
        src/main/c/modules/triagem.c

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

run: all
	./$(OUT)

clean:
	rm -f $(OUT) test_agendamento test_leito test_triagem test_paciente test_medico test_ala test_internacao test_relatorio test_prontuario test_exame

test:
	$(CC) $(CFLAGS) $(TEST_AGENDAMENTO) -o test_agendamento
	./test_agendamento
	$(CC) $(CFLAGS) $(TEST_LEITO) -o test_leito
	./test_leito
	$(CC) $(CFLAGS) $(TEST_TRIAGEM) -o test_triagem
	./test_triagem
	$(CC) $(CFLAGS) $(TEST_PACIENTE) -o test_paciente
	./test_paciente
	$(CC) $(CFLAGS) $(TEST_MEDICO) -o test_medico
	./test_medico
	$(CC) $(CFLAGS) $(TEST_ALA) -o test_ala
	./test_ala
	$(CC) $(CFLAGS) $(TEST_INTERNACAO) -o test_internacao
	./test_internacao
	$(CC) $(CFLAGS) $(TEST_RELATORIO) -o test_relatorio
	./test_relatorio
	$(CC) $(CFLAGS) $(TEST_PRONTUARIO) -o test_prontuario
	./test_prontuario
	$(CC) $(CFLAGS) $(TEST_EXAME) -o test_exame
	./test_exame
tree:
	find . -maxdepth 5 | sort
