#include "test_base.h"
#include "../core/apis.h"

static const char *WINDOW_TITLE = "Viewer";
static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 600;

static const vec3_t CAMERA_POSITION = {0, 0, 3.5};
static const vec3_t CAMERA_FORWARD = {0, 0, -1};

void test_base(tick_func_t tick_func, draw_func_t draw_func, void *userdata) {
    window_t *window;
    framebuffer_t *framebuffer;
    camera_t *camera;
    float aspect;
    float last_time;

    window = window_create(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
    framebuffer = framebuffer_create(WINDOW_WIDTH, WINDOW_HEIGHT);
    aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
    camera = camera_create(CAMERA_POSITION, CAMERA_FORWARD, aspect);

    last_time = (float)timer_get_time();
    while (!window_should_close(window)) {
        float curr_time = (float)timer_get_time();
        float delta_time = curr_time - last_time;
        last_time = curr_time;

        camera_process_input(camera, window, delta_time);
        tick_func(camera, userdata);

        framebuffer_clear(framebuffer, CLEAR_COLOR | CLEAR_DEPTH);
        draw_func(framebuffer, userdata);
        window_draw_buffer(window, framebuffer->colorbuffer);

        input_poll_events();
    }

    window_destroy(window);
    framebuffer_release(framebuffer);
    camera_release(camera);
}
