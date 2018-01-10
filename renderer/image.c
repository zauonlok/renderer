#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image.h"

/* helper functions */

static const char *extract_ext(const char *filename) {
    const char *dot_pos = strrchr(filename, '.');
    return (dot_pos == NULL) ? "" : dot_pos + 1;
}

static unsigned char read_byte(FILE *file) {
    int byte = fgetc(file);
    assert(byte != EOF);
    return (unsigned char)byte;
}

static void read_bytes(FILE *file, void *buffer, int size) {
    int count = fread(buffer, 1, size, file);
    assert(count == size);
}

static void write_bytes(FILE *file, void *buffer, int size) {
    int count = fwrite(buffer, 1, size, file);
    assert(count == size);
}

static void swap_bytes(unsigned char *a, unsigned char *b) {
    unsigned char t = *a;
    *a = *b;
    *b = t;
}

static int get_buffer_size(image_t *image) {
    return image->width * image->height * image->channels;
}

static unsigned char *get_pixel_ptr(image_t *image, int row, int col) {
    int index = row * image->width * image->channels + col * image->channels;
    return &(image->buffer[index]);
}

static unsigned char color2gray(color_t color) {
    int gray = ((int)color.b + (int)color.g + (int)color.r) / 3;
    return (unsigned char)gray;
}

static double linear_interp(double v0, double v1, double d) {
    return v0 + (v1 - v0) * d;
}

static double bilinear_interp(double v00, double v01, double v10, double v11,
                              double drow, double dcol) {
    double v0 = linear_interp(v00, v01, dcol);  /* row 0 */
    double v1 = linear_interp(v10, v11, dcol);  /* row 1 */
    return linear_interp(v0, v1, drow);
}

static int bound_index(int index, int length) {
    return (index > length - 1) ? length - 1 : index;
}

/* image creating/releasing */

image_t *image_create(int width, int height, int channels) {
    int buffer_size = width * height * channels;
    image_t *image;

    assert(width > 0 && height > 0 && channels >= 1 && channels <= 4);

    image = (image_t*)malloc(sizeof(image_t));
    image->width    = width;
    image->height   = height;
    image->channels = channels;
    image->buffer   = (unsigned char*)malloc(buffer_size);
    memset(image->buffer, 0, buffer_size);
    return image;
}

void image_release(image_t *image) {
    free(image->buffer);
    free(image);
}

image_t *image_clone(image_t *image) {
    int buffer_size = get_buffer_size(image);
    unsigned char *buffer = (unsigned char*)malloc(buffer_size);
    image_t *clone = (image_t*)malloc(sizeof(image_t));
    *clone = *image;
    clone->buffer = buffer;
    memcpy(clone->buffer, image->buffer, buffer_size);
    return clone;
}

/* image input/output */

static image_t *load_tga(const char *filename);
static void save_tga(image_t *image, const char *filename);

image_t *image_load(const char *filename) {
    const char *ext = extract_ext(filename);
    if (strcmp(ext, "tga") == 0) {
        return load_tga(filename);
    } else {
        assert(0);
        return NULL;
    }
}

void image_save(image_t *image, const char *filename) {
    const char *ext = extract_ext(filename);
    if (strcmp(ext, "tga") == 0) {
        save_tga(image, filename);
    } else {
        assert(0);
    }
}

static void load_tga_rle(FILE *file, image_t *image) {
    unsigned char pixel[4];
    unsigned char *buffer = image->buffer;
    int channels = image->channels;
    int buffer_size = get_buffer_size(image);
    int buffer_count = 0;
    while (buffer_count < buffer_size) {
        unsigned char header = read_byte(file);
        int is_rle_packet = header & 0x80;
        int pixel_count = (header & 0x7F) + 1;
        int expected_size = buffer_count + pixel_count * channels;
        assert(expected_size <= buffer_size);
        if (is_rle_packet) {  /* run-length packet */
            int i, j;
            for (j = 0; j < channels; j++) {
                pixel[j] = read_byte(file);
            }
            for (i = 0; i < pixel_count; i++) {
                for (j = 0; j < channels; j++) {
                    buffer[buffer_count++] = pixel[j];
                }
            }
        } else {              /* raw packet */
            int i, j;
            for (i = 0; i < pixel_count; i++) {
                for (j = 0; j < channels; j++) {
                    buffer[buffer_count++] = read_byte(file);
                }
            }
        }
    }
}

