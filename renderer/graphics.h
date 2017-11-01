#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "geometry.h"
#include "image.h"

void gfx_draw_point(image_t *image, vec2i_t point, color_t color);
void gfx_draw_line(image_t *image, vec2i_t point1, vec2i_t point2,
                   color_t color);
void gfx_draw_triangle(image_t *image, vec2i_t point1, vec2i_t point2,
                       vec2i_t point3, color_t color);
void gfx_fill_triangle(image_t *image, vec3i_t point0, vec3i_t point1,
                       vec3i_t point2, color_t color0, color_t color1, color_t color2, float *zbuffer, float intensity);

mat4f_t gfx_lookat_matrix(vec3f_t eye, vec3f_t center, vec3f_t up);
mat4f_t gfx_viewport_matrix(int x, int y, int width, int height);
mat4f_t gfx_projection_matrix(float coeff);

#endif
