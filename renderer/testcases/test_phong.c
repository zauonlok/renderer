#include "test_phong.h"
#include "../core/apis.h"
#include "../shaders/phong.h"

static const char *WINDOW_TITLE = "Phong Viewer";
static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 600;

static const vec3_t CAMERA_POSITION = {0, 0, 3};
static const vec3_t CAMERA_FORWARD = {0, 0, -1};

static const vec3_t LIGHT_DIRECTION = {-1, -1, -1};

static const char *MESH_PATH = "resources/african_head.obj";
static const char *NORMAL_PATH = "resources/african_head_nm.tga";
static const char *DIFFUSE_PATH = "resources/african_head_diffuse.tga";
static const char *SPECULAR_PATH = "resources/african_head_spec.tga";

static void setup_phong_uniforms(model_t *model, camera_t *camera) {
    phong_uniforms_t *uniforms = phong_get_uniforms(model);
    mat4_t model_matrix = mat4_rotate_y((float)timer_get_time());
    mat4_t model_it_matrix = mat4_inverse_transpose(model_matrix);
    mat4_t view_matrix = camera_get_view_matrix(camera);
    mat4_t proj_matrix = camera_get_proj_matrix(camera);
    mat4_t viewproj_matrix = mat4_mul_mat4(proj_matrix, view_matrix);

    uniforms->light_dir = LIGHT_DIRECTION;
    uniforms->camera_pos = camera_get_position(camera);
    uniforms->model_matrix = model_matrix;
    uniforms->model_it_matrix = model_it_matrix;
    uniforms->viewproj_matrix = viewproj_matrix;
}

void run_phong_testcase() {
    window_t *window;
    rendertarget_t *rendertarget;
    camera_t *camera;
    mesh_t *mesh;
    image_t *normal_map;
    image_t *diffuse_map;
    image_t *specular_map;
    model_t *model;
    float aspect;
    float last_time;

    /* create window, rendertarget, and camera */
    window = window_create(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
    rendertarget = rendertarget_create(WINDOW_WIDTH, WINDOW_HEIGHT);
    aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
    camera = camera_create(CAMERA_POSITION, CAMERA_FORWARD, aspect);

    /* load mesh and images */
    mesh = mesh_load(MESH_PATH);
    normal_map = image_load(NORMAL_PATH);
    diffuse_map = image_load(DIFFUSE_PATH);
    specular_map = image_load(SPECULAR_PATH);

    /* create phong model */
    model = phong_create_model(mesh, normal_map, diffuse_map, specular_map);

    /* event loop */
    last_time = (float)timer_get_time();
    while (!window_should_close(window)) {
        /* calculate delta time */
        float curr_time = (float)timer_get_time();
        float delta_time = curr_time - last_time;

        /* process events */
        camera_process_input(camera, window, delta_time);

        /* setup uniforms */

        /* render image */
        rendertarget_clear(rendertarget, CLEAR_COLOR | CLEAR_DEPTH);
        phong_draw_model(rendertarget, model);
        window_draw_buffer(window, rendertarget->colorbuffer);

        /* read events */
        input_poll_events();
    }

    /* release phong model */
    phong_release_model(model);

    /* release mesh and images */
    mesh_release(mesh);
    image_release(normal_map);
    image_release(diffuse_map);
    image_release(specular_map);

    /* release camera, rendertarget, and window */
    camera_release(camera);
    rendertarget_release(rendertarget);
    window_destroy(window);
}
