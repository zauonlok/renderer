#ifndef LAMBERT_SHADER_H
#define LAMBERT_SHADER_H

#include "../core/apis.h"

typedef struct {
    vec4_t ambient_factor;
    vec4_t diffuse_factor;
    vec4_t emission_factor;
    const char *diffuse_texture;
    const char *emission_texture;
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
    mat4_t model_it_matrix;
    /* from material */
    vec4_t ambient_factor;
    vec4_t diffuse_factor;
    vec4_t emission_factor;
    texture_t *diffuse_texture;
    texture_t *emission_texture;
} lambert_uniforms_t;

/* low-level apis */
vec4_t lambert_vertex_shader(void *attribs, void *varyings, void *uniforms);
vec4_t lambert_fragment_shader(void *varyings, void *uniforms);

/* high-level apis */
model_t *lambert_create_model(mat4_t transform, const char *mesh,
                              lambert_material_t material);
void lambert_release_model(model_t *model);
lambert_uniforms_t *lambert_get_uniforms(model_t *model);
void lambert_draw_model(framebuffer_t *framebuffer, model_t *model);

#endif
