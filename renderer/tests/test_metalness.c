#include <stddef.h>
#include "../core/api.h"
#include "../scenes/metalness_scenes.h"
#include "../shaders/metalness_shader.h"
#include "test_helper.h"
#include "test_metalness.h"

static creator_t g_creators[] = {
    {"helmet", metalness_helmet_scene},
    {"helmet2", metalness_helmet2_scene},
    {"ponycar", metalness_ponycar_scene},
    {NULL, NULL},
};

static void update_scene(scene_t *scene, camera_t *camera, vec3_t light_dir) {
    vec3_t camera_pos = camera_get_position(camera);
    mat4_t view_matrix = camera_get_view_matrix(camera);
    mat4_t proj_matrix = camera_get_proj_matrix(camera);
    mat4_t viewproj_matrix = mat4_mul_mat4(proj_matrix, view_matrix);
    int num_models = darray_size(scene->models);
    int i;
    light_dir = vec3_normalize(light_dir);
    for (i = 0; i < num_models; i++) {
        model_t *model = scene->models[i];
        mat4_t model_matrix = model->transform;
        mat4_t model_it_matrix = mat4_inverse_transpose(model_matrix);
        mat3_t normal_matrix = mat3_from_mat4(model_it_matrix);
        metalness_update_uniforms(model, light_dir, camera_pos,
                                  model_matrix, normal_matrix, viewproj_matrix);
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
        metalness_draw_model(model, framebuffer);
    }
}

static void tick_function(context_t *context, void *userdata) {
    scene_t *scene = (scene_t*)userdata;
    update_scene(scene, context->camera, context->light_dir);
    draw_scene(scene, context->framebuffer);
}

void test_metalness(int argc, char *argv[]) {
    const char *scene_name = argc > 2 ? argv[2] : NULL;
    scene_t *scene = scene_create(g_creators, scene_name);
    if (scene) {
        test_helper(tick_function, scene);
        scene_release(scene);
    }
}
