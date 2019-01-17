#include "test_blinn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../core/apis.h"
#include "../models/blinn_models.h"
#include "../shaders/blinn_shader.h"
#include "../tests/test_base.h"

static const char *MODEL_NAMES[] = {
    "craftsman",
};

static model_t **create_models(const char *model_name) {
    model_t **models = NULL;
    if (strcmp(model_name, "craftsman") == 0) {
        models = blinn_craftsman_models();
        printf("model: %s\n", model_name);
    } else {
        printf("model not found: %s\n", model_name);
    }
    return models;
}

static void tickfunc_function(camera_t *camera, vec3_t light_dir, void *userdata) {
    model_t **models = (model_t**)userdata;
    vec3_t camera_pos = camera_get_position(camera);
    mat4_t view_matrix = camera_get_view_matrix(camera);
    mat4_t proj_matrix = camera_get_proj_matrix(camera);
    mat4_t viewproj_matrix = mat4_mul_mat4(proj_matrix, view_matrix);
    mat4_t rotation_matrix = mat4_rotate_y(input_get_time());
    int num_models = darray_size(models);
    int i;
    for (i = 0; i < num_models; i++) {
        model_t *model = models[i];
        mat4_t model_matrix = mat4_mul_mat4(rotation_matrix, model->transform);
        mat4_t model_it_matrix = mat4_inverse_transpose(model_matrix);
        blinn_uniforms_t *uniforms = blinn_get_uniforms(model);
        uniforms->light_dir = light_dir;
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
        blinn_draw_model(framebuffer, model);
    }
}

static void tickfunc(context_t *context, void *userdata) {
    framebuffer_clear_color(context->framebuffer, vec4_new(0, 0, 0, 1));
    framebuffer_clear_depth(context->framebuffer, 1);
    tickfunc_function(context->camera, context->light_dir, userdata);
    draw_function(context->framebuffer, userdata);
}

void test_blinn(int argc, char *argv[]) {
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
        test_base(tickfunc, models);
        blinn_release_models(models);
    }
}
