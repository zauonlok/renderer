#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image.h"
#include "error.h"

// helper functions

static const char *extract_extension(const char *filename) {
    const char *dot_pos = strrchr(filename, '.');
    if (dot_pos == NULL || dot_pos == filename) {
        return "";
    } else {
        return dot_pos + 1;
    }
}

static unsigned char read_byte(FILE *file) {
    int byte = fgetc(file);
    FORCE(byte != EOF, "fgetc");
    return byte;
}

static void write_byte(FILE *file, unsigned char byte) {
    int result = fputc(byte, file);
    FORCE(result != EOF, "fputc");
}

static void read_bytes(FILE *file, void *buffer, int size) {
    int count = fread(buffer, 1, size, file);
    FORCE(count == size, "fread");
}

static void write_bytes(FILE *file, void *buffer, int size) {
    int count = fwrite(buffer, 1, size, file);
    FORCE(count == size, "fwrite");
}

static void swap_byte(unsigned char *x, unsigned char *y) {
    unsigned char t = *x;
    *x = *y;
    *y = t;
}

static unsigned char *get_pixel_ptr(image_t *image, int row, int col) {
    int index = row * image->pitch + col * image->channels;
    return &(image->buffer[index]);
}

static int get_buffer_size(image_t *image) {
    return image->width * image->height * image->channels;
}

// image creation/destruction

image_t *image_create(int width, int height, int channels) {
    int buffer_size = width * height * channels;
    image_t *image = (image_t*)malloc(sizeof(image_t));
    image->width    = width;
    image->height   = height;
    image->channels = channels;
    image->pitch    = width * channels;
    image->buffer   = (unsigned char *)malloc(buffer_size);
    memset(image->buffer, 0, buffer_size);
    return image;
}

void image_release(image_t *image) {
    free(image->buffer);
    free(image);
}

// image input/output

static image_t *load_tga(const char *filename);
static void save_tga(image_t *image, const char *filename);

image_t *image_load(const char *filename) {
    const char *extension = extract_extension(filename);
    if (strcmp(extension, "tga") == 0) {
        return load_tga(filename);
    } else {
        FATAL("unsupported format");
        return NULL;
    }
}

void image_save(image_t *image, const char *filename) {
    const char *extension = extract_extension(filename);
    if (strcmp(extension, "tga") == 0) {
        save_tga(image, filename);
    } else {
        FATAL("unsupported format");
    }
}

#pragma pack(push, 1)
struct tga_header {
    uint8_t idlength;
    uint8_t colormaptype;
    uint8_t imagetype;
    uint16_t colormaporigin;
    uint16_t colormaplength;
    uint8_t colormapdepth;
    uint16_t xorigin;
    uint16_t yorigin;
    uint16_t width;
    uint16_t height;
    uint8_t pixeldepth;
    uint8_t descriptor;
};
#pragma pack(pop)

static void load_tga_rle(FILE *file, image_t *image) {
    int channels = image->channels;
    unsigned char *pixel = (unsigned char*)malloc(channels);
    unsigned char *buffer = image->buffer;
    int buffer_size = get_buffer_size(image);
    int buffer_count = 0;
    int i, j;
    while (buffer_count < buffer_size) {
        int header = read_byte(file);
        if (header < 128) {  // raw packet
            int pixel_count = header + 1;
            int expected_size = buffer_count + pixel_count * channels;
            FORCE(expected_size <= buffer_size, "raw packet too large");
            for (i = 0; i < pixel_count; i++) {
                for (j = 0; j < channels; j++) {
                    buffer[buffer_count++] = read_byte(file);
                }
            }
        } else {             // run-length packet
            int pixel_count = header - 128 + 1;
            int expected_size = buffer_count + pixel_count * channels;
            FORCE(expected_size <= buffer_size, "run-length packet too large");
            for (j = 0; j < channels; j++) {
                pixel[j] = read_byte(file);
            }
            for (i = 0; i < pixel_count; i++) {
                for (j = 0; j < channels; j++) {
                    buffer[buffer_count++] = pixel[j];
                }
            }
        }
    }
    free(pixel);
}

