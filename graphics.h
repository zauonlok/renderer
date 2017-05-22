#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "image.h"
#include "model.h"

void graphics_draw_point(image_t *image, vec2i_t point, color_t color);
void graphics_draw_line(image_t *image, vec2i_t point1, vec2i_t point2,
                        color_t color);
void graphics_draw_triangle(image_t *image, vec2i_t point1, vec2i_t point2,
                            vec2i_t point3, color_t color);
void graphics_draw_model(model_t *model, image_t *image, color_t color);

#endif
