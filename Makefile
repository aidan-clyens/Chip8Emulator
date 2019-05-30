CC=gcc

C_SOURCES=$(wildcard src/*.c)
C_FLAGS= -g -Wall -Iinclude -lGL -lglut

all: emulator.o

emulator.o:
	${CC} -o emulator.o $(C_SOURCES) $(C_FLAGS)

.PHONY: clean
clean:
	rm *.o
