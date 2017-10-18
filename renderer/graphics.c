#include <assert.h>
#include <stdlib.h>
#include "graphics.h"
#include "geometry.h"
#include "image.h"

static void swap_point(vec2i_t *point0, vec2i_t *point1) {
    vec2i_t t = *point0;
    *point0 = *point1;
    *point1 = t;
}

static int linear_interp(int v0, int v1, double d) {
    return (int)(v0 + (v1 - v0) * d + 0.5);
}

static vec2i_t lerp_point(vec2i_t point0, vec2i_t point1, double d) {
    vec2i_t point;
    point.x = linear_interp(point0.x, point1.x, d);
    point.y = linear_interp(point0.y, point1.y, d);
    return point;
}

static void sort_point_y(vec2i_t *point0, vec2i_t *point1, vec2i_t *point2) {
    if (point0->y > point1->y) {
        swap_point(point0, point1);
    }
    if (point0->y > point2->y) {
        swap_point(point0, point2);
    }
    if (point1->y > point2->y) {
        swap_point(point1, point2);
    }
}

static void sort_point_x(vec2i_t *point0, vec2i_t *point1, vec2i_t *point2) {
    if (point0->x > point1->x) {
        swap_point(point0, point1);
    }
    if (point0->x > point2->x) {
        swap_point(point0, point2);
    }
    if (point1->x > point2->x) {
        swap_point(point1, point2);
    }
}

static void draw_scanline(image_t *image, vec2i_t point0, vec2i_t point1,
                          color_t color) {
    vec2i_t point;
    assert(point0.y == point1.y);
    if (point0.x > point1.x) {
        swap_point(&point0, &point1);
    }
    for (point = point0; point.x <= point1.x; point.x += 1) {
        gfx_draw_point(image, point, color);
    }
}

void gfx_draw_point(image_t *image, vec2i_t point, color_t color) {
    int row = point.y;
    int col = point.x;
    if (row < 0 || col < 0 || row >= image->height || col >= image->width) {
        assert(0);
    } else {
        image_set_color(image, row, col, color);
    }
}

void gfx_draw_line(image_t *image, vec2i_t point0, vec2i_t point1,
                   color_t color) {
    int x_distance = abs(point1.x - point0.x);
    int y_distance = abs(point1.y - point0.y);
    if (x_distance == 0 && y_distance == 0) {
        gfx_draw_point(image, point0, color);
    } else if (x_distance > y_distance) {
        int x;
        if (point0.x > point1.x) {
            swap_point(&point0, &point1);
        }
        for (x = point0.x; x <= point1.x; x++) {
            double d = (x - point0.x) / (double)x_distance;
            int y = linear_interp(point0.y, point1.y, d);
            gfx_draw_point(image, vec2i_new(x, y), color);
        }
    } else {
        int y;
        if (point0.y > point1.y) {
            swap_point(&point0, &point1);
        }
        for (y = point0.y; y <= point1.y; y++) {
            double d = (y - point0.y) / (double)y_distance;
            int x = linear_interp(point0.x, point1.x, d);
            gfx_draw_point(image, vec2i_new(x, y), color);
        }
    }
}

void gfx_draw_triangle(image_t *image, vec2i_t point0, vec2i_t point1,
                       vec2i_t point2, color_t color) {
    gfx_draw_line(image, point0, point1, color);
    gfx_draw_line(image, point1, point2, color);
    gfx_draw_line(image, point2, point0, color);
}

