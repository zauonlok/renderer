#include <stdlib.h>
#include "draw2d.h"
#include "graphics.h"
#include "maths.h"
#include "texture.h"

static void swap_integers(int *a, int *b) {
    int t = *a;
    *a = *b;
    *b = t;
}

static int lerp_integers(int a, int b, float t) {
    return (int)(a + (b - a) * t);
}

static void convert_color(vec4_t input, unsigned char output[4]) {
    output[0] = float_to_uchar(input.x);
    output[1] = float_to_uchar(input.y);
    output[2] = float_to_uchar(input.z);
    output[3] = float_to_uchar(input.w);
}

static void convert_point(framebuffer_t *framebuffer, vec2_t input,
                          int *row, int *col) {
    *row = (int)((framebuffer->height - 1) * float_saturate(input.y) + 0.5f);
    *col = (int)((framebuffer->width - 1) * float_saturate(input.x) + 0.5f);
}

static void draw_point(framebuffer_t *framebuffer, unsigned char color[4],
                       int row, int col) {
    int index = (row * framebuffer->width + col) * 4;
    int i;
    for (i = 0; i < 4; i++) {
        framebuffer->color_buffer[index + i] = color[i];
    }
}

/*
 * for Bresenham's line algorithm, see
 * https://github.com/ssloy/tinyrenderer/wiki/Lesson-1:-Bresenham%E2%80%99s-Line-Drawing-Algorithm
 */
static void draw_line(framebuffer_t *framebuffer, unsigned char color[4],
                      int row0, int col0, int row1, int col1) {
    int row_distance = abs(row0 - row1);
    int col_distance = abs(col0 - col1);
    if (row_distance == 0 && col_distance == 0) {
        draw_point(framebuffer, color, row0, col0);
    } else if (row_distance > col_distance) {
        int row;
        if (row0 > row1) {
            swap_integers(&row0, &row1);
            swap_integers(&col0, &col1);
        }
        for (row = row0; row <= row1; row++) {
            float t = (float)(row - row0) / (float)row_distance;
            int col = lerp_integers(col0, col1, t);
            draw_point(framebuffer, color, row, col);
        }
    } else {
        int col;
        if (col0 > col1) {
            swap_integers(&col0, &col1);
            swap_integers(&row0, &row1);
        }
        for (col = col0; col <= col1; col++) {
            float t = (float)(col - col0) / (float)col_distance;
            int row = lerp_integers(row0, row1, t);
            draw_point(framebuffer, color, row, col);
        }
    }
}

static void draw_texture(framebuffer_t *framebuffer, texture_t *texture,
                         int row, int col, int width, int height) {
    int src_r, src_c;
    for (src_r = 0; src_r < height; src_r++) {
        for (src_c = 0; src_c < width; src_c++) {
            int dst_r = row + src_r;
            int dst_c = col + src_c;
            int src_index = src_r * texture->width + src_c;
            int dst_index = (dst_r * framebuffer->width + dst_c) * 4;
            vec4_t *src_pixel = &texture->buffer[src_index];
            unsigned char *dst_pixel = &framebuffer->color_buffer[dst_index];
            dst_pixel[0] += float_to_uchar(src_pixel->x);
            dst_pixel[1] += float_to_uchar(src_pixel->y);
            dst_pixel[2] += float_to_uchar(src_pixel->z);
        }
    }
}

void draw2d_draw_point(framebuffer_t *framebuffer, vec4_t color_,
                       vec2_t point) {
    unsigned char color[4];
    int row, col;
    convert_color(color_, color);
    convert_point(framebuffer, point, &row, &col);
    draw_point(framebuffer, color, row, col);
}

void draw2d_draw_line(framebuffer_t *framebuffer, vec4_t color_,
                      vec2_t point0, vec2_t point1) {
    unsigned char color[4];
    int row0, col0, row1, col1;
    convert_color(color_, color);
    convert_point(framebuffer, point0, &row0, &col0);
    convert_point(framebuffer, point1, &row1, &col1);
    draw_line(framebuffer, color, row0, col0, row1, col1);
}

void draw2d_draw_triangle(framebuffer_t *framebuffer, vec4_t color_,
                          vec2_t point0, vec2_t point1, vec2_t point2) {
    unsigned char color[4];
    int row0, col0, row1, col1, row2, col2;
    convert_color(color_, color);
    convert_point(framebuffer, point0, &row0, &col0);
    convert_point(framebuffer, point1, &row1, &col1);
    convert_point(framebuffer, point2, &row2, &col2);
    draw_line(framebuffer, color, row0, col0, row1, col1);
    draw_line(framebuffer, color, row1, col1, row2, col2);
    draw_line(framebuffer, color, row2, col2, row0, col0);
}

void draw2d_draw_texture(framebuffer_t *framebuffer, texture_t *texture,
                         vec2_t origin) {
    int row, col, width, height;
    convert_point(framebuffer, origin, &row, &col);
    width = framebuffer->width - col;
    height = framebuffer->height - row;
    width = width < texture->width ? width : texture->width;
    height = height < texture->height ? height : texture->height;
    draw_texture(framebuffer, texture, row, col, width, height);
}
