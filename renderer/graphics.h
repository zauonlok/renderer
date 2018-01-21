#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "geometry.h"
#include "image.h"

typedef vec4_t vertex_shader_t(int nth_vertex, void *attribs,
                               void *varyings, void *uniforms);
typedef void interp_varyings_t(void *varyings, vec3_t weights);
typedef vec4_t fragment_shader_t(void *varyings, void *uniforms);

typedef struct {
    image_t *colorbuffer;
    float *depthbuffer;
    mat4_t viewport;
} context_t;

typedef struct {
    void *attribs;
    void *varyings;
    void *uniforms;
    vertex_shader_t *vertex_shader;
    fragment_shader_t *fragment_shader;
    interp_varyings_t *interp_varyings;
} program_t;

/* context management */
context_t *gfx_create_context(int width, int height);
void gfx_release_context(context_t *context);
void gfx_clear_buffers(context_t *context);

/* triangle rasterization */
void gfx_draw_triangle(context_t *context, program_t *program);

/* vector interpolation */
vec2_t gfx_interp_vec2(vec2_t vs[3], vec3_t weights);
vec3_t gfx_interp_vec3(vec3_t vs[3], vec3_t weights);
vec4_t gfx_interp_vec4(vec4_t vs[3], vec3_t weights);

/* utility functions */
vec4_t gfx_sample_texture(image_t *texture, vec2_t texcoord);
vec3_t gfx_reflect_light(vec3_t light, vec3_t normal);

#endif