static image_t *load_tga(const char *filename) {
    FILE *file;
    struct tga_header header;
    int width, height;
    int depth, channels;
    int image_type;
    image_t *image;

    file = fopen(filename, "rb");
    FORCE(file != NULL, "fopen");
    read_bytes(file, &header, sizeof(struct tga_header));

    FORCE(header.idlength == 0, "feature not implemented");
    width = header.width;
    height = header.height;
    FORCE(width > 0 && height > 0, "invalid width/height value");
    depth = header.pixeldepth;
    FORCE(depth == 8 || depth == 24 || depth == 32, "unsupported color depth");
    channels = depth / 8;
    image = image_create(width, height, channels);

    image_type = header.imagetype;
    if (image_type == 2 || image_type == 3) {           // uncompressed
        read_bytes(file, image->buffer, get_buffer_size(image));
    } else if (image_type == 10 || image_type == 11) {  // run-length encoded
        load_tga_rle(file, image);
    } else {
        FATAL("unsupported image type");
    }
    fclose(file);

    if (!(header.descriptor & 0x20)) {
        image_flip_v(image);
    }
    if (header.descriptor & 0x10) {
        image_flip_h(image);
    }
    return image;
}

static void save_tga(image_t *image, const char *filename) {
    FILE *file;
    struct tga_header header;
    int header_size = sizeof(struct tga_header);

    file = fopen(filename, "wb");
    FORCE(file != NULL, "fopen");

    memset(&header, 0, header_size);
    header.width      = image->width;
    header.height     = image->height;
    header.pixeldepth = image->channels * 8;
    header.imagetype  = (image->channels == 1) ? 3 : 2;
    header.descriptor = 0x20;  // top-left origin
    write_bytes(file, &header, header_size);

    write_bytes(file, image->buffer, get_buffer_size(image));
    fclose(file);
}

// image processing

void image_flip_h(image_t *image) {
    unsigned char *buffer = image->buffer;
    int half_width = image->width / 2;
    int i, j, k;
    for (i = 0; i < image->height; i++) {
        for (j = 0; j < half_width; j++) {
            int j2 = image->width - j - 1;
            unsigned char *pixel1 = get_pixel_ptr(image, i, j);
            unsigned char *pixel2 = get_pixel_ptr(image, i, j2);
            for (k = 0; k < image->channels; k++) {
                swap_byte(&pixel1[k], &pixel2[k]);
            }
        }
    }
}

void image_flip_v(image_t *image) {
    unsigned char *buffer = image->buffer;
    int half_height = image->height / 2;
    int i, j, k;
    for (i = 0; i < half_height; i++) {
        for (j = 0; j < image->width; j++) {
            int i2 = image->height - i - 1;
            unsigned char *pixel1 = get_pixel_ptr(image, i, j);
            unsigned char *pixel2 = get_pixel_ptr(image, i2, j);
            for (k = 0; k < image->channels; k++) {
                swap_byte(&pixel1[k], &pixel2[k]);
            }
        }
    }
}

static double linear_interp(double v0, double v1, double d) {
    return v0 + (v1 - v0) * d;
}

static double bilinear_interp(double v00, double v01, double v10, double v11,
                              double drow, double dcol) {
    double v0 = linear_interp(v00, v01, dcol);
    double v1 = linear_interp(v10, v11, dcol);
    return linear_interp(v0, v1, drow);
}

static int bound_index(int index, int length) {
    return (index > length - 1) ? length - 1 : index;
}

void image_resize(image_t *image, int width, int height) {
    int channels = image->channels;
    image_t old = *image;
    image_t *new = image;
    double scale_r, scale_c;
    int r, c, k;

    new->width  = width;
    new->height = height;
    new->pitch  = width * channels;
    new->buffer = (unsigned char*)malloc(width * height * channels);

    scale_r = (double)old.height / new->height;
    scale_c = (double)old.width / new->width;

    for (r = 0; r < new->height; r++) {
        for (c = 0; c < new->width; c++) {
            double mapped_r = r * scale_r;
            double mapped_c = c * scale_c;
            int old_r = (int)mapped_r;
            int old_c = (int)mapped_c;
            double drow = mapped_r - old_r;
            double dcol = mapped_c - old_c;
            int old_r_p1 = bound_index(old_r + 1, old.height);
            int old_c_p1 = bound_index(old_c + 1, old.width);

            unsigned char *pixel_00 = get_pixel_ptr(&old, old_r,    old_c);
            unsigned char *pixel_01 = get_pixel_ptr(&old, old_r,    old_c_p1);
            unsigned char *pixel_10 = get_pixel_ptr(&old, old_r_p1, old_c);
            unsigned char *pixel_11 = get_pixel_ptr(&old, old_r_p1, old_c_p1);

            unsigned char *pixel = get_pixel_ptr(new, r, c);
            for (k = 0; k < channels; k++) {
                double v00 = pixel_00[k];
                double v01 = pixel_01[k];
                double v10 = pixel_10[k];
                double v11 = pixel_11[k];

                pixel[k] = bilinear_interp(v00, v01, v10, v11, drow, dcol);
            }
        }
    }
    free(old.buffer);
}
