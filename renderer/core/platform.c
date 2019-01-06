#include "platform.h"
#include <assert.h>
#include "geometry.h"
#include "graphics.h"
#include "image.h"

static int min_integer(int a, int b) {
    return a < b ? a : b;
}

static unsigned char *get_pixel_ptr(image_t *image, int row, int col) {
    int index = row * image->width * image->channels + col * image->channels;
    return &(image->buffer[index]);
}

static vec4_t get_buffer_val(colorbuffer_t *buffer, int row, int col) {
    int index = row * buffer->width + col;
    return buffer->buffer[index];
}

void private_blit_bgr_image(image_t *src, image_t *dst) {
    int width = min_integer(src->width, dst->width);
    int height = min_integer(src->height, dst->height);
    int r, c;

    assert(width > 0 && height > 0);
    assert(src->channels >= 1 && src->channels <= 4);
    assert(dst->channels == 3 || dst->channels == 4);

    for (r = 0; r < height; r++) {
        for (c = 0; c < width; c++) {
            int src_r = src->height - 1 - r;  /* flip */
            unsigned char *src_pixel = get_pixel_ptr(src, src_r, c);
            unsigned char *dst_pixel = get_pixel_ptr(dst, r, c);
            if (src->channels == 3 || src->channels == 4) {
                dst_pixel[0] = src_pixel[0];
                dst_pixel[1] = src_pixel[1];
                dst_pixel[2] = src_pixel[2];
            } else {
                unsigned char gray = src_pixel[0];
                dst_pixel[0] = dst_pixel[1] = dst_pixel[2] = gray;
            }
        }
    }
}

void private_blit_rgb_image(image_t *src, image_t *dst) {
    int width = min_integer(src->width, dst->width);
    int height = min_integer(src->height, dst->height);
    int r, c;

    assert(width > 0 && height > 0);
    assert(src->channels >= 1 && src->channels <= 4);
    assert(dst->channels == 3 || dst->channels == 4);

    for (r = 0; r < height; r++) {
        for (c = 0; c < width; c++) {
            int src_r = src->height - 1 - r;  /* flip */
            unsigned char *src_pixel = get_pixel_ptr(src, src_r, c);
            unsigned char *dst_pixel = get_pixel_ptr(dst, r, c);
            if (src->channels == 3 || src->channels == 4) {
                dst_pixel[0] = src_pixel[2];
                dst_pixel[1] = src_pixel[1];
                dst_pixel[2] = src_pixel[0];
            } else {
                unsigned char gray = src_pixel[0];
                dst_pixel[0] = dst_pixel[1] = dst_pixel[2] = gray;
            }
        }
    }
}

void private_blit_bgr_buffer(colorbuffer_t *src, image_t *dst) {
    int width = min_integer(src->width, dst->width);
    int height = min_integer(src->height, dst->height);
    int r, c;

    assert(width > 0 && height > 0);
    assert(dst->channels == 3 || dst->channels == 4);

    for (r = 0; r < height; r++) {
        for (c = 0; c < width; c++) {
            int src_r = src->height - 1 - r;  /* flip */
            vec4_t src_value = get_buffer_val(src, src_r, c);
            unsigned char *dst_pixel = get_pixel_ptr(dst, r, c);
            dst_pixel[2] = (unsigned char)(src_value.x * 255);  /* red */
            dst_pixel[1] = (unsigned char)(src_value.y * 255);  /* green */
            dst_pixel[0] = (unsigned char)(src_value.z * 255);  /* blue */
        }
    }
}

void private_blit_rgb_buffer(colorbuffer_t *src, image_t *dst) {
    int width = min_integer(src->width, dst->width);
    int height = min_integer(src->height, dst->height);
    int r, c;

    assert(width > 0 && height > 0);
    assert(dst->channels == 3 || dst->channels == 4);

    for (r = 0; r < height; r++) {
        for (c = 0; c < width; c++) {
            int src_r = src->height - 1 - r;  /* flip */
            vec4_t src_value = get_buffer_val(src, src_r, c);
            unsigned char *dst_pixel = get_pixel_ptr(dst, r, c);
            dst_pixel[0] = (unsigned char)(src_value.x * 255);  /* red */
            dst_pixel[1] = (unsigned char)(src_value.y * 255);  /* green */
            dst_pixel[2] = (unsigned char)(src_value.z * 255);  /* blue */
        }
    }
}
