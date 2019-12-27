#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "maths.h"

typedef struct {
    int width, height;
    unsigned char *color_buffer;
    float *depth_buffer;
} framebuffer_t;

typedef struct program program_t;
typedef vec4_t vertex_shader_t(void *attribs, void *varyings, void *uniforms);
typedef vec4_t fragment_shader_t(void *varyings, void *uniforms,
                                 int *discard, int backface);

/* framebuffer management */
framebuffer_t *framebuffer_create(int width, int height);
void framebuffer_release(framebuffer_t *framebuffer);
void framebuffer_clear_color(framebuffer_t *framebuffer, vec4_t color);
void framebuffer_clear_depth(framebuffer_t *framebuffer, float depth);

/* program management */
program_t *program_create(
    vertex_shader_t *vertex_shader, fragment_shader_t *fragment_shader,
    int sizeof_attribs, int sizeof_varyings, int sizeof_uniforms,
    int double_sided, int enable_blend);
void program_release(program_t *program);
void *program_get_attribs(program_t *program, int nth_vertex);
void *program_get_uniforms(program_t *program);

/* graphics pipeline */
void graphics_draw_triangle(framebuffer_t *framebuffer, program_t *program);

#endif
