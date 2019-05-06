#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "typedefs.h"
#include "stack.h"
#include "file.h"
#include "debug.h"
#include "graphics.h"

const int REFRESH_RATE_HZ = 150;
double refresh_rate;

// Registers
byte_t v[16];
word_t I;
word_t pc;
word_t sp;

word_t opcode;

// Memory
byte_t *memory;
int mem_size;

// Display
byte_t *display;
int draw_flag;

// Font Set
byte_t fontset[80] = {
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

// Keys
byte_t key_pressed;

// Timers
byte_t delay_timer;

// Misc.
int cycle_count;

void game_loop();
void on_keypress(byte_t key, int x, int y);

/*
 * Initialize CPU registers
 */
int initialize(int argc, char **argv) {
    memory = malloc(4096 * sizeof(byte_t));
    display = &memory[DISPLAY_SPACE];
    memset(display, 0, DISPLAY_SIZE);

    for (int i=0; i < 80; i++) {
        memory[i] = fontset[i];
    }

    init_graphics(argc, argv, game_loop, on_keypress);

    draw_flag = 0;

    I = 0;
    pc = PROGRAM_SPACE;
    sp = STACK_SPACE;
    opcode = 0;

    delay_timer = 0;

    cycle_count = 0;

    key_pressed = -1;

    refresh_rate = 1000.0 / REFRESH_RATE_HZ;

    return 1;
}

// CPU
/*
 * Complete a CPU cycle
 */
int complete_cycle() {
    if (--cycle_count > 0) {
        return 0;
    }

    // Get opcode from program memory
    opcode = (memory[pc] << 8) | memory[pc+1];
    word_t val_word;
    byte_t val_byte;
    byte_t reg;
    byte_t regX;
    byte_t regY;

    cycle_count = 1;

    switch (opcode & 0xF000) {
        case 0x0000: ;
            // Decode opcode
            // No Operation
            switch (opcode & 0xFF) {
                case 0x00: ;
                    #ifdef DEBUG
                    printf("%x      NOP\n", pc);
                    #endif
                    break;

                // Erase Screen
                case 0xE0: ;
                    #ifdef DEBUG
                    printf("%x      ERASE\n", pc);
                    #endif

                    memset(display, 0, DISPLAY_SIZE);
                    draw_flag = 1;
                    break;
                
                // Return from Subroutine
                case 0xEE: ;
                    #ifdef DEBUG
                    printf("%x      RETURN\n", pc);
                    #endif

                    // Pop pc from stack
                    byte_t low = stack_pop();
                    byte_t high = stack_pop();

                    pc = (high << 8) | low;
                    break;
            }

            break;

        // Jump
        case 0x1000: ;
            val_word = opcode & 0xFFF;

            #ifdef DEBUG
            printf("%x      GOTO %x\n", pc, val_word);
            #endif

            pc = val_word-2;

            break;

        // Call Subroutine
        case 0x2000: ;
            val_word = opcode & 0xFFF;

            #ifdef DEBUG
            printf("%x      DO %x\n", pc, val_word);
            #endif

            // Push pc to stack and set pc to val_word
            stack_push(pc);

            pc = val_word-2;
        
            break;

        // Conditional Statements
        case 0x3000: ;
            reg = (opcode & 0xF00) >> 8;
            val_byte = opcode & 0xFF;

            #ifdef DEBUG
            printf("%x      SKF v%x=%x\n", pc, reg, val_byte);
            printf("         v%x=%x\n", reg, v[reg]);
            #endif

            if (v[reg] == val_byte) {
                pc += 2;
            }

            break;
        
        case 0x4000: ;
            reg = (opcode & 0xF00) >> 8;
            val_byte = opcode & 0xFF;

            #ifdef DEBUG
            printf("%x      SKF v%x!=%x\n", pc, reg, val_byte);
            printf("         v%x=%x\n", reg, v[reg]);
            #endif

            if (v[reg] != val_byte) {
                pc += 2;
            }

            break;
        
        case 0x5000: ;
            regX = (opcode & 0xF00) >> 8;
            regY = (opcode & 0xF0) >> 4;

            #ifdef DEBUG
            printf("%x      SKF v%x=v%x\n", pc, regX, regY);
            #endif

            if (v[regX] == v[regY]) {
                pc += 2;
            }

            break;

        // Move
        case 0x6000: ;
            reg = (opcode & 0xF00) >> 8;
            val_byte = opcode & 0xFF;

            #ifdef DEBUG
            printf("%x      MOV v%x, %x\n", pc, reg, val_byte);
            #endif

            v[reg] = val_byte;
            
            break;

        // Addition
        case 0x7000: ;
            reg = (opcode & 0xF00) >> 8;
            val_byte = opcode & 0xFF;

            #ifdef DEBUG
            printf("%x      v%x=v%x+%x\n", pc, reg, reg, val_byte);
            #endif

            v[reg] += val_byte;

            break;

        // Logical Operations
        case 0x8000: ;
            regX = (opcode & 0xF00) >> 8;
            regY = (opcode & 0xF0) >> 4;
            
            switch (opcode & 0xF) {
                // Copy VY to VX
                case 0x0: ;
                    #ifdef DEBUG
                    printf("%x      v%x=v%x\n", pc, regX, regY);
                    #endif

                    v[regX] = v[regY];
                
                    break;

                // Logical OR VX and VY
                case 0x1: ;
                    #ifdef DEBUG
                    printf("%x      v%x=v%x|v%x\n", pc, regX, regX, regY);
                    #endif

                    v[regX] = v[regX] | v[regY];

                    break;

                // Logical AND VX and VY
                case 0x2: ;
                    #ifdef DEBUG
                    printf("%x      v%x=v%x&v%x\n", pc, regX, regX, regY);
                    #endif

                    v[regX] = v[regX] & v[regY];

                    break;

                case 0x3: ;
                    #ifdef DEBUG
                    printf("%x      v%x=v%xXORv%x\n", pc, regX, regX, regY);
                    #endif

                    v[regX] = v[regX] ^ v[regY];

                    break;

                case 0x4: ;
                    #ifdef DEBUG
                    printf("%x      v%x=v%x+v%x\n", pc, regX, regX, regY);
                    #endif

                    v[regX] = v[regX] + v[regY];

                    break;

                case 0x5: ;
                    #ifdef DEBUG
                    printf("%x      v%x=v%x-v%x\n", pc, regX, regX, regY);
                    #endif

                    v[regX] = v[regX] - v[regY];

                    break;

                case 0x6: ;
                    #ifdef DEBUG
                    printf("%x      v%x=v%x>>1\n", pc, regX, regX);
                    #endif

                    v[0xF] = v[regX] & 0x1;

                    v[regX] = v[regX] >> 1;

                    break;
            }

            break;

        case 0x9000: ;        
        // Skip Instruction
            regX = (opcode & 0xF00) >> 8;
            regY = (opcode & 0xF0) >> 4;

            #ifdef DEBUG
            printf("%x      SKF v%x!=v%x\n", pc, regX, regY);
            #endif

            if (v[regX] != v[regY]) {
                pc += 2;
            }

            break;

        // Set Memory Pointer
        case 0xA000: ;
            val_word = opcode & 0xFFF;

            #ifdef DEBUG
            printf("%x      MOV I, %x\n", pc, val_word);
            #endif

            I = val_word;

            break;

        // Random Byte
        case 0xC000: ;
            reg = (opcode & 0xF00) >> 8;
            val_byte = opcode & 0xFF;
            byte_t num = rand() % 256;

            #ifdef DEBUG
            printf("%x      v%x=RAND&%x\n", pc, reg, val_byte);
            printf("%x      RAND=%x\n", pc, num);
            #endif

            v[reg] = val_byte & num;

            break;

        // Draw Sprite
        case 0xD000: ;
            byte_t x = (opcode & 0xF00) >> 8;
            byte_t y = (opcode & 0xF0) >> 4;
            byte_t N = opcode & 0xF;

            #ifdef DEBUG
            printf("%x      SPRITE v%x, v%x, %x\n", pc, x, y, N);
            printf("         v%x=%x, v%x=%x\n", x, v[x], y, v[y]);

            printf("         ");
            for (int j=0; j<N; j++) {
                printf("%x", memory[I+j]);
            }
            printf("\n");
            #endif
            
            // cycle_count = 4;

            v[0xF] = 0;

            load_sprite(v[x], v[y], N);

            draw_flag = 1;

            break;

        case 0xE000: ;
            reg = (opcode & 0xF00) >> 8;
            
            switch (opcode & 0xFF) {
                // Skip if Key Down
                case 0x9E: ;
                    #ifdef DEBUG
                    printf("%x      SKF v%x=KEY\n", pc, reg);
                    printf("         v%x=%x, KEY=%x\n", reg, v[reg], key_pressed);
                    #endif

                    if (v[reg] == key_pressed) {
                        pc += 2;
                    }

                    break;

                // Skip if Key is not Down
                case 0xA1: ;
                    #ifdef DEBUG
                    printf("%x      SKF v%x!=KEY\n", pc, reg);
                    printf("         v%x=%x, KEY=%x\n", reg, v[reg], key_pressed);
                    #endif

                    if (v[reg] != key_pressed) {
                        pc += 2;
                    }

                    break;
            }

            break;

        case 0xF000: ;
            reg = (opcode & 0xF00) >> 8;
            int i;

            switch (opcode & 0xFF) {
                case 0x00: ;
                    // TODO
                    
                    #ifdef DEBUG
                    printf("%x      STOP\n", pc);
                    #endif

                    break;

                // Get Timer Value
                case 0x07: ;
                    #ifdef DEBUG
                    printf("%x      v%x=TIME\n", pc, reg);
                    #endif
                    
                    v[reg] = delay_timer;

                    break;

                // Get Key Value
                case 0x0A: ;
                    // TODO: Get key value
                    #ifdef DEBUG
                    printf("%x      v%x=KEY\n", pc, reg);
                    printf("         KEY=%x\n", key_pressed);
                    #endif

                    if (key_pressed != 0xFF) {
                        v[reg] = key_pressed;
                    } else {
                        return 0;
                    }

                    break;

                // Set Timer Value
                case 0x15: ;
                    #ifdef DEBUG
                    printf("%x      TIME=v%x\n", pc, reg);
                    #endif

                    delay_timer = v[reg];

                    break;

                case 0x1E: ;
                    #ifdef DEBUG
                    printf("%x      I=I+v%x\n", pc, reg);
                    #endif

                    I += v[reg];

                    break;

                case 0x29: ;
                    #ifdef DEBUG
                    printf("%x      I=DSP,v%x\n", pc, reg);
                    printf("         v%x=%x\n", reg, v[reg]);
                    #endif

                    I = 5 * v[reg];

                    break;

                case 0x33: ;
                    #ifdef DEBUG
                    printf("%x      I=DEQ,v%x\n", pc, reg);
                    printf("         v%x\n", reg);
                    #endif

                    memory[I] = v[reg] / 100;
                    memory[I + 1] = (v[reg] / 10) % 10;
                    memory[I + 2] = v[reg] % 10;

                    break;

                case 0x55: ;
                    #ifdef DEBUG
                    printf("%x      MI=v0: ;v%x\n", pc, reg);
                    #endif

                    for (i=0; i <= reg; i++) {
                        memory[I+i] = v[i];
                    }

                    cycle_count = 4*i;

                    break;

                case 0x65: ;
                    #ifdef DEBUG
                    printf("%x      v0: ;v%x=MI\n", pc, reg);
                    #endif

                    for (i=0; i <= reg; i++) {
                        v[i] = memory[I+i];
                    }

                    break;
            }

            break;

        default: ;
            #ifdef DEBUG
            printf("Unknown opcode: %x\n", opcode);
            #endif

            break;
    }
    
    // Decrement timers
    if (delay_timer > 0) {
        delay_timer--;
    }

    // Increment Program Counter
    pc += 2;    

    return 1;
}

void game_loop() {
    complete_cycle();

    if (draw_flag) {
        draw_flag = 0;

        draw();
    }

    usleep(1000 * refresh_rate);
}

void on_keypress(byte_t key, int x, int y) {
    switch (key) {
        case '1':
            key_pressed = 0x1;
            break;
        case '2':
            key_pressed = 0x2;
            break;
        case '3':
            key_pressed = 0x3;
            break;
        case '4':
            key_pressed = 0xC;
            break;
        case 'q':
            key_pressed = 0x4;
            break;
        case 'w':
            key_pressed = 0x5;
            break;
        case 'e':
            key_pressed = 0x6;
            break;
        case 'r':
            key_pressed = 0xD;
            break;
        case 'a':
            key_pressed = 0x7;
            break;
        case 's':
            key_pressed = 0x8;
            break;
        case 'd':
            key_pressed = 0x9;
            break;
        case 'f':
            key_pressed = 0xE;
            break;
        case 'z':
            key_pressed = 0xA;
            break;
        case 'x':
            key_pressed = 0x0;
            break;
        case 'c':
            key_pressed = 0xB;
            break;
        case 'v':
            key_pressed = 0xF;
            break;
        default:
            key_pressed = 0xFF;
            break;
    }

    #ifdef DEBUG
    printf("key_pressed=%x\n", key_pressed);
    #endif
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Enter a game name.\n");
        return -1;
    }
    
    char *game_name = argv[1];
    time_t t;

    initialize(argc, argv);
    read_file(game_name);

    srand((unsigned) time(&t));

    update_screen();

    free(memory);

    return 0;
}
