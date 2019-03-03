#ifndef PHONG_SHADER_H
#define PHONG_SHADER_H

#include "../core/api.h"

typedef struct {
    float ambient;
    float shininess;
    const char *emission;
    const char *diffuse;
    const char *specular;
    /* render settings */
    int double_sided;
    int enable_blend;
} phong_material_t;

typedef struct {
    vec3_t position;
    vec2_t texcoord;
    vec3_t normal;
} phong_attribs_t;

typedef struct {
    vec3_t position;
    vec2_t texcoord;
    vec3_t normal;
} phong_varyings_t;

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
} phong_uniforms_t;

/* low-level api */
vec4_t phong_vertex_shader(void *attribs, void *varyings, void *uniforms);
vec4_t phong_fragment_shader(void *varyings, void *uniforms);

/* high-level api */
model_t *phong_create_model(const char *mesh, mat4_t transform,
                            phong_material_t material);
void phong_update_uniforms(
    model_t *model, vec3_t light_dir, vec3_t camera_pos,
    mat4_t model_matrix, mat3_t normal_matrix, mat4_t viewproj_matrix);

#endif
