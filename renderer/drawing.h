#ifndef DRAWING_H
#define DRAWING_H

#include "image.h"

typedef struct {int row, col;} point_t;

void image_draw_point(image_t *image, color_t color, point_t point);
void image_draw_line(image_t *image, color_t color,
                     point_t point0, point_t point1);
void image_draw_triangle(image_t *image, color_t color,
                         point_t point0, point_t point1, point_t point2);
void image_fill_triangle(image_t *image, color_t color,
                         point_t point0, point_t point1, point_t point2);

#endif
