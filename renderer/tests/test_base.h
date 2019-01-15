#ifndef TEST_BASE_H
#define TEST_BASE_H

#include "../core/apis.h"

#define ARRAY_LENGTH(array) (sizeof((array)) / sizeof((array)[0]))

typedef struct {
    window_t *window;
    framebuffer_t *framebuffer;
    camera_t *camera;
    vec3_t light_dir;
    float delta_time;
} context_t;

typedef void tick_t(context_t *context, void *userdata);

void test_base(tick_t *tick, void *userdata);

#endif
