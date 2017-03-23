#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "image.h"
#include "error.h"

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

static int read_byte(FILE *file) {
    int byte = fgetc(file);
    if (byte == EOF) {
        FATAL("fgetc");
    }
    return byte;
}

static void load_tga_rle(FILE *file, unsigned char *buffer,
                         int buffer_size, int channels) {
    unsigned char *pixel = (unsigned char*)malloc(channels);
    int buffer_count = 0;
    int i, j;
    do {
        int chunk_count = read_byte(file);
        if (chunk_count < 128) {    /* raw packet */
            chunk_count = chunk_count + 1;
            if (buffer_count + chunk_count * channels > buffer_size) {
                FATAL("tga: raw packet too large");
            }
            for (i = 0; i < chunk_count; i++) {
                for (j = 0; j < channels; j++) {
                    buffer[buffer_count] = read_byte(file);
                    buffer_count += 1;
                }
            }
        } else {                    /* run-length packet */
            chunk_count = chunk_count - 128 + 1;
            if (buffer_count + chunk_count * channels > buffer_size) {
                FATAL("tga: run-length packet too large");
            }
            for (j = 0; j < channels; j++) {
                pixel[j] = read_byte(file);
            }
            for (i = 0; i < chunk_count; i++) {
                for (j = 0; j < channels; j++) {
                    buffer[buffer_count] = pixel[j];
                    buffer_count += 1;
                }
            }
        }
    } while (buffer_count < buffer_size);
    free(pixel);
}

static image_t *load_tga(const char *filename) {
    FILE *file;
    struct tga_header tga_header;
    int width, height;
    int pixeldepth, channels;
    int image_type;
    int buffer_size;
    unsigned char *buffer;
    image_t *image;
    int count;

    file = fopen(filename, "rb");
    if (file == NULL) {
        FATAL("fopen");
    }
    count = fread(&tga_header, 1, sizeof(struct tga_header), file);
    if (count != sizeof(struct tga_header)) {
        FATAL("fread");
    }
    width = tga_header.width;
    height = tga_header.height;
    if (width <= 0 || height <= 0) {
        FATAL("tga: bad width/height value");
    }
    pixeldepth = tga_header.pixeldepth;
    channels = pixeldepth / 8;
    if (pixeldepth != 8 && pixeldepth != 24 && pixeldepth != 32) {
        FATAL("tga: unsupported color depth");
    }
    buffer_size = width * height * channels;
    buffer = (unsigned char *)malloc(buffer_size);
    image_type = tga_header.imagetype;
    if (image_type == 2 || image_type == 3) {
        count = fread(buffer, 1, buffer_size, file);
        if (count != buffer_size) {
            FATAL("fread");
        }
    } else if (image_type == 10 || image_type == 11) {
        load_tga_rle(file, buffer, buffer_size, channels);
    } else {
        FATAL("tga: unsupported image type");
    }
    fclose(file);

    image = (image_t*)malloc(sizeof(image_t));
    image->width    = width;
    image->height   = height;
    image->channels = channels;
    image->pitch    = width * channels;
    image->buffer   = buffer;

    if (!(tga_header.descriptor & 0x20)) {
        image_flip_v(image);
    }
    if (tga_header.descriptor & 0x10) {
        image_flip_h(image);
    }
    return image;
}

image_t *image_load(const char *file, const char *type) {
    if (strcmp(type, "tga") == 0) {
        return load_tga(file);
    } else {
        FATAL("image: unsupported format");
        return NULL;
    }
}

static void save_tga(image_t *image, const char *filename) {
    FILE *file;
    struct tga_header tga_header;
    int buffer_size;
    int count;

    file = fopen(filename, "wb");
    if (file == NULL) {
        FATAL("fopen");
    }
    memset(&tga_header, 0, sizeof(struct tga_header));
    tga_header.width      = image->width;
    tga_header.height     = image->height;
    tga_header.pixeldepth = image->channels * 8;
    tga_header.imagetype  = (image->channels == 1) ? 3 : 2;
    tga_header.descriptor = 0x20;   /* top-left origin */
    count = fwrite(&tga_header, 1, sizeof(struct tga_header), file);
    if (count != sizeof(struct tga_header)) {
        FATAL("fwrite");
    }
    buffer_size = image->width * image->height * image->channels;
    count = fwrite(image->buffer, 1, buffer_size, file);
    if (count != buffer_size) {
        FATAL("fwrite");
    }
    fclose(file);
}

void image_save(image_t *image, const char *file, const char *type) {
    if (strcmp(type, "tga") == 0) {
        save_tga(image, file);
    } else {
        FATAL("image: unsupported format");
    }
}

void image_free(image_t *image) {
    free(image->buffer);
    free(image);
}

void image_flip_h(image_t *image) {
    int half_width = image->width / 2;
    int i, j, k;
    for (i = 0; i < image->height; i++) {
        for (j = 0; j < half_width; j++) {
            int index1 = i * image->pitch + j * image->channels;
            int index2 = i * image->pitch + (image->width - j - 1) * image->channels;
            for (k = 0; k < image->channels; k++) {
                unsigned char tmp = image->buffer[index1 + k];
                image->buffer[index1 + k] = image->buffer[index2 + k];
                image->buffer[index2 + k] = tmp;
            }
        }
    }
}

void image_flip_v(image_t *image) {
    int half_height = image->height / 2;
    int i, j, k;
    for (i = 0; i < half_height; i++) {
        for (j = 0; j < image->width; j++) {
            int index1 = i * image->pitch + j * image->channels;
            int index2 = (image->height - i - 1) * image->pitch + j * image->channels;
            for (k = 0; k < image->channels; k++) {
                unsigned char tmp = image->buffer[index1 + k];
                image->buffer[index1 + k] = image->buffer[index2 + k];
                image->buffer[index2 + k] = tmp;
            }
        }
    }
}
