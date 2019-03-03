#ifndef SPECULAR_SHADER_H
#define SPECULAR_SHADER_H

#include "../core/api.h"
#include "pbr_helper.h"

typedef struct {
    vec4_t diffuse_factor;
    const char *diffuse_texture;                /* in srgb space */
    vec3_t specular_factor;
    const char *specular_texture;               /* in srgb space */
    float glossiness_factor;
    const char *glossiness_texture;
    /* additional maps */
    const char *normal_texture;
    const char *occlusion_texture;
    const char *emissive_texture;               /* in srgb space */
    /* render settings */
    int double_sided;
    int enable_blend;
} specular_material_t;

typedef struct {
    vec3_t position;
    vec2_t texcoord;
    vec3_t normal;
    vec4_t tangent;
} specular_attribs_t;

typedef struct {
    vec3_t position;
    vec2_t texcoord;
    mat3_t tbn_matrix;
} specular_varyings_t;

typedef struct {
    vec3_t light_dir;
    vec3_t camera_pos;
    mat4_t model_matrix;
    mat3_t normal_matrix;
    mat4_t viewproj_matrix;
    /* from material */
    vec4_t diffuse_factor;
    texture_t *diffuse_texture;
    vec3_t specular_factor;
    texture_t *specular_texture;
    float glossiness_factor;
    texture_t *glossiness_texture;
    texture_t *normal_texture;
    texture_t *occlusion_texture;
    texture_t *emissive_texture;
    /* for environment */
    ibldata_t *shared_ibldata;
} specular_uniforms_t;

/* low-level api */
vec4_t specular_vertex_shader(void *attribs, void *varyings, void *uniforms);
vec4_t specular_fragment_shader(void *varyings, void *uniforms);

/* high-level api */
model_t *specular_create_model(
    const char *mesh, mat4_t transform,
    specular_material_t material, const char *env_name);
void specular_update_uniforms(
    model_t *model, vec3_t light_dir, vec3_t camera_pos,
    mat4_t model_matrix, mat3_t normal_matrix, mat4_t viewproj_matrix);

#endif
