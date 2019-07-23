#ifndef TEST_HELPER_H
#define TEST_HELPER_H

#include "../core/api.h"

typedef struct {
    framebuffer_t *framebuffer;
    camera_t *camera;
    vec3_t light_dir;
    float frame_time;
    float delta_time;
} context_t;

typedef struct {
    const char *scene_name;
    scene_t *(*create)(void);
} creator_t;

typedef void tickfunc_t(context_t *context, void *userdata);

void test_enter_mainloop(tickfunc_t *tickfunc, void *userdata);
scene_t *test_create_scene(creator_t creators[], const char *scene_name);
void test_draw_scene(scene_t *scene, context_t *context);

#endif
