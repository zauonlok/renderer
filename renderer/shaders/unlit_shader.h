#ifndef UNLIT_SHADER_H
#define UNLIT_SHADER_H

#include "../core/api.h"

typedef struct {
    vec4_t factor;
    const char *texture;
    /* render settings */
    int double_sided;
    int enable_blend;
} unlit_material_t;

typedef struct {
    vec3_t position;
    vec2_t texcoord;
} unlit_attribs_t;

typedef struct {
    vec2_t texcoord;
} unlit_varyings_t;

typedef struct {
    mat4_t mvp_matrix;
    /* from material */
    vec4_t factor;
    texture_t *texture;
} unlit_uniforms_t;

/* low-level api */
vec4_t unlit_vertex_shader(void *attribs, void *varyings, void *uniforms);
vec4_t unlit_fragment_shader(void *varyings, void *uniforms);

/* high-level api */
model_t *unlit_create_model(const char *mesh, mat4_t transform,
                            unlit_material_t material);
void unlit_update_uniforms(model_t *model, mat4_t mvp_matrix);

#endif
