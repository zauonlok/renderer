#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image.h"
#include "macro.h"
#include "maths.h"
#include "private.h"

/* image creating/releasing */

image_t *image_create(int width, int height, int channels, format_t format) {
    int num_elems = width * height * channels;
    image_t *image;

    assert(width > 0 && height > 0 && channels >= 1 && channels <= 4);
    assert(format == FORMAT_LDR || format == FORMAT_HDR);

    image = (image_t*)malloc(sizeof(image_t));
    image->format = format;
    image->width = width;
    image->height = height;
    image->channels = channels;
    image->ldr_buffer = NULL;
    image->hdr_buffer = NULL;

    if (format == FORMAT_LDR) {
        int size = sizeof(unsigned char) * num_elems;
        image->ldr_buffer = (unsigned char*)malloc(size);
        memset(image->ldr_buffer, 0, size);
    } else {
        int size = sizeof(float) * num_elems;
        image->hdr_buffer = (float*)malloc(size);
        memset(image->hdr_buffer, 0, size);
    }

    return image;
}

void image_release(image_t *image) {
    free(image->ldr_buffer);
    free(image->hdr_buffer);
    free(image);
}

static image_t *load_tga_image(const char *filename);
static image_t *load_hdr_image(const char *filename);

image_t *image_load(const char *filename) {
    const char *extension = private_get_extension(filename);
    if (strcmp(extension, "tga") == 0) {
        return load_tga_image(filename);
    } else if (strcmp(extension, "hdr") == 0) {
        return load_hdr_image(filename);
    } else {
        assert(0);
        return NULL;
    }
}

static void save_tga_image(image_t *image, const char *filename);
static void save_hdr_image(image_t *image, const char *filename);

