CC=gcc
CFLAGS=-g -Wall -Wextra -fsanitize=signed-integer-overflow
CLIBS=-lX11

build:
	$(CC) $(CFLAGS) $(CLIBS) txbar.c -o txbar
