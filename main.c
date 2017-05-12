#include <stdio.h>
#include <stdlib.h>
#include "geometry.h"
#include "image.h"
#include "model.h"
#include "platform.h"

void swap_int(int *a, int *b) {
    int t = *a;
    *a = *b;
    *b = t;
}

void draw_point(image_t *image, int row, int col, color_t color) {
    if (row < 0 || col < 0 || row >= image->height || col >= image->width) {
        return;
    } else {
        image_set_color(image, row, col, color);
    }
}

color_t white = {255, 255, 255};

void draw_line(image_t *image, int x0, int y0, int x1, int y1) {
    if (abs(x1 - x0) > abs(y1 - y0)) {
        int x;
        if (x0 > x1) {
            swap_int(&x0, &x1);
            swap_int(&y0, &y1);
        }
        for (x = x0; x <= x1; x++) {
            double t = (x - x0) / (double)(x1 - x0);
            int y =  (int)(y0 + (y1 - y0) * t);
            draw_point(image, y, x, white);
        }
    } else {
        int y;
        if (y0 > y1) {
            swap_int(&y0, &y1);
            swap_int(&x0, &x1);
        }
        for (y = y0; y <= y1; y++) {
            double t = (y - y0) / (double)(y1 - y0);
            int x = (int)(x0 + (x1 - x0) * t);
            draw_point(image, y, x, white);
        }
    }
}

void draw_model(model_t *model, image_t *image) {
    int num_faces = model_get_num_faces(model);
    int width = image->width;
    int height = image->height;
    int i, j;

    for (i = 0; i < num_faces; i++) {
        for (j = 0; j < 3; j++) {
            vec3f_t v0 = model_get_vertex(model, i, j);
            vec3f_t v1 = model_get_vertex(model, i, (j + 1) % 3);

            int x0 = (int)((v0.x + 1) / 2 * width);
            int y0 = (int)((v0.y + 1) / 2 * height);
            int x1 = (int)((v1.x + 1) / 2 * width);
            int y1 = (int)((v1.y + 1) / 2 * height);

            draw_line(image, x0, y0, x1, y1);
        }
    }
    image_flip_v(image);
}

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

    draw_model(model, image);
    while (!window_should_close(window)) {
        window_draw_image(window, image);
        input_poll_events();
    }

    model_free(model);
    image_release(image);
    window_destroy(window);
    return 0;
}
