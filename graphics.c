#include <stdlib.h>
#include "graphics.h"
#include "error.h"
#include "geometry.h"
#include "image.h"
#include "model.h"

static void swap_point(vec2i_t *point1, vec2i_t *point2) {
    vec2i_t t = *point1;
    *point1 = *point2;
    *point2 = t;
}

static vec2i_t make_point(int x, int y) {
    vec2i_t point;
    point.x = x;
    point.y = y;
    return point;
}

void graphics_draw_point(image_t *image, vec2i_t point, color_t color) {
    int row = point.y;
    int col = point.x;
    if (row < 0 || col < 0 || row >= image->height || col >= image->width) {
        DEBUG("graphics_draw_point: row/col");
    } else {
        image_set_color(image, row, col, color);
    }
}

void graphics_draw_line(image_t *image, vec2i_t point1, vec2i_t point2,
                        color_t color) {
    int x_diff = abs(point2.x - point1.x);
    int y_diff = abs(point2.y - point1.y);
    if (x_diff == 0 && y_diff == 0) {
        graphics_draw_point(image, point1, color);
    } else if (x_diff > y_diff) {
        int x;
        if (point1.x > point2.x) {
            swap_point(&point1, &point2);
        }
        for (x = point1.x; x <= point2.x; x++) {
            double d = (x - point1.x) / (double)x_diff;
            int y = (int)(point1.y + (point2.y - point1.y) * d);
            graphics_draw_point(image, make_point(x, y), color);
        }
    } else {
        int y;
        if (point1.y > point2.y) {
            swap_point(&point1, &point2);
        }
        for (y = point1.y; y <= point2.y; y++) {
            double d = (y - point1.y) / (double)y_diff;
            int x = (int)(point1.x + (point2.x - point1.x) * d);
            graphics_draw_point(image, make_point(x, y), color);
        }
    }
}

void graphics_draw_triangle(image_t *image, vec2i_t point1, vec2i_t point2,
                            vec2i_t point3, color_t color) {
    graphics_draw_line(image, point1, point2, color);
    graphics_draw_line(image, point2, point3, color);
    graphics_draw_line(image, point3, point1, color);
}

void graphics_draw_model(model_t *model, image_t *image, color_t color) {
    int num_faces = model_get_num_faces(model);
    int width = image->width;
    int height = image->height;
    int i, j;

    for (i = 0; i < num_faces; i++) {
        vec2i_t points[3];
        for (j = 0; j < 3; j++) {
            vec3f_t vertex = model_get_vertex(model, i, j);
            points[j].x = (int)((vertex.x + 1) / 2 * (width - 1));
            points[j].y = (int)((vertex.y + 1) / 2 * (height - 1));
            points[j].y = (height - 1) - points[j].y;
        }
        graphics_draw_triangle(image, points[0], points[1], points[2], color);
    }
}