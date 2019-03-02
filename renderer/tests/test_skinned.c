#include <stddef.h>
#include "../core/api.h"
#include "../scenes/skinned_scenes.h"
#include "../shaders/skinned_shader.h"
#include "test_helper.h"
#include "test_skinned.h"

static creator_t g_creators[] = {
    {"assassin", skinned_assassin_scene},
    {NULL, NULL},
};

static void update_scene(scene_t *scene, camera_t *camera) {
    skin_t *skin = (skin_t*)scene->userdata;
    mat4_t view_matrix = camera_get_view_matrix(camera);
    mat4_t proj_matrix = camera_get_proj_matrix(camera);
    mat4_t viewproj_matrix = mat4_mul_mat4(proj_matrix, view_matrix);
    int num_models = darray_size(scene->models);
    int i;
    skin_update_joints(skin, input_get_time());
    for (i = 0; i < num_models; i++) {
        model_t *model = scene->models[i];
        mat4_t mvp_matrix = mat4_mul_mat4(viewproj_matrix, model->transform);
        skinned_update_uniforms(model, mvp_matrix, skin);
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
        skinned_draw_model(model, framebuffer);
    }
}

static void tick_function(context_t *context, void *userdata) {
    scene_t *scene = (scene_t*)userdata;
    update_scene(scene, context->camera);
    draw_scene(scene, context->framebuffer);
}

void test_skinned(int argc, char *argv[]) {
    const char *scene_name = argc > 2 ? argv[2] : NULL;
    scene_t *scene = scene_create(g_creators, scene_name);
    if (scene) {
        skin_t *skin = (skin_t*)scene->userdata;
        test_helper(tick_function, scene);
        skin_release(skin);
        scene_release(scene);
    }
}
