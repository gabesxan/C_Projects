CC = gcc
CFLAGS = -Wall -Wextra -pedantic -Iinclude
SRC = src/app/main.c src/modules/paciente.c src/modules/medico.c src/modules/agendamento.c src/modules/ala.c src/modules/leito.c src/modules/internacao.c src/modules/triagem.c src/modules/relatorio.c
OUT = sigeh

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

run: all
	./$(OUT)

clean:
	rm -f $(OUT)

test:
	@echo "Testes automatizados serao implementados depois."