void image_save(image_t *image, const char *filename) {
    const char *extension = private_get_extension(filename);
    if (strcmp(extension, "tga") == 0) {
        save_tga_image(image, filename);
    } else if (strcmp(extension, "hdr") == 0) {
        save_hdr_image(image, filename);
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

static void swap_floats(float *a, float *b) {
    float t = *a;
    *a = *b;
    *b = t;
}

static unsigned char *get_ldr_pixel(image_t *image, int row, int col) {
    int index = (row * image->width + col) * image->channels;
    return &image->ldr_buffer[index];
}

static float *get_hdr_pixel(image_t *image, int row, int col) {
    int index = (row * image->width + col) * image->channels;
    return &image->hdr_buffer[index];
}

void image_flip_h(image_t *image) {
    int half_width = image->width / 2;
    int r, c, k;
    for (r = 0; r < image->height; r++) {
        for (c = 0; c < half_width; c++) {
            int flipped_c = image->width - 1 - c;
            if (image->format == FORMAT_LDR) {
                unsigned char *pixel1 = get_ldr_pixel(image, r, c);
                unsigned char *pixel2 = get_ldr_pixel(image, r, flipped_c);
                for (k = 0; k < image->channels; k++) {
                    swap_bytes(&pixel1[k], &pixel2[k]);
                }
            } else {
                float *pixel1 = get_hdr_pixel(image, r, c);
                float *pixel2 = get_hdr_pixel(image, r, flipped_c);
                for (k = 0; k < image->channels; k++) {
                    swap_floats(&pixel1[k], &pixel2[k]);
                }
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
            if (image->format == FORMAT_LDR) {
                unsigned char *pixel1 = get_ldr_pixel(image, r, c);
                unsigned char *pixel2 = get_ldr_pixel(image, flipped_r, c);
                for (k = 0; k < image->channels; k++) {
                    swap_bytes(&pixel1[k], &pixel2[k]);
                }
            } else {
                float *pixel1 = get_hdr_pixel(image, r, c);
                float *pixel2 = get_hdr_pixel(image, flipped_r, c);
                for (k = 0; k < image->channels; k++) {
                    swap_floats(&pixel1[k], &pixel2[k]);
                }
            }
        }
    }
}

/* tga codec */

#define TGA_HEADER_SIZE 18

static unsigned char read_byte(FILE *file) {
    int byte = fgetc(file);
    assert(byte != EOF);
    return (unsigned char)byte;
}

static void read_bytes(FILE *file, void *buffer, int size) {
    int count = (int)fread(buffer, 1, size, file);
    assert(count == size);
    UNUSED_VAR(count);
}

static void write_bytes(FILE *file, void *buffer, int size) {
    int count = (int)fwrite(buffer, 1, size, file);
    assert(count == size);
    UNUSED_VAR(count);
}

static int get_num_elems(image_t *image) {
    return image->width * image->height * image->channels;
}

static void read_tga_header(FILE *file, int *width, int *height, int *channels,
                            int *is_rle, int *flip_h, int *flip_v) {
    unsigned char header[TGA_HEADER_SIZE];
    int depth, idlength, imgtype, imgdesc;

    read_bytes(file, header, TGA_HEADER_SIZE);

    *width = header[12] | (header[13] << 8);
    *height = header[14] | (header[15] << 8);
    assert(*width > 0 && *height > 0);

    depth = header[16];
    assert(depth == 8 || depth == 24 || depth == 32);
    *channels = depth / 8;

    idlength = header[0];
    assert(idlength == 0);
    UNUSED_VAR(idlength);

    imgtype = header[2];
    assert(imgtype == 2 || imgtype == 3 || imgtype == 10 || imgtype == 11);
    *is_rle = imgtype == 10 || imgtype == 11;

    imgdesc = header[17];
    *flip_h = imgdesc & 0x10;
    *flip_v = imgdesc & 0x20;
}

static void load_tga_rle_payload(FILE *file, image_t *image) {
    int num_elems = get_num_elems(image);
    int curr_size = 0;
    while (curr_size < num_elems) {
        unsigned char header = read_byte(file);
        int rle_packet = header & 0x80;
        int num_pixels = (header & 0x7F) + 1;
        unsigned char pixel[4];
        int i, j;
        assert(curr_size + num_pixels * image->channels <= num_elems);
        if (rle_packet) {                                   /* rle packet */
            for (j = 0; j < image->channels; j++) {
                pixel[j] = read_byte(file);
            }
            for (i = 0; i < num_pixels; i++) {
                for (j = 0; j < image->channels; j++) {
                    image->ldr_buffer[curr_size++] = pixel[j];
                }
            }
        } else {                                            /* raw packet */
            for (i = 0; i < num_pixels; i++) {
                for (j = 0; j < image->channels; j++) {
                    image->ldr_buffer[curr_size++] = read_byte(file);
                }
            }
        }
    }
    assert(curr_size == num_elems);
}

static image_t *load_tga_image(const char *filename) {
    int width, height, channels;
    int is_rle, flip_h, flip_v;
    image_t *image;
    FILE *file;

    file = fopen(filename, "rb");
    assert(file != NULL);
    read_tga_header(file, &width, &height, &channels,
                    &is_rle, &flip_h, &flip_v);
    image = image_create(width, height, channels, FORMAT_LDR);
    if (is_rle) {
        load_tga_rle_payload(file, image);
    } else {
        read_bytes(file, image->ldr_buffer, get_num_elems(image));
    }
    fclose(file);

    if (flip_h) {
        image_flip_h(image);
    }
    if (flip_v) {
        image_flip_v(image);
    }
    if (channels >= 3) {
        int r, c;
        for (r = 0; r < image->height; r++) {
            for (c = 0; c < image->width; c++) {
                unsigned char *pixel = get_ldr_pixel(image, r, c);
                swap_bytes(&pixel[0], &pixel[2]);           /* bgr to rgb */
            }
        }
    }

    return image;
}

static void save_tga_image(image_t *image, const char *filename) {
    unsigned char header[TGA_HEADER_SIZE];
    FILE *file;

    assert(image->format == FORMAT_LDR);

    file = fopen(filename, "wb");
    assert(file != NULL);

    memset(header, 0, TGA_HEADER_SIZE);
    header[2] = image->channels == 1 ? 3 : 2;               /* image type */
    header[12] = image->width & 0xFF;                       /* width, lsb */
    header[13] = (image->width >> 8) & 0xFF;                /* width, msb */
    header[14] = image->height & 0xFF;                      /* height, lsb */
    header[15] = (image->height >> 8) & 0xFF;               /* height, msb */
    header[16] = (image->channels * 8) & 0xFF;              /* image depth */
    write_bytes(file, header, TGA_HEADER_SIZE);

    if (image->channels >= 3) {
        int r, c;
        for (r = 0; r < image->height; r++) {
            for (c = 0; c < image->width; c++) {
                unsigned char *pixel = get_ldr_pixel(image, r, c);
                unsigned char channels[4];
                memcpy(channels, pixel, image->channels);
                swap_bytes(&channels[0], &channels[2]);     /* rgb to bgr */
                write_bytes(file, channels, image->channels);
            }
        }
    } else {
        write_bytes(file, image->ldr_buffer, get_num_elems(image));
    }

    fclose(file);
}

/* hdr codec */

static void read_line(FILE *file, char line[LINE_SIZE]) {
    if (fgets(line, LINE_SIZE, file) == NULL) {
        assert(0);
    }
}

static int starts_with(const char *string, const char *prefix) {
    return strncmp(string, prefix, strlen(prefix)) == 0;
}

static void rgbe_to_floats(unsigned char rgbe[4], float floats[3]) {
    float rm = rgbe[0];                             /* red mantissa */
    float gm = rgbe[1];                             /* green mantissa */
    float bm = rgbe[2];                             /* blue mantissa */
    int eb = rgbe[3];                               /* exponent biased */
    if (eb == 0) {
        floats[0] = floats[1] = floats[2] = 0;
    } else {
        int ev = eb - 128;                          /* exponent value */
        float factor = (float)((1.0 / 256) * pow(2, ev));
        floats[0] = rm * factor;                    /* red value */
        floats[1] = gm * factor;                    /* green value */
        floats[2] = bm * factor;                    /* blue value */
    }
}

static void floats_to_rgbe(float floats[3], unsigned char rgbe[4]) {
    float rv = floats[0];                           /* red value */
    float gv = floats[1];                           /* green value */
    float bv = floats[2];                           /* blue value */
    float max_v = float_max(float_max(rv, gv), bv);
    if (max_v < 1e-32f) {
        rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
    } else {
        int ev;                                     /* exponent value */
        float max_m = (float)frexp(max_v, &ev);
        float factor = (1 / max_v) * max_m * 256;
        rgbe[0] = (unsigned char)(rv * factor);     /* red mantissa */
        rgbe[1] = (unsigned char)(gv * factor);     /* green mantissa */
        rgbe[2] = (unsigned char)(bv * factor);     /* blue mantissa */
        rgbe[3] = (unsigned char)(ev + 128);        /* exponent biased */
    }
}

static void read_hdr_header(FILE *file, int *width, int *height) {
    char line[LINE_SIZE];
    int header_found = 0;
    int format_found = 0;
    int items;

    read_line(file, line);
    assert(starts_with(line, "#?"));

    while (1) {
        read_line(file, line);
        if (strlen(line) == 1 && line[0] == '\n') {
            header_found = 1;
            break;
        } else if (starts_with(line, "FORMAT=")) {
            assert(starts_with(line, "FORMAT=32-bit_rle_rgbe"));
            format_found = 1;
        } else if (starts_with(line, "GAMMA=")) {
            /* ignore gamma */
        } else if (starts_with(line, "EXPOSURE=")) {
            /* ignore exposure */
        } else if (starts_with(line, "#")) {
            /* ignore comments */
        } else {
            assert(0);
        }
    }
    assert(header_found != 0);
    assert(format_found != 0);
    UNUSED_VAR(header_found);
    UNUSED_VAR(format_found);

    read_line(file, line);
    items = sscanf(line, "-Y %d +X %d", height, width);
    assert(items == 2 && *width > 0 && *height > 0);
    UNUSED_VAR(items);
}

static void read_hdr_flat_scanline(FILE *file, image_t *image, int row) {
    int i;
    for (i = 0; i < image->width; i++) {
        float *pixel = get_hdr_pixel(image, row, i);
        unsigned char rgbe[4];
        read_bytes(file, rgbe, 4);
        rgbe_to_floats(rgbe, pixel);
    }
}

static void read_hdr_rle_scanline(FILE *file, image_t *image, int row) {
    unsigned char *channels[4];
    int i, j;

    for (i = 0; i < 4; i++) {
        channels[i] = (unsigned char*)malloc(image->width);
    }
    for (i = 0; i < 4; i++) {
        int size = 0;
        while (size < image->width) {
            unsigned char byte = read_byte(file);
            if (byte > 128) {
                int count = byte - 128;
                unsigned char value = read_byte(file);
                assert(count > 0 && size + count <= image->width);
                for (j = 0; j < count; j++) {
                    channels[i][size++] = value;
                }
            } else {
                int count = byte;
                assert(count > 0 && size + count <= image->width);
                for (j = 0; j < count; j++) {
                    channels[i][size++] = read_byte(file);
                }
            }
        }
        assert(size == image->width);
    }

    for (i = 0; i < image->width; i++) {
        float *pixel = get_hdr_pixel(image, row, i);
        unsigned char rgbe[4];
        for (j = 0; j < 4; j++) {
            rgbe[j] = channels[j][i];
        }
        rgbe_to_floats(rgbe, pixel);
    }
    for (i = 0; i < 4; i++) {
        free(channels[i]);
    }
}

static void read_hdr_scanline(FILE *file, image_t *image, int row) {
    if (image->width < 8 || image->width > 0x7fff) {
        read_hdr_flat_scanline(file, image, row);
    } else {
        unsigned char bytes[4];
        read_bytes(file, bytes, 4);
        if (bytes[0] != 2 || bytes[1] != 2 || bytes[2] & 0x80) {
            fseek(file, -4, SEEK_CUR);
            read_hdr_flat_scanline(file, image, row);
        } else {
            assert(bytes[2] * 256 + bytes[3] == image->width);
            read_hdr_rle_scanline(file, image, row);
        }
    }
}

static image_t *load_hdr_image(const char *filename) {
    int width, height;
    image_t *image;
    FILE *file;
    int i;

    file = fopen(filename, "rb");
    assert(file != NULL);
    read_hdr_header(file, &width, &height);
    image = image_create(width, height, 3, FORMAT_HDR);
    for (i = 0; i < height; i++) {
        int row = height - 1 - i;
        read_hdr_scanline(file, image, row);
    }
    fclose(file);

    return image;
}

static void save_hdr_image(image_t *image, const char *filename) {
    FILE *file;
    int r, c;

    assert(image->format == FORMAT_HDR && image->channels >= 3);

    file = fopen(filename, "wb");
    assert(file != NULL);

    fputs("#?RADIANCE\n", file);
    fputs("FORMAT=32-bit_rle_rgbe\n", file);
    fputs("\n", file);
    fprintf(file, "-Y %d +X %d\n", image->height, image->width);

    for (r = 0; r < image->height; r++) {
        for (c = 0; c < image->width; c++) {
            int flipped_r = image->height - 1 - r;
            float *pixel = get_hdr_pixel(image, flipped_r, c);
            unsigned char rgbe[4];
            floats_to_rgbe(pixel, rgbe);
            write_bytes(file, rgbe, 4);
        }
    }

    fclose(file);
}
