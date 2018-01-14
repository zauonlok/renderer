#ifndef IMAGE_H
#define IMAGE_H

typedef struct {int row, col;} point_t;
typedef struct {unsigned char b, g, r, a;} color_t;
typedef struct {
    int width, height, channels;
    unsigned char *buffer;
} image_t;

/* image creating/releasing */
image_t *image_create(int width, int height, int channels);
void image_release(image_t *image);
image_t *image_clone(image_t *image);

/* image input/output */
image_t *image_load(const char *filename);
void image_save(image_t *image, const char *filename);

/* color getting/setting */
color_t image_get_color(image_t *image, int row, int col);
void image_set_color(image_t *image, int row, int col, color_t color);

/* image processing */
void image_flip_h(image_t *image);
void image_flip_v(image_t *image);
void image_resize(image_t *image, int width, int height);

/* geometry drawing */
void image_draw_point(image_t *image, color_t color, point_t point);
void image_draw_line(image_t *image, color_t color,
                     point_t point0, point_t point1);
void image_draw_triangle(image_t *image, color_t color,
                         point_t point0, point_t point1, point_t point2);
void image_fill_triangle(image_t *image, color_t color,
                         point_t point0, point_t point1, point_t point2);

#endif
