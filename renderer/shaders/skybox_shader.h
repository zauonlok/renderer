#ifndef SKYBOX_SHADER_H
#define SKYBOX_SHADER_H

#include "../core/api.h"

typedef struct {
    vec3_t position;
} skybox_attribs_t;

typedef struct {
    vec3_t direction;
} skybox_varyings_t;

typedef struct {
    cubemap_t *skybox;
    mat4_t view_proj_matrix;
} skybox_uniforms_t;

/* low-level api */
vec4_t skybox_vertex_shader(void *attribs, void *varyings, void *uniforms);
vec4_t skybox_fragment_shader(void *varyings, void *uniforms, int *discard);

/* high-level api */
model_t *skybox_create_model(const char *skybox_name);

#endif
