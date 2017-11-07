#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "geometry.h"
#include "image.h"

typedef void vertex_shader_t(void *varyings, void *uniforms);

typedef void fragment_shader_t(void *varyings, void *uniforms);

mat4f_t gfx_lookat_matrix(vec3f_t eye, vec3f_t center, vec3f_t up);
mat4f_t gfx_viewport_matrix(int x, int y, int width, int height);
mat4f_t gfx_projection_matrix(float coeff);

void gfx_draw_triangle(
    context_t *context,
    vertex_shader_t *vertex_shader,
    fragment_shader_t *fragment_shader,

);

#endif
