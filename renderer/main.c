#include "camera.h"
#include "geometry.h"
#include "graphics.h"
#include "image.h"
#include "model.h"
#include "platform.h"
#include "shaders/phong_shader.h"

/* global constants */

static const char *WINDOW_TITLE = "Viewer";
static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 600;

static const vec3_t CAMERA_POSITION = {0.0f, 0.0f, 2.5f};
static const vec3_t CAMERA_FORWARD = {0.0f, 0.0f, -1.0f};

static const vec3_t LIGHT_DIRECTION = {-1.0f, -1.0f, -1.0f};
static const vec3_t LIGHT_AMBIENT = {0.2f, 0.2f, 0.2f};
static const vec3_t LIGHT_DIFFUSE = {0.5f, 0.5f, 0.5f};
static const vec3_t LIGHT_SPECULAR = {1.0f, 1.0f, 1.0f};

static const float SHININESS = 32.0f;

static const char *MODEL_PATH = "resources/african_head.obj";
static const char *DIFFUSE_PATH = "resources/african_head_diffuse.tga";
static const char *NORMAL_PATH = "resources/african_head_nm.tga";
static const char *SPECULAR_PATH = "resources/african_head_spec.tga";

/* model drawing */

void draw_model(context_t *context, camera_t *camera, model_t *model,
                image_t *diffuse_map, image_t *normal_map, image_t *spec_map) {
    phong_attribs_t attribs;
    phong_varyings_t varyings;
    phong_uniforms_t uniforms;
    program_t program;
    int num_faces = model_get_num_faces(model);
    int i, j;

    mat4_t model_matrix = mat4_rotation_y((float)timer_get_time());
    mat4_t view_matrix = camera_get_view_matrix(camera);
    mat4_t proj_matrix = camera_get_proj_matrix(camera);

    mat4_t model_view_matrix = mat4_mul_mat4(view_matrix, model_matrix);
    mat4_t normal_matrix = gfx_normal_matrix(model_view_matrix);
    mat4_t mvp_matrix = mat4_mul_mat4(proj_matrix, model_view_matrix);

    uniforms.light_direction = LIGHT_DIRECTION;
    uniforms.light_ambient   = LIGHT_AMBIENT;
    uniforms.light_diffuse   = LIGHT_DIFFUSE;
    uniforms.light_specular  = LIGHT_SPECULAR;

    uniforms.view_matrix     = view_matrix;
    uniforms.normal_matrix   = normal_matrix;
    uniforms.mv_matrix       = model_view_matrix;
    uniforms.mvp_matrix      = mvp_matrix;

    uniforms.diffuse_map     = diffuse_map;
    uniforms.normal_map      = normal_map;
    uniforms.specular_map    = spec_map;
    uniforms.shininess       = SHININESS;

    program.attribs         = &attribs;
    program.varyings        = &varyings;
    program.uniforms        = &uniforms;
    program.vertex_shader   = phong_vertex_shader;
    program.fragment_shader = phong_fragment_shader;
    program.interp_varyings = phong_interp_varyings;

    for (i = 0; i < num_faces; i++) {
        for (j = 0; j < 3; j++) {
            attribs.positions[j] = model_get_position(model, i, j);
            attribs.texcoords[j] = model_get_texcoord(model, i, j);
        }
        gfx_draw_triangle(context, &program);
    }
}

int main(int argc, char *argv[]) {
    window_t *window;
    context_t *context;
    camera_t *camera;

    model_t *model;
    image_t *diffuse_map;
    image_t *normal_map;
    image_t *specular_map;

    float aspect;
    float last_frame;

    /* create window, context, and camera */
    window = window_create(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
    context = gfx_create_context(WINDOW_WIDTH, WINDOW_HEIGHT);
    aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
    camera = camera_create(CAMERA_POSITION, CAMERA_FORWARD, aspect);

    /* load model and textures */
    if (argc == 5) {
        model = model_load(argv[1]);
        diffuse_map = image_load(argv[2]);
        normal_map = image_load(argv[3]);
        specular_map = image_load(argv[4]);
    } else {
        model = model_load(MODEL_PATH);
        diffuse_map = image_load(DIFFUSE_PATH);
        normal_map = image_load(NORMAL_PATH);
        specular_map = image_load(SPECULAR_PATH);
    }

    last_frame = (float)timer_get_time();
    while (!window_should_close(window)) {
        image_t *framebuffer;
        /* calculate delta time */
        float curr_frame = (float)timer_get_time();
        float delta_time = curr_frame - last_frame;
        last_frame = curr_frame;

        /* render image */
        gfx_clear_buffers(context);
        camera_process_input(camera, window, delta_time);
        draw_model(context, camera, model,
                   diffuse_map, normal_map, specular_map);

        /* display image */
        framebuffer = image_clone(context->colorbuffer);
        image_flip_v(framebuffer);
        window_draw_image(window, framebuffer);
        image_release(framebuffer);

        /* read events */
        input_poll_events();
    }

    /* release model and textures */
    model_release(model);
    image_release(diffuse_map);
    image_release(normal_map);
    image_release(specular_map);

    /* release window, context, and camera */
    camera_release(camera);
    gfx_release_context(context);
    window_destroy(window);
    return 0;
}
