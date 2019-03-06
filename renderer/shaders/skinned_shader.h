#ifndef SKINNED_SHADER_H
#define SKINNED_SHADER_H

#include "../core/api.h"

typedef struct {
    vec4_t factor;
    const char *texture;
    /* render settings */
    int double_sided;
    int enable_blend;
} skinned_material_t;

typedef struct {
    vec3_t position;
    vec2_t texcoord;
    vec4_t joint;
    vec4_t weight;
} skinned_attribs_t;

typedef struct {
    vec2_t texcoord;
} skinned_varyings_t;

typedef struct {
    mat4_t mvp_matrix;
    /* from material */
    vec4_t factor;
    texture_t *texture;
    /* for animation */
    int num_joints;
    mat4_t joint_matrices[MAX_JOINTS];
} skinned_uniforms_t;

/* low-level api */
vec4_t skinned_vertex_shader(void *attribs, void *varyings, void *uniforms);
vec4_t skinned_fragment_shader(void *varyings, void *uniforms);

/* high-level api */
model_t *skinned_create_model(const char *mesh, mat4_t transform,
                              skinned_material_t material);
void skinned_update_uniforms(model_t *model, mat4_t mvp_matrix,
                             skeleton_t *skeleton);

#endif
