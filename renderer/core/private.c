#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graphics.h"
#include "image.h"
#include "macro.h"
#include "maths.h"
#include "private.h"
#include "texture.h"

/* tga format */

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

static int get_buffer_size(image_t *image) {
    return image->width * image->height * image->channels;
}

static void load_tga_rle(FILE *file, image_t *image) {
    unsigned char *buffer = image->buffer;
    int channels = image->channels;
    int buffer_size = get_buffer_size(image);
    int elem_count = 0;
    while (elem_count < buffer_size) {
        unsigned char header = read_byte(file);
        int rle_packet = header & 0x80;
        int pixel_count = (header & 0x7F) + 1;
        unsigned char pixel[4];
        int i, j;
        assert(elem_count + pixel_count * channels <= buffer_size);
        if (rle_packet) {  /* rle packet */
            for (j = 0; j < channels; j++) {
                pixel[j] = read_byte(file);
            }
            for (i = 0; i < pixel_count; i++) {
                for (j = 0; j < channels; j++) {
                    buffer[elem_count++] = pixel[j];
                }
            }
        } else {           /* raw packet */
            for (i = 0; i < pixel_count; i++) {
                for (j = 0; j < channels; j++) {
                    buffer[elem_count++] = read_byte(file);
                }
            }
        }
    }
    assert(elem_count == buffer_size);
}

