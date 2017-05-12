#ifndef IMAGE_H
#define IMAGE_H

typedef struct {
    int width;
    int height;
    int channels;
    unsigned char *buffer;
} image_t;

typedef struct {
    unsigned char b, g, r, a;
} color_t;

image_t *image_create(int width, int height, int channels);
void image_release(image_t *image);
image_t *image_load(const char *filename);
void image_save(image_t *image, const char *filename);

color_t image_get_color(image_t *image, int row, int col);
void image_set_color(image_t *image, int row, int col, color_t color);
void image_blit_bgr(image_t *src, image_t *dst);
void image_blit_rgb(image_t *src, image_t *dst);
void image_flip_h(image_t *image);
void image_flip_v(image_t *image);
void image_resize(image_t *image, int width, int height);

#endif
