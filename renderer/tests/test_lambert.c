#include "test_lambert.h"
#include <stddef.h>
#include "../core/apis.h"
#include "../scenes/lambert_scenes.h"
#include "../shaders/lambert_shader.h"
#include "../tests/test_base.h"

static scene_entry_t g_scene_entries[] = {
    {"craftsman", lambert_craftsman_scene},
};

static void update_scene(scene_t *scene, camera_t *camera, vec3_t light_dir) {
    mat4_t view_matrix = camera_get_view_matrix(camera);
    mat4_t proj_matrix = camera_get_proj_matrix(camera);
    mat4_t viewproj_matrix = mat4_mul_mat4(proj_matrix, view_matrix);
    int num_models = darray_size(scene->models);
    int i;
    for (i = 0; i < num_models; i++) {
        model_t *model = scene->models[i];
        mat4_t model_matrix = model->transform;
        mat4_t mvp_matrix = mat4_mul_mat4(viewproj_matrix, model_matrix);
        mat4_t model_it_matrix = mat4_inverse_transpose(model_matrix);
        lambert_uniforms_t *uniforms = lambert_get_uniforms(model);
        uniforms->light_dir = light_dir;
        uniforms->mvp_matrix = mvp_matrix;
        uniforms->model_it_matrix = model_it_matrix;
    }
}

static void draw_scene(scene_t *scene, framebuffer_t *framebuffer) {
    int num_models = darray_size(scene->models);
    int i;
    framebuffer_clear_color(framebuffer, scene->background);
    framebuffer_clear_depth(framebuffer, 1);
    for (i = 0; i < num_models; i++) {
        model_t *model = scene->models[i];
        lambert_draw_model(model, framebuffer);
    }
}

static void tick_function(context_t *context, void *userdata) {
    scene_t *scene = (scene_t*)userdata;
    update_scene(scene, context->camera, context->light_dir);
    draw_scene(scene, context->framebuffer);
}

void test_lambert(int argc, char *argv[]) {
    int num_entries = ARRAY_LENGTH(g_scene_entries);
    const char *scene_name = (argc > 2) ? argv[2] : NULL;
    scene_t *scene = scene_create(g_scene_entries, num_entries, scene_name);
    if (scene) {
        test_base(tick_function, scene);
        scene_release(scene, lambert_release_model);
    }
}
