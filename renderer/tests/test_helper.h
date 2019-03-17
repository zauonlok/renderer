#ifndef TEST_HELPER_H
#define TEST_HELPER_H

#include "../core/api.h"

/* delegate related functions */

typedef struct {
    framebuffer_t *framebuffer;
    camera_t *camera;
    vec3_t light_dir;
    float delta_time;
} context_t;

typedef void tickfunc_t(context_t *context, void *userdata);

void test_helper(tickfunc_t *tickfunc, void *userdata);

/* scene related functions */

typedef struct {
    const char *scene_name;
    scene_t *(*create)(void);
} scene_creator_t;

scene_t *scene_create(scene_creator_t creators[], const char *scene_name);
void scene_release(scene_t *scene);
void scene_sort_models(scene_t *scene, mat4_t view_matrix);
void scene_draw_models(scene_t *scene, framebuffer_t *framebuffer);

#endif
