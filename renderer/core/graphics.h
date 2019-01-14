#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "geometry.h"
#include "image.h"
#include "mesh.h"

typedef struct {int width, height; vec4_t *buffer;} colorbuffer_t;
typedef struct {int width, height; float *buffer;} depthbuffer_t;
typedef struct {
    int width, height;
    colorbuffer_t *colorbuffer;
    depthbuffer_t *depthbuffer;
} framebuffer_t;

typedef vec4_t vertex_shader_t(void *attribs, void *varyings, void *uniforms);
typedef vec4_t fragment_shader_t(void *varyings, void *uniforms);
typedef struct {
    void *attribs[3];
    void *varyings[4];
    void *uniforms;
    vertex_shader_t *vertex_shader;
    fragment_shader_t *fragment_shader;
    int sizeof_varyings;
} program_t;

typedef struct {int width, height; vec4_t *buffer;} texture_t;
typedef struct {mat4_t transform; mesh_t *mesh; program_t *program;} model_t;

/* framebuffer management */
framebuffer_t *framebuffer_create(int width, int height);
void framebuffer_release(framebuffer_t *framebuffer);
void framebuffer_clear_color(framebuffer_t *framebuffer, vec4_t color);
void framebuffer_clear_depth(framebuffer_t *framebuffer, float depth);

/* program management */
program_t *program_create(
    vertex_shader_t *vertex_shader, fragment_shader_t *fragment_shader,
    int sizeof_attribs, int sizeof_varyings, int sizeof_uniforms);
void program_release(program_t *program);

/* texture management */
texture_t *texture_from_file(const char *filename);
texture_t *texture_from_image(image_t *image);
void texture_release(texture_t *texture);
vec4_t texture_sample(texture_t *texture, vec2_t texcoord);

/* triangle rasterization */
void graphics_draw_triangle(framebuffer_t *framebuffer, program_t *program);

#endif
