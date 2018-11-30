#ifndef TEST_BASE_H
#define TEST_BASE_H

#include "../core/apis.h"

#define ARRAY_LENGTH(array) (sizeof((array)) / sizeof((array)[0]))

typedef void tick_func_t(camera_t *camera, void *userdata);
typedef void draw_func_t(framebuffer_t *framebuffer, void *userdata);

void test_base(tick_func_t tick_func, draw_func_t draw_func, void *userdata);

#endif
