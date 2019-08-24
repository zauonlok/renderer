#ifndef DRAW2D_H
#define DRAW2D_H

#include "geometry.h"
#include "graphics.h"
#include "texture.h"

void draw2d_draw_point(framebuffer_t *framebuffer, vec4_t color,
                       int row, int col);
void draw2d_draw_line(framebuffer_t *framebuffer, vec4_t color,
                      int row0, int row1, int col0, int col1);
void draw2d_draw_triangle(framebuffer_t *framebuffer, vec4_t color,
                          int row0, int row1, int row2,
                          int col0, int col1, int col2);
void draw2d_draw_texture(framebuffer_t *framebuffer, texture_t *texture,
                         int row, int col);

#endif
