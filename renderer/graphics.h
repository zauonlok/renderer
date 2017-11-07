#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "geometry.h"
#include "image.h"

typedef void vertex_shader_t(int nth_vertex, void *varyings, const void *uniforms);
typedef void interp_varyings_t(vec3f_t weight, void *varyings);
typedef int fragment_shader_t(color_t *color, void *varyings, const void *uniforms);

mat4f_t gfx_lookat_matrix(vec3f_t eye, vec3f_t center, vec3f_t up);
mat4f_t gfx_viewport_matrix(int x, int y, int width, int height);
mat4f_t gfx_projection_matrix(float coeff);

void gfx_draw_triangle(
    context_t *context,
    vertex_shader_t *vert_shader,
    fragment_shader_t *frag_shader,
    interp_varyings_t *interp_varyings,
    void *varyings,
    void *uniforms
);

typedef struct {
    /* input of vertex shader */
    vec3f_t vs_in_positions[3];
    vec2f_t vs_in_texcoords[3];
    vec3f_t vs_in_normals[3];
    /* output of vertex shader */
    vec4f_t vs_out_positions[3];
    vec2f_t vs_out_texcoords[3];
    vec3f_t vs_out_normals[3];
    /* input of fragment shader */
    vec4f_t fs_in_position;
    vec2f_t fs_in_texcoord;
    vec3f_t fs_in_normal;

} varyings_t;

typedef struct {
    image_t *diffuse_map;
    image_t *normal_map;
    image_t *specular_map;
} uniforms_t;



context_t *gfx_create_context(int width, int height) {

}

void gfx_release_context(context_t *context) {

}


vec2f_t gfx_interp_vec2f(vec2f_t vs[3], vec3f_t weight_) {
    vec2f_t output;
    float weight[3];
    output.x = vs[0].x * weight[0]  + vs[1].x * weight[1] + vs[2].x * weight[2];
    output.y = vs[0].y * weight[0]  + vs[1].y * weight[1] + vs[2].y * weight[2];
    return output;
}

vec3f_t gfx_interp_vec3f(vec3f_t vs[3], vec3f_t weight_) {
    vec3f_t output;
    float weight[3];
    output.x = vs[0].x * weight[0]  + vs[1].x * weight[1] + vs[2].x * weight[2];
    output.y = vs[0].y * weight[0]  + vs[1].y * weight[1] + vs[2].y * weight[2];
    output.z = vs[0].z * weight[0]  + vs[1].z * weight[1] + vs[2].z * weight[2];
    return output;
}

vec4f_t gfx_interp_vec4f(vec4f_t vs[3], vec3f_t weight_) {
    vec4f_t output;
    float weight[3];
    output.x = vs[0].x * weight[0]  + vs[1].x * weight[1] + vs[2].x * weight[2];
    output.y = vs[0].y * weight[0]  + vs[1].y * weight[1] + vs[2].y * weight[2];
    output.z = vs[0].z * weight[0]  + vs[1].z * weight[1] + vs[2].z * weight[2];
    output.w = vs[0].w * weight[0]  + vs[1].w * weight[1] + vs[2].w * weight[2];
    return output;
}


void interp_varyings(vec3f_t weight, void *varyings_) {
    varyings_t *varyings = (varyings_t*)varyings_;
    varyings->fs_in_position = gfx_interp_vec4f(varyings->vs_out_positions, weight);
    varyings->fs_in_texcoord = gfx_interp_vec2f(varyings->vs_out_texcoords, weight);
    varyings->fs_in_normal = gfx_interp_vec3f(varyings->vs_out_normals, weight);
}

#include <assert.h>

color_t gfx_sample_texture(const image_t *texture, float u, float v) {
    int row, col;
    assert(u >= 0.0f && u <= 1.0f && v >= 0.0f && v <= 1.0f);
    col = (texture->width - 1) * u;
    row = (texture->height - 1) * v;
    return image_get_color(texture, row, col);
}

color_t gfx_sample_diffuse(const image_t *diffuse_map, float u, float v) {
    return gfx_sample_texture(diffuse_map, u, v);
}

vec3f_t gfx_sample_normal(const image_t *normal_map, float u, float v) {
    color_t color = gfx_sample_texture(normal_map, u, v);
    vec3f_t normal;
    normal.x = color.r / 255.0f * 2.0f - 1.0f;
    normal.y = color.g / 255.0f * 2.0f - 1.0f;
    normal.z = color.b / 255.0f * 2.0f - 1.0f;
    return normal;
}

float gfx_sample_specular(const image_t *specular_map, float u, float v) {
    color_t color = gfx_sample_texture(specular_map, u, v);
    assert(color.b == color.g && color.b == color.r);
    return (float)color.b;
}

#endif
