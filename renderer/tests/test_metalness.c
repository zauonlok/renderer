#include <stddef.h>
#include "../core/api.h"
#include "../scenes/metalness_scenes.h"
#include "../shaders/metalness_shader.h"
#include "test_base.h"
#include "test_metalness.h"

static scene_entry_t g_scene_entries[] = {
    {"helmet", metalness_helmet_scene},
    {"ponycar", metalness_ponycar_scene},
};

static void update_scene(scene_t *scene, camera_t *camera, vec3_t light_dir) {
    vec3_t camera_pos = camera_get_position(camera);
    mat4_t view_matrix = camera_get_view_matrix(camera);
    mat4_t proj_matrix = camera_get_proj_matrix(camera);
    mat4_t viewproj_matrix = mat4_mul_mat4(proj_matrix, view_matrix);
    int num_models = darray_size(scene->models);
    int i;
    for (i = 0; i < num_models; i++) {
        model_t *model = scene->models[i];
        mat4_t model_matrix = model->transform;
        mat4_t model_it_matrix = mat4_inverse_transpose(model_matrix);
        metalness_uniforms_t *uniforms = metalness_get_uniforms(model);
        uniforms->light_dir = vec3_normalize(light_dir);
        uniforms->camera_pos = camera_pos;
        uniforms->model_matrix = model_matrix;
        uniforms->normal_matrix = mat3_from_mat4(model_it_matrix);
        uniforms->viewproj_matrix = viewproj_matrix;
    }
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
    int num_entries = ARRAY_SIZE(g_scene_entries);
    const char *scene_name = argc > 2 ? argv[2] : NULL;
    scene_t *scene = scene_create(g_scene_entries, num_entries, scene_name);
    if (scene) {
        test_base(tick_function, scene);
        scene_release(scene, metalness_release_model);
    }
}
