CC = gcc
CFLAGS = -Wall -Wextra -std=c11

TARGET = assembler

SRCS = src/main.c \
       src/lexer.c \
       src/parser.c\
       src/emitter.c\
       src/symbol_table.c\
       src/rv32i.c

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)
	
clean:
	rm -f $(TARGET)

.PHONY: clean
