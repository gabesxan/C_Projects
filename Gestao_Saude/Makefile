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
	src/main/c/modules/relatorio.c

OUT = sigeh

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

run: all
	./$(OUT)

clean:
	rm -f $(OUT)

test:
	@echo "Testes automatizados serão implementados depois."

tree:
	find . -maxdepth 5 | sort
