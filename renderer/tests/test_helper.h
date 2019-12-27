#ifndef TEST_HELPER_H
#define TEST_HELPER_H

#include "../core/api.h"

typedef struct {
    framebuffer_t *framebuffer;
    camera_t *camera;
    vec3_t light_dir;
    vec2_t click_pos;
    int single_click;
    int double_click;
    float frame_time;
    float delta_time;
} context_t;

typedef struct {
    const char *scene_name;
    scene_t *(*create_scene)(void);
} creator_t;

typedef void tickfunc_t(context_t *context, void *userdata);

void test_enter_mainloop(tickfunc_t *tickfunc, void *userdata);
scene_t *test_create_scene(creator_t creators[], const char *scene_name);
perframe_t test_build_perframe(scene_t *scene, context_t *context);
void test_draw_scene(scene_t *scene, framebuffer_t *framebuffer,
                     perframe_t *perframe);

#endif
