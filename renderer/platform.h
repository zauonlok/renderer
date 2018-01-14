#ifndef PLATFORM_H
#define PLATFORM_H

#include "image.h"

typedef struct window window_t;
typedef enum {KEY_A, KEY_D, KEY_S, KEY_W, KEY_NUM} keycode_t;
typedef enum {BUTTON_L, BUTTON_R, BUTTON_NUM} button_t;

/* window related functions */
window_t *window_create(const char *title, int width, int height);
void window_destroy(window_t *window);
int window_should_close(window_t *window);
void window_draw_image(window_t *window, image_t *image);

/* input related functions */
void input_poll_events(void);
int input_key_pressed(window_t *window, keycode_t key);
int input_button_pressed(window_t *window, button_t button);
void input_query_cursor(window_t *window, int *xpos, int *ypos);

/* time related functions */
double timer_get_time(void);
void timer_sleep_for(int milliseconds);

#endif
