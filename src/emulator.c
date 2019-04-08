#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Types
typedef unsigned char byte_t;
typedef unsigned short word_t;

// Registers
byte_t v[16];
word_t I;
word_t pc;
word_t sp;

word_t opcode;

// Memory
byte_t memory[4096];
int mem_size;

// Keys
byte_t key[16];

// Timers
byte_t delay_timer;

// Debug
/*
 * Print memory contents
 */
void print_mem() {
    for (int j = 0; j < mem_size; j++) {
        printf("%x ", memory[j]);
    }
    printf("\n");
}

/*
 * Print register values
 */
void print_reg() {
    for (int j=0; j < sizeof(v); j++) {
        printf("v%x      %x\n", j, v[j]);
    }
    printf("I       %x\n", I);
    printf("pc      %x\n", pc);
    printf("sp      %x\n", sp);
}

/*
 * Initialize CPU registers
 */
int initialize() {
    I = 0;
    pc = 0x0200;
    sp = 0x003F;
    opcode = 0;

    delay_timer = 0;

    return 1;
}

// Stack
/*
 * Push to stack
 */
int stack_push(word_t val) {
    // If val is a word
    if (val > 0xFF) {
        byte_t high = (val & 0xFF00) >> 8;
        byte_t low = val & 0xFF;
        
        sp -= 2;
        memory[sp] = high;
        sp -=2;
        memory[sp] = low;
    }
    // If val is a byte
    else {
        sp -= 2;
        memory[sp] = (byte_t) val;
    }
}

/*
 * Pop from stack
 */
byte_t stack_pop() {
    byte_t val = memory[sp];
    sp += 2;

    return val;
}

// File Loading
/*
 * Load the game file into memory
 */
int read_file() {
    byte_t buffer[2];
    FILE *file;

    file = fopen("c8games/PONG", "rb");

    word_t i = pc;
    while (fread(buffer, sizeof(buffer), 1, file) != 0) {    
        memory[i] = buffer[0];
        memory[i+1] = buffer[1];

        i += 2;
    }
    mem_size = i;

    print_mem();

    fclose(file);

    return 1;
}

// CPU
/*
 * Complete a CPU cycle
 */