image_t *private_load_tga_image(const char *filename) {
    unsigned char header[TGA_HEADER_SIZE];
    int width, height, depth, channels;
    int idlength, imgtype, imgdesc;
    image_t *image;
    FILE *file;

    file = fopen(filename, "rb");
    assert(file != NULL);
    read_bytes(file, header, TGA_HEADER_SIZE);

    width = header[12] | (header[13] << 8);
    height = header[14] | (header[15] << 8);
    assert(width > 0 && height > 0);
    depth = header[16];
    assert(depth == 8 || depth == 24 || depth == 32);
    channels = depth / 8;
    image = image_create(width, height, channels);

    idlength = header[0];
    assert(idlength == 0);
    UNUSED_VAR(idlength);
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

void private_save_tga_image(image_t *image, const char *filename) {
    unsigned char header[TGA_HEADER_SIZE];
    FILE *file;

    file = fopen(filename, "wb");
    assert(file != NULL);

    memset(header, 0, TGA_HEADER_SIZE);
    header[2] = image->channels == 1 ? 3 : 2;     /* image type */
    header[12] = image->width & 0xFF;             /* width, lsb */
    header[13] = (image->width >> 8) & 0xFF;      /* width, msb */
    header[14] = image->height & 0xFF;            /* height, lsb */
    header[15] = (image->height >> 8) & 0xFF;     /* height, msb */
    header[16] = (image->channels * 8) & 0xFF;    /* image depth */
    write_bytes(file, header, TGA_HEADER_SIZE);

    write_bytes(file, image->buffer, get_buffer_size(image));
    fclose(file);
}

/* hdr format */

static void read_line(FILE *file, char line[LINE_SIZE]) {
    if (fgets(line, LINE_SIZE, file) == NULL) {
        assert(0);
    }
}

static int starts_with(const char *string, const char *prefix) {
    return strncmp(string, prefix, strlen(prefix)) == 0;
}

static void texel_from_rgbe(vec4_t *texel, unsigned char rgbe[4]) {
    float rm = rgbe[0];  /* red mantissa */
    float gm = rgbe[1];  /* green mantissa */
    float bm = rgbe[2];  /* blue mantissa */
    int eb = rgbe[3];    /* exponent biased */
    if (eb == 0) {
        *texel = vec4_new(0, 0, 0, 1);
    } else {
        int ev = eb - 128;       /* exponent value */
        float factor = (float)((1.0 / 256) * pow(2, ev));
        float rv = rm * factor;  /* red value */
        float gv = gm * factor;  /* green value */
        float bv = bm * factor;  /* blue value */
        *texel = vec4_new(rv, gv, bv, 1);
    }
}

static void rgbe_from_texel(unsigned char rgbe[4], vec4_t texel) {
    float rv = texel.x;  /* red value */
    float gv = texel.y;  /* green value */
    float bv = texel.z;  /* blue value */
    float max_v = float_max(float_max(rv, gv), bv);
    if (max_v < 1e-32f) {
        rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
    } else {
        int ev;          /* exponent value */
        float max_m = (float)frexp(max_v, &ev);
        float factor = (1 / max_v) * max_m * 256;
        rgbe[0] = (unsigned char)(rv * factor);  /* red mantissa */
        rgbe[1] = (unsigned char)(gv * factor);  /* green mantissa */
        rgbe[2] = (unsigned char)(bv * factor);  /* blue mantissa */
        rgbe[3] = (unsigned char)(ev + 128);     /* exponent biased */
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
            /* ignore, for now */
        } else if (starts_with(line, "EXPOSURE=")) {
            /* ignore, for now */
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

static void read_flat_scanline(FILE *file, texture_t *texture, int row) {
    int width = texture->width;
    int i, j;
    for (i = 0; i < width; i++) {
        int index = row * width + i;
        unsigned char rgbe[4];
        for (j = 0; j < 4; j++) {
            rgbe[j] = read_byte(file);
        }
        texel_from_rgbe(&texture->buffer[index], rgbe);
    }
}

static void read_rle_scanline(FILE *file, texture_t *texture, int row) {
    int width = texture->width;
    unsigned char *channels[4];
    int i, j;

    for (i = 0; i < 4; i++) {
        channels[i] = (unsigned char*)malloc(width);
    }
    for (i = 0; i < 4; i++) {
        int size = 0;
        while (size < width) {
            unsigned char byte = read_byte(file);
            if (byte > 128) {
                int count = byte - 128;
                unsigned char value = read_byte(file);
                assert(count > 0 && size + count <= width);
                for (j = 0; j < count; j++) {
                    channels[i][size++] = value;
                }
            } else {
                int count = byte;
                assert(count > 0 && size + count <= width);
                for (j = 0; j < count; j++) {
                    channels[i][size++] = read_byte(file);
                }
            }
        }
        assert(size == width);
    }

    for (i = 0; i < width; i++) {
        int index = row * width + i;
        unsigned char rgbe[4];
        for (j = 0; j < 4; j++) {
            rgbe[j] = channels[j][i];
        }
        texel_from_rgbe(&texture->buffer[index], rgbe);
    }
    for (i = 0; i < 4; i++) {
        free(channels[i]);
    }
}

static void read_hdr_scanline(FILE *file, texture_t *texture, int row) {
    int width = texture->width;
    if (width < 8 || width > 0x7fff) {
        read_flat_scanline(file, texture, row);
    } else {
        unsigned char bytes[4];
        int i;
        for (i = 0; i < 4; i++) {
            bytes[i] = read_byte(file);
        }
        if (bytes[0] != 2 || bytes[1] != 2 || bytes[2] & 0x80) {
            fseek(file, -4, SEEK_CUR);
            read_flat_scanline(file, texture, row);
        } else {
            assert(bytes[2] * 256 + bytes[3] == width);
            read_rle_scanline(file, texture, row);
        }
    }
}

texture_t *private_load_hdr_image(const char *filename) {
    texture_t *texture;
    int width, height;
    FILE *file;
    int i;

    file = fopen(filename, "rb");
    assert(file != NULL);
    read_hdr_header(file, &width, &height);
    texture = texture_create(width, height);
    for (i = 0; i < height; i++) {
        int row = height - 1 - i;
        read_hdr_scanline(file, texture, row);
    }
    fclose(file);

    return texture;
}

void private_save_hdr_image(texture_t *texture, const char *filename) {
    int width = texture->width;
    int height = texture->height;
    FILE *file;
    int r, c;

    file = fopen(filename, "wb");
    assert(file != NULL);

    fputs("#?RADIANCE\n", file);
    fputs("FORMAT=32-bit_rle_rgbe\n", file);
    fputs("\n", file);
    fprintf(file, "-Y %d +X %d\n", height, width);

    for (r = 0; r < height; r++) {
        for (c = 0; c < width; c++) {
            int flipped_r = height - 1 - r;
            int index = flipped_r * width + c;
            vec4_t texel = texture->buffer[index];
            unsigned char rgbe[4];
            rgbe_from_texel(rgbe, texel);
            write_bytes(file, rgbe, 4);
        }
    }

    fclose(file);
}

/* image blitting */

static unsigned char float_to_uchar(float value) {
    return (unsigned char)(value * 255);
}

static vec4_t get_buffer_value(framebuffer_t *buffer, int row, int col) {
    int index = row * buffer->width + col;
    return buffer->colorbuffer[index];
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
            unsigned char *src_pixel = private_get_pixel(src, flipped_r, c);
            unsigned char *dst_pixel = private_get_pixel(dst, r, c);
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
            unsigned char *src_pixel = private_get_pixel(src, flipped_r, c);
            unsigned char *dst_pixel = private_get_pixel(dst, r, c);
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

void private_blit_buffer_bgr(framebuffer_t *src, image_t *dst) {
    int width = int_min(src->width, dst->width);
    int height = int_min(src->height, dst->height);
    int r, c;

    assert(width > 0 && height > 0);
    assert(dst->channels == 3 || dst->channels == 4);

    for (r = 0; r < height; r++) {
        for (c = 0; c < width; c++) {
            int flipped_r = src->height - 1 - r;
            vec4_t src_value = get_buffer_value(src, flipped_r, c);
            unsigned char *dst_pixel = private_get_pixel(dst, r, c);
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
            vec4_t src_value = get_buffer_value(src, flipped_r, c);
            unsigned char *dst_pixel = private_get_pixel(dst, r, c);
            dst_pixel[0] = float_to_uchar(src_value.x);  /* red */
            dst_pixel[1] = float_to_uchar(src_value.y);  /* green */
            dst_pixel[2] = float_to_uchar(src_value.z);  /* blue */
        }
    }
}

/* misc functions */

const char *private_get_extension(const char *filename) {
    const char *dot_pos = strrchr(filename, '.');
    return dot_pos == NULL ? "" : dot_pos + 1;
}

unsigned char *private_get_pixel(image_t *image, int row, int col) {
    int index = row * image->width * image->channels + col * image->channels;
    return &(image->buffer[index]);
}
