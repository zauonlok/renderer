#include "geometry.h"
#include "graphics.h"
#include "image.h"
#include "platform.h"

color_t white = {255, 255, 255};
color_t blue = {255, 0, 0};
color_t green = {0, 255, 0};
color_t red = {0, 0, 255};

vec2i_t t0[3] = {{10, 70}, {50, 160}, {70, 80}};
vec2i_t t1[3] = {{180, 50}, {150, 1}, {70, 180}};
vec2i_t t2[3] = {{180, 150}, {120, 160}, {130, 180}};

int main(void) {
    window_t *window;
    image_t *image;
    const char *title = "Viewer";
    int width = 200;
    int height = 200;

    window = window_create(title, width, height);
    image = image_create(width, height, 3);

    gfx_fill_triangle(image, t0[0], t0[1], t0[2], blue);
    gfx_fill_triangle(image, t1[0], t1[1], t1[2], green);
    gfx_fill_triangle(image, t2[0], t2[1], t2[2], red);

    while (!window_should_close(window)) {
        window_draw_image(window, image);
        input_poll_events();
    }

    image_release(image);
    window_destroy(window);
    return 0;
}
