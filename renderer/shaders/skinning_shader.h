#ifndef SKINNING_SHADER_H
#define SKINNING_SHADER_H

#include "../core/api.h"

typedef struct {
    vec4_t factor;
    float alpha_cutoff;
    const char *texture;
    /* render settings */
    int double_sided;
    int enable_blend;
} skinning_material_t;

typedef struct {
    vec3_t position;
    vec2_t texcoord;
    vec4_t joint;
    vec4_t weight;
} skinning_attribs_t;

typedef struct {
    vec2_t texcoord;
} skinning_varyings_t;

typedef struct {
    mat4_t mvp_matrix;
    /* from material */
    vec4_t factor;
    float alpha_cutoff;
    texture_t *texture;
    /* for animation */
    int num_joints;
    mat4_t joint_matrices[MAX_JOINTS];
} skinning_uniforms_t;

/* low-level api */
vec4_t skinning_vertex_shader(void *attribs, void *varyings, void *uniforms);
vec4_t skinning_fragment_shader(void *varyings, void *uniforms, int *discard);

/* high-level api */
model_t *skinning_create_model(const char *mesh, mat4_t transform,
                               skinning_material_t material);
void skinning_update_uniforms(model_t *model, mat4_t mvp_matrix,
                              skeleton_t *skeleton);

#endif
