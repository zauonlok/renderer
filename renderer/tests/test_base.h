#ifndef TEST_BASE_H
#define TEST_BASE_H

#include "../core/apis.h"

#define ARRAY_LENGTH(array) (sizeof((array)) / sizeof((array)[0]))

typedef struct {
    framebuffer_t *framebuffer;
    camera_t *camera;
    vec3_t light_dir;
    float delta_time;
} context_t;

typedef void tickfunc_t(context_t *context, void *userdata);

void test_base(tickfunc_t *tickfunc, void *userdata);

#endif
