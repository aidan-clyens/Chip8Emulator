C_SOURCES=$(wildcard src/*.c)

C_FLAGS= -Iinclude

all: emulator.o

emulator.o:
	gcc -o emulator.o $(C_FLAGS) $(C_SOURCES)

clean:
	rm *.o
