#include "cpu.h"

void init_cpu() {
    I = 0;
    pc = PROGRAM_SPACE;
    sp = STACK_SPACE;

    delay_timer = 0;
}

void complete_cycle() {
    // Get opcode from program memory
    word_t opcode = get_opcode();
    run_instruction(opcode);

    // Decrement timers
    if (delay_timer > 0) {
        delay_timer--;
    }

    // Increment Program Counter
    pc += 2;
}

void run_instruction(word_t opcode) {
    word_t val_word;
    byte_t val_byte;
    byte_t reg;
    byte_t regX;
    byte_t regY;

    switch (opcode & 0xF000) {
    case 0x0000:;
        // Decode opcode
        // No Operation
        switch (opcode & 0xFF) {
        case 0x00:;
            #ifdef DEBUG
            printf("%x      NOP\n", pc);
            #endif
            break;

        // Erase Screen
        case 0xE0:;
            #ifdef DEBUG
            printf("%x      ERASE\n", pc);
            #endif

            memset(display, 0, DISPLAY_SIZE);
            draw_flag = 1;
            break;

        // Return from Subroutine
        case 0xEE:;
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
    case 0x1000:;
        val_word = opcode & 0xFFF;

        #ifdef DEBUG
        printf("%x      GOTO %x\n", pc, val_word);
        #endif

        pc = val_word - 2;

        break;

    // Call Subroutine
    case 0x2000:;
        val_word = opcode & 0xFFF;

        #ifdef DEBUG
        printf("%x      DO %x\n", pc, val_word);
        #endif

        // Push pc to stack and set pc to val_word
        stack_push(pc);

        pc = val_word - 2;

        break;

    // Conditional Statements
    case 0x3000:;
        reg = (opcode & 0xF00) >> 8;
        val_byte = opcode & 0xFF;

        #ifdef DEBUG
        printf("%x      SKF v%x=%x\n", pc, reg, val_byte);
        printf("         v%x=%x\n", reg, v[reg]);
        #endif

        if (v[reg] == val_byte)
        {
            pc += 2;
        }

        break;

    case 0x4000:;
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

    case 0x5000:;
        regX = (opcode & 0xF00) >> 8;
        regY = (opcode & 0xF0) >> 4;

        #ifdef DEBUG
        printf("%x      SKF v%x=v%x\n", pc, regX, regY);
        #endif

        if (v[regX] == v[regY])
        {
            pc += 2;
        }

        break;

    // Move
    case 0x6000:;
        reg = (opcode & 0xF00) >> 8;
        val_byte = opcode & 0xFF;

        #ifdef DEBUG
        printf("%x      MOV v%x, %x\n", pc, reg, val_byte);
        #endif

        v[reg] = val_byte;

        break;

    // Addition
    case 0x7000:;
        reg = (opcode & 0xF00) >> 8;
        val_byte = opcode & 0xFF;

        #ifdef DEBUG
        printf("%x      v%x=v%x+%x\n", pc, reg, reg, val_byte);
        #endif

        v[reg] += val_byte;

        break;

    // Logical Operations
    case 0x8000:;
        regX = (opcode & 0xF00) >> 8;
        regY = (opcode & 0xF0) >> 4;

        switch (opcode & 0xF) {
        // Copy VY to VX
        case 0x0:;
            #ifdef DEBUG
            printf("%x      v%x=v%x\n", pc, regX, regY);
            #endif

            v[regX] = v[regY];

            break;

        // Logical OR VX and VY
        case 0x1:;
            #ifdef DEBUG
            printf("%x      v%x=v%x|v%x\n", pc, regX, regX, regY);
            #endif

            v[regX] = v[regX] | v[regY];

            break;

        // Logical AND VX and VY
        case 0x2:;
            #ifdef DEBUG
            printf("%x      v%x=v%x&v%x\n", pc, regX, regX, regY);
            #endif

            v[regX] = v[regX] & v[regY];

            break;

        case 0x3:;
            #ifdef DEBUG
            printf("%x      v%x=v%xXORv%x\n", pc, regX, regX, regY);
            #endif

            v[regX] = v[regX] ^ v[regY];

            break;

        case 0x4:;
            #ifdef DEBUG
            printf("%x      v%x=v%x+v%x\n", pc, regX, regX, regY);
            #endif

            v[regX] = v[regX] + v[regY];

            break;

        case 0x5:;
            #ifdef DEBUG
            printf("%x      v%x=v%x-v%x\n", pc, regX, regX, regY);
            #endif

            v[regX] = v[regX] - v[regY];

            break;

        case 0x6:;
            #ifdef DEBUG
            printf("%x      v%x=v%x>>1\n", pc, regX, regX);
            #endif

            v[0xF] = v[regX] & 0x1;

            v[regX] = v[regX] >> 1;

            break;
        }

        break;

    case 0x9000:;
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
    case 0xA000:;
        val_word = opcode & 0xFFF;

        #ifdef DEBUG
        printf("%x      MOV I, %x\n", pc, val_word);
        #endif

        I = val_word;

        break;

    // Random Byte
    case 0xC000:;
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
    case 0xD000:;
        byte_t x = (opcode & 0xF00) >> 8;
        byte_t y = (opcode & 0xF0) >> 4;
        byte_t N = opcode & 0xF;

        #ifdef DEBUG
        printf("%x      SPRITE v%x, v%x, %x\n", pc, x, y, N);
        printf("         v%x=%x, v%x=%x\n", x, v[x], y, v[y]);

        printf("         ");
        for (int j = 0; j < N; j++)
        {
            printf("%x", mem_read(I + j));
        }
        printf("\n");
        #endif

        v[0xF] = 0;

        load_sprite(v[x], v[y], N);

        draw_flag = 1;

        break;

    case 0xE000:;
        reg = (opcode & 0xF00) >> 8;

        switch (opcode & 0xFF) {
        // Skip if Key Down
        case 0x9E:;
            #ifdef DEBUG
            printf("%x      SKF v%x=KEY\n", pc, reg);
            printf("         v%x=%x, KEY=%x\n", reg, v[reg], key_pressed);
            #endif

            if (v[reg] == key_pressed) {
                pc += 2;
            }

            break;

        // Skip if Key is not Down
        case 0xA1:;
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

    case 0xF000:;
        reg = (opcode & 0xF00) >> 8;
        int i;

        switch (opcode & 0xFF) {
        case 0x00:;
            // TODO
            #ifdef DEBUG
            printf("%x      STOP\n", pc);
            #endif

            break;

        // Get Timer Value
        case 0x07:;
            #ifdef DEBUG
            printf("%x      v%x=TIME\n", pc, reg);
            #endif

            v[reg] = delay_timer;

            break;

        // Get Key Value
        case 0x0A:;
            // TODO: Get key value
            #ifdef DEBUG
            printf("%x      v%x=KEY\n", pc, reg);
            printf("         KEY=%x\n", key_pressed);
            #endif

            if (key_pressed != 0xFF) {
                v[reg] = key_pressed;
            }
            else {
                break;
            }

            break;

        // Set Timer Value
        case 0x15:;
            #ifdef DEBUG
            printf("%x      TIME=v%x\n", pc, reg);
            #endif

            delay_timer = v[reg];

            break;

        case 0x1E:;
            #ifdef DEBUG
            printf("%x      I=I+v%x\n", pc, reg);
            #endif

            I += v[reg];

            break;

        case 0x29:;
            #ifdef DEBUG
            printf("%x      I=DSP,v%x\n", pc, reg);
            printf("         v%x=%x\n", reg, v[reg]);
            #endif

            I = 5 * v[reg];

            break;

        case 0x33:;
            #ifdef DEBUG
            printf("%x      I=DEQ,v%x\n", pc, reg);
            printf("         v%x\n", reg);
            #endif

            mem_write(I, v[reg] / 100);
            mem_write(I + 1, (v[reg] / 10) % 10);
            mem_write(I + 2, v[reg] % 10);

            break;

        case 0x55:;
            #ifdef DEBUG
            printf("%x      MI=v0: ;v%x\n", pc, reg);
            #endif

            for (i = 0; i <= reg; i++) {
                mem_write(I + i, v[i]);
            }

            break;

        case 0x65:;
            #ifdef DEBUG
            printf("%x      v0: ;v%x=MI\n", pc, reg);
            #endif

            for (i = 0; i <= reg; i++) {
                v[i] = mem_read(I + i);
            }

            break;
        }

        break;

    default:;
        #ifdef DEBUG
        printf("Unknown opcode: %x\n", opcode);
        #endif

        break;
    }
}

word_t get_opcode() {
    return (mem_read(pc) << 8) | mem_read(pc + 1);
}