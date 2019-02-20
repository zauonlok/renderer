#include <stddef.h>
#include "../core/api.h"
#include "../scenes/unlit_scenes.h"
#include "../shaders/unlit_shader.h"
#include "test_base.h"
#include "test_unlit.h"

static scene_entry_t g_scene_entries[] = {
    {"mccree", unlit_mccree_scene},
    {"elfgirl", unlit_elfgirl_scene},
};

static void update_scene(scene_t *scene, camera_t *camera) {
    mat4_t view_matrix = camera_get_view_matrix(camera);
    mat4_t proj_matrix = camera_get_proj_matrix(camera);
    mat4_t viewproj_matrix = mat4_mul_mat4(proj_matrix, view_matrix);
    int num_models = darray_size(scene->models);
    int i;
    for (i = 0; i < num_models; i++) {
        model_t *model = scene->models[i];
        mat4_t mvp_matrix = mat4_mul_mat4(viewproj_matrix, model->transform);
        unlit_uniforms_t *uniforms = unlit_get_uniforms(model);
        uniforms->mvp_matrix = mvp_matrix;
    }
    scene_sort_models(scene, view_matrix);
}

static void draw_scene(scene_t *scene, framebuffer_t *framebuffer) {
    int num_models = darray_size(scene->models);
    int i;
    framebuffer_clear_color(framebuffer, scene->background);
    framebuffer_clear_depth(framebuffer, 1);
    for (i = 0; i < num_models; i++) {
        model_t *model = scene->models[i];
        unlit_draw_model(model, framebuffer);
    }
}

static void tick_function(context_t *context, void *userdata) {
    scene_t *scene = (scene_t*)userdata;
    update_scene(scene, context->camera);
    draw_scene(scene, context->framebuffer);
}

void test_unlit(int argc, char *argv[]) {
    int num_entries = ARRAY_SIZE(g_scene_entries);
    const char *scene_name = argc > 2 ? argv[2] : NULL;
    scene_t *scene = scene_create(g_scene_entries, num_entries, scene_name);
    if (scene) {
        test_base(tick_function, scene);
        scene_release(scene, unlit_release_model);
    }
}
