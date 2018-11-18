#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "geometry.h"
#include "image.h"
#include "mesh.h"

typedef enum {CLEAR_COLOR = 1, CLEAR_DEPTH = 2} clearmask_t;
typedef struct {int width, height; vec4_t *buffer;} colorbuffer_t;
typedef struct {int width, height; float *buffer;} depthbuffer_t;
typedef struct {
    int width, height;
    colorbuffer_t *colorbuffer;
    depthbuffer_t *depthbuffer;
} rendertarget_t;

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
typedef struct {mesh_t *mesh; program_t *program;} model_t;

/* rendertarget management */
rendertarget_t *rendertarget_create(int width, int height);
void rendertarget_release(rendertarget_t *rendertarget);
void rendertarget_clear(rendertarget_t *rendertarget, clearmask_t clearmask);

/* triangle rasterization */
void graphics_draw_triangle(rendertarget_t *rendertarget, program_t *program);

/* texture management */
texture_t *texture_from_image(image_t *image);
texture_t *texture_from_colorbuffer(colorbuffer_t *colorbuffer);
texture_t *texture_from_depthbuffer(depthbuffer_t *depthbuffer);
void texture_release(texture_t *texture);
vec4_t texture_sample(texture_t *texture, float u, float v);

/* private blit functions */
void colorbuffer_blit_bgr(colorbuffer_t *src, image_t *dst);
void colorbuffer_blit_rgb(colorbuffer_t *src, image_t *dst);

#endif