void gfx_fill_triangle_2(image_t *image, vec2i_t point0, vec2i_t point1,
                       vec2i_t point2, color_t color) {
    sort_point_y(&point0, &point1, &point2);
    if (point0.y == point2.y) {
        sort_point_x(&point0, &point1, &point2);
        draw_scanline(image, point0, point2, color);
    } else {
        int total_height = point2.y - point0.y;
        int upper_height = point1.y - point0.y;
        int lower_height = point2.y - point1.y;

        if (upper_height == 0) {
            draw_scanline(image, point0, point1, color);
        } else {
            int y;
            for (y = point0.y; y <= point1.y; y++) {
                double d1 = (y - point0.y) / (double)upper_height;
                double d2 = (y - point0.y) / (double)total_height;
                vec2i_t p1 = lerp_point(point0, point1, d1);
                vec2i_t p2 = lerp_point(point0, point2, d2);
                p1.y = p2.y = y;
                draw_scanline(image, p1, p2, color);
            }
        }

        if (lower_height == 0) {
            draw_scanline(image, point1, point2, color);
        } else {
            int y;
            for (y = point1.y; y <= point2.y; y++) {
                double d0 = (y - point0.y) / (double)total_height;
                double d1 = (y - point1.y) / (double)lower_height;
                vec2i_t p0 = lerp_point(point0, point2, d0);
                vec2i_t p1 = lerp_point(point1, point2, d1);
                p0.y = p1.y = y;
                draw_scanline(image, p0, p1, color);
            }
        }
    }
}

/*
 * using barycentric coordinates, see http://blackpawn.com/texts/pointinpoly/
 * solve P = A + sAB + tAC
 *   --> AP = sAB + tAC
 *   --> s = (AC.y * AP.x - AC.x * AP.y) / (AB.x * AC.y - AB.y * AC.x)
 *   --> t = (AB.x * AP.y - AB.y * AP.x) / (AB.x * AC.y - AB.y * AC.x)
 * check if the point is in triangle: (s >= 0) && (t >= 0) && (s + t <= 1)
 */
#include <stdio.h>

static int in_triangle(vec2i_t A, vec2i_t B, vec2i_t C, vec2i_t P, double *sp, double *tp) {
    vec2i_t AB = vec2i_sub(B, A);
    vec2i_t AC = vec2i_sub(C, A);
    vec2i_t AP = vec2i_sub(P, A);
    double s, t;

    int denom = AB.x * AC.y - AB.y * AC.x;
    if (denom == 0) {
        /*printf("in_triangle: A=(%d,%d) B=(%d,%d) C=(%d,%d)\n",
               A.x, A.y, B.x, B.y, C.x, C.y);*/
    }
    /* assert(denom != 0); */

    s = (AC.y * AP.x - AC.x * AP.y) / (double)denom;
    t = (AB.x * AP.y - AB.y * AP.x) / (double)denom;

    *sp = s;
    *tp = t;

    return (s >=0 && t >= 0 && s + t <= 1);
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

void gfx_fill_triangle(image_t *image, vec3i_t point0, vec3i_t point1,
                       vec3i_t point2, color_t color, float *zbuffer) {
    int min_x = image->width - 1, min_y = image->height - 1;
    int max_x = 0, max_y = 0;
    int i, j;
    int width = image->width;

    min_x = MIN(point0.x, min_x);
    min_y = MIN(point0.y, min_y);
    max_x = MAX(point0.x, max_x);
    max_y = MAX(point0.y, max_y);

    min_x = MIN(point1.x, min_x);
    min_y = MIN(point1.y, min_y);
    max_x = MAX(point1.x, max_x);
    max_y = MAX(point1.y, max_y);

    min_x = MIN(point2.x, min_x);
    min_y = MIN(point2.y, min_y);
    max_x = MAX(point2.x, max_x);
    max_y = MAX(point2.y, max_y);

    min_x = MAX(0, min_x);
    min_y = MAX(0, min_y);
    max_x = MIN(image->width - 1, max_x);
    max_y = MIN(image->height - 1, max_y);



    for (i = min_x; i <= max_x; i++) {
        for (j = min_y; j <= max_y; j++) {
            vec2i_t point = vec2i_new(i, j);
            double s, t;
            vec2i_t point02 = vec2i_new(point0.x, point0.y);
            vec2i_t point12 = vec2i_new(point1.x, point1.y);
            vec2i_t point22 = vec2i_new(point2.x, point2.y);
            if (in_triangle(point02, point12, point22, point, &s, &t)) {
                float z = point0.z + s * point1.z + t * point2.z;
                /*if (zbuffer[j * width + i] < z) {*/
                    gfx_draw_point(image, point, color);
                /*    zbuffer[j * width + i] = z;
                }*/

            }
        }
    }
}
