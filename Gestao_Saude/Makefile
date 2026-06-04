CC = gcc
CFLAGS = -Wall -Wextra -pedantic -Iinclude
TARGET = sigeh
SOURCES = src/main.c src/paciente.c src/medico.c src/agendamento.c src/ala.c src/leito.c src/internacao.c src/triagem.c src/relatorio.c

.PHONY: all run clean test

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET) main *.o src/*.o

test:
	@echo "Testes serao implementados depois."