int complete_cycle() {
    // Get opcode from program memory
    opcode = (memory[pc] << 8) | memory[pc+1];

    // Decode opcode
    // No Operation
    if (opcode == 0x0000) {
        printf("%x      NOP\n", pc);
    }
    // Erase Screen
    else if (opcode == 0x00E0) {
        printf("%x      ERASE\n", pc);
        // Clear screen
    }
    // Return from Subroutine
    else if (opcode == 0x00EE) {
        printf("%x      RETURN\n", pc);

        // Pop pc from stack
        byte_t low = stack_pop();
        byte_t high = stack_pop();

        pc = (high << 8) | low;
    }
    // Jump
    else if ((opcode & 0xF000) == 0x1000) {
        word_t val = opcode & 0xFFF;

        printf("%x      GOTO %x\n", pc, val);

        pc = val-2;
    }
    // Call Subroutine
    else if ((opcode & 0xF000) == 0x2000) {
        word_t val = opcode & 0xFFF;

        printf("%x      DO %x\n", pc, val);
        // Push pc to stack and set pc to val
        stack_push(pc);

        pc = val-2;
    }
    // Conditional Statements
    else if ((opcode & 0xF000) == 0x3000) {
        byte_t reg = (opcode & 0xF00) >> 8;
        byte_t val = opcode & 0xFF;

        printf("%x      SKF v%x=%x\n", pc, reg, val);

        if (v[reg] == val) {
            pc += 2;
        }
    }
    else if ((opcode & 0xF000) == 0x4000) {
        byte_t reg = (opcode & 0xF00) >> 8;
        byte_t val = opcode & 0xFF;

        printf("%x      SKF v%x!=%x\n", pc, reg, val);

        if (v[reg] != val) {
            pc += 2;
        }
    }
    // Move
    else if ((opcode & 0xF000) == 0x6000) {
        byte_t reg = (opcode & 0xF00) >> 8;
        byte_t val = opcode & 0xFF;

        printf("%x      MOV v%x, %x\n", pc, reg, val);
        v[reg] = val;
    }
    // Addition
    else if ((opcode & 0xF000) == 0x7000) {
        byte_t reg = (opcode & 0xF00) >> 8;
        byte_t val = opcode & 0xFF;

        printf("%x      v%x=v%x+%x\n", pc, reg, reg, val);

        v[reg] += val;
    }
    // Logical Operations
    else if ((opcode & 0xF000) == 0x8000) {
        byte_t regX = (opcode & 0xF00) >> 8;
        byte_t regY = (opcode & 0xF0) >> 4;
        
        // Copy VY to VX
        if ((opcode & 0xF00F) == 0x8000) {
            printf("%x      v%x=v%x\n", pc, regX, regY);

            v[regX] = v[regY];
        }
        // Logical OR VX and VY
        else if ((opcode & 0xF00F) == 0x8001) {
            printf("%x      v%x=v%x|v%x\n", pc, regX, regX, regY);

            v[regX] = v[regX] | v[regY];
        }
        // Logical AND VX and VY
        else if ((opcode & 0xF00F) == 0x8002) {
            printf("%x      v%x=v%x&v%x\n", pc, regX, regX, regY);

            v[regX] = v[regX] & v[regY];
        }
    }
    // Set Memory Pointer
    else if ((opcode & 0xF000) == 0xA000) {
        word_t val = opcode & 0xFFF;

        printf("%x      MOV I, %x\n", pc, val);
        I = val;
    }
    // Random Byte
    else if ((opcode & 0xF000) == 0xC000) {
        byte_t reg = (opcode & 0xF00) >> 8;
        byte_t val = opcode & 0xFF;

        // TODO: Get random byte (value from 0 to 255)

        printf("%x      v%x=RAND&%x\n", pc, reg, val);
    }
    // Draw Sprite
    else if ((opcode & 0xF000) == 0xD000) {
        byte_t x = (opcode & 0xF00) >> 8;
        byte_t y = (opcode & 0xF0) >> 4;
        byte_t N = opcode & 0xF;

        printf("%x      SPRITE v%x, v%x, %x\n", pc, x, y, N);
        printf("v[%x] = %x, v[%x] = %x\n", x, v[x], y, v[y]);

        // TODO: Use OpenGL to draw graphics

        for (int j=0; j<N; j++) {
            printf("%x", memory[I+j]);
        }
        printf("\n");
    }
    else if ((opcode & 0xF000) == 0xE000) {
        byte_t reg = (opcode & 0xF00) >> 8;
        
        // Skip if Key Down
        if ((opcode & 0xF0FF) == 0xE09E) {
            // TODO: Skip if key is down

            printf("%x      SKF v%x=KEY\n", pc, reg);
        }
        // Skip if Key is not Down
        else if ((opcode & 0xF0FF) == 0xE0A1) {
            // TODO: Skip if key is not down
            
            printf("%x      SKF v%x!=KEY\n", pc, reg);
        }
    }
    else if ((opcode & 0xF000) == 0xF000) {
        if (opcode == 0xF000) {
            // TODO

            printf("%x      STOP\n", pc);
        }
        // Get Timer Value
        else if ((opcode & 0xF0FF) == 0xF007) {
            byte_t reg = (opcode & 0xF00) >> 8;

            printf("%x      v%x=TIME\n", pc, reg);
            v[reg] = delay_timer;
        }
        // Get Key Value
        else if ((opcode & 0xF0FF) == 0xF00A) {
            byte_t reg = (opcode & 0xF00) >> 8;

            // TODO: Get key value

            printf("%x      v%x=KEY\n", pc, reg);
        }
        // Set Timer Value
        else if ((opcode & 0xF0FF) == 0xF015) {
            byte_t reg = (opcode & 0xF00) >> 8;
            
            printf("%x      TIME=v%x\n", pc, reg);

            delay_timer = v[reg];
        }
    }
    else {
        printf("Unknown opcode: %x\n", opcode);
    }
    
    // Decrement timers
    if (delay_timer > 0) {
        delay_timer--;
    }

    // Increment Program Counter
    pc += 2;    

    return 1;
}

int main() {
    initialize();
    read_file();

    for (;;) {
        complete_cycle();
        usleep(1000*50);
    }

    return 0;
}
