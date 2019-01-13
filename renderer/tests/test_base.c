#include "test_base.h"
#include <string.h>
#include "../core/apis.h"

static const char *WINDOW_TITLE = "Viewer";
static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 600;

static const vec3_t CAMERA_POSITION = {0, 0, 3.5};
static const vec3_t CAMERA_TARGET = {0, 0, 0};

typedef struct {int orbiting, panning; vec2_t orbit_pos, pan_pos;} record_t;
typedef struct {record_t record; motion_t motion;} context_t;

static void button_callback(window_t *window, button_t button, int pressed) {
    context_t *context = (context_t*)window_get_userdata(window);
    record_t *record = &context->record;
    motion_t *motion = &context->motion;
    double xpos, ypos;
    input_query_cursor(window, &xpos, &ypos);
    if (button == BUTTON_L) {
        if (pressed) {
            record->orbiting = 1;
            record->orbit_pos.x = (float)xpos;
            record->orbit_pos.y = (float)ypos;
        } else {
            record->orbiting = 0;
            motion->orbit.x += (float)xpos - record->orbit_pos.x;
            motion->orbit.y += (float)ypos - record->orbit_pos.y;
        }
    } else if (button == BUTTON_R) {
        if (pressed) {
            record->panning = 1;
            record->pan_pos.x = (float)xpos;
            record->pan_pos.y = (float)ypos;
        } else {
            record->panning = 0;
            motion->pan.x += (float)xpos - record->pan_pos.x;
            motion->pan.y += (float)ypos - record->pan_pos.y;
        }
    }
}

static void scroll_callback(window_t *window, double offset) {
    context_t *context = (context_t*)window_get_userdata(window);
    context->motion.dolly += (float)offset;
}

static void update_camera(window_t *window, camera_t *camera,
                          context_t *context) {
    record_t *record = &context->record;
    motion_t *motion = &context->motion;
    double xpos, ypos;
    input_query_cursor(window, &xpos, &ypos);
    if (record->orbiting) {
        motion->orbit.x += (float)xpos - record->orbit_pos.x;
        motion->orbit.y += (float)ypos - record->orbit_pos.y;
        record->orbit_pos.x = (float)xpos;
        record->orbit_pos.y = (float)ypos;
    }
    if (record->panning) {
        motion->pan.x += (float)xpos - record->pan_pos.x;
        motion->pan.y += (float)ypos - record->pan_pos.y;
        record->pan_pos.x = (float)xpos;
        record->pan_pos.y = (float)ypos;
    }
    camera_orbit_update(camera, *motion);
    memset(motion, 0, sizeof(motion_t));
}

void test_base(tick_func_t tick_func, draw_func_t draw_func, void *userdata) {
    callbacks_t callbacks = {NULL, button_callback, scroll_callback};
    window_t *window;
    framebuffer_t *framebuffer;
    camera_t *camera;
    context_t context;
    float aspect;
    double last_time;

    window = window_create(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
    framebuffer = framebuffer_create(WINDOW_WIDTH, WINDOW_HEIGHT);
    aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
    camera = camera_create(CAMERA_POSITION, CAMERA_TARGET, aspect);

    memset(&context, 0, sizeof(context_t));
    window_set_userdata(window, &context);
    input_set_callbacks(window, callbacks);

    last_time = (float)input_get_time();
    while (!window_should_close(window)) {
        double curr_time = input_get_time();
        double delta_time = curr_time - last_time;
        last_time = curr_time;

        update_camera(window, camera, &context);
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
