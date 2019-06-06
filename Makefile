CC=gcc

C_SOURCES=$(wildcard src/*.c)
C_FLAGS= -g -Wall -Iinclude -lGL -lglut `pkg-config --cflags --libs check`

all: main.o

main.o:
	${CC} -o main.o main.c $(C_SOURCES) $(C_FLAGS)

test:
	${CC} -o tests/test.o tests/chip8_test.c $(C_SOURCES) $(C_FLAGS)
	./tests/test.o
	rm -f tests/test.o

.PHONY: clean
clean:
	rm *.o
