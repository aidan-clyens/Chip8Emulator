#include <check.h>
#include <string.h>

#include "chip8.h"
#include "typedefs.h"

extern uint8_t fontset[];

char *test_game = "PONG";

/*
 * CHIP-8 Initialization Tests
 */
// Check loading of fontset and display into memory
START_TEST (check_init_memory) {
    chip8_initialize(test_game);

    for (int i=0; i < 80; i++) {
        ck_assert_int_eq(memory[i], fontset[i]);
    }

    for (int i=0; i < DISPLAY_SIZE; i++) {
        ck_assert_int_eq(graphics_display[i], 0);
    }

    chip8_close();
}
END_TEST

// Check setting of CPU registers
START_TEST(check_init_registers) {
    chip8_initialize(test_game);

    ck_assert_int_eq(I, 0);
    ck_assert_int_eq(pc, PROGRAM_SPACE);
    ck_assert_int_eq(sp, STACK_SPACE);

    chip8_close();
}
END_TEST

// Check loading of program file into memory
START_TEST(check_init_file) {
    chip8_initialize(test_game);

    char file_path[strlen("c8games/") + strlen(test_game) + 1];
    uint8_t buffer[2];
    FILE *file;

    strcpy(file_path, "c8games/");
    strcat(file_path, test_game);

    file = fopen(file_path, "rb");

    uint16_t i = pc;
    ck_assert_int_eq(pc, PROGRAM_SPACE);
    while (fread(buffer, sizeof(buffer), 1, file) != 0) {
        ck_assert_int_eq(memory[i], buffer[0]);
        ck_assert_int_eq(memory[i+1], buffer[1]);

        i += 2;
    }

    fclose(file);

    chip8_close();
}
END_TEST

/*
 * CHIP-8 Opcode Tests
 */
// 0x6XNN
START_TEST (opcode_set_vx) {
    chip8_initialize(test_game);

    // Valid register and number
    uint16_t opcode = 0x6000;
    uint8_t regX = rand() % 0xE;
    uint8_t num = rand() % 0xFF;

    opcode |= (regX << 8);
    opcode |= num;

    cpu_run_instruction(opcode);
    ck_assert_int_eq(v[regX], num);

    chip8_close();
}
END_TEST

// 0x7XNN
START_TEST(opcode_add_vx) {
    chip8_initialize(test_game);

    // Without overflow
    uint16_t opcode = 0x7000;
    uint8_t regX = rand() % 0xE;
    uint8_t num = rand() % 0xFD;

    v[regX] = 0x02;

    opcode |= (regX << 8);
    opcode |= num;

    cpu_run_instruction(opcode);
    ck_assert_int_eq(v[regX], 0x02 + num);

    // With overflow
    opcode = 0x7000;
    num = 0x01;

    v[regX] = 0xFF;

    opcode |= (regX << 8);
    opcode |= num;

    cpu_run_instruction(opcode);
    ck_assert_int_eq(v[regX], 0xFF + num);

    chip8_close();
}
END_TEST

// 0x8XY0
START_TEST(opcode_assign_vx) {
    chip8_initialize(test_game);

    uint16_t opcode = 0x8000;
    uint8_t regX = rand() % 0x6;
    uint8_t regY = (rand() % 0x6) + 0x9;

    opcode |= (regX << 8);
    opcode |= (regY << 4);

    v[regY] = 0xA1;

    cpu_run_instruction(opcode);

    ck_assert_int_eq(v[regX], v[regY]);
    ck_assert_int_eq(v[regX], 0xA1);
    ck_assert_int_eq(v[regY], 0xA1);

    chip8_close();
}
END_TEST

// 0x8XY1
START_TEST(opcode_logical_or) {
    chip8_initialize(test_game);

    uint16_t opcode = 0x8001;
    uint8_t regX = rand() % 0x6;
    uint8_t regY = (rand() % 0x6) + 0x9;

    opcode |= (regX << 8);
    opcode |= (regY << 4);

    v[regX] = 0x23;
    v[regY] = 0xA1;

    uint8_t result = 0x23 | 0xA1;

    cpu_run_instruction(opcode);

    ck_assert_int_eq(v[regX], result);

    chip8_close();
}
END_TEST

// 0x8XY2
START_TEST(opcode_logical_and) {
    chip8_initialize(test_game);

    uint16_t opcode = 0x8002;
    uint8_t regX = rand() % 0x6;
    uint8_t regY = (rand() % 0x6) + 0x9;

    opcode |= (regX << 8);
    opcode |= (regY << 4);

    v[regX] = 0x23;
    v[regY] = 0xA1;

    uint8_t result = 0x23 & 0xA1;

    cpu_run_instruction(opcode);

    ck_assert_int_eq(v[regX], result);

    chip8_close();
}
END_TEST

