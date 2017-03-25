#ifndef IMAGE_H
#define IMAGE_H

typedef struct {
    int width;
    int height;
    int channels;
    int pitch;
    unsigned char *buffer;
} image_t;

image_t *image_create(int width, int height, int channels);
void image_release(image_t *image);

image_t *image_load(const char *filename);
void image_save(image_t *image, const char *filename);

void image_flip_h(image_t *image);
void image_flip_v(image_t *image);
void image_resize(image_t *image, int width, int height);

#endif
