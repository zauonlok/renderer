#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>
#include "image.h"

typedef struct window window_t;
typedef enum {KEY_A, KEY_D, KEY_S, KEY_W, KEY_NUM} keycode_t;
typedef enum {BUTTON_L, BUTTON_R, BUTTON_NUM} button_t;

window_t *platform_create_window(const char *title, int width, int height);
bool platform_window_should_close(window_t *window);
void platform_draw_image(window_t *window, image_t *image);
void platform_destroy_window(window_t *window);

void platform_poll_events(void);
bool platform_is_key_down(window_t *window, keycode_t key);
bool platform_is_button_down(window_t *window, button_t button);
void platform_get_cursor_pos(window_t *window, double *xpos, double *ypos);
double platform_get_time(void);

#endif
