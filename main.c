#include <stdio.h>
#include <stdlib.h>
#include "graphics.h"
#include "image.h"
#include "model.h"
#include "platform.h"

color_t white = {255, 255, 255};

int main(void) {
    window_t *window;
    image_t *image;
    model_t *model;
    const char *title = "Viewer";
    int width = 800;
    int height = 800;

    window = window_create(title, width, height);
    image = image_create(width, height, 1);
    model = model_load("resources/african_head.obj");

    graphics_draw_model(model, image, white);
    while (!window_should_close(window)) {
        window_draw_image(window, image);
        input_poll_events();
    }

    model_free(model);
    image_release(image);
    window_destroy(window);
    return 0;
}
