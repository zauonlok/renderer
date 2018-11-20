#include "test_phong.h"
#include <stdio.h>
#include <string.h>
#include "../core/apis.h"
#include "../shaders/phong.h"

static const char *WINDOW_TITLE = "Phong Viewer";
static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 600;

static const vec3_t CAMERA_POSITION = {0, 0, 3};
static const vec3_t CAMERA_FORWARD = {0, 0, -1};

static const vec3_t LIGHT_DIRECTION = {-1, -1, -1};

static const char *AFRICAN_HEAD_MESH_PATHS[] = {
    "resources/african_head/head.obj",
    "resources/african_head/eye_inner.obj",
    /* "resources/african_head/eye_outer.obj", */
};
static const char *AFRICAN_HEAD_NORMAL_PATHS[] = {
    "resources/african_head/head_nm.tga",
    "resources/african_head/eye_inner_nm.tga",
    /* "resources/african_head/eye_outer_nm.tga", */
};
static const char *AFRICAN_HEAD_DIFFUSE_PATHS[] = {
    "resources/african_head/head_diffuse.tga",
    "resources/african_head/eye_inner_diffuse.tga",
    /* "resources/african_head/eye_outer_diffuse.tga", */
};
static const char *AFRICAN_HEAD_SPECULAR_PATHS[] = {
    "resources/african_head/head_spec.tga",
    "resources/african_head/eye_inner_spec.tga",
    /* "resources/african_head/eye_outer_spec.tga", */
};

static model_t **create_models(const char *model_name) {
    model_t **models = NULL;

    if (strcmp(model_name, "african_head") == 0) {
        int i;
        for (i = 0; i < 2; i++) {
            mesh_t *mesh = mesh_load(AFRICAN_HEAD_MESH_PATHS[i]);
            image_t *normal_map = image_load(AFRICAN_HEAD_NORMAL_PATHS[i]);
            image_t *diffuse_map = image_load(AFRICAN_HEAD_DIFFUSE_PATHS[i]);
            image_t *specular_map = image_load(AFRICAN_HEAD_SPECULAR_PATHS[i]);
            model_t *model = phong_create_model(mesh, normal_map,
                                                diffuse_map, specular_map);
            image_release(normal_map);
            image_release(diffuse_map);
            image_release(specular_map);
            darray_push(models, model);
        }
    } else {
        printf("model not found: %s\n", model_name);
    }

    return models;
}

static void release_models(model_t **models) {
    int num_models = darray_size(models);
    int i;
    for (i = 0; i < num_models; i++) {
        model_t *model = models[i];
        mesh_t *mesh = model->mesh;
        mesh_release(mesh);
        phong_release_model(model);
    }
    darray_free(models);
}

static void update_uniforms(model_t **models, camera_t *camera) {
    mat4_t model_matrix = mat4_rotate_y((float)timer_get_time());
    mat4_t model_it_matrix = mat4_inverse_transpose(model_matrix);
    mat4_t view_matrix = camera_get_view_matrix(camera);
    mat4_t proj_matrix = camera_get_proj_matrix(camera);
    mat4_t viewproj_matrix = mat4_mul_mat4(proj_matrix, view_matrix);
    int num_models = darray_size(models);
    int i;
    for (i = 0; i < num_models; i++) {
        model_t *model = models[i];
        phong_uniforms_t *uniforms = phong_get_uniforms(model);
        uniforms->light_dir = LIGHT_DIRECTION;
        uniforms->camera_pos = camera_get_position(camera);
        uniforms->model_matrix = model_matrix;
        uniforms->model_it_matrix = model_it_matrix;
        uniforms->viewproj_matrix = viewproj_matrix;
    }
}

static void draw_models(framebuffer_t *framebuffer, model_t **models) {
    int num_models = darray_size(models);
    int i;
    for (i = 0; i < num_models; i++) {
        model_t *model = models[i];
        phong_draw_model(framebuffer, model);
    }
}

void test_phong(int argc, char *argv[]) {
    window_t *window;
    framebuffer_t *framebuffer;
    camera_t *camera;
    model_t **models;
    float aspect;
    float last_time;

    /* create models */
    if (argc > 2) {
        const char *model_name = argv[2];
        models = create_models(model_name);
    } else {
        models = create_models("african_head");
    }

    /* create window, framebuffer, and camera */
    window = window_create(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
    framebuffer = framebuffer_create(WINDOW_WIDTH, WINDOW_HEIGHT);
    aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
    camera = camera_create(CAMERA_POSITION, CAMERA_FORWARD, aspect);

    /* event loop */
    last_time = (float)timer_get_time();
    while (!window_should_close(window)) {
        /* calculate delta time */
        float curr_time = (float)timer_get_time();
        float delta_time = curr_time - last_time;
        last_time = curr_time;

        /* process events */
        camera_process_input(camera, window, delta_time);

        /* update uniforms */
        update_uniforms(models, camera);

        /* render image */
        framebuffer_clear(framebuffer, CLEAR_COLOR | CLEAR_DEPTH);
        draw_models(framebuffer, models);
        window_draw_buffer(window, framebuffer->colorbuffer);

        /* read events */
        input_poll_events();
    }

    /* release models */
    release_models(models);

    /* release camera, framebuffer, and window */
    camera_release(camera);
    framebuffer_release(framebuffer);
    window_destroy(window);
}
