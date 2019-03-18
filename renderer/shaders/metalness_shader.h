#ifndef METALNESS_SHADER_H
#define METALNESS_SHADER_H

#include "../core/api.h"
#include "shader_helper.h"

typedef struct {
    vec4_t basecolor_factor;
    float metallic_factor;
    float roughness_factor;
    float alpha_cutoff;
    const char *basecolor_texture;              /* in srgb space */
    const char *metallic_texture;
    const char *roughness_texture;
    /* additional maps */
    const char *normal_texture;
    const char *occlusion_texture;
    const char *emissive_texture;               /* in srgb space */
    /* render settings */
    int double_sided;
    int enable_blend;
} metalness_material_t;

typedef struct {
    vec3_t position;
    vec2_t texcoord;
    vec3_t normal;
    vec4_t tangent;
} metalness_attribs_t;

typedef struct {
    vec3_t position;
    vec2_t texcoord;
    vec3_t normal;
    mat3_t tbn_matrix;
} metalness_varyings_t;

typedef struct {
    vec3_t light_dir;
    vec3_t camera_pos;
    mat4_t model_matrix;
    mat3_t normal_matrix;
    mat4_t viewproj_matrix;
    /* from material */
    vec4_t basecolor_factor;
    float metallic_factor;
    float roughness_factor;
    float alpha_cutoff;
    texture_t *basecolor_texture;
    texture_t *metallic_texture;
    texture_t *roughness_texture;
    texture_t *normal_texture;
    texture_t *occlusion_texture;
    texture_t *emissive_texture;
    /* for environment */
    ibldata_t *shared_ibldata;
} metalness_uniforms_t;

/* low-level api */
vec4_t metalness_vertex_shader(void *attribs, void *varyings, void *uniforms);
vec4_t metalness_fragment_shader(void *varyings, void *uniforms, int *discard);

/* high-level api */
model_t *metalness_create_model(
    const char *mesh, mat4_t transform,
    metalness_material_t material, const char *env_name);
void metalness_update_uniforms(
    model_t *model, vec3_t light_dir, vec3_t camera_pos,
    mat4_t model_matrix, mat3_t normal_matrix, mat4_t viewproj_matrix);

#endif
