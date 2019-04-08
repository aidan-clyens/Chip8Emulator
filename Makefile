all: emulator.o

emulator.o:
	gcc -o emulator.o src/emulator.c

clean:
	rm *.o
