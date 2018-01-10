#include <assert.h>
#include <stdlib.h>
#include "drawing.h"

static point_t make_point(int row, int col) {
    point_t point;
    point.row = row;
    point.col = col;
    return point;
}

static void swap_points(point_t *a, point_t *b) {
    point_t t = *a;
    *a = *b;
    *b = t;
}

static void sort_points_by_row(point_t *p0, point_t *p1, point_t *p2) {
    if (p0->row > p1->row) {
        swap_points(p0, p1);
    }
    if (p0->row > p2->row) {
        swap_points(p0, p2);
    }
    if (p1->row > p2->row) {
        swap_points(p1, p2);
    }
}

static void sort_points_by_col(point_t *p0, point_t *p1, point_t *p2) {
    if (p0->col > p1->col) {
        swap_points(p0, p1);
    }
    if (p0->col > p2->col) {
        swap_points(p0, p2);
    }
    if (p1->col > p2->col) {
        swap_points(p1, p2);
    }
}

static int linear_interp_int(int v0, int v1, double d) {
    return (int)(v0 + (v1 - v0) * d + 0.5);
}

static point_t linear_interp_point(point_t p0, point_t p1, double d) {
    point_t point;
    point.row = linear_interp_int(p0.row, p1.row, d);
    point.col = linear_interp_int(p0.col, p1.col, d);
    return point;
}

static void draw_scanline(image_t *image, color_t color,
                          point_t p0, point_t p1) {
    point_t point;
    assert(p0.row == p1.row);
    if (p0.col > p1.col) {
        swap_points(&p0, &p1);
    }
    for (point = p0; point.col <= p1.col; point.col++) {
        image_draw_point(image, color, point);
    }
}

void image_draw_point(image_t *image, color_t color, point_t point) {
    assert(point.row >= 0 && point.row < image->height);
    assert(point.col >= 0 && point.col < image->width);
    image_set_color(image, point.row, point.col, color);
}

void image_draw_line(image_t *image, color_t color,
                     point_t point0, point_t point1) {
    int row_distance = abs(point1.row - point0.row);
    int col_distance = abs(point1.col - point0.col);
    if (row_distance == 0 && col_distance == 0) {
        image_draw_point(image, color, point0);
    } else if (row_distance > col_distance) {
        int row;
        if (point0.row > point1.row) {
            swap_points(&point0, &point1);
        }
        for (row = point0.row; row <= point1.row; row++) {
            double d = (row - point0.row) / (double)row_distance;
            int col = linear_interp_int(point0.col, point1.col, d);
            image_draw_point(image, color, make_point(row, col));
        }
    } else {
        int col;
        if (point0.col > point1.col) {
            swap_points(&point0, &point1);
        }
        for (col = point0.col; col <= point1.col; col++) {
            double d = (col - point0.col) / (double)col_distance;
            int row = linear_interp_int(point0.row, point1.row, d);
            image_draw_point(image, color, make_point(row, col));
        }
    }
}

void image_draw_triangle(image_t *image, color_t color,
                         point_t point0, point_t point1, point_t point2) {
    image_draw_line(image, color, point0, point1);
    image_draw_line(image, color, point1, point2);
    image_draw_line(image, color, point2, point0);
}

void image_fill_triangle(image_t *image, color_t color,
                         point_t point0, point_t point1, point_t point2) {
    sort_points_by_row(&point0, &point1, &point2);
    if (point0.row == point2.row) {
        sort_points_by_col(&point0, &point1, &point2);
        draw_scanline(image, color, point0, point2);
    } else {
        int total_height = point2.row - point0.row;
        int upper_height = point1.row - point0.row;
        int lower_height = point2.row - point1.row;

        /* fill the upper triangle */
        if (upper_height == 0) {
            draw_scanline(image, color, point0, point1);
        } else {
            int row;
            for (row = point0.row; row <= point1.row; row++) {
                double d1 = (row - point0.row) / (double)upper_height;
                double d2 = (row - point0.row) / (double)total_height;
                point_t p1 = linear_interp_point(point0, point1, d1);
                point_t p2 = linear_interp_point(point0, point2, d2);
                p1.row = p2.row = row;
                draw_scanline(image, color, p1, p2);
            }
        }

        /* fill the lower triangle */
        if (lower_height == 0) {
            draw_scanline(image, color, point1, point2);
        } else {
            int row;
            for (row = point1.row; row <= point2.row; row++) {
                double d0 = (row - point0.row) / (double)total_height;
                double d1 = (row - point1.row) / (double)lower_height;
                point_t p0 = linear_interp_point(point0, point2, d0);
                point_t p1 = linear_interp_point(point1, point2, d1);
                p0.row = p1.row = row;
                draw_scanline(image, color, p0, p1);
            }
        }
    }
}
