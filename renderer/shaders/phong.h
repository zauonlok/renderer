#ifndef SHADER_PHONG_H
#define SHADER_PHONG_H

#include "../core/apis.h"

typedef struct {
    vec3_t local_pos;
    vec2_t texcoord;
} phong_attribs_t;

typedef struct {
    vec3_t world_pos;
    vec2_t texcoord;
} phong_varyings_t;

typedef struct {
    vec3_t light_dir;
    vec3_t camera_pos;
    mat4_t model_matrix;
    mat4_t model_it_matrix;
    mat4_t viewproj_matrix;
    texture_t *normal_map;
    texture_t *diffuse_map;
    texture_t *specular_map;
} phong_uniforms_t;

/* low-level apis */
vec4_t phong_vertex_shader(void *attribs, void *varyings, void *uniforms);
vec4_t phong_fragment_shader(void *varyings, void *uniforms);

/* high-level apis */
model_t *phong_create_model(mesh_t *mesh, image_t *normal_map,
                            image_t *diffuse_map, image_t *specular_map);
void phong_release_model(model_t *model);
phong_uniforms_t *phong_get_uniforms(model_t *model);
void phong_draw_model(framebuffer_t *framebuffer, model_t *model);

#endif
