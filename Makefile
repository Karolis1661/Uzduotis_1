CC = gcc

CFLAGS = -g -Wall

TRGT = programa
SRC = programa.c

default = $(TRGT)

$(TRGT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $@
