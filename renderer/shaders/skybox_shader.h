#ifndef SKYBOX_SHADER_H
#define SKYBOX_SHADER_H

#include "../core/api.h"

/* low-level api */

typedef struct {
    vec3_t position;
} skybox_attribs_t;

typedef struct {
    vec3_t direction;
} skybox_varyings_t;

typedef struct {
    mat4_t vp_matrix;
    cubemap_t *skybox;
} skybox_uniforms_t;

vec4_t skybox_vertex_shader(void *attribs, void *varyings, void *uniforms);
vec4_t skybox_fragment_shader(void *varyings, void *uniforms,
                              int *discard, int backface);

/* high-level api */

model_t *skybox_create_model(const char *skybox_name, int blur_level);

#endif
