#ifndef DRAW2D_H
#define DRAW2D_H

#include "graphics.h"
#include "maths.h"
#include "texture.h"

void draw2d_draw_point(framebuffer_t *framebuffer, vec4_t color,
                       vec2_t point);
void draw2d_draw_line(framebuffer_t *framebuffer, vec4_t color,
                      vec2_t point0, vec2_t point1);
void draw2d_draw_triangle(framebuffer_t *framebuffer, vec4_t color,
                          vec2_t point0, vec2_t point1, vec2_t point2);
void draw2d_draw_texture(framebuffer_t *framebuffer, texture_t *texture,
                         vec2_t origin);

#endif