// 0x8XY3
START_TEST(opcode_logical_xor) {
    chip8_initialize(test_game);

    uint16_t opcode = 0x8003;
    uint8_t regX = rand() % 0x6;
    uint8_t regY = (rand() % 0x6) + 0x9;

    opcode |= (regX << 8);
    opcode |= (regY << 4);

    v[regX] = 0x23;
    v[regY] = 0xA1;

    uint8_t result = 0x23 ^ 0xA1;

    cpu_run_instruction(opcode);

    ck_assert_int_eq(v[regX], result);

    chip8_close();
}
END_TEST

// 0x8XY4
START_TEST(opcode_reg_add) {
    chip8_initialize(test_game);

    uint16_t opcode = 0x8004;
    uint8_t regX = rand() % 0x6;
    uint8_t regY = (rand() % 0x6) + 0x9;

    uint8_t x, y;
    uint8_t result, carry;

    opcode |= (regX << 8);
    opcode |= (regY << 4);

    // Without carry
    x = 0x0F;
    y = 0xDA;
    v[regX] = x;
    v[regY] = y;

    result = (x + y) & 0xFF;
    carry = ((x + y) >> 8) & 0x1;

    cpu_run_instruction(opcode);

    ck_assert_int_eq(v[regX], result);
    ck_assert_int_eq(v[0xF], carry);

    // With carry
    x = 0xA0;
    y = 0xDA;
    v[regX] = x;
    v[regY] = y;

    result = (x + y) & 0xFF;
    carry = ((x + y) >> 8) & 0x1;

    cpu_run_instruction(opcode);

    ck_assert_int_eq(v[regX], result);
    ck_assert_int_eq(v[0xF], carry);

    chip8_close();
}
END_TEST

// 0x8XY5
START_TEST(opcode_reg_subtract) {
    chip8_initialize(test_game);

    uint16_t opcode = 0x8005;
    uint8_t regX = rand() % 0x6;
    uint8_t regY = (rand() % 0x6) + 0x9;

    uint8_t x, y;
    uint8_t result, borrow;

    opcode |= (regX << 8);
    opcode |= (regY << 4);

    // Positive difference
    x = 0xF0;
    y = 0x0E;
    v[regX] = x;
    v[regY] = y;

    result = (x - y) & 0xFF;
    borrow = ((x - y) >> 8) & 0x1;

    cpu_run_instruction(opcode);
    ck_assert_int_eq(v[regX], result);
    ck_assert_int_eq(v[0xF], borrow);

    // Negative difference
    x = 0x0E;
    y = 0xF0;
    v[regX] = x;
    v[regY] = y;

    result = (x - y) & 0xFF;
    borrow = ((x - y) >> 8) & 0x1;

    cpu_run_instruction(opcode);
    ck_assert_int_eq(v[regX], result);
    ck_assert_int_eq(v[0xF], borrow);

    chip8_close();
}
END_TEST

// 0x8XY6
START_TEST(opcode_reg_bitshift) {
    chip8_initialize(test_game);

    uint16_t opcode = 0x8006;
    uint8_t regX = rand() % 0x6;

    uint8_t x;
    uint8_t result, lsb;

    opcode |= (regX << 8);

    // LSB = 1
    x = 0x03;
    v[regX] = x;

    result = x >> 1;
    lsb = x & 0x1;

    cpu_run_instruction(opcode);
    ck_assert_int_eq(v[regX], result);
    ck_assert_int_eq(v[0xF], lsb);

    // LSB = 0
    x = 0x02;
    v[regX] = x;

    result = x >> 1;
    lsb = x & 0x1;

    cpu_run_instruction(opcode);
    ck_assert_int_eq(v[regX], result);
    ck_assert_int_eq(v[0xF], lsb);

    chip8_close();
}
END_TEST

Suite *chip8_suite() {
    Suite *s;
    TCase *tc_init;
    TCase *tc_opcodes;

    // Test Suite
    s = suite_create("Chip8");

    // Core Test Case
    tc_init = tcase_create("Init");
    tcase_add_test(tc_init, check_init_memory);
    tcase_add_test(tc_init, check_init_registers);
    tcase_add_test(tc_init, check_init_file);

    tc_opcodes = tcase_create("Opcodes");
    tcase_add_test(tc_opcodes, opcode_set_vx);
    tcase_add_test(tc_opcodes, opcode_add_vx);
    tcase_add_test(tc_opcodes, opcode_assign_vx);
    tcase_add_test(tc_opcodes, opcode_logical_or);
    tcase_add_test(tc_opcodes, opcode_logical_and);
    tcase_add_test(tc_opcodes, opcode_logical_xor);
    tcase_add_test(tc_opcodes, opcode_reg_add);
    tcase_add_test(tc_opcodes, opcode_reg_subtract);
    tcase_add_test(tc_opcodes, opcode_reg_bitshift);

    suite_add_tcase(s, tc_init);
    suite_add_tcase(s, tc_opcodes);

    return s;
}

int main() {
    Suite *s;
    SRunner *sr;

    time_t t;
    srand((unsigned)time(&t));

    s = chip8_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    int number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE; 
}
