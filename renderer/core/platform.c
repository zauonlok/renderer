#include <assert.h>
#include "geometry.h"
#include "graphics.h"
#include "image.h"

static unsigned char *get_pixel_ptr(image_t *image, int row, int col) {
    int index = row * image->width * image->channels + col * image->channels;
    return &(image->buffer[index]);
}

void private_blit_image_bgr(image_t *src, image_t *dst) {
    int width = int_min(src->width, dst->width);
    int height = int_min(src->height, dst->height);
    int r, c;

    assert(width > 0 && height > 0);
    assert(src->channels >= 1 && src->channels <= 4);
    assert(dst->channels == 3 || dst->channels == 4);

    for (r = 0; r < height; r++) {
        for (c = 0; c < width; c++) {
            int flipped_r = src->height - 1 - r;
            unsigned char *src_pixel = get_pixel_ptr(src, flipped_r, c);
            unsigned char *dst_pixel = get_pixel_ptr(dst, r, c);
            if (src->channels == 3 || src->channels == 4) {
                dst_pixel[0] = src_pixel[0];  /* blue */
                dst_pixel[1] = src_pixel[1];  /* green */
                dst_pixel[2] = src_pixel[2];  /* red */
            } else {
                unsigned char gray = src_pixel[0];
                dst_pixel[0] = dst_pixel[1] = dst_pixel[2] = gray;
            }
        }
    }
}

void private_blit_image_rgb(image_t *src, image_t *dst) {
    int width = int_min(src->width, dst->width);
    int height = int_min(src->height, dst->height);
    int r, c;

    assert(width > 0 && height > 0);
    assert(src->channels >= 1 && src->channels <= 4);
    assert(dst->channels == 3 || dst->channels == 4);

    for (r = 0; r < height; r++) {
        for (c = 0; c < width; c++) {
            int flipped_r = src->height - 1 - r;
            unsigned char *src_pixel = get_pixel_ptr(src, flipped_r, c);
            unsigned char *dst_pixel = get_pixel_ptr(dst, r, c);
            if (src->channels == 3 || src->channels == 4) {
                dst_pixel[0] = src_pixel[2];  /* red */
                dst_pixel[1] = src_pixel[1];  /* green */
                dst_pixel[2] = src_pixel[0];  /* blue */
            } else {
                unsigned char gray = src_pixel[0];
                dst_pixel[0] = dst_pixel[1] = dst_pixel[2] = gray;
            }
        }
    }
}

static unsigned char float_to_uchar(float value) {
    return (unsigned char)(value * 255);
}

static vec4_t get_buffer_val(framebuffer_t *buffer, int row, int col) {
    int index = row * buffer->width + col;
    return buffer->colorbuffer[index];
}

void private_blit_buffer_bgr(framebuffer_t *src, image_t *dst) {
    int width = int_min(src->width, dst->width);
    int height = int_min(src->height, dst->height);
    int r, c;

    assert(width > 0 && height > 0);
    assert(dst->channels == 3 || dst->channels == 4);

    for (r = 0; r < height; r++) {
        for (c = 0; c < width; c++) {
            int flipped_r = src->height - 1 - r;
            vec4_t src_value = get_buffer_val(src, flipped_r, c);
            unsigned char *dst_pixel = get_pixel_ptr(dst, r, c);
            dst_pixel[0] = float_to_uchar(src_value.z);  /* blue */
            dst_pixel[1] = float_to_uchar(src_value.y);  /* green */
            dst_pixel[2] = float_to_uchar(src_value.x);  /* red */
        }
    }
}

void private_blit_buffer_rgb(framebuffer_t *src, image_t *dst) {
    int width = int_min(src->width, dst->width);
    int height = int_min(src->height, dst->height);
    int r, c;

    assert(width > 0 && height > 0);
    assert(dst->channels == 3 || dst->channels == 4);

    for (r = 0; r < height; r++) {
        for (c = 0; c < width; c++) {
            int flipped_r = src->height - 1 - r;
            vec4_t src_value = get_buffer_val(src, flipped_r, c);
            unsigned char *dst_pixel = get_pixel_ptr(dst, r, c);
            dst_pixel[0] = float_to_uchar(src_value.x);  /* red */
            dst_pixel[1] = float_to_uchar(src_value.y);  /* green */
            dst_pixel[2] = float_to_uchar(src_value.z);  /* blue */
        }
    }
}
