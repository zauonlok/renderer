#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "image.h"
#include "maths.h"
#include "private.h"

/* image creating/releasing */

image_t *image_create(int width, int height, int channels) {
    int buffer_size = width * height * channels;
    image_t *image;

    assert(width > 0 && height > 0 && channels >= 1 && channels <= 4);

    image = (image_t*)malloc(sizeof(image_t));
    image->width = width;
    image->height = height;
    image->channels = channels;
    image->buffer = (unsigned char*)malloc(buffer_size);
    memset(image->buffer, 0, buffer_size);
    return image;
}

void image_release(image_t *image) {
    free(image->buffer);
    free(image);
}

image_t *image_load(const char *filename) {
    const char *extension = private_get_extension(filename);
    if (strcmp(extension, "tga") == 0) {
        return private_load_tga_image(filename);
    } else {
        assert(0);
        return NULL;
    }
}

void image_save(image_t *image, const char *filename) {
    const char *extension = private_get_extension(filename);
    if (strcmp(extension, "tga") == 0) {
        private_save_tga_image(image, filename);
    } else {
        assert(0);
    }
}

/* image processing */

static void swap_bytes(unsigned char *a, unsigned char *b) {
    unsigned char t = *a;
    *a = *b;
    *b = t;
}

void image_flip_h(image_t *image) {
    int half_width = image->width / 2;
    int r, c, k;
    for (r = 0; r < image->height; r++) {
        for (c = 0; c < half_width; c++) {
            int flipped_c = image->width - 1 - c;
            unsigned char *pixel1 = private_get_pixel(image, r, c);
            unsigned char *pixel2 = private_get_pixel(image, r, flipped_c);
            for (k = 0; k < image->channels; k++) {
                swap_bytes(&pixel1[k], &pixel2[k]);
            }
        }
    }
}

void image_flip_v(image_t *image) {
    int half_height = image->height / 2;
    int r, c, k;
    for (r = 0; r < half_height; r++) {
        for (c = 0; c < image->width; c++) {
            int flipped_r = image->height - 1 - r;
            unsigned char *pixel1 = private_get_pixel(image, r, c);
            unsigned char *pixel2 = private_get_pixel(image, flipped_r, c);
            for (k = 0; k < image->channels; k++) {
                swap_bytes(&pixel1[k], &pixel2[k]);
            }
        }
    }
}

image_t *image_resize(image_t *source, int width, int height) {
    int channels = source->channels;
    image_t *target;
    float scale_r, scale_c;
    int dst_r, dst_c, k;

    assert(width > 0 && height > 0);
    target = image_create(width, height, channels);
    scale_r = (float)source->height / (float)height;
    scale_c = (float)source->width / (float)width;
    for (dst_r = 0; dst_r < height; dst_r++) {
        for (dst_c = 0; dst_c < width; dst_c++) {
            float mapped_r = (float)dst_r * scale_r;
            float mapped_c = (float)dst_c * scale_c;
            int src_r0 = (int)mapped_r;
            int src_c0 = (int)mapped_c;
            int src_r1 = int_min(src_r0 + 1, source->height - 1);
            int src_c1 = int_min(src_c0 + 1, source->width - 1);
            float delta_r = mapped_r - (float)src_r0;
            float delta_c = mapped_c - (float)src_c0;

            unsigned char *pixel_00 = private_get_pixel(source, src_r0, src_c0);
            unsigned char *pixel_01 = private_get_pixel(source, src_r0, src_c1);
            unsigned char *pixel_10 = private_get_pixel(source, src_r1, src_c0);
            unsigned char *pixel_11 = private_get_pixel(source, src_r1, src_c1);
            unsigned char *pixel = private_get_pixel(target, dst_r, dst_c);
            for (k = 0; k < channels; k++) {
                float v00 = pixel_00[k];  /* row 0, col 0 */
                float v01 = pixel_01[k];  /* row 0, col 1 */
                float v10 = pixel_10[k];  /* row 1, col 0 */
                float v11 = pixel_11[k];  /* row 1, col 1 */
                float v0 = float_lerp(v00, v01, delta_c);  /* row 0 */
                float v1 = float_lerp(v10, v11, delta_c);  /* row 1 */
                float value = float_lerp(v0, v1, delta_r);
                pixel[k] = (unsigned char)(value + 0.5f);
            }
        }
    }
    return target;
}