#define TGA_HEADER_SIZE 18

static image_t *load_tga(const char *filename) {
    unsigned char header[TGA_HEADER_SIZE];
    int width, height, depth, channels;
    int idlength, imgtype, imgdesc;
    image_t *image;
    FILE *file;

    file = fopen(filename, "rb");
    assert(file != NULL);
    read_bytes(file, header, TGA_HEADER_SIZE);

    width = header[12] + (header[13] << 8);
    height = header[14] + (header[15] << 8);
    assert(width > 0 && height > 0);
    depth = header[16];
    assert(depth == 8 || depth == 24 || depth == 32);
    channels = depth / 8;
    image = image_create(width, height, channels);

    idlength = header[0];
    assert(idlength == 0);
    imgtype = header[2];
    if (imgtype == 2 || imgtype == 3) {           /* uncompressed */
        read_bytes(file, image->buffer, get_buffer_size(image));
    } else if (imgtype == 10 || imgtype == 11) {  /* run-length encoded */
        load_tga_rle(file, image);
    } else {
        assert(0);
    }
    fclose(file);

    imgdesc = header[17];
    if (imgdesc & 0x20) {
        image_flip_v(image);
    }
    if (imgdesc & 0x10) {
        image_flip_h(image);
    }
    return image;
}

static void save_tga(image_t *image, const char *filename) {
    unsigned char header[TGA_HEADER_SIZE];
    FILE *file;

    file = fopen(filename, "wb");
    assert(file != NULL);

    memset(header, 0, TGA_HEADER_SIZE);
    header[2]  = (image->channels == 1) ? 3 : 2;  /* image type */
    header[12] = image->width & 0xFF;             /* width, lsb */
    header[13] = (image->width >> 8) & 0xFF;      /* width, msb */
    header[14] = image->height & 0xFF;            /* height, lsb */
    header[15] = (image->height >> 8) & 0xFF;     /* height, msb */
    header[16] = (image->channels * 8) & 0xFF;    /* image depth */
    write_bytes(file, header, TGA_HEADER_SIZE);

    write_bytes(file, image->buffer, get_buffer_size(image));
    fclose(file);
}

/* color getting/setting */

color_t image_get_color(image_t *image, int row, int col) {
    int channels = image->channels;
    color_t color = {0, 0, 0, 255};
    unsigned char *pixel;

    assert(row >= 0 && row < image->height && col >= 0 && col < image->width);

    pixel = get_pixel_ptr(image, row, col);
    if (channels == 1) {
        color.b = color.g = color.r = pixel[0];
    } else if (channels == 2) {
        color.b = color.g = color.r = pixel[0];
        color.a = pixel[1];
    } else if (channels == 3) {
        color.b = pixel[0];
        color.g = pixel[1];
        color.r = pixel[2];
    } else if (channels == 4) {
        color.b = pixel[0];
        color.g = pixel[1];
        color.r = pixel[2];
        color.a = pixel[3];
    } else {
        assert(0);
    }

    return color;
}

void image_set_color(image_t *image, int row, int col, color_t color) {
    int channels = image->channels;
    unsigned char *pixel;

    assert(row >= 0 && row < image->height && col >= 0 && col < image->width);

    pixel = get_pixel_ptr(image, row, col);
    if (channels == 1) {
        unsigned char gray = color2gray(color);
        pixel[0] = gray;
    } else if (channels == 2) {
        unsigned char gray = color2gray(color);
        pixel[0] = gray;
        pixel[1] = color.a;
    } else if (channels == 3) {
        pixel[0] = color.b;
        pixel[1] = color.g;
        pixel[2] = color.r;
    } else if (channels == 4) {
        pixel[0] = color.b;
        pixel[1] = color.g;
        pixel[2] = color.r;
        pixel[3] = color.a;
    } else {
        assert(0);
    }
}

