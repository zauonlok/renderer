#include <math.h>
#include <stdio.h>
#include "camera.h"
#include "geometry.h"
#include "graphics.h"
#include "image.h"
#include "model.h"
#include "platform.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct {
    /* input of vertex shader */
    vec3_t positions[3];
    vec2_t texcoords[3];
} attribs_t;

typedef struct {
    /* output of vertex shader */
    vec2_t vs_out_texcoords[3];
    /* input of fragment shader */
    vec2_t fs_in_texcoord;
} varyings_t;

typedef struct {
    /* geometry uniforms */
    vec3_t light_dir;
    mat4_t mvp_matrix;
    mat4_t mvp_it_mat;
    /* texture uniforms */
    image_t *diffuse_map;
    image_t *normal_map;
    image_t *specular_map;
} uniforms_t;

vec4_t vertex_shader(int nth_vertex, void *attribs_,
                     void *varyings_, void *uniforms_) {
    attribs_t *attribs = (attribs_t*)attribs_;
    varyings_t *varyings = (varyings_t*)varyings_;
    uniforms_t *uniforms = (uniforms_t*)uniforms_;

    /* for convenience */
    vec3_t in_position = attribs->positions[nth_vertex];
    vec2_t in_texcoord = attribs->texcoords[nth_vertex];
    vec2_t *out_texcoord = &varyings->vs_out_texcoords[nth_vertex];
    mat4_t mvp_matrix = uniforms->mvp_matrix;

    /* setup position */
    vec4_t position = vec4_from_vec3(in_position, 1.0f);
    vec4_t clip_coord = mat4_mul_vec4(mvp_matrix, position);

    /* setup texcoord */
    *out_texcoord = in_texcoord;

    return clip_coord;
}

void interp_varyings(void *varyings_, vec3_t weights) {
    varyings_t *varyings = (varyings_t*)varyings_;
    vec2_t *vs_out_texcoords = varyings->vs_out_texcoords;
    varyings->fs_in_texcoord = gfx_interp_vec2(vs_out_texcoords, weights);
}

vec4_t fragment_shader(void *varyings_, void *uniforms_) {
    varyings_t *varyings = (varyings_t*)varyings_;
    uniforms_t *uniforms = (uniforms_t*)uniforms_;

    /* for convenience */
    vec2_t in_texcoord = varyings->fs_in_texcoord;
    vec3_t light_dir = uniforms->light_dir;
    mat4_t mvp_matrix = uniforms->mvp_matrix;
    mat4_t mvp_it_mat = uniforms->mvp_it_mat;
    image_t *diffuse_map = uniforms->diffuse_map;
    image_t *normal_map = uniforms->normal_map;
    image_t *specular_map = uniforms->specular_map;

    vec3_t normal, light, reflected;
    float diffuse, specular;
    color_t color;

    /* transform normal */
    {
        vec3_t in_normal = gfx_sample_normal(normal_map, in_texcoord);
        vec4_t normal_4f = vec4_from_vec3(in_normal, 0.0f);
        normal_4f = mat4_mul_vec4(mvp_it_mat, normal_4f);
        normal = vec3_normalize(vec3_from_vec4(normal_4f));
    }
    /* transform light */
    {
        vec4_t light_4f = vec4_from_vec3(light_dir, 0.0f);
        light_4f = mat4_mul_vec4(mvp_matrix, light_4f);
        light = vec3_normalize(vec3_from_vec4(light_4f));
    }
    /* calculate reflected light */
    reflected = gfx_reflect_light(normal, light);
    /* calculate specular factor */
    {
        float in_specular = gfx_sample_specular(specular_map, in_texcoord);
        float base = MAX(-reflected.z, 0.0f);
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

    return vec4_new(color.b / 255.0f, color.g / 255.0f, color.r / 255.0f, 255.0f);
}

void draw_model(context_t *context, model_t *model, image_t *diffuse_map,
                image_t *normal_map, image_t *specular_map, mat4_t viewproj) {
    program_t program;
    attribs_t attribs;
    varyings_t varyings;
    uniforms_t uniforms;
    int num_faces = model_get_num_faces(model);
    int i, j;

    vec3_t light_dir = vec3_new(1.0f, 1.0f, 1.0f);

    mat4_t mvp_matrix = viewproj;
    mat4_t mvp_it_mat = mat4_inverse_transpose(mvp_matrix);

    uniforms.light_dir    = light_dir;
    uniforms.mvp_matrix   = mvp_matrix;
    uniforms.mvp_it_mat   = mvp_it_mat;
    uniforms.diffuse_map  = diffuse_map;
    uniforms.normal_map   = normal_map;
    uniforms.specular_map = specular_map;

    program.vertex_shader   = vertex_shader;
    program.fragment_shader = fragment_shader;
    program.interp_varyings = interp_varyings;
    program.attribs         = &attribs;
    program.varyings        = &varyings;
    program.uniforms        = &uniforms;

    for (i = 0; i < num_faces; i++) {
        for (j = 0; j < 3; j++) {
            attribs.positions[j] = model_get_position(model, i, j);
            attribs.texcoords[j] = model_get_texcoord(model, i, j);
        }
        gfx_draw_triangle(context, &program);
    }
}

static const vec3_t CAMERA_POSITION = {1.0f, 1.0f, 3.0f};
static const vec3_t CAMERA_FORWARD = {0.0f, 0.0f, -1.0f};

static const int WINDOW_WIDTH = 1200;
static const int WINDOW_HEIGHT = 600;
static const char *WINDOW_TITLE = "Viewer";

static const vec3_t LIGHT_DIRECTION = {1.0f, 1.0f, 1.0f};


int main(void) {
    window_t *window;
    context_t *context;
    camera_t *camera;
    model_t *model;
    image_t *diffuse_map, *normal_map, *specular_map;
    image_t *framebuffer;
    double last_frame;
    double delta_time;
    float aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
    mat4_t model_matrix;

    window = window_create(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
    context = gfx_create_context(WINDOW_WIDTH, WINDOW_HEIGHT);
    camera = camera_new(CAMERA_POSITION, CAMERA_FORWARD, aspect);

    model = model_load("resources/african_head.obj");
    diffuse_map = image_load("resources/african_head_diffuse.tga");
    normal_map = image_load("resources/african_head_nm.tga");
    specular_map = image_load("resources/african_head_spec.tga");

    last_frame = timer_get_time();
    while (!window_should_close(window)) {
        mat4_t viewproj;
        double now = timer_get_time();
        delta_time = now - last_frame;
        last_frame = now;

        camera_process_input(camera, window, (float)delta_time);
        model_matrix = mat4_rotation_y((float)now);
        viewproj = camera_get_viewproj_matrix(camera);
        viewproj = mat4_mul_mat4(viewproj, model_matrix);

        draw_model(context, model, diffuse_map, normal_map, specular_map, viewproj);
        framebuffer = image_clone(context->colorbuffer);
        image_flip_v(framebuffer);
        gfx_clear_buffers(context);
        window_draw_image(window, framebuffer);
        image_release(framebuffer);

        input_poll_events();
    }

    model_free(model);
    image_release(diffuse_map);
    image_release(normal_map);
    image_release(specular_map);

    camera_free(camera);
    gfx_release_context(context);
    window_destroy(window);
    return 0;
}
