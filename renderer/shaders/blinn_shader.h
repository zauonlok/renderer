#ifndef BLINN_SHADER_H
#define BLINN_SHADER_H

#include "../core/apis.h"

typedef struct {
    vec4_t ambient_factor;
    vec4_t diffuse_factor;
    vec4_t emission_factor;
    vec4_t specular_factor;
    float shininess;
    const char *diffuse_texture;
    const char *emission_texture;
    const char *specular_texture;
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
    mat4_t model_it_matrix;
    mat4_t viewproj_matrix;
    /* from material */
    vec4_t ambient_factor;
    vec4_t diffuse_factor;
    vec4_t emission_factor;
    vec4_t specular_factor;
    float shininess;
    texture_t *diffuse_texture;
    texture_t *emission_texture;
    texture_t *specular_texture;
} blinn_uniforms_t;

/* low-level apis */
vec4_t blinn_vertex_shader(void *attribs, void *varyings, void *uniforms);
vec4_t blinn_fragment_shader(void *varyings, void *uniforms);

/* high-level apis */
model_t *blinn_create_model(mat4_t transform, const char *mesh,
                            blinn_material_t material);
void blinn_release_model(model_t *model);
blinn_uniforms_t *blinn_get_uniforms(model_t *model);
void blinn_draw_model(framebuffer_t *framebuffer, model_t *model);

#endif
