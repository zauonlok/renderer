#include "test_phong.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../core/apis.h"
#include "../models/phong_models.h"
#include "../shaders/phong_shader.h"
#include "../tests/test_base.h"

static const vec3_t LIGHT_DIRECTION = {-1, -1, -1};

static const char *MODEL_NAMES[] = {
    "demon_hunter",
};

static model_t **create_models(const char *model_name) {
    model_t **models = NULL;
    if (strcmp(model_name, "demon_hunter") == 0) {
        models = phong_demon_hunter_models();
        printf("using model: %s\n", model_name);
    } else {
        printf("model not found: %s\n", model_name);
    }
    return models;
}

static void tick_function(camera_t *camera, void *userdata) {
    model_t **models = (model_t**)userdata;
    vec3_t camera_pos = camera_get_position(camera);
    mat4_t view_matrix = camera_get_view_matrix(camera);
    mat4_t proj_matrix = camera_get_proj_matrix(camera);
    mat4_t viewproj_matrix = mat4_mul_mat4(proj_matrix, view_matrix);
    mat4_t rotation_matrix = mat4_rotate_y((float)timer_get_time());
    int num_models = darray_size(models);
    int i;
    for (i = 0; i < num_models; i++) {
        model_t *model = models[i];
        mat4_t model_matrix = mat4_mul_mat4(rotation_matrix, model->transform);
        mat4_t model_it_matrix = mat4_inverse_transpose(model_matrix);
        phong_uniforms_t *uniforms = phong_get_uniforms(model);
        uniforms->light_dir = LIGHT_DIRECTION;
        uniforms->camera_pos = camera_pos;
        uniforms->model_matrix = model_matrix;
        uniforms->model_it_matrix = model_it_matrix;
        uniforms->viewproj_matrix = viewproj_matrix;
    }
}

static void draw_function(framebuffer_t *framebuffer, void *userdata) {
    model_t **models = (model_t**)userdata;
    int num_models = darray_size(models);
    int i;
    for (i = 0; i < num_models; i++) {
        model_t *model = models[i];
        phong_draw_model(framebuffer, model);
    }
}

void test_phong(int argc, char *argv[]) {
    model_t **models;
    if (argc > 2) {
        const char *model_name = argv[2];
        models = create_models(model_name);
    } else {
        int index = rand() % ARRAY_LENGTH(MODEL_NAMES);
        const char *model_name = MODEL_NAMES[index];
        models = create_models(model_name);
    }
    if (models) {
        test_base(tick_function, draw_function, models);
        phong_release_models(models);
    }
}
