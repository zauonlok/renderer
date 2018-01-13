#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "geometry.h"
#include "image.h"

typedef vec4_t vertex_shader_t(int nth_vertex, void *varyings, void *uniforms);
typedef void interp_varyings_t(vec3_t weights, void *varyings);
typedef color_t fragment_shader_t(void *varyings, void *uniforms);

typedef struct {
    image_t *framebuffer;
    float *depthbuffer;
    mat4_t viewport;
} context_t;

typedef struct {
    vertex_shader_t *vertex_shader;
    fragment_shader_t *fragment_shader;
    interp_varyings_t *interp_varyings;
    void *varyings;
    void *uniforms;
} program_t;

/* context management */
context_t *gfx_create_context(int width, int height);
void gfx_release_context(context_t *context);
void gfx_clear_buffers(context_t *context);

/* triangle rasterization */
void gfx_draw_triangle(context_t *context, program_t *program);

/* common matrices */
mat4_t gfx_lookat_matrix(vec3_t eye, vec3_t center, vec3_t up);
mat4_t gfx_projection_matrix(float coeff);

/* texture sampling */
color_t gfx_sample_texture(image_t *texture, vec2_t texcoord);
color_t gfx_sample_diffuse(image_t *diffuse_map, vec2_t texcoord);
vec3_t gfx_sample_normal(image_t *normal_map, vec2_t texcoord);
float gfx_sample_specular(image_t *specular_map, vec2_t texcoord);

/* vector interpolation */
vec2_t gfx_interp_vec2(vec2_t vs[3], vec3_t weights);
vec3_t gfx_interp_vec3(vec3_t vs[3], vec3_t weights);
vec4_t gfx_interp_vec4(vec4_t vs[3], vec3_t weights);

/* utility functions */
vec3_t gfx_reflect_light(vec3_t normal, vec3_t light);

#endif
