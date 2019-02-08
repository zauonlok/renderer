#ifndef CONSTANT_SHADER_H
#define CONSTANT_SHADER_H

#include "../core/api.h"

typedef struct {
    vec4_t ambient_factor;
    vec4_t emission_factor;
    const char *emission_texture;
} constant_material_t;

typedef struct {
    vec3_t position;
    vec2_t texcoord;
} constant_attribs_t;

typedef struct {
    vec2_t texcoord;
} constant_varyings_t;

typedef struct {
    mat4_t mvp_matrix;
    /* from material */
    vec4_t ambient_factor;
    vec4_t emission_factor;
    texture_t *emission_texture;
} constant_uniforms_t;

/* low-level api */
vec4_t constant_vertex_shader(void *attribs, void *varyings, void *uniforms);
vec4_t constant_fragment_shader(void *varyings, void *uniforms);

/* high-level api */
model_t *constant_create_model(const char *mesh_filename, mat4_t transform,
                               constant_material_t material);
void constant_release_model(model_t *model);
constant_uniforms_t *constant_get_uniforms(model_t *model);
void constant_draw_model(model_t *model, framebuffer_t *framebuffer);

#endif
