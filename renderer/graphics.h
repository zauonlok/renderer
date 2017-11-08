#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "geometry.h"
#include "image.h"

typedef vec4f_t vertex_shader_t(int nth_vertex, void *varyings, const void *uniforms);
typedef void interp_varyings_t(vec3f_t weights, void *varyings);
typedef color_t fragment_shader_t(void *varyings, const void *uniforms);

mat4f_t gfx_lookat_matrix(vec3f_t eye, vec3f_t center, vec3f_t up);
mat4f_t gfx_viewport_matrix(int x, int y, int width, int height);
mat4f_t gfx_projection_matrix(float coeff);

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
    /*vec4f_t fs_in_position;*/
    vec2f_t fs_in_texcoord;
    vec3f_t fs_in_normal;

} varyings_t;

typedef struct {
    mat4f_t model_view;
    mat4f_t projection;

    image_t *diffuse_map;
    image_t *normal_map;
    image_t *specular_map;
} uniforms_t;



typedef struct {
    image_t *framebuffer;
    float *depthbuffer;
    mat4f_t viewport;
} context_t;


context_t *gfx_create_context(int width, int height) {
    context_t *context = (context_t*)malloc(sizeof(context_t));
    context->framebuffer = image_create(width, height, 3);
    context->depthbuffer = (float*)malloc(sizeof(float) * width * height);
    context->viewport = gfx_viewport_matrix(0, 0, width, height);
    return context;
}

void gfx_release_context(context_t *context) {
    image_release(context->framebuffer);
    free(context->depthbuffer);
}


typedef struct {
    vertex_shader_t *vertex_shader;
    fragment_shader_t *fragment_shader;
    interp_varyings_t *interp_varyings;
    void *varyings;
    void *uniforms;
} program_t;

vec4f_t vec4f_from_vec3f(vec3f_t v, float w) {
    return vec4f_new(v.x, v.y, v.z, w);
}

vec3f_t vec3f_from_vec4f(vec4f_t v) {
    return vec3f_new(v.x, v.y, v.z);
}


mat4f_t mat4f_invert(mat4f_t m) {

}


mat4f_t mat4f_transpose(mat4f_t m) {

}



vec4f_t vertex_shader(int nth_vertex, void *varyings_, const void *uniforms_) {
    varyings_t *varyings = (varyings_t*)varyings_;
    uniforms_t *uniforms = (uniforms_t*)uniforms_;


    mat4f_t MVP = mat4f_mul_mat4f(uniforms->projection, uniforms->model_view);
    mat4f_t MVPit = mat4f_transpose(mat4f_invert(MVP));


    /* setup uv */
    varyings->vs_out_texcoords[nth_vertex] = varyings->vs_in_texcoords[nth_vertex];


    /* setup normal */
    vec4f_t normal = vec4f_from_vec3f(varyings->vs_in_normals[nth_vertex], 0.0f);
    vec4f_t normal_renorm = mat4f_mul_vec4f(MVPit, normal);
    varyings->vs_out_normals[nth_vertex] = vec3f_from_vec4f(normal_renorm);

    /* setup position */

    vec4f_t position = vec4f_from_vec3f(varyings->vs_in_positions[nth_vertex], 1.0f);
    vec4f_t position_renorm = mat4f_mul_vec4f(MVP, position);

    vec3f_t ndc_position = vec3f_new(
        position_renorm.x / position_renorm.w,
        position_renorm.y / position_renorm.w,
        position_renorm.z / position_renorm.w
    );
    varyings->vs_out_positions[nth_vertex] = ndc_position;

    return position_renorm;
}


typedef struct {float m[3][3];} mat3f_t;


color_t fragment_shader(void *varyings_, const void *uniforms_) {
    varyings_t *varyings = (varyings_t*)varyings_;
    uniforms_t *uniforms = (uniforms_t*)uniforms_;

    /* calc tangent space normal mapping */
    mat3f_t A;
    vec3f_t A0 = vec3f_sub(varyings->vs_out_positions[1], varyings->vs_out_positions[0]);
    vec3f_t A1 = vec3f_sub(varyings->vs_out_positions[2], varyings->vs_out_positions[0]);
    vec3f_t A2 = varyings->fs_in_normal;


    A = mat3f_from_vec3f_rows(A0, A1, A2);
    mat3f_t Ainv = mat3f_invert(A);



    vec3f_t u1_minus_u0 = vec3f_new(
        varyings->vs_out_texcoords[1].x - varyings->vs_out_texcoords[0].x,
        varyings->vs_out_texcoords[2].x - varyings->vs_out_texcoords[0].x,
        0
    );
    vec3f_t v1_minus_v0 = vec3f_new(
        varyings->vs_out_texcoords[1].y - varyings->vs_out_texcoords[0].y,
        varyings->vs_out_texcoords[2].y - varyings->vs_out_texcoords[0].y,
        0
    );



    vec3f_t i = mat3f_mul_vec3f(Minv, u1_minus_u0);
    vec3f_t j = mat3f_mul_vec3f(Minv, v1_minus_v0);

    mat3f_t B = mat3f_from_vec3f_cols(vec3f_normalize(i), vec3f_normalize(j), varyings->fs_in_normal);



    vec3f_t normal_tex_value = gfx_sample_normal(uniforms->normal_map, varyings->fs_in_texcoord.x, varyings->fs_in_texcoord.y);

    vec3f_t normal  = vec3f_normalize(mat3f_mul_vec3f(B, normal_tex_value));


    float diff  = MAX(0.0f, vec3f_dot(normal, light_dir));
    color_t color = gfx_sample_diffuse(uniforms->diffuse_map, varyings->fs_in_texcoord.x, varyings->fs_in_texcoord.y);
    color = color_apply_mul(color, diff)

    return color;
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
    /*varyings->fs_in_position = gfx_interp_vec4f(varyings->vs_out_positions, weight);*/
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
