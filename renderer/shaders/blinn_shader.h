#ifndef BLINN_SHADER_H
#define BLINN_SHADER_H

#include "../core/api.h"

typedef struct {
    float ambient;
    float shininess;
    const char *emission;
    const char *diffuse;
    const char *specular;
} blinn_material_t;

typedef struct {
    vec3_t position;
    vec2_t texcoord;
    vec3_t normal;
} blinn_attribs_t;

typedef struct {
    vec3_t position;
    vec2_t texcoord;
    vec3_t normal;
} blinn_varyings_t;

typedef struct {
    vec3_t light_dir;
    vec3_t camera_pos;
    mat4_t model_matrix;
    mat3_t normal_matrix;
    mat4_t viewproj_matrix;
    /* from material */
    float ambient;
    float shininess;
    texture_t *emission;
    texture_t *diffuse;
    texture_t *specular;
} blinn_uniforms_t;

/* low-level api */
vec4_t blinn_vertex_shader(void *attribs, void *varyings, void *uniforms);
vec4_t blinn_fragment_shader(void *varyings, void *uniforms);

/* high-level api */
model_t *blinn_create_model(const char *mesh, mat4_t transform,
                            blinn_material_t material);
void blinn_release_model(model_t *model);
blinn_uniforms_t *blinn_get_uniforms(model_t *model);
void blinn_draw_model(model_t *model, framebuffer_t *framebuffer);

#endif
