#include <assert.h>
#include <stdlib.h>
#include "draw2d.h"
#include "geometry.h"
#include "graphics.h"
#include "texture.h"

static void draw_point(framebuffer_t *framebuffer, vec4_t color,
                       int row, int col) {
    int index = row * framebuffer->width + col;
    framebuffer->colorbuffer[index] = color;
}

/*
 * for Bresenham's line algorithm, see
 * https://github.com/ssloy/tinyrenderer/wiki/Lesson-1:-Bresenham%E2%80%99s-Line-Drawing-Algorithm
 */
static void draw_line(framebuffer_t *framebuffer, vec4_t color,
                      int row0, int row1, int col0, int col1) {
    int row_distance = abs(row0 - row1);
    int col_distance = abs(col0 - col1);
    if (row_distance == 0 && col_distance == 0) {
        draw_point(framebuffer, color, row0, col0);
    } else if (row_distance > col_distance) {
        int row;
        if (row0 > row1) {
            int_swap(&row0, &row1);
            int_swap(&col0, &col1);
        }
        for (row = row0; row <= row1; row++) {
            float t = (float)(row - row0) / (float)row_distance;
            int col = int_lerp(col0, col1, t);
            draw_point(framebuffer, color, row, col);
        }
    } else {
        int col;
        if (col0 > col1) {
            int_swap(&col0, &col1);
            int_swap(&row0, &row1);
        }
        for (col = col0; col <= col1; col++) {
            float t = (float)(col - col0) / (float)col_distance;
            int row = int_lerp(row0, row1, t);
            draw_point(framebuffer, color, row, col);
        }
    }
}

void draw2d_draw_point(framebuffer_t *framebuffer, vec4_t color,
                       int row, int col) {
    assert(row >= 0 && row < framebuffer->height);
    assert(col >= 0 && col < framebuffer->width);
    draw_point(framebuffer, color, row, col);
}

void draw2d_draw_line(framebuffer_t *framebuffer, vec4_t color,
                      int row0, int row1, int col0, int col1) {
    assert(row0 >= 0 && row0 < framebuffer->height);
    assert(row1 >= 0 && row1 < framebuffer->height);
    assert(col0 >= 0 && col0 < framebuffer->width);
    assert(col1 >= 0 && col1 < framebuffer->width);
    draw_line(framebuffer, color, row0, row1, col0, col1);
}

void draw2d_draw_triangle(framebuffer_t *framebuffer, vec4_t color,
                          int row0, int row1, int row2,
                          int col0, int col1, int col2) {
    assert(row0 >= 0 && row0 < framebuffer->height);
    assert(row1 >= 0 && row1 < framebuffer->height);
    assert(row2 >= 0 && row2 < framebuffer->height);
    assert(col0 >= 0 && col0 < framebuffer->width);
    assert(col1 >= 0 && col1 < framebuffer->width);
    assert(col2 >= 0 && col2 < framebuffer->width);
    draw_line(framebuffer, color, row0, row1, col0, col1);
    draw_line(framebuffer, color, row1, row2, col1, col2);
    draw_line(framebuffer, color, row2, row0, col2, col0);
}

void draw2d_draw_texture(framebuffer_t *framebuffer, texture_t *texture,
                         int row, int col) {
    int src_r, src_c;
    assert(row >= 0 && row + texture->height <= framebuffer->height);
    assert(col >= 0 && col + texture->width <= framebuffer->width);
    for (src_r = 0; src_r < texture->height; src_r++) {
        for (src_c = 0; src_c < texture->width; src_c++) {
            int dst_r = row + src_r;
            int dst_c = col + src_c;
            int src_index = src_r * texture->width + src_c;
            int dst_index = dst_r * framebuffer->width + dst_c;
            vec4_t src_color = texture->buffer[src_index];
            vec4_t dst_color = framebuffer->colorbuffer[dst_index];
            vec4_t fin_color = vec4_saturate(vec4_add(src_color, dst_color));
            framebuffer->colorbuffer[dst_index] = fin_color;
        }
    }
}
