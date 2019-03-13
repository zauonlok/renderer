#include <stddef.h>
#include "../core/api.h"
#include "../scenes/unlit_scenes.h"
#include "../shaders/unlit_shader.h"
#include "test_helper.h"
#include "test_unlit.h"

static scene_creator_t g_scene_creators[] = {
    {"mccree", unlit_mccree_scene},
    {"elfgirl", unlit_elfgirl_scene},
    {"witch", unlit_witch_scene},
    {NULL, NULL},
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
        unlit_update_uniforms(model, mvp_matrix);
    }
    scene_sort_models(scene, view_matrix);
}

static void draw_scene(scene_t *scene, framebuffer_t *framebuffer) {
    framebuffer_clear_color(framebuffer, scene->background);
    framebuffer_clear_depth(framebuffer, 1);
    scene_draw_models(scene, framebuffer);
}

static void tick_function(context_t *context, void *userdata) {
    scene_t *scene = (scene_t*)userdata;
    update_scene(scene, context->camera);
    draw_scene(scene, context->framebuffer);
}

void test_unlit(int argc, char *argv[]) {
    const char *scene_name = argc > 2 ? argv[2] : NULL;
    scene_t *scene = scene_create(g_scene_creators, scene_name);
    if (scene) {
        test_helper(tick_function, scene);
        scene_release(scene);
    }
}
