#include <math.h>
#include <stdlib.h>
#include "geometry.h"
#include "graphics.h"
#include "image.h"
#include "model.h"
#include "platform.h"

vec3f_t vec3f_cross(vec3f_t a, vec3f_t b) {
    vec3f_t result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

float vec3f_dot(vec3f_t a, vec3f_t b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

vec3f_t vec3f_normalize(vec3f_t v) {
    float length = (float)sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
    return vec3f_new(v.x / length, v.y / length, v.z / length);
}

vec3f_t vec3f_sub(vec3f_t a, vec3f_t b) {
    return vec3f_new(a.x - b.x, a.y - b.y, a.z - b.z);
}

void draw_model(model_t *model, image_t *image) {
    int num_faces = model_get_num_faces(model);
    int width = image->width;
    int height = image->height;
    int i, j;

    vec3f_t light = {0, 0, -1};

    for (i = 0; i < num_faces; i++) {
        vec2i_t points[3];
        vec3f_t coords[3];
        vec3f_t normal;
        float intensity;
        for (j = 0; j < 3; j++) {
            vec3f_t vertex = model_get_vertex(model, i, j);
            points[j].x = (int)((vertex.x + 1) / 2 * (width - 1));
            points[j].y = (int)((vertex.y + 1) / 2 * (height - 1));
            points[j].y = (height - 1) - points[j].y;

            coords[j] = vertex;
        }
        normal = vec3f_cross(
            vec3f_sub(coords[2], coords[0]),
            vec3f_sub(coords[1], coords[0])
        );
        normal = vec3f_normalize(normal);

        intensity = vec3f_dot(normal, light);
        if (intensity > 0) {
            color_t color;
            color.b = (unsigned char)(intensity * 255);
            color.g = (unsigned char)(intensity * 255);
            color.r = (unsigned char)(intensity * 255);
            gfx_fill_triangle(image, points[0], points[1], points[2], color);
        }
    }
}

int main(void) {
    window_t *window;
    image_t *image;
    model_t *model;
    const char *title = "Viewer";
    int width = 800;
    int height = 800;

    window = window_create(title, width, height);
    image = image_create(width, height, 3);

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
