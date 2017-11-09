#include <math.h>
#include "geometry.h"
#include "graphics.h"
#include "image.h"
#include "model.h"
#include "platform.h"

#define WIDTH 800
#define HEIGHT 800

typedef struct {
    /* input of vertex shader */
    vec3f_t vs_in_positions[3];
    vec2f_t vs_in_texcoords[3];
    /* output of vertex shader */
    vec2f_t vs_out_texcoords[3];
    /* input of fragment shader */
    vec2f_t fs_in_texcoord;
} varyings_t;

typedef struct {
    vec3f_t light_dir;
    /* matrix uniforms */
    mat4f_t model_view;
    mat4f_t projection;
    /* texture uniforms */
    image_t *diffuse_map;
    image_t *normal_map;
    image_t *specular_map;
} uniforms_t;

vec4f_t vertex_shader(int nth_vertex, void *varyings_, void *uniforms_) {
    varyings_t *varyings = (varyings_t*)varyings_;
    uniforms_t *uniforms = (uniforms_t*)uniforms_;

    /* setup position */
    vec4f_t position = vec4f_from_vec3f(varyings->vs_in_positions[nth_vertex], 1.0f);
    mat4f_t transform = mat4f_mul_mat4f(uniforms->projection, uniforms->model_view);
    vec4f_t clip_coord = mat4f_mul_vec4f(transform, position);

    /* setup texcoord */
    varyings->vs_out_texcoords[nth_vertex] = varyings->vs_in_texcoords[nth_vertex];

    return clip_coord;
}

void interp_varyings(vec3f_t weights, void *varyings_) {
    varyings_t *varyings = (varyings_t*)varyings_;
    varyings->fs_in_texcoord = gfx_interp_vec2f(varyings->vs_out_texcoords, weights);
}

vec3f_t reflect_light(vec3f_t normal, vec3f_t light) {
    float n_dot_l = vec3f_dot(normal, light);
    vec3f_t n_what = vec3f_new(normal.x * n_dot_l * 2.0f,
                                normal.y * n_dot_l * 2.0f,
                                normal.z * n_dot_l * 2.0f);
    return vec3f_normalize(vec3f_sub(n_what, light));

}

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

color_t fragment_shader(void *varyings_, void *uniforms_) {
    varyings_t *varyings = (varyings_t*)varyings_;
    uniforms_t *uniforms = (uniforms_t*)uniforms_;

    vec2f_t texcoord = varyings->fs_in_texcoord;
    mat4f_t MVP = mat4f_mul_mat4f(uniforms->projection, uniforms->model_view);
    mat4f_t MVPit = mat4f_invert_transpose(MVP);

    vec3f_t normal = gfx_sample_normal(uniforms->normal_map, texcoord.x, texcoord.y);
    vec4f_t normal_4f = mat4f_mul_vec4f(MVPit, vec4f_from_vec3f(normal, 1.0f));
    vec3f_t normal_3f = vec3f_normalize(vec3f_from_vec4f(normal_4f));

    vec3f_t light = uniforms->light_dir;
    vec4f_t light_4f = mat4f_mul_vec4f(MVP, vec4f_from_vec3f(light, 1.0f));
    vec3f_t light_3f = vec3f_normalize(vec3f_from_vec4f(light_4f));

    vec3f_t reflect = reflect_light(normal_3f, light_3f);

    float specular = gfx_sample_specular(uniforms->specular_map, texcoord.x, texcoord.y);
    float spec = (float)pow(MAX(reflect.z, 0.0f), specular);
    float diff = MAX(0.f, vec3f_dot(normal_3f, light_3f));

    color_t diffuse = gfx_sample_diffuse(uniforms->diffuse_map, texcoord.x, texcoord.y);
    color_t color;

    color.b = (unsigned char)MIN(255, 20 + diffuse.b * (1.2 * diff + 0.6 * spec));
    color.g = (unsigned char)MIN(255, 20 + diffuse.g * (1.2 * diff + 0.6 * spec));
    color.r = (unsigned char)MIN(255, 20 + diffuse.r * (1.2 * diff + 0.6 * spec));

    return color;
}

void draw_model(context_t *context, model_t *model, image_t *diffuse_map,
                image_t *normal_map, image_t *specular_map) {
    program_t program;
    varyings_t varyings;
    uniforms_t uniforms;
    int num_faces = model_get_num_faces(model);
    int i, j;

    vec3f_t eye = vec3f_new(1.0f, 1.0f, 4.0f);
    vec3f_t center = vec3f_new(0.0f, 0.0f, 0.0f);
    vec3f_t up = vec3f_new(0.0f, 1.0f, 0.0f);

    uniforms.light_dir    = vec3f_new(1.0f, 1.0f, 0.0f);
    uniforms.model_view   = gfx_lookat_matrix(eye, center, up);
    uniforms.projection   = gfx_projection_matrix(eye, center);
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
    window_t *window;
    model_t *model;
    const char *title = "Viewer";
    int width = WIDTH;
    int height = HEIGHT;
    image_t *diffuse_map, *normal_map, *specular_map;
    context_t *context;

    window = window_create(title, width, height);
    context = gfx_create_context(width, height);

    model = model_load("resources/african_head.obj");
    diffuse_map = image_load("resources/african_head_diffuse.tga");
    normal_map = image_load("resources/african_head_nm.tga");
    specular_map = image_load("resources/african_head_spec.tga");

    draw_model(context, model, diffuse_map, normal_map, specular_map);

    while (!window_should_close(window)) {
        window_draw_image(window, context->framebuffer);
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
