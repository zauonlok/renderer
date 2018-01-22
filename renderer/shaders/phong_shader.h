#ifndef PHONG_SHADER_H
#define PHONG_SHADER_H

#include "../geometry.h"
#include "../graphics.h"
#include "../image.h"

typedef struct {
    vec3_t positions[3];
    vec2_t texcoords[3];
} phong_attribs_t;

typedef struct {
    /* output of vertex shader */
    vec2_t vs_out_texcoords[3];
    vec3_t vs_out_view_pos[3];
    /* input of fragment shader */
    vec2_t fs_in_texcoord;
    vec3_t fs_in_view_pos;
} phong_varyings_t;

typedef struct {
    /* light uniforms */
    vec3_t light_direction;
    vec3_t light_ambient;
    vec3_t light_diffuse;
    vec3_t light_specular;
    /* geometry uniforms */
    mat4_t view_matrix;
    mat4_t normal_matrix;
    mat4_t mv_matrix;
    mat4_t mvp_matrix;
    /* texture uniforms */
    image_t *diffuse_map;
    image_t *normal_map;
    image_t *specular_map;
    float shininess;
} phong_uniforms_t;

vec4_t phong_vertex_shader(int nth_vertex, void *attribs,
                           void *varyings, void *uniforms);
void phong_interp_varyings(void *varyings, vec3_t weights);
vec4_t phong_fragment_shader(void *varyings, void *uniforms);

#endif
