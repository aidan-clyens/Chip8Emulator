#include <string.h>
#include <GL/gl.h>
#include <GL/glut.h>

#include "typedefs.h"
#include "debug.h"

byte_t screen[SCREEN_HEIGHT][SCREEN_WIDTH][3];

void init_graphics(int argc, char **argv, void *loop, void *keypress);
void load_sprite(int x, int y, int N);
void draw_pixel(int row, int col, byte_t color);
void draw_cell(int row, int col, byte_t color);
void draw();
void update_screen();