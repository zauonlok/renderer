#include <math.h>
#include <stdio.h>
#include "geometry.h"
#include "graphics.h"
#include "image.h"
#include "model.h"
#include "platform.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct {
    /* input of vertex shader */
    vec3_t vs_in_positions[3];
    vec2_t vs_in_texcoords[3];
    /* output of vertex shader */
    vec2_t vs_out_texcoords[3];
    /* input of fragment shader */
    vec2_t fs_in_texcoord;
} varyings_t;

typedef struct {
    /* geometry uniforms */
    vec3_t light_dir;
    mat4f_t mvp_matrix;
    mat4f_t mvp_it_mat;
    /* texture uniforms */
    image_t *diffuse_map;
    image_t *normal_map;
    image_t *specular_map;
} uniforms_t;

vec4f_t vertex_shader(int nth_vertex, void *varyings_, void *uniforms_) {
    varyings_t *varyings = (varyings_t*)varyings_;
    uniforms_t *uniforms = (uniforms_t*)uniforms_;

    /* for convenience */
    vec3_t in_position = varyings->vs_in_positions[nth_vertex];
    vec2_t in_texcoord = varyings->vs_in_texcoords[nth_vertex];
    vec2_t *out_texcoord = &varyings->vs_out_texcoords[nth_vertex];
    mat4f_t mvp_matrix = uniforms->mvp_matrix;

    /* setup position */
    vec4f_t position = vec4f_from_vec3(in_position, 1.0f);
    vec4f_t clip_coord = mat4f_mul_vec4f(mvp_matrix, position);

    /* setup texcoord */
    *out_texcoord = in_texcoord;

    return clip_coord;
}

void interp_varyings(vec3_t weights, void *varyings_) {
    varyings_t *varyings = (varyings_t*)varyings_;
    vec2_t *vs_out_texcoords = varyings->vs_out_texcoords;
    varyings->fs_in_texcoord = gfx_interp_vec2(vs_out_texcoords, weights);
}

color_t fragment_shader(void *varyings_, void *uniforms_) {
    varyings_t *varyings = (varyings_t*)varyings_;
    uniforms_t *uniforms = (uniforms_t*)uniforms_;

    /* for convenience */
    vec2_t in_texcoord = varyings->fs_in_texcoord;
    vec3_t light_dir = uniforms->light_dir;
    mat4f_t mvp_matrix = uniforms->mvp_matrix;
    mat4f_t mvp_it_mat = uniforms->mvp_it_mat;
    image_t *diffuse_map = uniforms->diffuse_map;
    image_t *normal_map = uniforms->normal_map;
    image_t *specular_map = uniforms->specular_map;

    vec3_t normal, light, reflected;
    float diffuse, specular;
    color_t color;

    /* transform normal */
    {
        vec3_t in_normal = gfx_sample_normal(normal_map, in_texcoord);
        vec4f_t normal_4f = vec4f_from_vec3(in_normal, 0.0f);
        normal_4f = mat4f_mul_vec4f(mvp_it_mat, normal_4f);
        normal = vec3_normalize(vec3_from_vec4f(normal_4f));
    }
    /* transform light */
    {
        vec4f_t light_4f = vec4f_from_vec3(light_dir, 0.0f);
        light_4f = mat4f_mul_vec4f(mvp_matrix, light_4f);
        light = vec3_normalize(vec3_from_vec4f(light_4f));
    }
    /* calculate reflected light */
    reflected = gfx_reflect_light(normal, light);
    /* calculate specular factor */
    {
        float in_specular = gfx_sample_specular(specular_map, in_texcoord);
        float base = MAX(reflected.z, 0.0f);
        specular = (float)pow(base, in_specular);
    }
    /* calculate diffuse factor */
    {
        float n_dot_l = vec3_dot(normal, light);
        diffuse = MAX(n_dot_l, 0.0f);
    }

    /* using Phong reflection model */
    {
        color_t in_color = gfx_sample_diffuse(diffuse_map, in_texcoord);
        float color_b = 5.0f + in_color.b * (diffuse + 0.6f * specular);
        float color_g = 5.0f + in_color.g * (diffuse + 0.6f * specular);
        float color_r = 5.0f + in_color.r * (diffuse + 0.6f * specular);
        color.b = (unsigned char)MIN(color_b, 255.0f);
        color.g = (unsigned char)MIN(color_g, 255.0f);
        color.r = (unsigned char)MIN(color_r, 255.0f);
    }

    return color;
}

void draw_model(context_t *context, model_t *model, image_t *diffuse_map,
                image_t *normal_map, image_t *specular_map) {
    program_t program;
    varyings_t varyings;
    uniforms_t uniforms;
    int num_faces = model_get_num_faces(model);
    int i, j;

    vec3_t light_dir = vec3_new(1.0f, 1.0f, 1.0f);
    vec3_t eye = vec3_new(1.0f, 1.0f, 3.0f);
    vec3_t center = vec3_new(0.0f, 0.0f, 0.0f);
    vec3_t up = vec3_new(0.0f, 1.0f, 0.0f);

    mat4f_t model_view = gfx_lookat_matrix(eye, center, up);
    float coeff = -1.0f / vec3_length(vec3_sub(eye, center));
    mat4f_t projection = gfx_projection_matrix(coeff);

    mat4f_t mvp_matrix = mat4f_mul_mat4f(projection, model_view);
    mat4f_t mvp_it_mat = mat4f_invert_transpose(mvp_matrix);

    uniforms.light_dir    = light_dir;
    uniforms.mvp_matrix   = mvp_matrix;
    uniforms.mvp_it_mat   = mvp_it_mat;
    uniforms.diffuse_map  = diffuse_map;
    uniforms.normal_map   = normal_map;
    uniforms.specular_map = specular_map;

    program.vertex_shader   = vertex_shader;
    program.fragment_shader = fragment_shader;
    program.interp_varyings = interp_varyings;
    program.varyings        = &varyings;
    program.uniforms        = &uniforms;

    for (i = 0; i < num_faces; i++) {
        for (j = 0; j < 3; j++) {
            varyings.vs_in_positions[j] = model_get_position(model, i, j);
            varyings.vs_in_texcoords[j] = model_get_texcoord(model, i, j);
        }
        gfx_draw_triangle(context, &program);
    }
}

int main(void) {
    const char *title = "Viewer";
    int width = 800;
    int height = 800;

    window_t *window;
    context_t *context;
    model_t *model;
    image_t *diffuse_map, *normal_map, *specular_map;
    image_t *framebuffer;
    double start;

    window = window_create(title, width, height);
    context = gfx_create_context(width, height);

    model = model_load("resources/african_head.obj");
    diffuse_map = image_load("resources/african_head_diffuse.tga");
    normal_map = image_load("resources/african_head_nm.tga");
    specular_map = image_load("resources/african_head_spec.tga");

    start = timer_get_time();
    draw_model(context, model, diffuse_map, normal_map, specular_map);
    printf("It takes %f seconds.\n", timer_get_time() - start);
    framebuffer = image_clone(context->framebuffer);
    image_flip_v(framebuffer);

    while (!window_should_close(window)) {
        window_draw_image(window, framebuffer);
        input_poll_events();
    }

    model_free(model);
    image_release(diffuse_map);
    image_release(normal_map);
    image_release(specular_map);

    gfx_release_context(context);
    window_destroy(window);
    return 0;
}
