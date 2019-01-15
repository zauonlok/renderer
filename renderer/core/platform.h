#ifndef PLATFORM_H
#define PLATFORM_H

#include "geometry.h"
#include "graphics.h"
#include "image.h"

typedef struct window window_t;
typedef enum {KEY_A, KEY_D, KEY_S, KEY_W, KEY_SPACE, KEY_NUM} keycode_t;
typedef enum {BUTTON_L, BUTTON_R, BUTTON_NUM} button_t;
typedef struct {
    void (*key_callback)(window_t *window, keycode_t key, int pressed);
    void (*button_callback)(window_t *window, button_t button, int pressed);
    void (*scroll_callback)(window_t *window, double offset);
} callbacks_t;

/* window related functions */
window_t *window_create(const char *title, int width, int height);
void window_destroy(window_t *window);
int window_should_close(window_t *window);
void window_set_userdata(window_t *window, void *userdata);
void *window_get_userdata(window_t *window);
void window_draw_image(window_t *window, image_t *image);
void window_draw_buffer(window_t *window, colorbuffer_t *buffer);

/* input related functions */
void input_poll_events(void);
int input_key_pressed(window_t *window, keycode_t key);
int input_button_pressed(window_t *window, button_t button);
vec2_t input_query_cursor(window_t *window);
void input_set_callbacks(window_t *window, callbacks_t callbacks);
float input_get_time(void);

#endif