/* image processing */

void image_flip_h(image_t *image) {
    int half_width = image->width / 2;
    int r, c, k;
    for (r = 0; r < image->height; r++) {
        for (c = 0; c < half_width; c++) {
            int c2 = image->width - c - 1;
            unsigned char *pixel1 = get_pixel_ptr(image, r, c);
            unsigned char *pixel2 = get_pixel_ptr(image, r, c2);
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
            int r2 = image->height - r - 1;
            unsigned char *pixel1 = get_pixel_ptr(image, r, c);
            unsigned char *pixel2 = get_pixel_ptr(image, r2, c);
            for (k = 0; k < image->channels; k++) {
                swap_bytes(&pixel1[k], &pixel2[k]);
            }
        }
    }
}

void image_resize(image_t *image, int width, int height) {
    int channels = image->channels;
    image_t src = *image;  /* tricks */
    image_t *dst = image;  /* tricks */
    double scale_r, scale_c;
    int r, c, k;

    assert(width > 0 && height > 0);

    dst->width  = width;
    dst->height = height;
    dst->buffer = (unsigned char*)malloc(width * height * channels);

    scale_r = (double)src.height / dst->height;
    scale_c = (double)src.width / dst->width;

    for (r = 0; r < dst->height; r++) {
        for (c = 0; c < dst->width; c++) {
            double mapped_r = r * scale_r;
            double mapped_c = c * scale_c;
            int src_r = (int)mapped_r;
            int src_c = (int)mapped_c;
            double drow = mapped_r - src_r;
            double dcol = mapped_c - src_c;
            int src_r_p1 = bound_index(src_r + 1, src.height);
            int src_c_p1 = bound_index(src_c + 1, src.width);

            unsigned char *pixel_00 = get_pixel_ptr(&src, src_r, src_c);
            unsigned char *pixel_01 = get_pixel_ptr(&src, src_r, src_c_p1);
            unsigned char *pixel_10 = get_pixel_ptr(&src, src_r_p1, src_c);
            unsigned char *pixel_11 = get_pixel_ptr(&src, src_r_p1, src_c_p1);

            unsigned char *pixel = get_pixel_ptr(dst, r, c);
            for (k = 0; k < channels; k++) {
                double v00 = pixel_00[k];
                double v01 = pixel_01[k];
                double v10 = pixel_10[k];
                double v11 = pixel_11[k];
                double value = bilinear_interp(v00, v01, v10, v11, drow, dcol);
                pixel[k] = (unsigned char)(value + 0.5);
            }
        }
    }
    free(src.buffer);
}

/* private blit functions, used by window_draw_image */

static void blit_truecolor(image_t *src, image_t *dst, int swap_rb) {
    int r, c;

    assert(dst->channels == 3 || dst->channels == 4);

    memset(dst->buffer, 0, get_buffer_size(dst));
    for (r = 0; r < src->height && r < dst->height; r++) {
        for (c = 0; c < src->width && c < dst->width; c++) {
            unsigned char *src_pixel = get_pixel_ptr(src, r, c);
            unsigned char *dst_pixel = get_pixel_ptr(dst, r, c);
            if (src->channels == 1 || src->channels == 2) {  /* gray */
                unsigned char gray = src_pixel[0];
                dst_pixel[0] = dst_pixel[1] = dst_pixel[2] = gray;
            } else {
                if (swap_rb) {  /* rgb */
                    dst_pixel[0] = src_pixel[2];
                    dst_pixel[1] = src_pixel[1];
                    dst_pixel[2] = src_pixel[0];
                } else {        /* bgr */
                    dst_pixel[0] = src_pixel[0];
                    dst_pixel[1] = src_pixel[1];
                    dst_pixel[2] = src_pixel[2];
                }
            }
        }
    }
}

void image_blit_bgr(image_t *src, image_t *dst) {
    int swap_rb = 0;
    blit_truecolor(src, dst, swap_rb);
}

void image_blit_rgb(image_t *src, image_t *dst) {
    int swap_rb = 1;
    blit_truecolor(src, dst, swap_rb);
}
