#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <string.h>
#include <GL/gl.h>
#include <GL/glut.h>

#include "typedefs.h"
#include "debug.h"
#include "mem.h"
#include "cpu.h"

uint8_t screen[SCREEN_HEIGHT][SCREEN_WIDTH][3];

// Display
uint8_t *graphics_display;
int graphics_draw_flag;

void graphics_init(void *loop, void *keypress);
void graphics_load_sprite(int x, int y, int N);
void graphics_load_pixel(int row, int col, uint8_t color);
void graphics_draw_cell(int row, int col, uint8_t color);
void graphics_draw();
void graphics_update_screen();

#endif