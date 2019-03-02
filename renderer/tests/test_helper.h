#ifndef TEST_HELPER_H
#define TEST_HELPER_H

#include "../core/api.h"

typedef struct {
    framebuffer_t *framebuffer;
    camera_t *camera;
    vec3_t light_dir;
    float delta_time;
} context_t;

typedef void tickfunc_t(context_t *context, void *userdata);

void test_helper(tickfunc_t *tickfunc, void *userdata);

#endif
