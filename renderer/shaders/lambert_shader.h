#ifndef LAMBERT_SHADER_H
#define LAMBERT_SHADER_H

#include "../core/api.h"

typedef struct {
    float ambient;
    const char *emission;
    const char *diffuse;
    /* render settings */
    int double_sided;
    int enable_blend;
} lambert_material_t;

typedef struct {
    vec3_t position;
    vec2_t texcoord;
    vec3_t normal;
} lambert_attribs_t;

typedef struct {
    vec2_t texcoord;
    vec3_t normal;
} lambert_varyings_t;

typedef struct {
    vec3_t light_dir;
    mat4_t mvp_matrix;
    mat3_t normal_matrix;
    /* from material */
    float ambient;
    texture_t *emission;
    texture_t *diffuse;
} lambert_uniforms_t;

/* low-level api */
vec4_t lambert_vertex_shader(void *attribs, void *varyings, void *uniforms);
vec4_t lambert_fragment_shader(void *varyings, void *uniforms);

/* high-level api */
model_t *lambert_create_model(const char *mesh, mat4_t transform,
                              lambert_material_t material);
void lambert_release_model(model_t *model);
lambert_uniforms_t *lambert_get_uniforms(model_t *model);
void lambert_draw_model(model_t *model, framebuffer_t *framebuffer);

#endif
