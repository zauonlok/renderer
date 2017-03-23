#include "platform.h"
#include "image.h"

int main(void) {
    window_t *window;
    image_t *image;

    window = platform_create_window("Hello", 800, 600);
    image = image_load("example.tga", "tga");

    while (!platform_window_should_close(window)) {
        platform_draw_image(window, image);
        platform_poll_events();
    }

    image_free(image);
    platform_destroy_window(window);

    return 0;
}
